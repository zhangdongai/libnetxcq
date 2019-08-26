#include "listen/listen_service_factory.h"

#include "listen/tcp_listen_service.h"
#include "listen/udp_listen_service.h"

using ListenServicePtr = std::shared_ptr<ListenService>;

ListenServiceFactory::ListenServiceFactory() {
    init();
}

void ListenServiceFactory::init() {
    listen_services_.emplace("SOCK_STREAM", []() -> ListenServicePtr {
            return std::static_pointer_cast<ListenService>(
                std::make_shared<TCPListenService>());
        });
    listen_services_.emplace("SOCK_DGRAM", []() -> ListenServicePtr {
            return std::static_pointer_cast<ListenService>(
                std::make_shared<UDPListenService>());
        });
}

ListenServicePtr ListenServiceFactory::create_service(
  const std::string& service_type) const {
    if (listen_services_.find(service_type) != listen_services_.end())
        return listen_services_.at(service_type)();
    return nullptr;
}
