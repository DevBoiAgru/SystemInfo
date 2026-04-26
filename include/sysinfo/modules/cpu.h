//
// Created by devboi on 4/26/26.
//

#ifndef SYSTEMINFO_CPU_H
#define SYSTEMINFO_CPU_H

#include <cstdint>
#include <string>
#include <vector>
#include "sysinfo/modules/module.h"
#include "json/nlohmann.h"

namespace si {

    namespace InfoTypes {
        struct CPUInfo {
            std::string model;
            uint32_t cores{0};
            uint32_t threads{0};
            uint64_t frequency{0};        // MHz
            double usage_percent{0.0};     // 0-100
            double load_1min{0.0};
            double load_5min{0.0};
            double load_15min{0.0};
            uint32_t temperature{0};       // millidegrees Celsius

            nlohmann::json toJson() const;
        };
    }

    class CPUModule : public InfoModule<InfoTypes::CPUInfo> {
    public:
        CPUModule();
        InfoTypes::CPUInfo fetchData() override;

        // Static helper to get CPU count (systems can have multiple CPUs/sockets)
        static int getCPUCount();

    private:
        // Helper methods
        static std::string readCPUModel();
        static uint32_t readCores();
        static uint32_t readThreads();
        static uint64_t readFrequency();
        static double readCPUUsage();
        static void readLoadAverages(double& load1, double& load5, double& load15);
        static uint32_t readTemperature();
    };
}

#endif //SYSTEMINFO_CPU_H
