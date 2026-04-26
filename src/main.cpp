#include <cstring>
#include <sysinfo/app/app.h>

int main(const int argc, char* argv[]) {

    bool startWeb = false;

    if (argc > 1 && (strcmp(argv[1], "--web") == 0 || strcmp(argv[1], "-w") == 0)) {
        startWeb = true;
    }

    si::app app(1000, startWeb, 6002, "0.0.0.0");

    return app.run();
}