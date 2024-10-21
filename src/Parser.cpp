#include "Parser.hpp"
#include "AST.hpp"
#include "ErrorHandler.hpp"
#include "Token.hpp"
#include <iostream>
#include <list>
#include <sys/types.h>

using namespace ast;

namespace{

bool isDataType(Token& token){
    return token.m_tokenType.keywordType == Keyword::INT ||
                token.m_tokenType.keywordType == Keyword::CHAR ||
                token.m_tokenType.keywordType == Keyword::FLOAT;
}

bool isType(Token& token, Type type){
    return token.m_tokenType.type == type;
}

bool isKeyword(Token& token, Keyword keyword){
    return token.m_tokenType.keywordType == keyword;
}

Token* createTokenCopy(Token& token){
    Token* tokenCopy = new Token(std::move(token));
    return tokenCopy;
}

bool isSymbol(Token& token, char symbol){
    return token.m_tokenType.symbol == symbol;
}

}

Parser::Parser(Tokenizer& tokenizer): m_tokenizer(tokenizer){
}

TokenBuffer Parser::prefetchToken(char end){
    TokenBuffer tokenBuffer;
    int bracketLevel = 0;
    Token temp = m_tokenizer.nextToken();
    while(!isSymbol(temp, end) || bracketLevel > 0){
        if(isSymbol(temp, '(')){
            bracketLevel++;
        }else if(isSymbol(temp, ')')){
            bracketLevel--;
        }
        
        if(tokenBuffer.size == MAX_TOKEN_BUFFER_SIZE){
            reportError(error::EXPR_HUGE, temp);
        }
        tokenBuffer.tokens[tokenBuffer.size++] = std::move(temp);
        temp = m_tokenizer.nextToken();
    }
    return tokenBuffer;
}

Token Parser::verifyNextToken(char symbol){
    Token nextToken = m_tokenizer.nextToken();
    if(!isSymbol(nextToken, symbol)){
        reportError( std::string("Expected : ")+symbol, nextToken);
    }
    return nextToken;
}

Token Parser::verifyNextToken(Type tokenType){
    Token nextToken = m_tokenizer.nextToken();
    if(!isType(nextToken, tokenType)){
        reportError(error::INV_TOKEN, nextToken);
    }
    return nextToken;
}

Token Parser::verifyNextToken(Keyword keywordType){
    Token nextToken = m_tokenizer.nextToken();
    if(!isKeyword(nextToken, keywordType)){
        reportError(error::INV_TOKEN, nextToken);
    }
    return nextToken;
}

File Parser::evaluate(){
    Token currentToken = m_tokenizer.nextToken();
    File file;
    while(!isType(currentToken, Type::NIL)){
        if(isKeyword(currentToken, Keyword::IMPORT)){
            Token importPackage = verifyNextToken(Type::STRING_LITERAL);
            Token* importPackageCopy = createTokenCopy(importPackage);
            file.importPackages.push_back(importPackageCopy); 
        }else if(isKeyword(currentToken, Keyword::FUNC)){
            Function* function = evaluateFunctionDefinition();
            file.functions.push_back(function);
        }else{
            reportError(error::INVALID_EXPR, currentToken);
        }
        currentToken = m_tokenizer.nextToken();
    }
    return file;
}

Function* Parser::evaluateFunctionDefinition(){
    Token expectedReturnType = m_tokenizer.nextToken();
    if(!isKeyword(expectedReturnType, Keyword::VOID) && !isDataType(expectedReturnType)){
        reportError("Expectec valid return type ", expectedReturnType);
    }
    Token identifier = verifyNextToken(Type::IDENTIFIER);
    verifyNextToken('(');

    Token* returnTypeCopy = createTokenCopy(expectedReturnType);
    Token* identifierCopy = createTokenCopy(identifier);
    Function* function = new Function(returnTypeCopy, identifierCopy);

    extractParams(function->m_parameters);
    verifyNextToken('{');

    Statement* stmnt = evaluateStatement();
    while(stmnt != nullptr){
        function->m_statements.push_back(stmnt);
        stmnt = evaluateStatement();
    }
    return function;
}

