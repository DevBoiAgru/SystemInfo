//
// Created by devboi on 4/22/26.
//

#ifndef SYSTEMINFO_APP_H
#define SYSTEMINFO_APP_H

#include <atomic>
#include <string>
#include <utility>

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
    };


}

#endif //SYSTEMINFO_APP_H
