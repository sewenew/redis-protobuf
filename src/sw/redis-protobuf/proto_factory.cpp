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

#include "proto_factory.h"
#include "io_utils.h"
#include "errors.h"

namespace sw {

namespace redis {

namespace pb {

void FactoryErrorCollector::_add_error(const std::string &type,
                                        const std::string &filename,
                                        int line,
                                        int column,
                                        const std::string &message) {
    auto err = type + ":" + filename + ":"
                + std::to_string(line) + ":"
                + std::to_string(column) + ":"
                + message;

    _errors.push_back(std::move(err));
}

std::string FactoryErrorCollector::last_errors() const {
    std::string err_str;
    for (const auto &err : _errors) {
        if (!err_str.empty()) {
            err_str += "\n";
        }

        err_str += err;
    }

    return err_str;
}

ProtoFactory::ProtoFactory(const std::string &proto_dir) :
                            _proto_dir(proto_dir),
                            _importer(&_source_tree, &_error_collector) {
    _source_tree.MapPath("", _proto_dir);

    _load_protos(_proto_dir);
}

MsgUPtr ProtoFactory::create(const std::string &type) {
    const auto *desc = descriptor(type);
    if (desc == nullptr) {
        return nullptr;
    }

    const auto *prototype = _factory.GetPrototype(desc);

    assert(prototype != nullptr);

    return MsgUPtr(prototype->New());
}

const gpb::Descriptor* ProtoFactory::descriptor(const std::string &type) {
    return _importer.pool()->FindMessageTypeByName(type);
}

void ProtoFactory::_load_protos(const std::string &proto_dir) {
    auto files = io::list_dir(proto_dir);
    for (const auto &file : files) {
        if (!io::is_regular(proto_dir + "/" + file) || io::extension(file) != "proto") {
            continue;
        }

        _load(file);
    }
}

void ProtoFactory::_load(const std::string &file) {
    // Clear last errors.
    _error_collector.clear();

    auto *desc = _importer.Import(file);
    if (desc == nullptr || _error_collector.has_error()) {
        throw Error("failed to load " + file + "\n" + _error_collector.last_errors());
    }
}

}

}

}
