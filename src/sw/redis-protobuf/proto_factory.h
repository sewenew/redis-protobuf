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

#ifndef SEWENEW_REDISPROTOBUF_PROTO_FACTORY_H
#define SEWENEW_REDISPROTOBUF_PROTO_FACTORY_H

#include <string>
#include <memory>
#include <google/protobuf/message.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

namespace sw {

namespace redis {

namespace pb {

namespace gpb = google::protobuf;

class FactoryErrorCollector : public gpb::compiler::MultiFileErrorCollector {
public:
    virtual void AddError(const std::string &file_name,
                            int line,
                            int column,
                            const std::string &message) override {
        _add_error("error", file_name, line, column, message);
    }

    virtual void AddWarning(const std::string &file_name,
                            int line,
                            int column,
                            const std::string &message) override {
        _add_error("warning", file_name, line, column, message);
    }

    std::string last_errors() const;

    bool has_error() const {
        return !_errors.empty();
    }

    void clear() {
        _errors.clear();
    }

private:
    void _add_error(const std::string &type,
                    const std::string &filename,
                    int line,
                    int column,
                    const std::string &message);

    std::vector<std::string> _errors;
};

using MsgUPtr = std::unique_ptr<gpb::Message>;

class ProtoFactory {
public:
    explicit ProtoFactory(const std::string &proto_dir);

    ProtoFactory(const ProtoFactory &) = delete;
    ProtoFactory& operator=(const ProtoFactory &) = delete;

    ProtoFactory(ProtoFactory &&) = delete;
    ProtoFactory& operator=(ProtoFactory &&) = delete;

    ~ProtoFactory() = default;

    MsgUPtr create(const std::string &type);

    const gpb::Descriptor* descriptor(const std::string &type);

private:
    void _load_protos(const std::string &proto_dir);

    void _load(const std::string &file);

    // Dir where .proto file are saved.
    std::string _proto_dir;

    gpb::compiler::DiskSourceTree _source_tree;

    FactoryErrorCollector _error_collector;

    gpb::compiler::Importer _importer;

    gpb::DynamicMessageFactory _factory;
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_PROTO_FACTORY_H
