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
#include "sysinfo/modules/memory.h"
#include "sysinfo/utils/ansi.h"

int si::app::run() {
    const auto batteriesPath = si::BatteryModule::findBatteries();

    for (const auto& path: batteriesPath) {
        m_batteries.push_back(std::make_unique<si::BatteryModule>(path));
    }

    // Initialize CPU module
    m_cpu = std::make_unique<si::CPUModule>();

    // Initialize Memory module
    m_memory = std::make_unique<si::MemoryModule>();

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

    // Register Memory module fetcher
    m_moduleFetchers["memory"] = [this]() -> nlohmann::json {
        const auto memoryData = m_memory->fetchData();
        return memoryData.toJson();
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
    using namespace si::utils;

    while (m_running.load(std::memory_order::relaxed)) {
        // Clear the console only in pretty mode
        if (m_prettyPrint) {
            std::printf(ANSI_CLEAR_SCREEN);
        }

        // Display CPU info
        if (m_cpu && m_cpu->isAvailable) {
            auto cpuData = m_cpu->fetchData();

            const std::string indent = m_prettyPrint ? "  " : "";
            std::cout << header("CPU:", m_prettyPrint) << "\n";
            std::cout << label(indent + "Model:", m_prettyPrint) << " " << cpuData.model << "\n";
            std::cout << label(indent + "Cores:", m_prettyPrint) << " " << cpuData.cores << "\n";
            std::cout << label(indent + "Threads:", m_prettyPrint) << " " << cpuData.threads << "\n";
            std::cout << label(indent + "Frequency:", m_prettyPrint) << " " << cpuData.frequency << " MHz\n";
            std::cout << label(indent + "Usage:", m_prettyPrint) << " " << value(std::format("{:.1f}", cpuData.usage_percent), m_prettyPrint) << "%\n";
            std::cout << label(indent + "Load:", m_prettyPrint) << " " << cpuData.load_1min << " " << cpuData.load_5min << " " << cpuData.load_15min << "\n";
            std::cout << label(indent + "Temperature:", m_prettyPrint) << " " << value_magenta(std::format("{:.1f}", cpuData.temperature / 1000.0), m_prettyPrint) << "°C\n\n";
        }

        // Display Memory info
        if (m_memory) {
            auto memoryData = m_memory->fetchData();

            const std::string indent = m_prettyPrint ? "  " : "";
            std::cout << header("MEMORY:", m_prettyPrint) << "\n";
            std::cout << label(indent + "Total:", m_prettyPrint) << " " << static_cast<float>(memoryData.total) / 1024 / 1024 << " GB\n";
            std::cout << label(indent + "Used:", m_prettyPrint) << " " << value_red(std::format("{:.1f}", static_cast<float>(memoryData.used) / 1024 / 1024), m_prettyPrint) << " GB\n";
            std::cout << label(indent + "Available:", m_prettyPrint) << " " << static_cast<float>(memoryData.available) / 1024 / 1024 << " GB\n";
            std::cout << label(indent + "Usage:", m_prettyPrint) << " " << value(std::format("{:.1f}", static_cast<float>(memoryData.usage_percent)), m_prettyPrint) << "%\n";
            std::cout << label(indent + "Cached:", m_prettyPrint) << " " << static_cast<float>(memoryData.cached) / 1024 << " MB\n";
            std::cout << label(indent + "Buffers:", m_prettyPrint) << " " << static_cast<float>(memoryData.buffers) / 1024 << " MB\n";
            std::cout << label(indent + "Swap:", m_prettyPrint) << " " << static_cast<float>(memoryData.swap_used) / 1024 / 1024 << " GB / " << static_cast<float>(memoryData.swap_total) / 1024 / 1024 << " GB\n\n";
        }

        // Display Battery info
        for (size_t i = 0; i < m_batteries.size(); ++i) {
            auto batteryData = m_batteries[i]->fetchData();

            const std::string indent = m_prettyPrint ? "  " : "";
            std::cout << header(std::format("BATTERY {}:", i), m_prettyPrint) << "\n";
            std::cout << label(indent + "Name:", m_prettyPrint) << " " << batteryData.modelName << "\n";
            std::cout << label(indent + "Capacity:", m_prettyPrint) << " " << value(std::format("{}", batteryData.capacity), m_prettyPrint) << "%\n";
            std::cout << label(indent + "Energy:", m_prettyPrint) << " " << batteryData.energy_now << " µWh\n";
            std::cout << label(indent + "Voltage:", m_prettyPrint) << " " << batteryData.voltage_now / 1000 << " mV\n";
            std::cout << label(indent + "Current:", m_prettyPrint) << " " << batteryData.current_now / 1000 << " mA\n";
            std::cout << label(indent + "Power:", m_prettyPrint) << " " << batteryData.power_now << " mW\n";
            std::cout << label(indent + "Status:", m_prettyPrint) << " " << si::BatteryModule::statusToString(batteryData.status) << "\n";
        }

        // Add separator in plain mode
        if (!m_prettyPrint) {
            std::cout << "----------------------------------------\n";
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