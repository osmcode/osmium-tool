/*

Osmium -- OpenStreetMap data manipulation command line tool
https://osmcode.org/osmium-tool/

Copyright (C) 2013-2026  Jochen Topf <jochen@topf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "extract.hpp"

#include <osmium/io/writer_options.hpp>

#include <memory>
#include <sstream>
#include <string>
#include <utility>

void Extract::writer_loop() {
    try {
        while (true) {
            std::unique_lock<std::mutex> lock{m_mutex};
            m_cv.wait(lock, [this]{ return m_flush_pending || m_shutdown; });

            if (m_shutdown && !m_flush_pending) {
                break;
            }

            // Reset m_flush_pending under the lock so that swap_and_flush()
            // can observe the transition to false only after the buffer has
            // been moved out and is no longer shared.
            m_clean->apply_to(m_flush_buffer);
            auto buf = std::move(m_flush_buffer);
            m_flush_buffer = osmium::memory::Buffer{buffer_size,
                                                     osmium::memory::Buffer::auto_grow::no};
            m_flush_pending = false;
            lock.unlock();
            m_cv.notify_one();

            // The osmium Writer call (compression + I/O) runs outside the lock
            // so the main thread can keep filling m_fill_buffer concurrently.
            (*m_writer)(std::move(buf));
        }
    } catch (...) {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_writer_exception = std::current_exception();
        m_flush_pending = false;
        m_shutdown = true;
        lock.unlock();
        m_cv.notify_all();
    }
}

void Extract::check_writer_exception() {
    if (m_writer_exception) {
        std::rethrow_exception(m_writer_exception);
    }
}

void Extract::swap_and_flush() {
    std::unique_lock<std::mutex> lock{m_mutex};
    m_cv.wait(lock, [this]{ return !m_flush_pending || m_shutdown; });
    check_writer_exception();

    std::swap(m_fill_buffer, m_flush_buffer);
    m_fill_buffer = osmium::memory::Buffer{buffer_size,
                                           osmium::memory::Buffer::auto_grow::no};
    m_flush_pending = true;
    lock.unlock();
    m_cv.notify_one();
}

void Extract::open_file(const osmium::io::Header& header,
                        osmium::io::overwrite output_overwrite,
                        osmium::io::fsync sync,
                        OptionClean const* clean) {
    m_clean = clean;
    m_writer = std::make_unique<osmium::io::Writer>(m_output_file, header,
                                                     output_overwrite, sync);
    m_writer_thread = std::thread{&Extract::writer_loop, this};
}

void Extract::close_file() {
    if (!m_writer) {
        return;
    }

    if (m_fill_buffer.committed() > 0) {
        swap_and_flush();
    }

    {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_cv.wait(lock, [this]{ return !m_flush_pending || m_shutdown; });
        check_writer_exception();
        m_shutdown = true;
    }
    m_cv.notify_one();
    m_writer_thread.join();

    check_writer_exception();
    m_writer->close();
}

Extract::~Extract() {
    if (m_writer_thread.joinable()) {
        {
            std::lock_guard<std::mutex> lock{m_mutex};
            m_shutdown = true;
        }
        m_cv.notify_one();
        m_writer_thread.join();
    }
}

void Extract::write(const osmium::memory::Item& item) {
    if (m_fill_buffer.capacity() - m_fill_buffer.committed() < item.padded_size()) {
        swap_and_flush();
    }
    m_fill_buffer.push_back(item);
}

std::string Extract::envelope_as_text() const {
    std::stringstream ss;
    ss << m_envelope;
    return ss.str();
}
