#include <string>
#include <iostream>
#include <format>
#include <sysinfo/app/app.h>

int main(const int argc, char* argv[]) {

    // Defaults
    std::string host = "0.0.0.0";
    bool startWeb = false;
    bool prettyPrint = false;
    int port = 6002;

    // Generate help text
    const std::string helpText = std::format(
        "{} - System monitoring tool\n\n"
        "Usage: {} [OPTIONS]\n\n"
        "Options:\n"
        "  -w, --web        Start web server mode.\n"
        "  -p, --pretty     Enable pretty console output with colors\n"
        "  --host HOST     Set web server host (default: 0.0.0.0)\n"
        "  --port PORT     Set web server port (default: 6002)\n"
        "  -h, --help      Show this help message and exit.\n"
        ,
        argv[0],
        argv[0]
    );

    // Parse command line flags
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            std::cout << helpText << std::endl;
            return 0;
        } 
        else if (arg == "--web" || arg == "-w") {
            startWeb = true;
        }
        else if (arg == "--pretty" || arg == "-p") {
            prettyPrint = true;
        }
        else if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        }
        else if (arg == "--port" && i + 1 < argc) {
            try {
                port = std::stoi(argv[++i]);
            } catch (...) {
                std::cout << "Invalid port\n";
                return 1;
            }
        }
        else {
            std::cout << "Unknown argument: " << arg << "\n";
            std::cout << helpText << std::endl;
            return 1;
        }
    }

    si::app app(1000, startWeb, port, host, prettyPrint);

    return app.run();
}