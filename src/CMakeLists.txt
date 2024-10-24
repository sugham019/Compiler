
add_subdirectory(StandardLibrary)

set(this
    src
)

find_package(LLVM REQUIRED CONFIG)

set(Sources
    Tokenizer.cpp
    Parser.cpp
    Analyzer.cpp
    ErrorHandler.cpp
    SymbolTableHandler.cpp
    Compiler.cpp
    IRGenerator.cpp
) 

llvm_map_components_to_libnames(LLVM_LIBS support core irreader)


add_library(${this} STATIC ${Sources})
target_include_directories(${this} PUBLIC ${LLVM_INCLUDE_DIRS})
target_link_libraries(${this} PRIVATE ${LLVM_LIBS})


add_executable(Compiler main.cpp)
target_link_libraries(Compiler PRIVATE src)