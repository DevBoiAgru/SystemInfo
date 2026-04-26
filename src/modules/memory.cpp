//
// Created by devboi on 4/26/26.
//

#include "sysinfo/modules/memory.h"
#include <fstream>
#include <sstream>
#include <charconv>

si::MemoryModule::MemoryModule() {
    // Memory info is always available on Linux systems
    isAvailable = true;
}

si::InfoTypes::MemoryInfo si::MemoryModule::fetchData() {
    auto data = si::InfoTypes::MemoryInfo{};

    readMemoryInfo(data.total, data.free, data.available, data.cached, data.buffers);
    readSwapInfo(data.swap_total, data.swap_free);
    data.swap_used = data.swap_total - data.swap_free;
    data.used = data.total - data.available;
    data.usage_percent = calculateUsage(data.total, data.available);

    return data;
}

uint64_t si::MemoryModule::parseMemValue(const std::string& line) {
    // Format: "MemTotal:       16384000 kB"
    size_t pos = line.find(':');
    if (pos == std::string::npos) return 0;

    std::string valueStr = line.substr(pos + 1);
    size_t start = valueStr.find_first_not_of(" \t");
    if (start == std::string::npos) return 0;

    size_t end = valueStr.find_first_of(" \t", start);
    if (end == std::string::npos) end = valueStr.size();

    std::string numStr = valueStr.substr(start, end - start);

    uint64_t value = 0;
    std::from_chars(numStr.data(), numStr.data() + numStr.size(), value);
    return value;
}

void si::MemoryModule::readMemoryInfo(uint64_t& total, uint64_t& free, uint64_t& available,
                                      uint64_t& cached, uint64_t& buffers) {
    std::ifstream file("/proc/meminfo");
    std::string line;

    total = free = available = cached = buffers = 0;

    while (std::getline(file, line)) {
        if (line.find("MemTotal:") != std::string::npos) {
            total = parseMemValue(line);
        } else if (line.find("MemFree:") != std::string::npos) {
            free = parseMemValue(line);
        } else if (line.find("MemAvailable:") != std::string::npos) {
            available = parseMemValue(line);
        } else if (line.find("Cached:") != std::string::npos) {
            cached = parseMemValue(line);
        } else if (line.find("Buffers:") != std::string::npos) {
            buffers = parseMemValue(line);
        }
    }

    // Fallback: if MemAvailable is not available (older kernels), calculate it
    if (available == 0) {
        available = free + cached + buffers;
    }
}

void si::MemoryModule::readSwapInfo(uint64_t& swap_total, uint64_t& swap_free) {
    std::ifstream file("/proc/meminfo");
    std::string line;

    swap_total = swap_free = 0;

    while (std::getline(file, line)) {
        if (line.find("SwapTotal:") != std::string::npos) {
            swap_total = parseMemValue(line);
        } else if (line.find("SwapFree:") != std::string::npos) {
            swap_free = parseMemValue(line);
        }
    }
}

double si::MemoryModule::calculateUsage(uint64_t total, uint64_t available) {
    if (total == 0) return 0.0;
    return 100.0 * (1.0 - static_cast<double>(available) / total);
}

nlohmann::json si::InfoTypes::MemoryInfo::toJson() const {
    return nlohmann::json{
        {"total_kb", total},
        {"used_kb", used},
        {"free_kb", free},
        {"available_kb", available},
        {"cached_kb", cached},
        {"buffers_kb", buffers},
        {"swap_total_kb", swap_total},
        {"swap_free_kb", swap_free},
        {"swap_used_kb", swap_used},
        {"usage_percent", usage_percent},
        {"total_mb", static_cast<float>(total) / 1024},
        {"used_mb", static_cast<float>(used) / 1024},
        {"available_mb", static_cast<float>(available) / 1024},
        {"total_gb", static_cast<float>(total) / 1024 / 1024},
        {"used_gb", static_cast<float>(used) / 1024 / 1024},
        {"available_gb", static_cast<float>(available) / 1024 / 1024}
    };
}
