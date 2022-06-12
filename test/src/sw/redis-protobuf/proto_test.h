/**************************************************************************
   Copyright (c) 2022 sewenew

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

#ifndef SEWENEW_REDISPROTOBUF_TEST_PROTO_TEST_H
#define SEWENEW_REDISPROTOBUF_TEST_PROTO_TEST_H

#include <sw/redis++/redis++.h>

namespace sw {

namespace redis {

namespace pb {

namespace test {

class ProtoTest {
public:
    ProtoTest(const std::string &name, sw::redis::Redis &r) :
        _name(name), _redis(r) {}

    virtual ~ProtoTest() = default;

    void run();

    const std::string& name() const {
        return _name;
    }

private:
    virtual void _run(sw::redis::Redis &r) = 0;

    std::string _name;

    sw::redis::Redis &_redis;
};

}

}

}

}

#endif // endif SEWENEW_REDISPROTOBUF_TEST_PROTO_TEST_H