ConditionalStatement* Parser::evaluateIfConditionalStatement(){
    verifyNextToken('(');
    TokenBuffer tokenBuffer = prefetchToken(')');
    Expression* expr = evaluateExpression(tokenBuffer.tokens, 0, tokenBuffer.size-1);
    verifyNextToken('{');

    ConditionalStatement* conditionalStatement = new ConditionalStatement(expr);
    Statement* stmnt = evaluateStatement();
    while(stmnt != nullptr){
        conditionalStatement->m_stmnts.push_back(stmnt);
        stmnt = evaluateStatement();
    }
    Token nextToken = m_tokenizer.peekToken();
    if(nextToken.m_tokenType.keywordType == Keyword::ELSE){
        m_tokenizer.nextToken();
        Token nextToken1 = m_tokenizer.nextToken();

        if(isKeyword(nextToken1, Keyword::IF)){
            ConditionalStatement* elseIfCondition = evaluateIfConditionalStatement();
            conditionalStatement->m_else = elseIfCondition;
        }else if(isSymbol(nextToken1, '{')){
            ConditionalStatement* elseCondition = new ConditionalStatement();
            Statement* elseBlockStmnts = evaluateStatement();
            while(elseBlockStmnts != nullptr){
                elseCondition->m_stmnts.push_back(elseBlockStmnts);
                elseBlockStmnts = evaluateStatement();
            }
            conditionalStatement->m_else = elseCondition;
        }else{
            reportError("Expected {", nextToken1);
        }
    }
    return conditionalStatement;
}

ReturnStatement* Parser::evaluateReturnStatement(){
    Token semi_colon = m_tokenizer.peekToken();
    if(isSymbol(semi_colon, STATEMENT_TERMINATOR)){
        m_tokenizer.nextToken();
        return new ReturnStatement();
    }
    TokenBuffer tokenBuffer = prefetchToken(';');
    Expression* expr = evaluateExpression(tokenBuffer.tokens, 0, tokenBuffer.size-1);
    ReturnStatement* returnStatement = new ReturnStatement(expr);
    return returnStatement;
}

WhileLoop* Parser::evaluateWhileLoop(){
    verifyNextToken('(');
    TokenBuffer exprTokens = prefetchToken(')');
    Expression* expr = evaluateExpression(exprTokens.tokens, 0, exprTokens.size-1);
    WhileLoop* whileLoop = new WhileLoop(expr);
    verifyNextToken('{');

    Statement* stmnt = evaluateStatement();
    while(stmnt != nullptr){
        whileLoop->m_stmnts.push_back(stmnt);
        stmnt = evaluateStatement();
    }
    return whileLoop;
}

Statement* Parser::evaluateStatement(){
    Token startToken = m_tokenizer.nextToken();
    if(isSymbol(startToken, '}')) return nullptr;

    if(isDataType(startToken)){
        DeclarativeStatement* declarativeStatement = evaluateDeclarativeStatement(startToken, false);
        return new Statement(declarativeStatement);
    }else if(isKeyword(startToken, Keyword::CONST)){
        Token dataType = m_tokenizer.nextToken();
        if(isDataType(dataType)){
            DeclarativeStatement* declarativeStatement = evaluateDeclarativeStatement(dataType, true);
        }
    }else if(isType(startToken, Type::IDENTIFIER)){
        Token nextToken = m_tokenizer.nextToken();
        if(isSymbol(nextToken, '=')){
            AssignmentStatement* assignmentStatement = evaluateAssignmentStatement(startToken);
            return new Statement(assignmentStatement);
        }else if(isSymbol(nextToken, '(')){
            FunctionCallStatement* functionCallStatement = evaluateFunctionCallStatement(startToken);
            return new Statement(functionCallStatement);
        }
    }else if(isKeyword(startToken, Keyword::IF)){
        ConditionalStatement* conditionalStatement = evaluateIfConditionalStatement();
        return new Statement(conditionalStatement);
    }else if(isKeyword(startToken, Keyword::RETURN)){
        ReturnStatement* returnStatement = evaluateReturnStatement();
        return new Statement(returnStatement);
    }else if(isKeyword(startToken, Keyword::WHILE)){
        WhileLoop* whileLoop = evaluateWhileLoop();
        return new Statement(whileLoop);
    }
    reportError("Could not evaluate statement", startToken);
    return nullptr;
}


bool Parser::extractParams(std::list<Parameter>& params){
    Token dataType = m_tokenizer.nextToken();
    if(isDataType(dataType)){
        Token* dataTypeCopy = createTokenCopy(dataType);
        Token identifier = m_tokenizer.nextToken();

        if(identifier.m_tokenType.type != TokenType::Type::IDENTIFIER){
            delete dataTypeCopy;
            return false;
        }
        Token* identifierCopy = createTokenCopy(identifier);
        Token endToken = m_tokenizer.nextToken();
        params.push_back(Parameter{dataTypeCopy, identifierCopy});
        if(isSymbol(endToken, ',')){
            extractParams(params);
        }else if(!isSymbol(endToken, ')')){
            reportError("Expected )", endToken);
        }
        return true;
    }
    if(isSymbol(dataType, ')')) return true;
    reportError("Expected Datatype", dataType);
    return false;
}

