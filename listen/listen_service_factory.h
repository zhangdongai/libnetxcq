#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "common/macros.h"
#include "listen/listen_service.h"

class ListenServiceFactory {
public:
    std::shared_ptr<ListenService> create_service(const std::string& type) const;

private:
    void init();

    std::map<std::string, std::function<std::shared_ptr<ListenService>()>>
        listen_services_;
    SINGLETON_DECLARE(ListenServiceFactory);
};
