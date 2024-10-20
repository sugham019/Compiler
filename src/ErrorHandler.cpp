#include "ErrorHandler.hpp"
#include "AST.hpp"
#include <iostream>
#include <system_error>

// Todo

void reportError(const std::string& errorMsg){
    std::cout << errorMsg << std::endl;
    std::terminate();
}

void reportError(const std::string& errorMsg, Token& token){
    reportError(errorMsg.c_str(), token);
}

void reportError(const char* errorMsg, Token& token){
    reportError(errorMsg);
}

void reportError(const char* errorMsg, std::ifstream& file){
    reportError(errorMsg);
}

void reportError(const std::error_code& error){
    reportError(error.message());
}

void reportError(const char* errorMsg, ast::Expression& expr){
    reportError(errorMsg);
}

