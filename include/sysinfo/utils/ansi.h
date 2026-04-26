//
// Created by devboi on 4/26/26.
//

#ifndef SYSTEMINFO_ANSI_H
#define SYSTEMINFO_ANSI_H

#include <string>

// ANSI color codes
#define ANSI_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_CYAN "\033[1;36m"
#define ANSI_YELLOW "\033[1;33m"
#define ANSI_GREEN "\033[1;32m"
#define ANSI_RED "\033[1;31m"
#define ANSI_MAGENTA "\033[1;35m"
#define ANSI_CLEAR_SCREEN "\033[2J\033[H"

namespace si::utils {

    // Helper functions for conditional formatting
    inline std::string header(const std::string& text, bool pretty) {
        return pretty ? ANSI_CYAN + text + ANSI_RESET : text;
    }

    inline std::string label(const std::string& text, bool pretty) {
        return pretty ? ANSI_YELLOW + text + ANSI_RESET : text;
    }

    inline std::string value(const std::string& text, bool pretty) {
        return pretty ? ANSI_GREEN + text + ANSI_RESET : text;
    }

    inline std::string value_red(const std::string& text, bool pretty) {
        return pretty ? ANSI_RED + text + ANSI_RESET : text;
    }

    inline std::string value_magenta(const std::string& text, bool pretty) {
        return pretty ? ANSI_MAGENTA + text + ANSI_RESET : text;
    }

}

#endif //SYSTEMINFO_ANSI_H
