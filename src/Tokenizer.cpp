#include "Tokenizer.hpp"
#include "ErrorHandler.hpp"

Tokenizer::Tokenizer(const std::string& filename) : m_filename(filename){
    loadFileData(filename);
}

namespace{

    constexpr int maxKeywordLength = 12;
    constexpr int totalKeywords = 11;

    char keywords[totalKeywords][maxKeywordLength] = {
        "int", "char", "void",
        "import", "if", "else",
        "const", "float", "return",
        "func", "while"
    };
    
    /*
        This Keyword Type array should be aligned with keyword string array.
        Eg: if given keyword in keywords string array is found at index 2 then 
        KeywordType will also be taken from index 2
    */
    Keyword keywordTypes[totalKeywords] = {
        Keyword::INT, Keyword::CHAR, Keyword::VOID,
        Keyword::IMPORT, Keyword::IF, Keyword::ELSE,
        Keyword::CONST, Keyword::FLOAT, Keyword::RETURN,
        Keyword::FUNC, Keyword::WHILE
    };

    bool isSymbol(char ch){
        return (ch >= 33 && ch <= 47) ||
                (ch >= 58 && ch <= 64) ||
                (ch >= 91 && ch <= 96) ||
                (ch >= 123 && ch <= 126);  
    }

    bool isDotSymbol(char ch){
        return ch == 46;
    }
}

void Tokenizer::skipSpaces(){
    char ch;
    while(m_file.get(ch)){
        if(!std::isspace(ch)){
            m_file.seekg(-1, std::ios::cur);
            break;
        }
        if(ch == '\n') m_currentLineNum++;
    }
}

bool Tokenizer::processKeyword(Token& token){
    m_buffer[m_bufferLength] = 0;
    for(int i=0;i<totalKeywords;i++){
        if(strcmp(keywords[i], m_buffer) == 0){
            TokenType tokenType = TokenType(TokenType::Type::KEYWORD, keywordTypes[i]);
            token = Token(tokenType, nullptr, 0 , m_currentLineNum);
            return true;
        }
    }
    return false;
}

bool Tokenizer::processStringLiteral(Token& token){
    if(m_buffer[0] != '\"') return false;
    char ch;
    while(m_file.get(ch)){
        if(ch == '\"'){
            char* data = new char[m_bufferLength-1];
            for(int i=0; i<m_bufferLength-1;i++){
                data[i] = m_buffer[i+1]; 
            }
            token = Token(TokenType(TokenType::Type::STRING_LITERAL), 
                    data, static_cast<u_int16_t>(m_bufferLength-1), m_currentLineNum);
            return true;
        }else if(ch == '\n'){
            m_currentLineNum++;
        }
        updateBuffer(ch);
    }
    reportError("Missing \" to terminate string", m_file);
    return false;
}


bool Tokenizer::processNumericLiteral(Token& token){
    if(!std::isdigit(m_buffer[0])) return false;
    char ch;
    bool dotSymbolFound = false;
    while(m_file.get(ch)){
        if(isDotSymbol(ch)){
            if(dotSymbolFound){
                reportError(error::INVALID_NUMERIC_LITERAL, m_file);
            }
            dotSymbolFound = true;
        }else if(!std::isdigit(ch)){
            m_file.seekg(-1, std::ios::cur);
            break;  
        }
        updateBuffer(ch);
    }
    char* data = new char[m_bufferLength];
    copyFromBuffer(data);
    token = Token(TokenType(TokenType::Type::NUMERIC_LITERAL), 
        data, m_bufferLength, m_currentLineNum);
    token.m_tokenType.isFloatingPointValue = dotSymbolFound;
    return true;
}

void Tokenizer::buildWord(){
    char ch;
    while(m_file.get(ch)){
        if(std::isspace(ch) || isSymbol(ch)){
            m_file.seekg(-1, std::ios::cur);
            break;
        }
        updateBuffer(ch);
    }
}

Token Tokenizer::peekToken(){
    std::streampos currentPos = m_file.tellg();
    Token nxtToken = nextToken(); 
    m_file.seekg(currentPos);
    return nxtToken;
}

Token Tokenizer::nextToken() {
    skipSpaces();
    Token token;
    if(m_file.eof()) return token;

    m_file.get(m_buffer[0]);
    m_bufferLength = 1;
    if(processStringLiteral(token) || processNumericLiteral(token)){
        return token;
    } else if(isSymbol(m_buffer[0])){
        token = Token(TokenType(TokenType::Type::SYMBOL, m_buffer[0]),
                 nullptr, 0, m_currentLineNum);
        return token;
    }
    buildWord();
    if(processKeyword(token)){
        return token;
    }
    // else create token for identifier
    char* data = new char[m_bufferLength];
    copyFromBuffer(data);
    token = Token(TokenType(TokenType::Type::IDENTIFIER), data, m_bufferLength, m_currentLineNum);
    return token;
}

void Tokenizer::loadFileData(const std::string& filename){
    m_file.open(filename);
    if (!m_file.is_open()){
        reportError("Error opening file : " + filename);
    }
}
