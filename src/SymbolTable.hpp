#pragma once
#include <string>
#include <sys/types.h>
#include "Token.hpp"
#include <vector>
#include "AST.hpp"
#include <unordered_map>

enum class SymbolType: uint8_t{
    FUNCTION,
    VARIABLE
};

struct SymbolTableEntry{

    SymbolType symbolType;
    Keyword dataType;
    bool isInitialized;
    bool isConst;
    bool isArray;
    std::vector<Keyword> paramTypes;
};


using SymbolTable = std::unordered_map<std::string_view, SymbolTableEntry>;
