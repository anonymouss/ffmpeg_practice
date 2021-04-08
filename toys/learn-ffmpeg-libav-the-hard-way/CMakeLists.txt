cmake_minimum_required(VERSION 3.10)

project(learning_libav)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED true)

file(MAKE_DIRECTORY out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY out/)

set(FF_BUILD ../../ffmpeg_build)
list(APPEND FF_INCLUDE "${FF_BUILD}/include" "utils/include")
list(APPEND FF_LIBS "${FF_BUILD}/lib")
include_directories(${FF_INCLUDE})
link_directories(${FF_LIBS})

list(APPEND FF_SHARED_LIBS avcodec avformat avutil swscale)
list(APPEND UTILS_SOURCE "utils/ff_logging.cpp")

add_executable(00_hello_world 00_hello_world.cpp ${UTILS_SOURCE})
target_link_libraries(00_hello_world ${FF_SHARED_LIBS})

add_executable(02_remuxing 02_remuxing.cpp ${UTILS_SOURCE})
target_link_libraries(02_remuxing ${FF_SHARED_LIBS})
