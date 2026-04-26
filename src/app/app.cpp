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
#include "sysinfo/modules/cpu.h"

int si::app::run() {
    const auto batteriesPath = si::BatteryModule::findBatteries();

    for (const auto& path: batteriesPath) {
        m_batteries.push_back(std::make_unique<si::BatteryModule>(path));
    }

    // Initialize CPU module
    m_cpu = std::make_unique<si::CPUModule>();

    // Register modules with their fetch functions
    registerModules();

    if (!m_startWeb) {
        return runConsoleMode();
    }

    return runWebMode();
}

void si::app::registerModules() {
    // Register battery module fetcher
    m_moduleFetchers["battery"] = [this]() -> nlohmann::json {
        nlohmann::json batteries_j = nlohmann::json::array();
        for (size_t i = 0; i < m_batteries.size(); ++i) {
            const auto batteryData = m_batteries[i]->fetchData();
            batteries_j.push_back(batteryData.toJson(i));
        }
        return nlohmann::json{{"batteries", batteries_j}};
    };

    // Register CPU module fetcher
    m_moduleFetchers["cpu"] = [this]() -> nlohmann::json {
        const auto cpuData = m_cpu->fetchData();
        return cpuData.toJson();
    };
}

void si::app::handleModuleRequest(const std::string& moduleName, const httplib::Request &req, httplib::Response &res) {
    auto it = m_moduleFetchers.find(moduleName);
    if (it != m_moduleFetchers.end()) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(it->second().dump(), "application/json");
    } else {
        res.status = 404;
        res.set_content(R"({"error": "Module not found"})", "application/json");
    }
}

int si::app::runConsoleMode() {
    while (m_running.load(std::memory_order::relaxed)) {
        // Clear the console
        std::printf("\033[2J\033[H");

        // Display CPU info
        if (m_cpu && m_cpu->isAvailable) {
            auto cpuData = m_cpu->fetchData();

            std::cout << std::format(
                "CPU:\nModel: {}\nCores: {}\nThreads: {}\nFrequency: {} MHz\nUsage: {:.1f}%\nLoad: {:.2f} {:.2f} {:.2f}\nTemperature: {:.1f}°C\n\n",
                cpuData.model,
                cpuData.cores,
                cpuData.threads,
                cpuData.frequency,
                cpuData.usage_percent,
                cpuData.load_1min,
                cpuData.load_5min,
                cpuData.load_15min,
                cpuData.temperature / 1000.0) << std::endl;
        }

        // Display Battery info
        for (size_t i = 0; i < m_batteries.size(); ++i) {
            auto batteryData = m_batteries[i]->fetchData();

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

int si::app::runWebMode() {
    httplib::Server svr;

    // Handler for all modules (must be registered before generic handler)
    svr.Get("/all", [this](const httplib::Request &req, httplib::Response &res) {
        nlohmann::json j;
        for (const auto& [name, fetcher] : m_moduleFetchers) {
            j[name] = fetcher();
        }
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(j.dump(), "application/json");
    });

    // Generic handler for any registered module (excludes /all)
    svr.Get(R"(/(\w+))", [this](const httplib::Request &req, httplib::Response &res) {
        std::string moduleName = req.matches[1];
        if (moduleName == "all") {
            res.status = 404;
            res.set_content(R"({"error": "Use /all endpoint"})", "application/json");
            return;
        }
        handleModuleRequest(moduleName, req, res);
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