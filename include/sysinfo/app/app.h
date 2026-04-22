//
// Created by devboi on 4/22/26.
//

#ifndef SYSTEMINFO_APP_H
#define SYSTEMINFO_APP_H

#include <atomic>

namespace si {
    class app {

    public:
        explicit app(const int updateRate = 1000) : m_updateRate(updateRate) {

        }

        int run();

    private:
        int m_updateRate{1000};                 // Milliseconds to wait between updates
        std::atomic<bool> m_running{true};
    };


}

#endif //SYSTEMINFO_APP_H
