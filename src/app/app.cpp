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

    // Find batteries
    const auto batteriesPath = si::BatteryModule::findBatteries();
    std::vector<std::unique_ptr<si::BatteryModule>> batteries;

    for (const auto& path: batteriesPath) {
        batteries.push_back(std::make_unique<si::BatteryModule>(path));
    }
    assert(batteries.size() < 2 && "More than 1 battery detected! The program only supports outputting data for a single battery for now!");



    if (!m_startWeb) {
        while (m_running.load(std::memory_order::relaxed)) {

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
                    "BATTERY:\nName: {}\nCapacity: {}\nEnergy: {}\nVoltage: {}\nCurrent: {}\nStatus: {}\n",
                    batteryData.modelName,
                    batteryData.capacity,
                    batteryData.energy_now,
                    batteryData.voltage_now,
                    batteryData.current_now,
                    batteryStatus) << std::endl;


                std::this_thread::sleep_for(std::chrono::milliseconds(m_updateRate));
            }
        }
        return 0;
    }


    // HTTP Stuff
    httplib::Server svr;

    svr.Get("/battery", [&batteries](const httplib::Request &req, httplib::Response &res) {
        for (const auto& batt: batteries) {
                const auto batteryData = batt->fetchData();

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

                nlohmann::json j= {
                    {"status", batteryStatus},
                    {"capacity", batteryData.capacity},
                    {"voltage", batteryData.voltage_now},
                    {"current", batteryData.current_now},
                    {"energy", batteryData.energy_now},
                };

                // NOTE!: This disables CORS! Remove this if you need CORS
                res.set_header("Access-Control-Allow-Origin", "*");

                res.set_content(j.dump(), "application/json");
            }
    });

    svr.Options(R"(/.*)", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Allow-Credentials", "true");
        res.status = 204;
    });


    svr.listen(m_webHost, m_webPort);

    return 0;

}