FunctionCallStatement* Parser::evaluateFunctionCallStatement(Token& functionName){
    TokenBuffer argsToken = prefetchToken(')');
    std::list<Expression*> args; 
    extractArgsInFunctionCall(args, argsToken.tokens, 0, argsToken.size-1);
    Token* functionNameCopy = createTokenCopy(functionName);

    verifyNextToken(';');
    return new FunctionCallStatement(functionNameCopy, args);
}

void Parser::extractArgsInFunctionCall(std::list<Expression*>& args, Token* tokens, int start, int end){
    int bracketLevel = 0;
    for(int i=start; i<= end; i++){
        if(isSymbol(tokens[i], '(')){
            bracketLevel++;
        }else if(isSymbol(tokens[i], ')')){
            bracketLevel--;
        }
        if(bracketLevel != 0) continue;

        if(isSymbol(tokens[i], ',')){
            args.push_back(evaluateExpression(tokens, start, i-1));
            start = i + 1;
        }
    }

    if(start <= end){
        args.push_back(evaluateExpression(tokens, start, end));
    }
}

AssignmentStatement* Parser::evaluateAssignmentStatement(Token& identifier){
    TokenBuffer tokenBuffer = prefetchToken(';');
    Expression* expression = evaluateExpression(tokenBuffer.tokens, 0, tokenBuffer.size-1);
    Token* identifierCopy = new Token(std::move(identifier));
    return new AssignmentStatement(identifierCopy, expression);
}

DeclarativeStatement* Parser::evaluateDeclarativeStatement(Token& dataType, bool isConst){
    Token identifier = verifyNextToken(Type::IDENTIFIER);
    
    Token nextToken = m_tokenizer.nextToken();
    if(isSymbol(nextToken, STATEMENT_TERMINATOR)){
        return new DeclarativeStatement(
            createTokenCopy(dataType), createTokenCopy(identifier), isConst);
    }else if(isSymbol(nextToken, '=')){
        TokenBuffer tokenBuffer = prefetchToken(';');
        Expression* expression = evaluateExpression(tokenBuffer.tokens, 0, tokenBuffer.size-1);
        return new DeclarativeStatement(
            createTokenCopy(dataType), createTokenCopy(identifier), expression, isConst);
    }else{
        reportError("Expected Semicolon", nextToken);
        return nullptr;
    }
}

Factor* Parser::evaluateFactor(Token* tokens, int start, int end){
    Token& firstToken = tokens[start];

    if(start == end){
        if(isType(firstToken, Type::NUMERIC_LITERAL) || isType(firstToken, Type::STRING_LITERAL) || isType(firstToken, Type::IDENTIFIER)){
            Token* newToken = createTokenCopy(firstToken);
            return new Factor(newToken);
        }
    }
    if(isType(firstToken, Type::IDENTIFIER)){
        if(isSymbol(tokens[start+1], '(')){
            if(!isSymbol(tokens[end], ')')){
                reportError("Expected )", tokens[end]);
            }
            std::list<Expression*> args;
            extractArgsInFunctionCall(args, tokens, start+2, end-1);
            Token* identifierCopy = createTokenCopy(firstToken);
            FunctionCallStatement* functionCall = new FunctionCallStatement(identifierCopy, args);
            
            return new Factor(functionCall);
        }else{
            reportError("Invalid expression", tokens[start+1]);
        }
    }
    Token& lastToken = tokens[end];
    if(isSymbol(firstToken,'(') && isSymbol(lastToken, ')')){
        Expression* expression = evaluateExpression(tokens, start+1, end-1);
        return new Factor(expression);
    }
    return nullptr;
}

Term* Parser::evaluateTerm(Token* tokens, int start, int end){
    u_int16_t bracketLevel = 0;
    Term* term = new Term();
    Factor** factorPtr = &term->m_factor;
    TermTail** termTailPtr = &term->m_termTail;

    auto update = [&](int i, Opcode opcode){
        *factorPtr = evaluateFactor(tokens, start, i-1);
        TermTail* temp = new TermTail();
        temp->m_opcode = opcode;
        *termTailPtr = temp;
        factorPtr = &temp->m_factor;
        termTailPtr = &temp->m_termTail;
        start = i+1;
    };

    for(int i=start; i<=end; i++){
        Token& currentToken = tokens[i];
        if(isSymbol(currentToken,'(')){
            bracketLevel++;
        }else if(isSymbol(currentToken,')')){
            bracketLevel--;
        }
        if(bracketLevel != 0) continue;

        if(isSymbol(currentToken, '*')){
            update(i, Opcode::MULTIPLICATION);
        }else if(isSymbol(currentToken, '/')){
            update(i, Opcode::DIVISION);
        }
    }
    *factorPtr = evaluateFactor(tokens, start, end);
    return term;
}


