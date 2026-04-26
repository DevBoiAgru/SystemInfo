//
// Created by devboi on 4/26/26.
//

#ifndef SYSTEMINFO_MEMORY_H
#define SYSTEMINFO_MEMORY_H

#include <cstdint>
#include <string>
#include "sysinfo/modules/module.h"
#include "json/nlohmann.h"

namespace si {

    namespace InfoTypes {
        struct MemoryInfo {
            uint64_t total{0};        // KB
            uint64_t used{0};         // KB
            uint64_t free{0};         // KB
            uint64_t available{0};    // KB
            uint64_t cached{0};       // KB
            uint64_t buffers{0};      // KB
            uint64_t swap_total{0};   // KB
            uint64_t swap_free{0};    // KB
            uint64_t swap_used{0};    // KB
            double usage_percent{0.0}; // 0-100

            nlohmann::json toJson() const;
        };
    }

    class MemoryModule : public InfoModule<InfoTypes::MemoryInfo> {
    public:
        MemoryModule();
        InfoTypes::MemoryInfo fetchData() override;

    private:
        // Helper methods
        static uint64_t parseMemValue(const std::string& line);
        static void readMemoryInfo(uint64_t& total, uint64_t& free, uint64_t& available,
                                    uint64_t& cached, uint64_t& buffers);
        static void readSwapInfo(uint64_t& swap_total, uint64_t& swap_free);
        static double calculateUsage(uint64_t total, uint64_t available);
    };
}

#endif //SYSTEMINFO_MEMORY_H
