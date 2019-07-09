project(redis-protobuf)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    cmake_minimum_required(VERSION 3.0.0)
else()
    cmake_minimum_required(VERSION 2.8.0)
endif()

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -W -Werror -fPIC -Wno-unused-parameter")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/sw/redis-protobuf)

file(GLOB PROJECT_SOURCE_FILES "${PROJECT_SOURCE_DIR}/*.cpp" "${PROJECT_SOURCE_DIR}/*.cc")

set(SHARED_LIB shared)

add_library(${SHARED_LIB} SHARED ${PROJECT_SOURCE_FILES})

# protobuf dependency
find_path(PROTOBUF_HEADER google)
target_include_directories(${SHARED_LIB} PUBLIC ${PROTOBUF_HEADER})

find_library(PROTOBUF_LIB libprotobuf.a)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${SHARED_LIB} -Wl,-force_load ${PROTOBUF_LIB})
    target_link_libraries(${SHARED_LIB} z)
else()
    target_link_libraries(${SHARED_LIB} -Wl,--whole-archive ${PROTOBUF_LIB} -Wl,--no-whole-archive)
endif()

set_target_properties(${SHARED_LIB} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

set_target_properties(${SHARED_LIB} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# Install shared lib.
install(TARGETS ${SHARED_LIB}
        LIBRARY DESTINATION lib)
