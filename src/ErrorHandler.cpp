#include "ErrorHandler.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

ErrorHandler::ErrorHandler(std::ifstream& currentFile)
    : m_currentFile(currentFile){

}

void ErrorHandler::reportError(const std::string& errorMsg) const{
    std::cerr << errorMsg << std::endl;
    std::exit(EXIT_FAILURE);
}

void ErrorHandler::reportError(const std::string& errorMsg, const int lineNum) const{

    auto moveToLine = [](std::ifstream& file, int lineNum){
        file.seekg(0);
        int i = 1;
        char ch;
        while(file.get(ch)){
            if(i == lineNum) break;
            if(ch == '\n'){
                i++;
            }
        }
    };

    auto getLineText = [](std::ifstream& file, std::string& buffer){
        char ch;
        while(file.get(ch)){
            buffer += ch;
            if(ch == '\n'){
                break;
            }
        }
    };
    moveToLine(m_currentFile, lineNum-3);
    std::string text = "\n\n";
    text.reserve(1000);

    for(int i=lineNum-2; i<lineNum+3;i++){
        if(i == lineNum){
            text += "\033[31m";
            text += std::to_string(i) + "\t";
            getLineText(m_currentFile, text);
            text += "\033[0m";
        }else{
            text += std::to_string(i) + "\t";
            getLineText(m_currentFile, text);
        }   
    }
    text += "\n\n\n"+errorMsg;

    std::cerr << text << std::endl;
    std::exit(EXIT_FAILURE);
}

void ErrorHandler::reportError(const std::string& errorMsg, const Token& token) const{
    reportError(errorMsg, token.m_lineNumber);
}
