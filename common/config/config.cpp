#include "common/config/config.h"

#include <iostream>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace config {

bool getproto_fromfile(const char* file_path,
                       google::protobuf::Message* msg) {
    int file_d = open(file_path, O_RDONLY);
    if (file_d < 0) {
        std::cout << "can not open " << file_path << std::endl;
        return false;
    }

    google::protobuf::io::ZeroCopyInputStream* input =
        new google::protobuf::io::FileInputStream(file_d);
    bool suc = google::protobuf::TextFormat::Parse(input, msg);
    if (!suc) {
        std::cout << "parse failed!" << std::endl;
    }

    delete input;
    close(file_d);

    return suc;
}

}
