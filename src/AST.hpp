#pragma once
#include <cstddef>
#include <list>
#include <string>
#include <sys/types.h>
#include "Token.hpp"

namespace ast{

constexpr char STATEMENT_TERMINATOR = ';';

struct Relational;
struct Statement;
struct ExpressionTail;


enum class Access{
    PUBLIC,
    PRIVATE
};

enum class Opcode{
    LOGICAL_AND,
    LOGICAL_OR,
    GREATER_THAN,
    SMALLER_THAN,
    GREATER_OR_EQUAL,
    SMALLER_OR_EQUAL,
    EQUAL_TO,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION
} ;

struct Expression{
    Relational* m_relational = nullptr;
    ExpressionTail* m_expressionTail = nullptr;

    Expression(){

    }

    ~Expression();

};

struct ConditionalStatement{
    Expression* m_expr;
    std::list<Statement*> m_stmnts;
    ConditionalStatement* m_else;

    ConditionalStatement(Expression* expr, std::list<Statement*> stmnts)
        : m_stmnts(std::move(stmnts)), m_expr(expr), m_else(nullptr){
    }

    ConditionalStatement(Expression* expr)
        : m_expr(expr), m_else(nullptr){
    }

    ConditionalStatement()
        : m_expr(nullptr), m_else(nullptr){
    }

    ConditionalStatement(Expression* expr, std::list<Statement*> stmnts, ConditionalStatement* else1)
        : m_stmnts(std::move(stmnts)), m_expr(expr), m_else(else1){
    }

    ~ConditionalStatement();
};

struct WhileLoop{
    Expression* m_expr;
    std::list<Statement*> m_stmnts;

    WhileLoop(Expression* expr)
    : m_expr(expr) {

    }

    ~WhileLoop();
};


struct Parameter{
    Token* m_dataType;
    Token* m_identifier;
};

struct FunctionCallStatement{
    Token* m_identifier;
    std::list<Expression*> m_args;

    FunctionCallStatement(Token* identifier, std::list<Expression*> args):
        m_identifier(identifier), m_args(std::move(args)){
    }

    ~FunctionCallStatement(){
        delete m_identifier;
        for(Expression* arg: m_args){
            delete arg;
        }
    }
};

struct ReturnStatement{
    Expression* m_expr;

    ReturnStatement(Expression* expr)
        : m_expr(expr){
    }
    
    ReturnStatement()
        : m_expr(nullptr){

    }

    ~ReturnStatement(){
        delete m_expr;
    }
};

struct AssignmentStatement{
    Token* m_identifier;
    Expression* m_expression;

    AssignmentStatement(Token* identifier, Expression* expression)
        : m_identifier(identifier), m_expression(expression){
    }

    ~AssignmentStatement(){
        delete m_identifier;
        delete m_expression;
    }

};



struct DeclarativeStatement{
    Token* m_dataType;
    Token* m_identifier;
    bool m_isConst;
    bool m_isInitialized;
    Expression* m_expression;

    DeclarativeStatement(Token* dataType, Token* identifier, bool isConst)
        : m_dataType(dataType), m_identifier(identifier), m_expression(nullptr), m_isConst(isConst), m_isInitialized(false){
    }

    DeclarativeStatement(Token* dataType, Token* identifier, Expression* expression, bool isConst)
        : m_dataType(dataType), m_identifier(identifier), m_expression(expression), m_isConst(isConst), m_isInitialized(true){
    }

    ~DeclarativeStatement(){
        delete m_dataType;
        delete m_identifier;
        delete m_expression;
    }
};

struct Statement{

    union Data{
        DeclarativeStatement* declarativeStatement;
        AssignmentStatement* assignmentStatement;
        ConditionalStatement* conditionalStatement;
        FunctionCallStatement* functionalCallStatement;
        ReturnStatement* returnStatement;
        WhileLoop* whileLoop;
    } m_data;

    enum class Type: uint8_t{
        DECLARATIVE,
        ASSIGNMENT,
        CONDITIONAL,
        FUNCTION_CALL,
        RETURN,
        WHILE_LOOP
    } m_type;

    Statement(DeclarativeStatement* declarativeStatement){
        m_data.declarativeStatement = declarativeStatement;
        m_type = Type::DECLARATIVE;
    }

    Statement(AssignmentStatement* assignmentStatement){
        m_data.assignmentStatement = assignmentStatement;
        m_type = Type::ASSIGNMENT;
    }

    Statement(ConditionalStatement* conditionalStatement){
        m_data.conditionalStatement = conditionalStatement;
        m_type = Type::CONDITIONAL;
    }

    Statement(FunctionCallStatement* functionCallStatement){
        m_data.functionalCallStatement = functionCallStatement;
        m_type = Type::FUNCTION_CALL;
    }

    Statement(WhileLoop* whileLoop){
        m_data.whileLoop = whileLoop;
        m_type = Type::WHILE_LOOP;
    }

    Statement(ReturnStatement* returnStatement){
        m_data.returnStatement = returnStatement;
        m_type = Type::RETURN;
    }

