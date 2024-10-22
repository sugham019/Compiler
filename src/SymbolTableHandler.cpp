#include "SymbolTableHandler.hpp"
#include "AST.hpp"
#include "SymbolTable.hpp"
#include "Token.hpp"
#include <list>
#include "ErrorHandler.hpp"
#include <string_view>
#include <sys/types.h>
#include <utility>

namespace{

SymbolTableEntry createFunctionSymbol(Keyword returnType, std::vector<Keyword> paramTypes){
    return SymbolTableEntry{SymbolType::FUNCTION, returnType, false, false, false, paramTypes};
}

}

SymbolTable SymbolTableHandler::standardLibFuncSymbols = {
    {"printChar", createFunctionSymbol(Keyword::VOID, std::vector<Keyword>(1, Keyword::CHAR))},
    {"printlnChar", createFunctionSymbol(Keyword::VOID, std::vector<Keyword>(1, Keyword::CHAR))},
    {"printInt", createFunctionSymbol(Keyword::VOID, std::vector<Keyword>(1, Keyword::INT))},
    {"printlnInt", createFunctionSymbol(Keyword::VOID, std::vector<Keyword>(1, Keyword::INT))},
    {"getNextInt", createFunctionSymbol(Keyword::INT, std::vector<Keyword>())},
    {"getNextChar", createFunctionSymbol(Keyword::CHAR, std::vector<Keyword>())}
};

SymbolTableHandler::SymbolTableHandler(const ErrorHandler& errorHandler)
    : m_errorHandler(errorHandler){

}

void SymbolTableHandler::updateSymbolTable(ast::Function& function){
    SymbolTable& symbolTable = m_symbolTableList.front();
    Token& identifierToken = *function.m_identifier;
    const std::string_view identifier(identifierToken.m_value, identifierToken.m_valueSize);
    if(functionSymbolExists(identifier)){
        m_errorHandler.reportError(error::DUPLICATE_FUNC, identifierToken);
    }
    Keyword returnType = function.m_returnType->m_tokenType.keywordType;
    SymbolTableEntry entry = {SymbolType::FUNCTION, returnType,false, false,false};
    for(ast::Parameter& param: function.m_parameters){
        entry.paramTypes.push_back(param.m_dataType->m_tokenType.keywordType);
    }
    symbolTable.insert({identifier, entry});
}

void SymbolTableHandler::updateSymbolTable(Keyword dataType, const std::string_view& identifier, bool isInitialized, bool isConst){

    SymbolTable& symbolTable = m_symbolTableList.back();
    SymbolTableEntry entry = {SymbolType::VARIABLE, dataType, isInitialized, isConst, false};
    symbolTable.insert(std::make_pair(identifier, entry));
}

void SymbolTableHandler::updateSymbolTable(ast::DeclarativeStatement& declarativeStatement){
    Token& identifierToken = *declarativeStatement.m_identifier;
    const std::string_view identifier(identifierToken.m_value, identifierToken.m_valueSize);
    if(variableSymbolExists(identifier)){
        m_errorHandler.reportError(error::DUPLICATE_VAR, identifierToken);
    }
    bool isInitialized = (declarativeStatement.m_expression != nullptr) ? true : false;
    TokenType::KeywordType dataType = declarativeStatement.m_dataType->m_tokenType.keywordType;
    updateSymbolTable(dataType, identifier, isInitialized, declarativeStatement.m_isConst);
}

bool SymbolTableHandler::variableSymbolExists(const std::string_view& identifier){
    auto it = m_symbolTableList.begin();
    if(it != m_symbolTableList.end()) it++;

    for(auto i=it; i != m_symbolTableList.end(); i++){
        auto element = i->find(identifier);
        if(element != i->end()){
            return true;
        }
    }
    return false;
}

bool SymbolTableHandler::functionSymbolExists(const std::string_view& functionIdentifier){
    SymbolTable& symbolTable = m_symbolTableList.front();
    auto it = symbolTable.find(functionIdentifier);
    if(it != symbolTable.end()){
        return true;
    }
    auto it2 = standardLibFuncSymbols.find(functionIdentifier);
    return it2 != symbolTable.end();
}

std::pair<bool, SymbolTableEntry> SymbolTableHandler::findFunctionSymbol(const std::string_view& identifier){
    SymbolTable& topSymbolTable = m_symbolTableList.front();
    auto it = topSymbolTable.find(identifier);
    if(it == topSymbolTable.end()){
        auto it2 = standardLibFuncSymbols.find(identifier);
        if(it2 == standardLibFuncSymbols.end()){
            return std::make_pair(false, SymbolTableEntry());
        }
        return std::make_pair(true, it2->second);
    }
    return std::make_pair(true, it->second);
}

std::pair<bool, SymbolTableEntry> SymbolTableHandler::findVariableSymbol(const std::string_view& identifier){
    auto it = m_symbolTableList.begin();
    if(it != m_symbolTableList.end()) it++;

    for(auto i=it; i != m_symbolTableList.end(); i++){
        auto element = i->find(identifier);
        if(element != i->end()){
            return std::make_pair(true, element->second);
        }
    }
    return std::make_pair(false, SymbolTableEntry());
}
