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

            std::cout << std::format(
                "Name: {}\nCapacity: {}\nEnergy: {}\nVoltage (microVolts): {}",
                batteryData.modelName,
                batteryData.capacity,
                batteryData.energy_now,
                batteryData.voltage_now) << "\n---------" << std::endl;
        }



        std::this_thread::sleep_for(std::chrono::milliseconds(m_updateRate));
    }

    return 0;
}