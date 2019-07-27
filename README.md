# redis-protobuf

- [Overview](#overview)
    - [Motivation](#motivation)
- [Installation](#installation)
    - [Run redis-protobuf With Docker](#run-redis-protobuf-with-docker)
    - [Install redis-protobuf With Source Code](#install-redis-protobuf-with-source-code)
    - [Load redis-protobuf](#load-redis-protobuf)
- [Getting Started](#getting-started)
    - [redis-cli](#redis-cli)
    - [C++ Client](#c-client)
    - [Python Client](#python-client)
- [Commands](#commands)
    - [Path](#path)
    - [PB.SET](#pbset)
    - [PB.GET](#pbget)
    - [PB.DEL](#pbdel)
    - [PB.APPEND](#pbappend)
    - [PB.LEN](#pblen)
    - [PB.CLEAR](#pbclear)
    - [PB.MERGE](#pbmerge)
    - [PB.TYPE](#pbtype)
    - [PB.SCHEMA](#pbschema)
- [Author](#author)

## Overview

This is a [Redis Module](https://redis.io/topics/modules-intro) for reading and writing protobuf messages (only support Protobuf version 3, i.e. `syntax="proto3";`).

**NOTE**: In order to use this module, you should read the [Protobuf doc](https://developers.google.com/protocol-buffers) to learn how to define a protobuf message.

**NOTE**: I'm not a native speaker. So if the documentation is unclear, please feel free to open an issue or pull request. I'll response ASAP.

### Motivation

Redis supports multiple data structures, such as string, list, and hash. In order to keep things simple, Redis doesn't support nested data structures, e.g. you cannot have a list whose element is a list. However, sometimes nested data structures are useful, and [RedisJSON](https://github.com/RedisJSON/RedisJSON) is an option. With *RedisJSON*, you can save JSON object into Redis, and read and write JSON fields with that module. Since JSON can be nested, you can define your own nested data structure.

In fact, *redis-protobuf* is inspired by *RedisJSON*.

Both JSON and Protobuf are methods for serializing structured data. You can find many articles comparing these methods. Basically, Protobuf is faster and smaller than JSON, while JSON is more human friendly, since the former is binary while the latter is text. Also, you can convert a Protobuf to JSON and vice versa.

In order to make the nested data structure fast and memory efficient, I wrote this *redis-protobuf* module. With this module, you can define a Protobuf message, and *redis-protobuf* will use reflection to read and write message fields.

## Installation

### Run redis-protobuf With Docker

Run the following command to start *redis-protobuf* with Docker.

```
docker run -p 6379:6379 sewenew/redis-protobuf:latest
```

In this case, Docker runs Redis with a *redis.conf* file located at */usr/lib/redis/conf/redis.conf*. Also, you should put your *.proto* files in */usr/lib/redis/proto* directory. However, by default, the Docker image ships with an [example.proto](https://github.com/sewenew/redis-protobuf/blob/master/docker/example.proto) file, so that you can run the following examples without creating extra *.proto* files.

After running the Docker image, you can go to the [Getting Started section](#getting-started) to see how to run *redis-protobuf* commands.

### Install redis-protobuf With Source Code

You can also install redis-protobuf with source code.

#### Install Protobuf

First of all, you need to install Protobuf-C++. However, [the offical release](https://github.com/sewenew/redis-protobuf) doesn't expose reflection API for fields of map type (see [this issue](https://github.com/protocolbuffers/protobuf/issues/1322) for detail). So I modified the code to expose some internal interfaces. I published the modified code with *redis-protobuf* release, and you can download it from [here](https://github.com/sewenew/redis-protobuf/releases/download/0.0.1/protobuf-3.8.0-map-reflection.tar.gz). Also, you can check the *CHANGES-BY-SEWENEW.md* file for detail on what I've modifed.

Install modified Protobuf with the following commands:

```
curl -L -k https://github.com/sewenew/redis-protobuf/releases/download/0.0.1/protobuf-3.8.0-map-reflection.tar.gz -o protobuf-3.8.0-map-reflection.tar.gz

tar xfz protobuf-3.8.0-map-reflection.tar.gz

cd protobuf-3.8.0-map-reflection

./configure "CFLAGS=-fPIC" "CXXFLAGS=-fPIC"

make -j 4

make install
```

If you want to install Protobuf at a non-default location, you can specify the `--prefix=/path/to/install/location` option.

```
./configure "CFLAGS=-fPIC" "CXXFLAGS=-fPIC" --prefix=/usr
```

**NOTE**: You must specify the `"CFLAGS=-fPIC" "CXXFLAGS=-fPIC"` options when compiling Protobuf.

#### Install redis-protobuf

*redis-protobuf* is built with [CMAKE](https://cmake.org).

```
git clone https://github.com/sewenew/redis-protobuf.git

cd redis-protobuf

mkdir compile

cd compile

cmake -DCMAKE_BUILD_TYPE=Release ..

make
```

If Protobuf is installed at non-default location, you should use `CMAKE_PREFIX_PATH` to specify the installation path of Protobuf.

```
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/Protobuf ..
```

When `make` is done, you should find *libredis-protobuf.so* (or *libredis-protobuf.dylib* on MacOS) under the *redis-protobuf/compile* directory.

### Load redis-protobuf

Redis Module is supported since Redis 4.0, so you must install Redis 4.0 or above.

In order to load *redis-protobuf*, you need to modify the *redis.conf* file to add the `loadmodule` directive:

```
loadmodule /path/to/libredis-protobuf.so --dir proto-directory
```

*proto-directory* is the directory where your *.proto* files located. You must ensure that the directory exists and put your *.proto* files in this directory, so that *redis-protobuf* can load these *.proto* files dynamically.

Now, you can start your Redis instance:

```
redis-server /path/to/redis.conf
```

If Redis loads the module successfully, you can get the following message from the log:

```
Module 'PB' loaded from /path/to/libredis-protobuf.so
```

**NOTE**: If any of the given *.proto* file is invalid, Redis fails to load the module.

#### redis-protobuf Options

## Getting Started

After [loading the module](#load-redis-protobuf), you can use any Redis client to send *redis-protobuf* [commands](#Commands).

We'll use the following *.proto* file as example, unless otherwise stated. In order to test examples in this doc, you need to put the following *.proto* file in the *proto-directory*.

**NOTE**: The [Docker image](https://cloud.docker.com/repository/docker/sewenew/redis-protobuf) also ships with this *.proto* file. So you can run the following examples with Docker too.

```
syntax = "proto3";

message SubMsg {
    string s = 1;
    int32 i = 2;
}

message Msg {
    int32 i = 1;
    SubMsg sub = 2;
    repeated int32 arr = 3;
}
```

As we mentioned before, *redis-protobuf* only supports Protobuf version 3. So you must put `syntax = "proto3";` at the beginning of your *.proto* file.

### redis-cli

The following examples use the offical Redis client, i.e. *redis-cli*, to send *redis-protobuf* commands.

List module info:

```
127.0.0.1:6379> MODULE LIST
1) 1) "name"
   2) "PB"
   3) "ver"
   4) (integer) 0
```

Set message:

```
127.0.0.1:6379> PB.SET key Msg '{"i" : 1, "sub" : {"s" : "string", "i" : 2}, "arr" : [1, 2, 3]}'
(integer) 1
```

**NOTE**: As we mentioned before, Protobuf is not human friendly. So *redis-protobuf* also supports setting JSON string as value, and the module will convert the JSON string to Protobuf message automatically. Check the [C++ client section](c-client) to see an example of setting binary string as value.

Get message:

```
127.0.0.1:6379> PB.GET key --FORMAT JSON Msg
"{\"i\":1,\"sub\":{\"s\":\"string\",\"i\":2},\"arr\":[1,2,3]}"
```

Set fields:

```
127.0.0.1:6379> PB.SET key Msg.i 10
(integer) 1
127.0.0.1:6379> PB.SET key Msg.sub.s redis-protobuf
(integer) 1
127.0.0.1:6379> PB.SET key Msg.arr[0] 2
(integer) 1
```

Get fields:

```
127.0.0.1:6379> PB.GET key Msg.i
(integer) 10
127.0.0.1:6379> PB.GET key Msg.sub.s
"redis-protobuf"
127.0.0.1:6379> PB.GET key Msg.arr[0]
(integer) 2
127.0.0.1:6379> PB.GET key --FORMAT JSON Msg.sub
"{\"s\":\"redis-protobuf\",\"i\":2}"
```

Delete message:

```
127.0.0.1:6379> PB.DEL key Msg
(integer) 1
```

### C++ Client

If you are using C++, you can use [redis-plus-plus](https://github.com/sewenew/redis-plus-plus) to send *redis-protobuf* commands:

```C++
try {
    auto redis = Redis("tcp://127.0.0.1");

    // Set Protobuf message.
    Msg msg;
    msg.set_i(1);
    auto *sub = msg.mutable_sub();
    sub->set_s("string");
    sub->set_i(2);
    msg.add_arr(1);
    msg.add_arr(2);
    msg.add_arr(3);

    // Serialize Protobuf message.
    std::string s;
    if (!msg.SerializeToString(&s)) {
        throw Error("failed to serialize protobuf message");
    }

    // Set value with the serialized message.
    redis.command("PB.SET", "key", "Msg", s);

    // Get the message in binary format.
    s = redis.command<std::string>("PB.GET", "key", "--FORMAT", "BINARY", "Msg");

    // Create a new message with the binary string.
    Msg msg_new;
    if (!msg_new.ParseFromString(s)) {
        throw Error("failed to parse string to protobuf message");
    }

    // Set and get a message field.
    redis.command("PB.SET", "key", "Msg.i", 10);
    assert(redis.command<long long>("PB.GET", "key", "Msg.i") == 10);

    // Set and get a nested message field.
    redis.command("PB.SET", "key", "Msg.sub.s", "redis-protobuf");
    assert(redis.command<std::string>("PB.GET", "key", "Msg.sub.s") == "redis-protobuf");

    // Delete the message.
    redis.command("PB.DEL", "key", "Msg");
} catch (const Error &e) {
    // Error handling
}
```

### Python Client

If you are using Python, you can use [redis-py](https://github.com/andymccurdy/redis-py) to send *redis-protobuf* commands:

```
>>> import redis
>>> r = redis.StrictRedis(host='localhost', port=6379, db=0)
>>> r.execute_command('PB.SET', 'key', 'Msg', '{"i" : 1, "sub" : {"s" : "string", "i" : 2}, "arr" : [1, 2, 3]}')
1L
>>> r.execute_command('PB.GET', 'key', 'Msg')
'\x08\x01\x12\n\n\x06string\x10\x02\x1a\x03\x01\x02\x03'
>>> r.execute_command('PB.GET', 'key', '--FORMAT', 'JSON', 'Msg')
'{"i":1,"sub":{"s":"string","i":2},"arr":[1,2,3]}'
>>> r.execute_command('PB.SET', 'key', 'Msg.i', 2)
1L
>>> r.execute_command('PB.GET', 'key', 'Msg.i')
2L
>>> r.execute_command('PB.SET', 'key', 'Msg.sub.s', 'redis-protobuf')
1L
>>> r.execute_command('PB.GET', 'key', 'Msg.sub.s')
'redis-protobuf'
>>> r.execute_command('PB.SET', 'key', 'Msg.arr[0]', 100)
1L
>>> r.execute_command('PB.GET', 'key', 'Msg.arr[0]')
100L
```

## Commands

### Path

Most commands have a *path* argument, which specifies the message type or field. *path* is consist of package name (optional), message type (required), field name (optional), array index (optional) and map key (optional). I'll use the following *.proto* file as an example to show you how to specify a *path*.

**NOTE**: *path* IS CASE SENSITIVE.

```
syntax = "proto3";

package redis.pb;

message SubMsg {
    string s = 1;
}

message Msg {
    int32 i = 1;
    SubMsg sub = 2;
    repeated string str_arr = 3;
    repeated SubMsg msg_arr = 4;
    map<string, SubMsg> m = 5;
};
```

If you specify package name in the *.proto* definition, e.g. `package redis.pb`, you must also specify the pacakge name in the path, and separate each name with double colons, i.e. `::`. For example, we can use the following to specify the type the message:

```
redis::pb::Msg
```

However, if you don't specify package name in the *.proto* definition, you can use the message type directly without any prefix.

You can use a dot, i.e. `.`, to specify the field of a message. If the specified field is of message type, you can use another dot to specify the nested field:

```
redis::pb::Msg.i

redis::pb.Msg.sub.s
```

If the field is an array, you can use the square bracket and an index, i.e. `[i]`, to specify the ith element. If the element is of message type, you can use a dot to specify the field of the element:

```
redis::pb::Msg.str_arr[2]

redis::pb::Msg.msg_arr[1].s
```

The index is 0-based, and if the index is out-of-range, *redis-protobuf* will reply with an error.

If the field is a map, you can use the square bracket and a key, i.e. `[key]`, to specify the corresponding value. If the value of message type, again, you can use a dot to specify the field of the value:

```
redis::pb::Msg.m[key]

redis::pb::Msg.m[key].s
```

### PB.SET

#### Syntax

```
PB.SET key [--NX | --XX] [--EX seconds | --PX milliseconds] path value
```

- If the *path* specifies the message type, set the whole message with the given *value*.
- If the *path* specifies a field, set the corresponding field with the given *value*.

**NOTE**: Since Protobuf fields are optional, when setting the whole message, you can only set parts of fields of this message. The following example only sets `Msg.i`, and leave other fields unset.

```
127.0.0.1:6379> PB.SET key Msg '{"i" : 1}'
(integer) 1
```

Protobuf message has pre-defined schema, so the *value* should match the type of the corresponding field.

- If the field is of integer type, i.e. `int32`, `int64`, `uint32`, `uint64`, `sint32`, `sint64`, `fixed32`, `fixed64`, `sfixed32`, `sfixed64`, the *value* string should be converted to the corresponding integer.
- If the field is of floating-point type, i.e. `float`, `double`, the *value* string should be converted to the corresponding floating-point number.
- If the field is of boolean type, i.e. `bool`, the *value* string should be *true* or *false*, or an integer (`0` is `false`, none `0` is `true`).
- If the field is of string type, i.e. `string`, `byte`, the *value* string can be any binary string.
- If the field is of enum type, i.e. `enum`, the *value* string should be converted to an integer.
- If the field is of message type, the *value* string should be a binary string that serialized from the corresponding Protobuf message, or a JSON string that can be converted to the corresponding Protobuf message.

#### Options

- **--NX**: Only set the key if it doesn't exist.
- **--XX**: Only set the key if it already exists.
- **--EX seconds**: Set the key with the specified expiration in seconds.
- **--PX milliseconds**: Set the key with the specified expiration in milliseconds.

#### Return Value

Integer reply: 1 if set successfully. 0, otherwise, e.g. option *--NX* has been set, while the key already exists.

#### Error

Return an error reply in the following cases:

- *path* specifies a field, and the field doesn't exist.
- *path* specifies a message type, and the type doesn't match the type of the message saved in *key*, i.e. try to overwrite a *key*, in which the message is of a different type. See the examples part for an example.
- *value* doesn't match the type of the field.

#### Examples

```
127.0.0.1:6379> PB.SET key Msg '{"i" : 1, "sub" : {"s" : "string", "i" : 2}, "arr" : [1, 2, 3]}'
(integer) 1
127.0.0.1:6379> PB.SET key Msg.i 10
(integer) 1
127.0.0.1:6379> PB.SET key Msg.sub.s redis-protobuf
(integer) 1
127.0.0.1:6379> PB.SET key Msg.arr[0] 2
(integer) 1
127.0.0.1:6379> PB.SET key SubMsg '{"s" : "hello"}'
(error) ERR type mismatch
```

### PB.GET

#### Syntax

```
PB.GET key [--FORMAT BINARY|JSON] path
```

- If *path* specifies a field, return the value of that field.
- If *path* specifies a message type, return the whole message in *key*.

**NOTE**: Even if you want to get the whole Protobuf message, you need to specify the *path* as the type of the message. If type mismatches, you'll get an error reply.

#### Options

- **--FORMAT**: If the field at *path* is of message type, this option specifies the format of the return value. If the field is of other types, this option is ignored.
    - **BINARY**: return the value as a binary string by serializing the Protobuf message. This is the default format.
    - **JSON**: return the value as a JSON string by converting the Protobuf message to JSON.

#### Return Value

Reply type is depends on the type of the field at *path*.

- Integer reply: if the field is of integer or enum type.
- Bulk string reply: if the field is of string or message type.
- Simple string reply: if the field is of boolean or floating-point type.
- Array reply: if the field is repeated.
- Nil reply: if *key* doesn't exist.

#### Error

Return an error reply in the following cases:

- If *path* specifies a field, and the field doesn't exist.
- If *path* specifies a message type, and the type doesn't match the type of the message saved in *key*.

#### Examples

```
127.0.0.1:6379> PB.GET key Msg
"\b\n\x12\x12\n\x0eredis-protobuf\x10\x02\x1a\x03\x02\x02\x03"
127.0.0.1:6379> PB.GET key --FORMAT JSON Msg
"{\"i\":10,\"sub\":{\"s\":\"redis-protobuf\",\"i\":2},\"arr\":[2,2,3]}"
127.0.0.1:6379> PB.GET key Msg.i
(integer) 10
127.0.0.1:6379> PB.GET key --FORMAT JSON Msg.sub
"{\"s\":\"redis-protobuf\",\"i\":2}"
127.0.0.1:6379> PB.GET key Msg.arr[0]
(integer) 2
127.0.0.1:6379> PB.GET key Msg.arr
1) (integer) 1
2) (integer) 2
3) (integer) 3
```

### PB.DEL

#### Syntax

```
PB.DEL key path
```

- If *path* specifies an array element, e.g. `Msg.arr[0]`, delete the corresponding element from the array.
- If *path* specifies a map value, e.g. `Msg.m[key]`, delete the corresponding key-value pair from the map.
- If *path* specifies a message type, delete the key.

#### Return Value

Integer reply: 1 if *key* exists, 0 othewise.

#### Error

Return an error reply in the following cases:

- *path* specifies a field, and the field doesn't exist
- The field is not an array element or a map value.
- *path* specifies a message type, and the type doesn't match the type of the message saved in *key*.

#### Examples

```
127.0.0.1:6379> PB.DEL key Msg.arr[0]
(integer) 1
127.0.0.1:6379> PB.DEL key Msg
(integer) 1
```

### PB.APPEND

#### Syntax

```
PB.APPEND key path value
```

- If the field at *path* is a string, append *value* string to the field.
- If the field at *path* is an array, append the *value* as an element to the array.

If *key* doesn't exist, create an empty message, and do the append operation to the new message.

#### Return Value

Integer reply: The length of the string or the size of the array after the append operation.

#### Error

Return an error reply in the following cases:

- *path* doesn't exist.
- The field at *path* is not a string or array.

#### Examples

```
127.0.0.1:6379> pb.append key Msg.sub.s WithTail
(integer) 14
127.0.0.1:6379> pb.append key Msg.arr 4
(integer) 4
```

### PB.LEN

#### Syntax

```
PB.LEN key path
```

- If the field at *path* is a string, return the length of the string.
- If the field at *path* is an array, return the size of the array.
- If the field at *path* is a map, return the size of the map.
- If the field at *path* is a message, return the length of the serialized binary string of the message.

#### Return Value

Integer reply: 0 if *key* doesn't exist. Otherwise, the length of the string/array/map/message.

#### Error

Return an error reply in the following cases:

- *path* doesn't exist.
- The field at *path* is not a string/array/map/message.

#### Examples

```
127.0.0.1:6379> PB.LEN key Msg
(integer) 28
127.0.0.1:6379> PB.LEN key Msg.sub.s
(integer) 14
127.0.0.1:6379> PB.LEN key Msg.arr
(integer) 4
```

### PB.CLEAR

#### Syntax

```
PB.CLEAR key path
```

- If *path* specifies a message type, clear the message in *key*.
- If *path* specifies a field, clear the field.

Please check the Protubuf doc for the definition of **clear**.

#### Return Value

Integer reply: 1 if the *key* exists, 0 otherwise.

#### Error

Return an error reply in the following cases:

- *path* specifies a message type, and the type doesn't match the type of the message saved in *key*.
- *path* doesn't exist.

#### Examples

```
127.0.0.1:6379> PB.CLEAR key i
(integer) 1
127.0.0.1:6379> PB.CLEAR key Msg.arr
(integer) 1
127.0.0.1:6379> PB.CLEAR non-exist-key Msg
(integer) 0
```

### PB.MERGE

#### Syntax

```
PB.MERGE key path value
```

- If *path* specifies a message type, parse *value* to a message, and merge it into the message in *key*.
- If *path* specifies a field, merge the *value* into the field.
- If *key* doesn't exist, this command behaves *PB.SET*.

Please check the Protubuf doc for the definition of **merge**.

#### Return Value

Integer reply: 1 if the *key* exists, 0 otherwise.

#### Error

Return an error reply in the following cases:

- *path* specifies a message type, and the type doesn't match the type of the message saved in *key*.
- *path* doesn't exist.

#### Examples

```
```

### PB.TYPE

#### Syntax

```
PB.TYPE key
```

Get the message type of message in *key*.

#### Return Value

- Simple string reply: The Protobuf message type, if *key* exists.
- Nil reply: If *key* doesn't exist.

#### Error

Return an error reply in the following cases:

#### Examples

```
127.0.0.1:6379> PB.TYPE key
Msg
```

### PB.SCHEMA

#### Syntax

```
PB.SCHEMA type
```

Get the schema of the given Protobuf message *type*.

#### Return Value

- Bulk string reply: The schema of the given *type*.
- Nil reply: If the *type* doesn't exist.

#### Error

Return an error reply in the following cases:

#### Examples

```
127.0.0.1:6379> PB.SCHEMA Msg
"message Msg {\n  int32 i = 1;\n  SubMsg sub = 2;\n  repeated int32 arr = 3;\n}\n"
```

## Author

*redis-protobuf* is written by [sewenew](https://github.com/sewenew), who is also active on [StackOverflow](https://stackoverflow.com/users/5384363/for-stack).