    Statement() {
        switch (m_type) {
            case Type::DECLARATIVE:
                delete m_data.declarativeStatement;
                break;
            case Type::ASSIGNMENT:
                delete m_data.assignmentStatement;
                break;
            case Type::CONDITIONAL:
                delete m_data.conditionalStatement;
                break;
            case Type::FUNCTION_CALL:
                delete m_data.functionalCallStatement;
                break;
            case Type::RETURN:
                delete m_data.returnStatement;
                break;
            case Type::WHILE_LOOP:
                delete m_data.whileLoop;
                break;
            }
    }
};

struct Function{
    Token* m_returnType;
    Token* m_identifier;
    std::list<Parameter> m_parameters;
    std::list<Statement*> m_statements;

    Function(Token* returnType, Token* identifier)
        : m_returnType(returnType), m_identifier(identifier){
    }

    ~Function(){
        delete m_returnType;
        delete m_identifier;
        for(Parameter param: m_parameters){
            delete param.m_identifier;
            delete param.m_dataType;
        }
        for(Statement* stmnt: m_statements){
            delete stmnt;
        }
    }
};

struct FunctionPrototype{
    Keyword returnType;
    const std::string_view identifier;
    std::list<Parameter>* m_parameters;
};

struct File{
    std::list<Token*> importPackages;
    std::list<Function*> functions;
    std::list<FunctionPrototype*> functionPrototypes;

    void free(){
        for(Function* function: functions){
            delete function;
        }
        for(Token* package: importPackages){
            delete package;
        }
        for(FunctionPrototype* funcPrototype: functionPrototypes){
            delete funcPrototype;
        }
    }
};

struct Factor{

    union Operand{
        Token* value; // for literal or variable
        Expression* expression; // for expression inside expression eg ( 5 + 2 ) + 2
        FunctionCallStatement* functionCall; // for function calls within an expression eg : sum(2, 5)
    } operand;

    enum class OperandType{
        VALUE, EXPR, FUNCTION_CALL
    } operandType;

    Factor(Expression* expression){
        operand.expression = expression;
        operandType = OperandType::EXPR;
    }

    Factor(Token* value){
        operand.value = value;
        operandType = OperandType::VALUE;
    }

    Factor(FunctionCallStatement* functionCall){
        operand.functionCall = functionCall;
        operandType = OperandType::FUNCTION_CALL;
    }

    ~Factor(){
        switch(operandType){
            case OperandType::VALUE:
                delete operand.value;
                break;
            case OperandType::EXPR:
                delete operand.expression;
                break;
            case OperandType::FUNCTION_CALL:
                delete operand.functionCall;
        };
    }

};

struct TermTail{
    Opcode m_opcode;
    Factor* m_factor = nullptr;
    TermTail* m_termTail = nullptr;

    TermTail(){

    }

    ~TermTail(){
        delete m_termTail;
        delete m_factor;
    }
};

struct Term{
    Factor* m_factor = nullptr;
    TermTail* m_termTail = nullptr;

    Term(){

    }

    ~Term(){
        delete m_termTail;
        delete m_factor;
    }
};

struct AdditiveTail{
    Opcode m_opcode;
    Term* m_term = nullptr;
    AdditiveTail* m_additiveTail = nullptr;

    AdditiveTail(){

    }

    ~AdditiveTail(){
        delete m_additiveTail;
        delete m_term;
    }
};

struct Additive{
    Term* m_term = nullptr;
    AdditiveTail* m_additiveTail = nullptr;

    Additive(){

    }

    ~Additive(){
        delete m_additiveTail;
        delete m_term;
    }
};


struct RelationalTail{
    Opcode m_opcode;
    Additive* m_additive = nullptr;
    RelationalTail* m_relationalTail = nullptr;

    RelationalTail(){

    }

    ~RelationalTail(){
        delete m_additive;
        delete m_relationalTail;
    }
};

struct Relational{
    Additive* m_additive = nullptr;
    RelationalTail* m_relationalTail = nullptr;

    Relational(){

    }

    ~Relational(){
        delete m_relationalTail;
        delete m_additive;
    }
};

struct ExpressionTail{
    
    Opcode m_opcode;
    Relational* m_relational = nullptr;
    ExpressionTail* m_expressionTail = nullptr;

    ExpressionTail(){

    }

    ExpressionTail(Opcode opcode, Relational* relational, ExpressionTail* expressionTail)
            :m_opcode(opcode), m_relational(relational), m_expressionTail(expressionTail){
    }

    ExpressionTail(Opcode opcode, Relational* relational)
            : m_opcode(opcode), m_relational(relational), m_expressionTail(nullptr){
    }

    ~ExpressionTail(){
        delete m_expressionTail;
        delete m_relational;
    }
};

inline Expression::~Expression(){      
    delete m_expressionTail;
    delete m_relational;
}

inline ConditionalStatement::~ConditionalStatement(){
    delete m_else;
    for(Statement* stmnt: m_stmnts){
        delete stmnt;
    } 
}

inline WhileLoop::~WhileLoop(){
    delete m_expr;
    for(Statement* stmnt : m_stmnts){
        delete stmnt;
    }
}

}