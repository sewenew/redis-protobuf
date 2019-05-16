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

#include "io_utils.h"
#include <sys/stat.h>
#include <dirent.h>
#include "errors.h"

namespace {

mode_t file_type(const std::string &file);

}

namespace sw {

namespace redis {

namespace pb {

namespace io {

bool is_regular(const std::string &file) {
    return S_ISREG(file_type(file));
}

bool is_directory(const std::string &file) {
    return S_ISDIR(file_type(file));
}

std::vector<std::string> list_dir(const std::string &path) {
    if (!is_directory(path)) {
        throw Error(path + " is not a directory");
    }

    auto *dir = opendir(path.c_str());
    if (dir == nullptr) {
        throw Error("failed to open directory: " + path);
    }

    std::vector<std::string> files;
    dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;

        // Skip "." and ".."
        if (name == "." || name == "..") {
            continue;
        }

        files.push_back(name);
    }

    closedir(dir);

    return files;
}

std::string extension(const std::string &file) {
    auto pos = file.rfind(".");
    if (pos == std::string::npos) {
        return {};
    }

    return file.substr(pos + 1);
}

}

}

}

}

namespace {

mode_t file_type(const std::string &file) {
    struct stat buf;
    if (stat(file.c_str(), &buf) < 0) {
        throw sw::redis::pb::Error("failed to get file status: " + file);
    }

    return buf.st_mode;
}

}
