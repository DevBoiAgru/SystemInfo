//
// Created by devboi on 4/22/26.
//

#ifndef SYSTEMINFO_BATTERY_H
#define SYSTEMINFO_BATTERY_H

#include <cstdint>
#include <vector>
#include <string>
#include "sysinfo/modules/module.h"
#include "sysinfo/utils/sysfsReader.h"

namespace si {

    namespace InfoTypes {
        enum class BatteryStatus {
            Unknown = 0,
            Charging,
            Discharging,
            NotCharging,
            Full,
        };

        struct BatteryInfo {
            uint8_t capacity{0};
            uint32_t voltage_now{0};
            uint32_t energy_now{0};
            uint32_t current_now{0};

            uint32_t energy_max{0};
            uint32_t energy_max_design{0};

            std::string_view modelName;
            BatteryStatus status{BatteryStatus::Unknown};
        };
    }

    class BatteryModule : InfoModule<InfoTypes::BatteryInfo> {
    public:
        explicit BatteryModule(std::string sysfsFolder);
        InfoTypes::BatteryInfo fetchData() override;

        // Helper to find battery node locations
        static std::vector<std::string> findBatteries();

    private:
        std::string m_sysfsFolder;          // The sysfs path. Example: /sys/class/power_supply/BAT{X}
        std::string m_modelName;

        SysFsReader m_capacityReader;       // Reader to read the capacity file
        SysFsReader m_voltageNowReader;     // Reader to read the current voltage
        SysFsReader m_currentNowReader;     // Reader to read the current
        SysFsReader m_energyNowReader;      // Reader to read the current energy

        uint32_t m_energy_max{0};
        uint32_t m_energy_max_design{0};
    };
}

#endif //SYSTEMINFO_BATTERY_H
