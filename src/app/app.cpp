//
// Created by devboi on 4/22/26.
//

#include "sysinfo/app/app.h"

#include <format>
#include <thread>
#include <filesystem>
#include <iostream>
#include <cassert>

#include "sysinfo/modules/battery.h"
#include "json/nlohmann.h"
#include "http/httplib.h"

int si::app::run() {
    const auto batteriesPath = si::BatteryModule::findBatteries();
    std::vector<std::unique_ptr<si::BatteryModule>> batteries;

    for (const auto& path: batteriesPath) {
        batteries.push_back(std::make_unique<si::BatteryModule>(path));
    }

    if (!m_startWeb) {
        return runConsoleMode(batteries);
    }

    return runWebMode(batteries);
}

int si::app::runConsoleMode(const std::vector<std::unique_ptr<si::BatteryModule>>& batteries) {
    while (m_running.load(std::memory_order::relaxed)) {
        std::printf("\033[2J\033[H");

        for (size_t i = 0; i < batteries.size(); ++i) {
            auto batteryData = batteries[i]->fetchData();

            std::cout << std::format(
                "BATTERY {}:\nName: {}\nCapacity: {}%\nEnergy: {} µWh\nVoltage: {} mV\nCurrent: {} mA\nPower: {} mW\nStatus: {}\n",
                i,
                batteryData.modelName,
                batteryData.capacity,
                batteryData.energy_now,
                batteryData.voltage_now / 1000,
                batteryData.current_now / 1000,
                batteryData.power_now,
                si::BatteryModule::statusToString(batteryData.status)) << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(m_updateRate));
    }
    return 0;
}

int si::app::runWebMode(const std::vector<std::unique_ptr<si::BatteryModule>>& batteries) {
    httplib::Server svr;

    svr.Get("/battery", [&batteries](const httplib::Request &req, httplib::Response &res) {
        nlohmann::json j;
        nlohmann::json batteries_j = nlohmann::json::array();

        for (size_t i = 0; i < batteries.size(); ++i) {
            const auto batteryData = batteries[i]->fetchData();
            batteries_j.push_back(batteryData.toJson(i));
        }

        j["batteries"] = batteries_j;

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(j.dump(), "application/json");
    });

    svr.Options(R"(/.*)", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Credentials", "true");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, Cache-Control");
        res.status = 204;
    });

    svr.listen(m_webHost, m_webPort);
    return 0;
}