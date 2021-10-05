
set(OSMIUM_INSTALL_FILES
    share/man/man1/osmium-add-locations-to-ways.1
    share/man/man1/osmium-apply-changes.1
    share/man/man1/osmium-cat.1
    share/man/man1/osmium-changeset-filter.1
    share/man/man1/osmium-check-refs.1
    share/man/man1/osmium-create-locations-index.1
    share/man/man1/osmium-derive-changes.1
    share/man/man1/osmium-diff.1
    share/man/man1/osmium-export.1
    share/man/man1/osmium-extract.1
    share/man/man1/osmium-fileinfo.1
    share/man/man1/osmium-getid.1
    share/man/man1/osmium-getparents.1
    share/man/man1/osmium-merge-changes.1
    share/man/man1/osmium-merge.1
    share/man/man1/osmium-query-locations-index.1
    share/man/man1/osmium-removeid.1
    share/man/man1/osmium-renumber.1
    share/man/man1/osmium-show.1
    share/man/man1/osmium-sort.1
    share/man/man1/osmium-tags-count.1
    share/man/man1/osmium-tags-filter.1
    share/man/man1/osmium-time-filter.1
    share/man/man1/osmium.1
    share/man/man5/osmium-file-formats.5
    share/man/man5/osmium-index-types.5
    share/man/man5/osmium-output-headers.5
    bin/osmium
)

execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/test_install -P ${CMAKE_BINARY_DIR}/cmake_install.cmake)

file(READ ${CMAKE_BINARY_DIR}/install_manifest.txt _manifest)

string(REPLACE "\n" ";" _files "${_manifest}")
string(REPLACE "${CMAKE_BINARY_DIR}/test_install/" "" _file_list "${_files}")

list(SORT OSMIUM_INSTALL_FILES)
list(SORT _file_list)

if(NOT "${OSMIUM_INSTALL_FILES}" STREQUAL "${_file_list}")
    foreach(_file IN LISTS OSMIUM_INSTALL_FILES)
        list(FIND _file_list ${_file} _result)
        if(${_result} EQUAL -1)
            message(STATUS "Missing file in install: ${_file}")
        else()
            list(REMOVE_ITEM _file_list ${_file})
        endif()
    endforeach()
    if(NOT "${_file_list}" STREQUAL "")
        message(STATUS "Installed files that should not be: ${_file_list}")
    endif()
    message(FATAL_ERROR "Install broken")
endif()

