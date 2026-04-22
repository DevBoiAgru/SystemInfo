//
// Created by devboi on 4/22/26.
//

#include "sysinfo/modules/battery.h"

#include <format>
#include <utility>
#include <charconv>
#include <filesystem>

si::BatteryModule::BatteryModule(std::string sysfsFolder)
    :m_sysfsFolder(std::move(sysfsFolder)),
    m_capacityReader(m_sysfsFolder + "/capacity"),
    m_voltageNowReader(m_sysfsFolder + "/power_now"),
    m_energyNowReader(m_sysfsFolder + "/voltage_now") {

    // We already get a battery number, always assume it exists (BAT0, BAT1 etc)
    isAvailable = true;

    // Try to find the model name for this battery
    std::string modelName(si::SysFsReader(m_sysfsFolder + "/model_name").read());
    if (modelName.empty()) {
        modelName = std::string(si::SysFsReader(m_sysfsFolder + "/serial_number").read());
        modelName = "Battery " + modelName;     // Prepend "Battery" to get "Battery <Serial Number>"

        // If still empty, fallback to "Battery"
        if (modelName.empty()) {
            modelName = "Battery";
        }
    }

    // Remove whitespace from the front and end
    auto const first = std::ranges::find_if(modelName, [](const unsigned char ch) {
        return !std::isspace(ch);
    });

    const auto last = std::ranges::find_if(modelName.rbegin(), modelName.rend(), [](const unsigned char ch) {
        return !std::isspace(ch);
    }).base();

    m_modelName = std::string(first, last);
}

si::InfoTypes::BatteryInfo si::BatteryModule::BatteryModule::fetchData() {
    auto data = si::InfoTypes::BatteryInfo{};

    const std::string_view capacityNow_sv = m_capacityReader.read();
    const std::string_view voltageNow_sv = m_voltageNowReader.read();
    const std::string_view energyNow_sv = m_energyNowReader.read();

    if (!capacityNow_sv.empty()) {
        std::from_chars(capacityNow_sv.data(), capacityNow_sv.data() + capacityNow_sv.size(), data.capacity);
    }
    if (!voltageNow_sv.empty()) {
        std::from_chars(voltageNow_sv.data(), voltageNow_sv.data() + voltageNow_sv.size(), data.voltage_now);
    }
    if (!energyNow_sv.empty()) {
        std::from_chars(energyNow_sv.data(), energyNow_sv.data() + energyNow_sv.size(), data.capacity);
    }

    data.modelName = m_modelName;

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
        if (std::getline(typeFile, type) && type == "Battery") {
            batteryPaths.push_back(entry.path().string());
        }
    }
    return batteryPaths;
}
