//
// Created by devboi on 4/22/26.
//

#include "sysinfo/modules/battery.h"
#include "sysinfo/utils/stringFuncs.h"

#include <utility>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

si::BatteryModule::BatteryModule(std::string sysfsFolder)
    :m_sysfsFolder(std::move(sysfsFolder)),
    m_capacityReader(m_sysfsFolder + "/capacity"),
    m_voltageNowReader(m_sysfsFolder + "/voltage_now"),
    m_currentNowReader(m_sysfsFolder + "/current_now"),
    m_energyNowReader(m_sysfsFolder + "/energy_now") {

    // Validate that the sysfs path exists before marking as available
    namespace fs = std::filesystem;
    isAvailable = fs::exists(m_sysfsFolder) && fs::is_directory(m_sysfsFolder);

    if (!isAvailable) {
        return;
    }

    // Determine model name
    m_modelName = determineModelName(m_sysfsFolder);

    // Initialise the max values, these shouldn't change so safe to set them once on init
    readAndParseUint32(m_sysfsFolder + "/energy_full", m_energy_max);
    readAndParseUint32(m_sysfsFolder + "/energy_full_design", m_energy_max_design);
}

si::InfoTypes::BatteryInfo si::BatteryModule::fetchData() {
    auto data = si::InfoTypes::BatteryInfo{};

    parseUint32(m_capacityReader.read(), data.capacity);
    parseUint32(m_voltageNowReader.read(), data.voltage_now);
    parseUint32(m_energyNowReader.read(), data.energy_now);
    parseUint32(m_currentNowReader.read(), data.current_now);

    // Calculate power: P = V × I (in µW)
    data.power_now = static_cast<uint32_t>(
        static_cast<uint64_t>(data.voltage_now) * data.current_now / 1000000
    );

    // Update status
    data.status = parseStatus(si::utils::trim(SysFsReader(m_sysfsFolder + "/status").read()));

    data.modelName = m_modelName;
    data.energy_max = m_energy_max;
    data.energy_max_design = m_energy_max_design;

    return data;
}

std::vector<std::string> si::BatteryModule::findBatteries() {
    // Crawl the /sys/class/power_supply/ directory and find paths for the battery nodes
    // This is needed because the battery can be named differently on kernels, for example
    // in some places it might be named BATT0, BATT1, battery and so on.
    namespace fs = std::filesystem;

    std::vector<std::string> batteryPaths;
    const std::string basePath = "/sys/class/power_supply/";

    if (!fs::exists(basePath)) return batteryPaths;

    for (const auto& entry : fs::directory_iterator(basePath)) {
        // Check if the 'type' file contains "Battery"
        std::ifstream typeFile(entry.path() / "type");
        std::string type;
        if (std::getline(typeFile, type) && si::utils::trim(type) == "Battery") {
            batteryPaths.push_back(entry.path().string());
        }
    }
    return batteryPaths;
}

si::InfoTypes::BatteryStatus si::BatteryModule::parseStatus(std::string_view statusStr) {
    // Convert to lowercase for case-insensitive comparison
    std::string lowerStatus;
    lowerStatus.reserve(statusStr.size());
    std::transform(statusStr.begin(), statusStr.end(), std::back_inserter(lowerStatus),
                   [](unsigned char c) { return std::tolower(c); });

    if (lowerStatus == "not charging") {
        return InfoTypes::BatteryStatus::NotCharging;
    } else if (lowerStatus == "discharging") {
        return InfoTypes::BatteryStatus::Discharging;
    } else if (lowerStatus == "charging") {
        return InfoTypes::BatteryStatus::Charging;
    } else if (lowerStatus == "full") {
        return InfoTypes::BatteryStatus::Full;
    } else {
        return InfoTypes::BatteryStatus::Unknown;
    }
}

bool si::BatteryModule::parseUint32(std::string_view str, uint32_t& value) {
    if (str.empty()) {
        return false;
    }
    uint32_t result = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    if (ec == std::errc()) {
        value = result;
        return true;
    }
    return false;
}

bool si::BatteryModule::readAndParseUint32(const std::string& path, uint32_t& value) {
    return parseUint32(SysFsReader(path).read(), value);
}

std::string si::BatteryModule::determineModelName(const std::string& sysfsFolder) {
    std::string modelName(si::SysFsReader(sysfsFolder + "/model_name").read());

    if (modelName.empty()) {
        // Fallback to Battery + model name, for example: Battery LIPC101
        modelName = std::string(si::SysFsReader(sysfsFolder + "/serial_number").read());
        if (!modelName.empty()) {
            modelName = "Battery " + modelName;
        }
    }

    if (modelName.empty()) {
        modelName = "Unknown battery";
    }

    return si::utils::trim(modelName);
}

std::string si::BatteryModule::statusToString(InfoTypes::BatteryStatus status) {
    switch (status) {
        case InfoTypes::BatteryStatus::Unknown: return "Unknown";
        case InfoTypes::BatteryStatus::Full: return "Full";
        case InfoTypes::BatteryStatus::Discharging: return "Discharging";
        case InfoTypes::BatteryStatus::Charging: return "Charging";
        case InfoTypes::BatteryStatus::NotCharging: return "Not charging";
        default: return "Unknown";
    }
}

nlohmann::json si::InfoTypes::BatteryInfo::toJson(int index) const {
    nlohmann::json j{
        {"name", modelName},
        {"status", BatteryModule::statusToString(status)},
        {"capacity", capacity},
        {"voltage", voltage_now},
        {"current", current_now},
        {"energy", energy_now},
        {"power", power_now},
        {"energy_max", energy_max},
        {"energy_max_design", energy_max_design}
    };

    if (index >= 0) {
        j["index"] = index;
    }

    return j;
}
