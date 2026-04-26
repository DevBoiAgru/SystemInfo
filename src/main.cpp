#include <sysinfo/app/app.h>

int main() {
    si::app app(1000, true, 6002, "0.0.0.0");

    return app.run();
}