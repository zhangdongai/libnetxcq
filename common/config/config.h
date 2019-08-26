#pragma once

#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"

namespace config {

bool getproto_fromfile(const char* file_path,
                       google::protobuf::Message* msg);

}
