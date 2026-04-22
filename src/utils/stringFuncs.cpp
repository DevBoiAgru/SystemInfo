//
// Created by devboi on 4/22/26.
//

#include "sysinfo/utils/stringFuncs.h"

namespace si::utils {

    std::string trim(std::string_view sv) {
        auto is_space = [](const unsigned char ch) { return std::isspace(ch); };

        // left trim
        while (!sv.empty() && is_space(sv.front())) {
            sv.remove_prefix(1);
        }

        // right trim
        while (!sv.empty() && is_space(sv.back())) {
            sv.remove_suffix(1);
        }

        return std::string(sv);
    }
}