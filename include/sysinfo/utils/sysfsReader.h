//
// Created by devboi on 4/22/26.
//

#ifndef SYSTEMINFO_SYSFSREADER_H
#define SYSTEMINFO_SYSFSREADER_H

#include <fstream>
#include <string>
#include <string_view>

namespace si {
    class SysFsReader {

    public:
        explicit SysFsReader(std::string path);
        std::string_view read();
        ~SysFsReader();

    private:
        std::string m_path;
        std::ifstream m_file;
        std::string m_buffer;
    };
}

#endif //SYSTEMINFO_SYSFSREADER_H
