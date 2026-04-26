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
#include "json/nlohmann.h"

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
            uint32_t capacity{0};
            uint32_t voltage_now{0};
            uint32_t energy_now{0};
            uint32_t current_now{0};
            uint32_t power_now{0};

            uint32_t energy_max{0};
            uint32_t energy_max_design{0};

            std::string modelName;
            BatteryStatus status{BatteryStatus::Unknown};

            // Convert to JSON with optional index
            nlohmann::json toJson(int index = -1) const;
        };
    }

    class BatteryModule : InfoModule<InfoTypes::BatteryInfo> {
    public:
        explicit BatteryModule(std::string sysfsFolder);
        InfoTypes::BatteryInfo fetchData() override;

        // Helper to find battery node locations
        static std::vector<std::string> findBatteries();

        // Static helper to convert BatteryStatus enum to string
        static std::string statusToString(InfoTypes::BatteryStatus status);

    private:
        std::string m_sysfsFolder;          // The sysfs path. Example: /sys/class/power_supply/BAT{X}
        std::string m_modelName;

        SysFsReader m_capacityReader;       // Reader to read the capacity file
        SysFsReader m_voltageNowReader;     // Reader to read the current voltage
        SysFsReader m_currentNowReader;     // Reader to read the current
        SysFsReader m_energyNowReader;      // Reader to read the current energy

        uint32_t m_energy_max{0};
        uint32_t m_energy_max_design{0};

        // Helper method to parse battery status string
        static InfoTypes::BatteryStatus parseStatus(std::string_view statusStr);
        // Helper method to safely parse uint32_t from string_view
        static bool parseUint32(std::string_view str, uint32_t& value);
        // Helper method to read and parse a sysfs file in one call
        static bool readAndParseUint32(const std::string& path, uint32_t& value);
        // Helper method to determine battery model name
        static std::string determineModelName(const std::string& sysfsFolder);
    };
}

#endif //SYSTEMINFO_BATTERY_H
