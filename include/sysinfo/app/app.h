//
// Created by devboi on 4/22/26.
//

#ifndef SYSTEMINFO_APP_H
#define SYSTEMINFO_APP_H

#include <atomic>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

#include "json/nlohmann.h"
#include "http/httplib.h"
#include "sysinfo/modules/battery.h"
#include "sysinfo/modules/cpu.h"
#include "sysinfo/modules/memory.h"

namespace si {
    class app {

    public:
        explicit app(const int updateRate, const bool startWeb, const int port, std::string host) :
        m_updateRate(updateRate),
        m_startWeb(startWeb), m_webPort(port), m_webHost(std::move(host)) {

        }

        int run();

    private:
        int m_updateRate{1000};                 // Milliseconds to wait between updates
        bool m_startWeb{true};                  // Start webserver to serve information or no
        int m_webPort{6002};                    // Port on which the web server will be hosted on
        std::string m_webHost;                     // Host for the web server
        std::atomic<bool> m_running{true};

        // Module storage
        std::vector<std::unique_ptr<si::BatteryModule>> m_batteries;
        std::unique_ptr<si::CPUModule> m_cpu;
        std::unique_ptr<si::MemoryModule> m_memory;

        // Module registry - stores fetch functions for different modules
        using ModuleFetcher = std::function<nlohmann::json()>;
        std::unordered_map<std::string, ModuleFetcher> m_moduleFetchers;

        // Register modules
        void registerModules();

        // Generic handler for any module
        void handleModuleRequest(const std::string& moduleName, const httplib::Request &req, httplib::Response &res);

        // Different run entry points for different modes
        int runConsoleMode();
        int runWebMode();
    };


}

#endif //SYSTEMINFO_APP_H
