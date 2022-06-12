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

#include "del_test.h"
#include "utils.h"

namespace sw {

namespace redis {

namespace pb {

namespace test {

void DelTest::_run(sw::redis::Redis &r) {
    auto key = test_key("del");

    KeyDeleter deleter(r, key);

    REDIS_ASSERT(r.command<long long>("PB.SET", key, "Msg",
                R"({"i" : 1, "arr" : [1, 2], "m" : {"key" : "val"}})") == 1,
            "failed to test pb.del command");

    REDIS_ASSERT(r.command<long long>("PB.DEL", key, "Msg", "/arr/0") == 1 &&
                r.command<long long>("PB.LEN", key, "Msg", "/arr") == 1,
            "failed to test del array element");
    /*
    TODO: support delete whole array and whole map, and map element
    REDIS_ASSERT(r.command<long long>("PB.DEL", key, "Msg", "/arr") == 1 &&
                r.command<long long>("PB.LEN", key, "Msg", "/arr") == 0,
            "failed to test del array");

    REDIS_ASSERT(r.command<long long>("PB.DEL", key, "Msg", "/m") == 1 &&
                r.command<long long>("PB.LEN", key, "Msg", "/m") == 0,
            "failed to test del map");
            */

    REDIS_ASSERT(r.command<long long>("PB.DEL", key, "Msg") == 1 &&
                r.exists(key) == 0,
            "failed to test del message");
}

}

}

}

}
