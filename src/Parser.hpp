#pragma once
#include "Token.hpp"
#include "Tokenizer.hpp"
#include "AST.hpp"
#include <sys/types.h>

constexpr int MAX_TOKEN_BUFFER_SIZE = 100;

struct TokenBuffer{
    Token tokens[MAX_TOKEN_BUFFER_SIZE];
    int size = 0;
};

class Parser{

public:
    Parser(Tokenizer& tokenizer);
    ast::File evaluate();

private:
    ast::Function* evaluateFunctionDefinition(Token& returnType);
    ast::Statement* evaluateStatement();
    TokenBuffer prefetchToken(char end);
    ast::DeclarativeStatement* evaluateDeclarativeStatement(Token& keyword, bool isConst);
    ast::ConditionalStatement* evaluateIfConditionalStatement();
    ast::ReturnStatement* evaluateReturnStatement();
    bool extractParams(std::list<ast::Parameter>& params);
    ast::AssignmentStatement* evaluateAssignmentStatement(Token& identifier);
    ast::FunctionCallStatement* evaluateFunctionCallStatement(Token& functionName);
    ast::Expression* evaluateExpression(Token* tokens, int start, int end);
    ast::Additive* evaluateAdditive(Token* tokens, int start, int end);
    ast::Relational* evaluateRelational(Token* tokens, int start, int end);
    ast::Term* evaluateTerm(Token* tokens, int start, int end);
    ast::Factor* evaluateFactor(Token* tokens, int start, int end);
    Token verifyNextToken(Keyword keyword);
    Token verifyNextToken(char symbol);
    Token verifyNextToken(Type tokenType);
    void extractArgsInFunctionCall(std::list<ast::Expression*>& args, Token* tokens, int start, int end);

    Tokenizer& m_tokenizer;
    constexpr static int MAX_TOKEN_BUFFER_SIZE = 120;
};  