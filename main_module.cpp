#include "main_module.h"

#include "common/global.h"
#include "common/config/listen_flags.h"

namespace libnetwork {

std::shared_ptr<ListenService> main_module_run() {
    const char* conf_path = "data/config/listen.conf";
    char flag_file[128] = {0};
    snprintf(flag_file, sizeof(flag_file), "%s%s", g_prefix_res_path, conf_path);
    google::SetCommandLineOption("flagfile", flag_file);

    std::shared_ptr<ListenService> listen_service =
        ListenServiceFactory::instance()->create_service(FLAGS_communication_type);
    if (!listen_service)
        return nullptr;

    bool init_ret = listen_service->init();
    if (!init_ret)
        return nullptr;

    listen_service->run();

    return listen_service;
}

}
