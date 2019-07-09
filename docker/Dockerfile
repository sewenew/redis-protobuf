From redis:latest

ENV LIBDIR /usr/lib/redis/modules
ENV DEPS "make g++ curl cmake"

# Install dependencies
RUN set -ex;\
	deps="$DEPS";\
        apt-get update;\
        apt-get install -y --no-install-recommends $deps;

# Install protobuf
RUN set -ex;\
        mkdir -p /usr/src;\
        cd /usr/src;\
        curl -L -k https://github.com/protocolbuffers/protobuf/releases/download/v3.8.0/protobuf-cpp-3.8.0.tar.gz -o protobuf-cpp-3.8.0.tar.gz;\
        tar xfz protobuf-cpp-3.8.0.tar.gz;\
        cd protobuf-3.8.0;\
        ./configure "CFLAGS=-fPIC" "CXXFLAGS=-fPIC" --prefix=/usr;\
        make -j 4;\
        make install;

# Build redis-protobuf
RUN set -ex;\
        cd /usr/src;\
        curl -L -k 'https://github.com/sewenew/redis-protobuf/archive/0.0.1.tar.gz' -o redis-protobuf-0.0.1.tar.gz;\
        tar xfz redis-protobuf-0.0.1.tar.gz;\
        cd redis-protobuf-0.0.1;\
        mkdir compile;\
        cd compile;\
        cmake -DCMAKE_BUILD_TYPE=Release ..;\
        make;

# Load redis-protobuf
ENV REDISDIR /usr/lib/redis
RUN set -ex;\
        mkdir -p "$REDISDIR/proto" "$REDISDIR/modules" "$REDISDIR/conf";\
        cp /usr/src/redis-protobuf-0.0.1/docker/example.proto "$REDISDIR/proto";\
        cp /usr/src/redis-protobuf-0.0.1/compile/libredis-protobuf.so "$REDISDIR/modules";\
        echo 'loadmodule /usr/lib/redis/modules/libredis-protobuf.so --dir /usr/lib/redis/proto' > "$REDISDIR/conf/redis.conf";

# Cleanup
RUN set -ex;\
	deps="$DEPS";\
	apt-get purge -y --auto-remove $deps;\
	rm -rf /usr/src/*;

CMD ["redis-server", "/usr/lib/redis/conf/redis.conf"]
