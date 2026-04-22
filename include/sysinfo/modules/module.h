//
// Created by devboi on 4/22/26.
//

#ifndef SYSTEMINFO_MODULE_H
#define SYSTEMINFO_MODULE_H

namespace si {

    // This is the base class for every module which can return some data.
    // T is the struct of which type of data the module reads.
    // It is defined by the child class, the module.
    template <typename T>
    class InfoModule {
    public:
        bool isAvailable{false};

        virtual ~InfoModule() = default;
        virtual T fetchData() = 0;
    };
}

#endif //SYSTEMINFO_MODULE_H
