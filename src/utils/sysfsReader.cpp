//
// Created by devboi on 4/22/26.
//

#include <utility>

#include "sysinfo/utils/sysfsReader.h"

si::SysFsReader::SysFsReader(std::string path): m_path(std::move(path)) {
}

si::SysFsReader::~SysFsReader() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

std::string_view si::SysFsReader::read() {
    m_file.open(m_path);
    m_file.seekg(0);

    if (!m_file.is_open()) {
        return "";          // Read nothing if file is not open, for cases like where file doesnt exist
    }

    m_buffer.clear();

    // Read the contents to a temporary buffer which is later added to the std string buffer
    char temp[4096];
    m_file.read(temp, sizeof(temp));
    m_file.close();

    m_buffer.append(temp, m_file.gcount());
    return m_buffer;
}
