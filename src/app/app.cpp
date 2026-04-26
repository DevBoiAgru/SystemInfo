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

        return 0;
    }


    // HTTP Stuff
    httplib::Server svr;

    svr.Get("/battery", [&batteries](const httplib::Request &req, httplib::Response &res) {
        nlohmann::json j;
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

                j= {
                    {"status", batteryStatus},
                    {"capacity", batteryData.capacity},
                    {"voltage", batteryData.voltage_now},
                    {"current", batteryData.current_now},
                    {"energy", batteryData.energy_now},
                };

                res.set_content(j.dump(), "application/json");
            }
    });


    svr.listen(m_webHost, m_webPort);

    return 0;

}