cmake_minimum_required(VERSION 3.12)

project(learning_video)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

file(MAKE_DIRECTORY out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY out/)

# set(FF_BUILD ../../ffmpeg_build)
list(APPEND INCLUDE_DIR "data_proc")
# list(APPEND FF_LIBS "${FF_BUILD}/lib")
include_directories(${INCLUDE_DIR})
# link_directories(${FF_LIBS})

# list(APPEND FF_SHARED_LIBS avcodec avformat avutil swscale)
# list(APPEND UTILS_SOURCE "utils/ff_logging.cpp")

add_executable(data_proc data_proc.cpp)
# target_link_libraries(00_hello_world ${FF_SHARED_LIBS})

