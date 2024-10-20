cmake_minimum_required(VERSION 3.8.0)

project(Compiler VERSION 1.0.0 LANGUAGES CXX C ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(Headers
        ${CMAKE_SOURCE_DIR}/extra/
)
include_directories(${Headers})


add_subdirectory(src)
add_subdirectory(tests)


include(CTest)
enable_testing()