cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DASIO_STANDALONE")

include(cmake/common.cmake)
include(cmake/options.cmake)
include(cmake/http_parser.cmake)
#include(cmake/uv.cmake)

add_subdirectory(deps/uv)
add_subdirectory(deps/websocketpp)

project(BlockChainServer)

if(MSVC)
  add_definitions(/W2)
else()
  add_definitions(-Wall -Wextra -Wno-unused-parameter)
  set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

include_directories(deps/asio/include)
include_directories(deps/uv/include)
include_directories(deps/http_parser)
include_directories(deps/hash-library)
include_directories(deps/websocketpp)
include_directories(.)

aux_source_directory(. SRC_LIST)
aux_source_directory(deps/hash-library SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} http_parser uv)
