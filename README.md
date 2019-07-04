# redis-protobuf

- [Overview](#overview)
    - Motivation(#motivation)
- [Installation](#installation)
    - [Run redis-protobuf With Docker](#run-redis-protobuf-with-docker)
    - [Install redis-protobuf With Source Code](#install-redis-protobuf-with-source-code)
    - [Load redis-protobuf](#load-redis-protobuf)
- [Getting Started](#getting-started)
    - [redis-cli](#redis-cli)
    - [C++ Client](#c-client)
    - [Python Client](#python-client)
- [Commands](#commands)
    - [Convention](#convention)
    - [PB.SET](#pb-set)
    - [PB.GET](#pb-get)
    - [PB.DEL](#pb-del)
    - [PB.APPEND](#pb-append)
    - [PB.LEN](#pb-len)
    - [PB.CLEAR](#pb-clear)
    - [PB.MERGE](#pb-merge)
    - [PB.TYPE](#pb-type)
    - [PB.SCHEMA](#pb-schema)
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

**TODO**

### Install redis-protobuf With Source Code

First of all, you should follow the [instruction](https://github.com/protocolbuffers/protobuf/tree/master/src) to install Protobuf-C++.

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

## Getting Started

After [loading the module](#load-redis-protobuf), you can use any Redis client to send *redis-protobuf* [commands](#Commands).

We'll use the following *.proto* file as example, unless otherwise stated. In order to test examples in this doc, you need to put the following *.proto* file in the *proto-directory*.

```
syntax = "proto3";

message SubMsg {
    string s = 1;
    int32 i = 2;
}

message Msg {
    int32 i = 1;
    SubMsg m = 2;
    repeated int32 a = 3;
    repeated SubMsg ms = 4;
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

**NOTE**: As we mentioned before, Protobuf is NOT human friendly. So *redis-protobuf* also supports setting JSON string as value, and the module will convert the JSON string to Protobuf message automatically. Check the [C++ client section](c-client-section) to see an example of setting binary string as value.

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

### Convention

Most *redis-protobuf* commands that write the message has the following pattern:

```
PB.COMMAND key [options] path value
```

While most commands that read the message has another pattern:

```
PB.COMMAND key [options] path
```

### PB.SET

```
PB.SET key [--NX | --XX] [--EX seconds | --PX milliseconds] path value
```

#### Return Value

#### Error

#### Examples

### PB.GET

### PB.DEL

### PB.APPEND

### PB.LEN

### PB.CLEAR

### PB.MERGE

### PB.TYPE

### PB.SCHEMA

## Author

*redis-protobuf* is written by [sewenew](https://github.com/sewenew), who is also active on [StackOverflow](https://stackoverflow.com/users/5384363/for-stack).
