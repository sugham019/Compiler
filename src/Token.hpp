#pragma once
#include <sys/types.h>
#include <utility>

struct TokenType{

    enum class Type : u_int8_t{
        KEYWORD,
        SYMBOL,
        STRING_LITERAL,
        IDENTIFIER,
        NUMERIC_LITERAL,
        NIL
    }type = Type::NIL;

    enum KeywordType : u_int8_t{
        INT,
        CHAR,
        VOID,
        IMPORT,
        IF,
        ELSE,
        CONST,
        FLOAT,
        RETURN,
        FUNCTION,
        NIL
    }keywordType = KeywordType::NIL;    

    char symbol = 0;
    bool isFloatingPointValue = false;

    bool operator==(const TokenType& other) const{
        return (type == other.type) && (keywordType == other.keywordType) && (symbol == other.symbol);
    }

    TokenType() noexcept = default;

    TokenType(Type type): type(type){
    }

    TokenType(Type type, KeywordType keywordType) : type(type), keywordType(keywordType){
    }

    TokenType(Type type, char symbol): type(type), symbol(symbol){
    }
};

using Keyword = TokenType::KeywordType;
using Type = TokenType::Type;

struct Token{

    char* m_value = nullptr;
    u_int32_t m_lineNumber;
    TokenType m_tokenType;
    u_int16_t m_valueSize = 0;
    
    Token() noexcept = default;
    
    Token(TokenType tokenType, char* value, u_int16_t valueSize, u_int32_t lineNumber) noexcept
            :m_tokenType(tokenType), m_value(value), m_valueSize(valueSize), m_lineNumber(lineNumber){
    }

    void moveResources(Token&& token){
        m_value = token.m_value;
        m_tokenType = token.m_tokenType;
        m_valueSize = token.m_valueSize;
        m_lineNumber = token.m_lineNumber;
        token.m_value = nullptr;
    }

    Token(Token&& token) noexcept{
        moveResources(std::move(token));
    }

    Token& operator=(Token&& token) noexcept{
        moveResources(std::move(token));
        return *this;
    }

    ~Token(){
        delete[] m_value;
    }
};

   