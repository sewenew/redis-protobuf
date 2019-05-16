/**************************************************************************
   Copyright (c) 2019 sewenew

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 *************************************************************************/

#ifndef SEWENEW_REDISPROTOBUF_IO_UTILS_H
#define SEWENEW_REDISPROTOBUF_IO_UTILS_H

#include <string>
#include <vector>

namespace sw {

namespace redis {

namespace pb {

namespace io {

bool is_regular(const std::string &file);

bool is_directory(const std::string &file);

std::vector<std::string> list_dir(const std::string &path);

std::string extension(const std::string &file);

}

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_IO_UTILS_H
