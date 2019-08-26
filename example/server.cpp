#include "main_module.h"

#include "log/log.h"

int main(int, char**) {
    comlog::Log::instance()->init("server.log");

    std::shared_ptr<ListenService> ls = libnetwork::main_module_run();
    if (!ls)
        return 0;

    while (1) {}

    ls->stop();

    return 0;
}
