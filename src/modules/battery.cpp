//
// Created by devboi on 4/22/26.
//

#include "sysinfo/modules/battery.h"
#include "sysinfo/utils/stringFuncs.h"

#include <utility>
#include <charconv>
#include <filesystem>
#include <iostream>

si::BatteryModule::BatteryModule(std::string sysfsFolder)
    :m_sysfsFolder(std::move(sysfsFolder)),
    m_capacityReader(m_sysfsFolder + "/capacity"),
    m_voltageNowReader(m_sysfsFolder + "/power_now"),
    m_currentNowReader(m_sysfsFolder + "/current_now"),
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
            modelName = "Unknown battery";
        }
    }

    // Initialise the max values, these shouldn't change so safe to set them once on init
    si::SysFsReader energyFullReader(m_sysfsFolder + "/energy_full");
    si::SysFsReader energyFullDesignReader(m_sysfsFolder + "/energy_full");
    const std::string_view energyMax_sv = energyFullReader.read();
    const std::string_view energyMaxDesign_sv = energyFullDesignReader.read();

    if (!energyMax_sv.empty()) {
        std::from_chars(energyMax_sv.data(), energyMax_sv.data() + energyMax_sv.size(), m_energy_max);
    }

    if (!energyMaxDesign_sv.empty()) {
        std::from_chars(energyMaxDesign_sv.data(), energyMaxDesign_sv.data() + energyMaxDesign_sv.size(), m_energy_max_design);
    }


    // Remove whitespace from the front and end
    m_modelName = si::utils::trim(modelName);
}

si::InfoTypes::BatteryInfo si::BatteryModule::BatteryModule::fetchData() {
    auto data = si::InfoTypes::BatteryInfo{};

    const std::string_view capacityNow_sv = m_capacityReader.read();
    const std::string_view voltageNow_sv = m_voltageNowReader.read();
    const std::string_view energyNow_sv = m_energyNowReader.read();
    const std::string_view currentNow_sv = m_currentNowReader.read();

    if (!capacityNow_sv.empty()) {
        std::from_chars(capacityNow_sv.data(), capacityNow_sv.data() + capacityNow_sv.size(), data.capacity);
    }
    if (!voltageNow_sv.empty()) {
        std::from_chars(voltageNow_sv.data(), voltageNow_sv.data() + voltageNow_sv.size(), data.voltage_now);
    }
    if (!energyNow_sv.empty()) {
        std::from_chars(energyNow_sv.data(), energyNow_sv.data() + energyNow_sv.size(), data.energy_now);
    }

    if (!currentNow_sv.empty()) {
        std::from_chars(currentNow_sv.data(), currentNow_sv.data() + currentNow_sv.size(), data.current_now);
    }

    // Update status
    si::SysFsReader statusReader(m_sysfsFolder + "/status");
    auto battStatus_sv = statusReader.read();
    auto batteryStatus = si::utils::trim(battStatus_sv);

    if (batteryStatus == "Not charging") {
        data.status = InfoTypes::BatteryStatus::NotCharging;
    } else if (batteryStatus == "Discharging") {
        data.status = InfoTypes::BatteryStatus::Discharging;
    } else if (batteryStatus == "Charging") {
        data.status = InfoTypes::BatteryStatus::Charging;
    } else if (batteryStatus == "Full") {
        data.status = InfoTypes::BatteryStatus::Full;
    } else {
        data.status = InfoTypes::BatteryStatus::Unknown;
    }
    
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
        if (std::getline(typeFile, type) && type == "Battery") {
            batteryPaths.push_back(entry.path().string());
        }
    }
    return batteryPaths;
}
