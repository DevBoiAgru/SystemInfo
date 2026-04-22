//
// Created by devboi on 4/22/26.
//

#include "sysinfo/app/app.h"

#include <format>
#include <thread>
#include <filesystem>
#include <iostream>

#include "sysinfo/modules/battery.h"


int si::app::run() {

    // Find batteries
    const auto batteriesPath = si::BatteryModule::findBatteries();
    std::vector<std::unique_ptr<si::BatteryModule>> batteries;

    for (const auto& path: batteriesPath) {
        batteries.push_back(std::make_unique<si::BatteryModule>(path));
    }

    while (m_running.load(std::memory_order_relaxed)) {

        std::printf("\033[2J\033[H");

        for (const auto& batt: batteries) {
            auto batteryData = batt->fetchData();

            std::string batteryStatus;

            switch (batteryData.status) {
                case si::InfoTypes::BatteryStatus::Unknown:
                    batteryStatus = "Unknown";
                    break;
                case si::InfoTypes::BatteryStatus::Full:
                    batteryStatus = "Full";
                    break;
                case si::InfoTypes::BatteryStatus::Discharging:
                    batteryStatus = "Discharging";
                    break;
                case si::InfoTypes::BatteryStatus::Charging:
                    batteryStatus = "Charging";
                    break;
                case si::InfoTypes::BatteryStatus::NotCharging:
                    batteryStatus = "Not charging";
                    break;
            }

            std::cout << std::format(
                "Name: {}\nCapacity: {}\nEnergy: {}\nVoltage (microVolts): {}\nStatus: {}",
                batteryData.modelName,
                batteryData.capacity,
                batteryData.energy_now,
                batteryData.voltage_now,
                batteryStatus) << "\n---------" << std::endl;
        }



        std::this_thread::sleep_for(std::chrono::milliseconds(m_updateRate));
    }

    return 0;
}