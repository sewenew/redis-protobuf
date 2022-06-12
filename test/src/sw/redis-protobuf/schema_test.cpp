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

#include "schema_test.h"
#include "utils.h"

namespace sw {

namespace redis {

namespace pb {

namespace test {

void SchemaTest::_run(sw::redis::Redis &r) {
    auto schema = r.command<sw::redis::OptionalString>("PB.SCHEMA", "Msg");
    REDIS_ASSERT(schema && !schema->empty(), "failed to test pb.schema command");

    schema = r.command<sw::redis::OptionalString>("PB.SCHEMA",
            "sw.redis.pb.not-exist-Msg-type");
    REDIS_ASSERT(!schema, "failed to test pb.schema command");
}

}

}

}

}
