#pragma once
#include "Token.hpp"
#include <filesystem>
#include <fstream>
#include <cctype>
#include <fstream>
#include "Token.hpp"
#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <cctype>
#include "ErrorHandler.hpp"

class Tokenizer{

public:
    Tokenizer(std::ifstream& file, const ErrorHandler& errorHandler);
    Token nextToken();
    Token peekToken();


    static std::ifstream* activeFile;

private:
    void skipSpaces();
    bool processStringLiteral(Token& tokenData);
    bool processNumericLiteral(Token& tokenData);
    bool processKeyword(Token& tokenData);

    void buildWord();


    void updateBuffer(char ch){
        if(m_bufferLength == m_maxBufferLength){
            m_errorHandler.reportError("Token Length limit exceeded", m_currentLineNum);
        }
        m_buffer[m_bufferLength++] = ch;
    }

    void copyFromBuffer(char* destination) const {
        for(int i=0;i<m_bufferLength;i++){
            destination[i] = m_buffer[i];
        }
    }

    const ErrorHandler& m_errorHandler;
    std::ifstream& m_file;
    const std::string m_filename;
    u_int32_t m_currentLineNum = 1;
    static constexpr int m_maxBufferLength = 512;
    char m_buffer[m_maxBufferLength];
    u_int16_t m_bufferLength = 0;
};
