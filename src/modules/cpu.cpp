//
// Created by devboi on 4/26/26.
//

#include "sysinfo/modules/cpu.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <charconv>

si::CPUModule::CPUModule() {
    // CPU is always available on Linux systems
    isAvailable = true;
}

si::InfoTypes::CPUInfo si::CPUModule::fetchData() {
    auto data = si::InfoTypes::CPUInfo{};

    data.model = readCPUModel();
    data.cores = readCores();
    data.threads = readThreads();
    data.frequency = readFrequency();
    data.usage_percent = readCPUUsage();
    readLoadAverages(data.load_1min, data.load_5min, data.load_15min);
    data.temperature = readTemperature();

    return data;
}

int si::CPUModule::getCPUCount() {
    // Count CPU directories in /sys/devices/system/cpu
    namespace fs = std::filesystem;
    const std::string cpuPath = "/sys/devices/system/cpu";

    if (!fs::exists(cpuPath)) return 1;

    int count = 0;
    for (const auto& entry : fs::directory_iterator(cpuPath)) {
        if (entry.path().filename().string().starts_with("cpu") &&
            entry.path().filename().string() != "cpu") {
            count++;
        }
    }
    return count > 0 ? count : 1;
}

std::string si::CPUModule::readCPUModel() {
    std::ifstream file("/proc/cpuinfo");
    std::string line;

    while (std::getline(file, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string model = line.substr(pos + 1);
                // Trim leading whitespace
                size_t start = model.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    return model.substr(start);
                }
                return model;
            }
        }
    }

    // Falback to Hardware: <model>
    while (std::getline(file, line)) {
        if (line.find("Hardware") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string model = line.substr(pos + 1);
                // Trim leading whitespace
                size_t start = model.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    return model.substr(start);
                }
                return model;
            }
        }
    }

    return "Unknown CPU";
}

uint32_t si::CPUModule::readCores() {
    std::ifstream file("/proc/cpuinfo");
    std::string line;
    uint32_t cores = 0;

    while (std::getline(file, line)) {
        if (line.find("cpu cores") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                size_t start = value.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    uint32_t result = 0;
                    std::from_chars(value.data() + start, value.data() + value.size(), result);
                    if (result > 0) return result;
                }
            }
        }
    }
    return getCPUCount();
}

uint32_t si::CPUModule::readThreads() {
    return getCPUCount();
}

uint64_t si::CPUModule::readFrequency() {
    // Try to read from cpufreq first (more accurate)
    std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (file.is_open()) {
        uint64_t freq_khz = 0;
        file >> freq_khz;
        if (freq_khz > 0) {
            return freq_khz / 1000; // Convert to MHz
        }
    }

    // Fallback to cpuinfo
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;

    while (std::getline(cpuinfo, line)) {
        if (line.find("cpu MHz") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                size_t start = value.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    double freq = 0.0;
                    std::istringstream(value.substr(start)) >> freq;
                    return static_cast<uint64_t>(freq);
                }
            }
        }
    }
    return 0;
}

double si::CPUModule::readCPUUsage() {
    static uint64_t prev_idle = 0, prev_total = 0;

    std::ifstream file("/proc/stat");
    std::string line;
    uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0;

    if (std::getline(file, line) && line.starts_with("cpu ")) {
        std::istringstream iss(line);
        std::string cpu;

        // Line example: cpuX  828381 2288 358761 2451951 5372 37846 26412 0 0 0
        iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    }

    uint64_t total = user + nice + system + idle + iowait + irq + softirq + steal;
    uint64_t total_diff = total - prev_total;
    uint64_t idle_diff = idle - prev_idle;

    prev_total = total;
    prev_idle = idle;

    if (total_diff == 0) return 0.0;

    return 100.0 * (1.0 - static_cast<double>(idle_diff) / total_diff);
}

void si::CPUModule::readLoadAverages(double& load1, double& load5, double& load15) {
    std::ifstream file("/proc/loadavg");
    // Example: 2.05 2.37 2.92 3/1792 2718472
    file >> load1 >> load5 >> load15;
}

uint32_t si::CPUModule::readTemperature() {
    namespace fs = std::filesystem;
    const std::string thermalPath = "/sys/class/thermal";

    if (!fs::exists(thermalPath)) return 0;

    // Look for thermal zones
    for (const auto& entry : fs::directory_iterator(thermalPath)) {
        std::string zonePath = entry.path().string();
        std::string typePath = zonePath + "/type";
        std::string tempPath = zonePath + "/temp";

        std::ifstream typeFile(typePath);
        std::string type;
        if (std::getline(typeFile, type)) {
            // Look for CPU-related thermal zones
            if (type.find("cpu") != std::string::npos ||
                type.find("CPU") != std::string::npos ||
                type.find("x86") != std::string::npos) {
                std::ifstream tempFile(tempPath);
                uint32_t temp = 0;
                if (tempFile >> temp) {
                    return temp; // Already in millidegrees
                }
            }
        }
    }

    return 0;
}

nlohmann::json si::InfoTypes::CPUInfo::toJson() const {
    return nlohmann::json{
        {"model", model},
        {"cores", cores},
        {"threads", threads},
        {"frequency_mhz", frequency},
        {"usage_percent", usage_percent},
        {"load_1min", load_1min},
        {"load_5min", load_5min},
        {"load_15min", load_15min},
        {"temperature_c", temperature / 1000.0} // Convert to Celsius
    };
}
