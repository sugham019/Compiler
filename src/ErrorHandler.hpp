#pragma once
#include "Token.hpp"
#include <fstream>
#include "AST.hpp"
#include <system_error>

class ErrorHandler{

public:
    ErrorHandler(std::ifstream& currentFile);
    void reportError(const std::string& errorMsg) const;
    void reportError(const std::string& erroMsg, const int lineNum) const;
    void reportError(const std::string& errorMsg, const Token& token) const;

private:
    std::ifstream& m_currentFile;
};

namespace error{
    constexpr const char* INVALID_RETURN = "Function return expression does not match function return type.";
    constexpr const char* FUCTION_NOT_FOUND = "Could not find function with given identifier.";
    constexpr const char* VARIABLE_NOT_FOUND = "Could not find variable with given identifier.";
    constexpr const char* ARGS_PARAM_ERROR = "Function call args do not match with function definition parameter.";
    constexpr const char* DUPLICATE_VAR = "Variable is already declared with given identifier.";
    constexpr const char* DUPLICATE_FUNC = "Function is already declarared with given identifier";
    constexpr const char* INVALID_NUMERIC_LITERAL = "The following expression is not a valid numeric literal.";
    constexpr const char* EXPECTED_RETURN_TYPE = "Expected return type here.";
    constexpr const char* INVALID_EXPR = "Could not evaluate the expression";
    constexpr const char* MAIN_FUNC_PARAM = "Main function cannot have parameter";
    constexpr const char* UNEXPECTED_RETURN = "Function return type does not match the expression";
    constexpr const char* EXPECTED_SEMI = "Expected semi colon";
    constexpr const char* CHAR_LENGTH_EXCEED = "Char literal cannot have length more than 1.";
    constexpr const char* EXPR_HUGE = "Expression exceed maximum size limit.";
    constexpr const char* EXPECTED_RETURN = "Expected return statement at the end of function";
    constexpr const char* MAIN_FUNC_RET = "Main function should return int";
    constexpr const char* INV_TOKEN = "Invalid token";
};

