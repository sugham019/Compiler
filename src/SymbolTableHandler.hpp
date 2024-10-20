#pragma once
#include "AST.hpp"
#include "SymbolTable.hpp"
#include "Token.hpp"
#include <list>
#include <vector>

class SymbolTableHandler{

public:
    void updateSymbolTable(ast::Function& function);
    void updateSymbolTable(ast::DeclarativeStatement& declarativeStatement);
    void updateSymbolTable(Keyword dataType, const std::string_view& identifier, bool isInitialized, bool isConst);
    void updateSymbolTable(const std::string_view& functionName, std::list<ast::Parameter>* functionParams);
    std::pair<bool, SymbolTableEntry> findFunctionSymbol(const std::string_view& identifier);
    std::pair<bool, SymbolTableEntry> findVariableSymbol(const std::string_view& identifier);
   

    void createSymbolTable(){
        m_symbolTableList.emplace_back();
    }

    void popSymbolTabe(){
        m_symbolTableList.pop_back();
    }

    static SymbolTable standardLibFuncSymbols;

private:
    bool variableSymbolExists(const std::string_view& identifier);
    bool functionSymbolExists(const std::string_view& identifier);
    std::list<SymbolTable> m_symbolTableList;
};