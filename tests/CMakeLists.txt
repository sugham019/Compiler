cmake_minimum_required(VERSION 3.8.0)

set(This tests)

find_package(GTest REQUIRED)

set(Sources 
    # TokenizerTest.cpp
    # ParserTest.cpp
    # AnalyzerTest.cpp
    # IRGeneratorTest.cpp
    CompilerTest.cpp
)

set(Headers
    ../src/
)         

set(Libraries
    src
    GTest::GTest
    GTest::gtest_main
    ${LLVM_LIBRARIES}
)


add_executable(${This} ${Sources})

add_compile_options(-g)

target_include_directories(${This} PRIVATE ${Headers})

target_link_libraries(${This} PRIVATE ${Libraries})


add_test(
    NAME ${This}
    COMMAND ${This}
)