#pragma once
#include "AST.hpp"
#include "ErrorHandler.hpp"
#include "SymbolTable.hpp"
#include "Token.hpp"
#include <list>
#include "SymbolTableHandler.hpp"

class Analyzer{

public:
    Analyzer(ast::File& syntaxTree, const ErrorHandler& errorHandler);
    void analyze();
    
private:    
    void analyzeStatement(ast::Statement& statement, ast::Function& currentFunction);
    void analyzeConditionalStatement(ast::ConditionalStatement& conditionalStatement, ast::Function& currentFunction);
    void analyzeDeclarativeStatement(ast::DeclarativeStatement& declarativeStatement);
    void analyzeAssignmentStatement(ast::AssignmentStatement& assignmentStatement);
    Keyword analyzeFunctionCallStatement(ast::FunctionCallStatement& functionCallStatement);
    void analyzeReturnStatement(ast::ReturnStatement& returnStatement, ast::Function& currentFunction);
    void analyzeNestedScope(std::list<ast::Statement*> stmnts, ast::Function& currentFunction);
    void analyzeWhileLoop(ast::WhileLoop& whileLoop, ast::Function& currentFunction);
    void performTypeChecking(ast::DeclarativeStatement& declarativeStatement);
    void performTypeChecking(ast::ReturnStatement& returnStatement, ast::Function& function);
    void performTypeChecking(ast::AssignmentStatement& assignmentStatement);
    void performTypeChecking(ast::FunctionCallStatement& functionCallStatement);
    void performTypeChecking(ast::Expression& expression, Keyword expectedDataType);
    void performTypeChecking(ast::Relational& relational, Keyword expectedDataType);
    void performTypeChecking(ast::Additive& additive, Keyword expectedDataType);
    void performTypeChecking(ast::Term& term, Keyword expectedDataType);
    void performTypeChecking(ast::Factor& additive, Keyword expectedDataType);
    void performTypeChecking(ast::Statement& statement, ast::Function& function);

    Keyword findFirstValueType(ast::Expression& expr);
    Keyword findFirstValueType(ast::Relational& relational);

    const ErrorHandler& m_errorHandler;
    Keyword findVariableType(Token& identifier);
    std::list<SymbolTable> m_symbolTableList;
    SymbolTableHandler m_symbolTableHandler;
    ast::File& m_syntaxTree;
};

