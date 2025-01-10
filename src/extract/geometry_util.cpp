
#include "geometry_util.hpp"

#include <vector>

double calculate_double_area(const std::vector<osmium::Location>& coordinates) {
    assert(coordinates.size() > 1);

    double total = 0.0;
    auto prev = coordinates.front();

    for (unsigned i = 1; i < coordinates.size(); ++i) {
        auto const cur = coordinates[i];
        total += prev.lon() * cur.lat() - cur.lon() * prev.lat();
        prev = cur;
    }

    return total;
}

double calculate_double_area(const std::vector<osmium::geom::Coordinates>& coordinates) {
    assert(coordinates.size() > 1);

    double total = 0.0;
    auto prev = coordinates.front();

    for (unsigned i = 1; i < coordinates.size(); ++i) {
        auto const cur = coordinates[i];
        total += prev.x * cur.y - cur.x * prev.y;
        prev = cur;
    }

    return total;
}