Additive* Parser::evaluateAdditive(Token* tokens, int start, int end){
    u_int16_t bracketLevel = 0;
    Additive* additive = new Additive();
    Term** termPtr = &additive->m_term;
    AdditiveTail** additiveTailPtr = &additive->m_additiveTail;

    auto update = [&](int i, Opcode opcode){
        *termPtr = evaluateTerm(tokens, start, i-1);
        AdditiveTail* temp = new AdditiveTail();
        temp->m_opcode = opcode;
        *additiveTailPtr = temp;
        termPtr = &temp->m_term;
        additiveTailPtr = &temp->m_additiveTail;
        start = i+1;
    };

    for(int i=start; i<=end; i++){
        Token& currentToken = tokens[i];
        if(isSymbol(currentToken,'(')){
            bracketLevel++;
        }else if(isSymbol(currentToken,')')){
            bracketLevel--;
        }
        if(bracketLevel != 0) continue;

        if(isSymbol(currentToken, '+')){
            update(i, Opcode::ADDITION);
        }else if(isSymbol(currentToken, '-')){
            update(i, Opcode::SUBTRACTION);
        }
    }
    *termPtr = evaluateTerm(tokens, start, end);
    return additive;
}

Relational* Parser::evaluateRelational(Token* tokens, int start, int end){
    u_int16_t bracketLevel = 0;
    Relational* relational = new Relational();
    Additive** additivePtr = &relational->m_additive;
    RelationalTail** relationalTailPtr = &relational->m_relationalTail;

    auto update = [&](int i, Opcode opcode){
        *additivePtr = evaluateAdditive(tokens, start, i-1);
        RelationalTail* temp = new RelationalTail();
        temp->m_opcode = opcode;
        *relationalTailPtr = temp;
        additivePtr = &temp->m_additive;
        relationalTailPtr = &temp->m_relationalTail;
        if(opcode == Opcode::GREATER_THAN || opcode == Opcode::SMALLER_THAN){
            start = i + 1;
        }else {
            start = i +2;
        }
    };

    for(int i=start; i<=end; i++){
        Token& currentToken = tokens[i];

        if(isSymbol(currentToken,'(')){
            bracketLevel++;
        }else if(isSymbol(currentToken,')')){
            bracketLevel--;
        }
        if(bracketLevel != 0) continue;
        if((i+1) > end) break;
        Token& nextToken = tokens[i+1];

        if(isSymbol(currentToken, '>')){
            if(isSymbol(nextToken, '=')){
                update(i, Opcode::GREATER_OR_EQUAL);
            }else{
                update(i, Opcode::GREATER_THAN);
            }
            
        }else if(isSymbol(currentToken, '<')){
            if(isSymbol(nextToken, '=')){
                update(i, Opcode::SMALLER_OR_EQUAL);
            }else{
                update(i, Opcode::SMALLER_THAN);
            }
        }else if(isSymbol(currentToken, '=') && isSymbol(nextToken, '=')){
            update(i, Opcode::EQUAL_TO);
        }
    }
    
    *additivePtr = evaluateAdditive(tokens, start, end);
    return relational;
}


Expression* Parser::evaluateExpression(Token* tokens, int start, int end){
    u_int16_t bracketLevel = 0;
    Expression* expression = new Expression();
    Relational** relationalPtr = &expression->m_relational;
    ExpressionTail** exprTailPtr = &expression->m_expressionTail;

    auto update = [&](int i, Opcode opcode){
        *relationalPtr = evaluateRelational(tokens, start, i-1);
        ExpressionTail* temp = new ExpressionTail();
        temp->m_opcode = opcode;
        *exprTailPtr = temp;
        relationalPtr = &temp->m_relational;
        exprTailPtr = &temp->m_expressionTail;
        start = i+2;
    };

    for(int i=start; i<=end; i++){
        Token& currentToken = tokens[i];

        if(isSymbol(currentToken,'(')){
            bracketLevel++;
        }else if(isSymbol(currentToken,')')){
            bracketLevel--;
        }
        if(bracketLevel != 0) continue;
        if((i+1) > end) break;
        Token& nextToken = tokens[i+1];

        if(isSymbol(currentToken, '&') && isSymbol(nextToken, '&')){
            
            update(i, Opcode::LOGICAL_AND);
        }else if(isSymbol(currentToken, '&') && isSymbol(nextToken, '&')){
            update(i, Opcode::LOGICAL_OR);
        }
    }
    *relationalPtr = evaluateRelational(tokens, start, end);
    return expression;
}