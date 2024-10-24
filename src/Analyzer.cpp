#include "Analyzer.hpp"
#include "AST.hpp"
#include "SymbolTable.hpp"
#include "Token.hpp"
#include <list>
#include "ErrorHandler.hpp"
#include <string_view>
#include <sys/types.h>
#include <type_traits>
#include <utility>


Analyzer::Analyzer(ast::File& syntaxTree, const ErrorHandler& errorHandler)
    : m_syntaxTree(syntaxTree), m_errorHandler(errorHandler), m_symbolTableHandler(errorHandler){
}

void Analyzer::analyze(){
    m_symbolTableHandler.createSymbolTable(); 
    
    auto evaluateFunction = [&](ast::Function& function){
        const std::string_view functionName(function.m_identifier->m_value, function.m_identifier->m_valueSize);
        if(functionName == "main" ){
            if(!function.m_parameters.empty()){
                m_errorHandler.reportError(error::MAIN_FUNC_PARAM, *function.m_identifier);
            }else if(function.m_returnType->m_tokenType.keywordType != Keyword::INT){
                m_errorHandler.reportError("Main function should return int", *function.m_returnType);
            }
        }
        m_symbolTableHandler.createSymbolTable();
        for(ast::Parameter param: function.m_parameters){
            std::string_view paramIdentifier(param.m_identifier->m_value, param.m_identifier->m_valueSize);
            m_symbolTableHandler.updateSymbolTable(param.m_dataType->m_tokenType.keywordType, paramIdentifier, true, false);
        }
        bool returnStatementFound = false;
        for(auto statement: function.m_statements){
            if(statement->m_type == ast::Statement::Type::RETURN){
                returnStatementFound = true;
            }
           analyzeStatement(*statement, function);
        }
        if(!returnStatementFound){
            m_errorHandler.reportError(error::EXPECTED_RETURN, *function.m_returnType);
        }
        m_symbolTableHandler.popSymbolTabe();
    };
    
    for(auto function: m_syntaxTree.functions){
        m_symbolTableHandler.updateSymbolTable(*function);
        evaluateFunction(*function);
    }
}

void Analyzer::analyzeDeclarativeStatement(ast::DeclarativeStatement& declarativeStatement){
    Keyword expectedType = declarativeStatement.m_dataType->m_tokenType.keywordType;\
    if(declarativeStatement.m_expression != nullptr){
        performTypeChecking(*declarativeStatement.m_expression, expectedType);
    }
    m_symbolTableHandler.updateSymbolTable(declarativeStatement);
}

void Analyzer::analyzeAssignmentStatement(ast::AssignmentStatement& assignmentStatement){
   Token& identifierToken = *assignmentStatement.m_identifier;
   Keyword dataType = findVariableType(identifierToken);
   performTypeChecking(*assignmentStatement.m_expression, dataType);
}


Keyword Analyzer::findFirstValueType(ast::Relational& relational){
    ast::Factor& factor = *relational.m_additive->m_term->m_factor;

    if(factor.operandType == ast::Factor::OperandType::VALUE){
        Token& value = *factor.operand.value;
        if(value.m_tokenType.type == Type::NUMERIC_LITERAL){
            
            if(value.m_tokenType.isFloatingPointValue){
                return Keyword::FLOAT;
            }
            return Keyword::INT;
        }else if(value.m_tokenType.type == Type::STRING_LITERAL){
            return Keyword::CHAR;
        }else if(value.m_tokenType.type == Type::IDENTIFIER){
            return findVariableType(value);
        }

    }else if(factor.operandType == ast::Factor::OperandType::EXPR){
        return findFirstValueType(*factor.operand.expression);
    }else if(factor.operandType == ast::Factor::OperandType::FUNCTION_CALL){
        return analyzeFunctionCallStatement(*factor.operand.functionCall);
    }
    return Keyword::NIL;
}

Keyword Analyzer::findFirstValueType(ast::Expression& expression){
    return findFirstValueType(*expression.m_relational);
}


void Analyzer::analyzeConditionalStatement(ast::ConditionalStatement& conditionalStatement, ast::Function& currentFunction){
    if(conditionalStatement.m_expr != nullptr){
        ast::Expression& expr = *conditionalStatement.m_expr;
        Keyword expectedType = findFirstValueType(expr);
        performTypeChecking(expr, expectedType);
    }
    analyzeNestedScope(conditionalStatement.m_stmnts, currentFunction);
    if(conditionalStatement.m_else != nullptr){
        analyzeConditionalStatement(*conditionalStatement.m_else, currentFunction);
    }
}   

void Analyzer::analyzeNestedScope(std::list<ast::Statement*> stmnts, ast::Function& currentFunction){
    m_symbolTableHandler.createSymbolTable();
    for(auto statement: stmnts){
        analyzeStatement(*statement, currentFunction);
    }
    m_symbolTableHandler.popSymbolTabe();
}

void Analyzer::analyzeReturnStatement(ast::ReturnStatement& returnStatement, ast::Function& currentFunction){
    if(returnStatement.m_expr == nullptr){
        return;
    }
    Keyword expectedType = currentFunction.m_returnType->m_tokenType.keywordType;
    performTypeChecking(*returnStatement.m_expr, expectedType);
}

void Analyzer::analyzeWhileLoop(ast::WhileLoop& whileLoop, ast::Function& currentFunction){
    ast::Expression& expr = *whileLoop.m_expr;
    Keyword expectedType = findFirstValueType(expr);
    performTypeChecking(expr, expectedType);

    analyzeNestedScope(whileLoop.m_stmnts, currentFunction);
}

Keyword Analyzer::analyzeFunctionCallStatement(ast::FunctionCallStatement& functionCallStatement){
    Token& functionIdentifier = *functionCallStatement.m_identifier;
    const std::string_view identifier(functionIdentifier.m_value, functionIdentifier.m_valueSize);
    auto result = m_symbolTableHandler.findFunctionSymbol(identifier);
    if(result.first == false){
        m_errorHandler.reportError(error::FUCTION_NOT_FOUND, functionIdentifier);
    }
    auto isArgsAndParamEqual = [&] (std::list<ast::Expression*>& args, std::vector<Keyword>& params)->bool{
        if(args.size() != params.size()){
            return false;
        }
        if(args.size() == 0 && params.size() == 0){
            return true;
        }
        std::vector<Keyword>::iterator param = params.begin();
        for(ast::Expression* expr : args){
            performTypeChecking(*expr, *param);
            param++;
        }
        return true;
    };
    if(!isArgsAndParamEqual(functionCallStatement.m_args, result.second.paramTypes)){
        m_errorHandler.reportError(error::ARGS_PARAM_ERROR, functionIdentifier);
    }
    return result.second.dataType;
}

void Analyzer::analyzeStatement(ast::Statement& statement, ast::Function& currentFunction){
    switch(statement.m_type){
        case ast::Statement::Type::CONDITIONAL:
            analyzeConditionalStatement(*statement.m_data.conditionalStatement, currentFunction);
            break;
        case ast::Statement::Type::DECLARATIVE:
            analyzeDeclarativeStatement(*statement.m_data.declarativeStatement);
            break;
        case ast::Statement::Type::ASSIGNMENT:
            analyzeAssignmentStatement(*statement.m_data.assignmentStatement);
            break;
        case ast::Statement::Type::FUNCTION_CALL:
            analyzeFunctionCallStatement(*statement.m_data.functionalCallStatement);
            break;
        case ast::Statement::Type::RETURN:
            analyzeReturnStatement(*statement.m_data.returnStatement, currentFunction);
            break;
        case ast::Statement::Type::WHILE_LOOP:
            analyzeWhileLoop(*statement.m_data.whileLoop, currentFunction);
            break;
        }
}

void Analyzer::performTypeChecking(ast::Factor& factor, Keyword expectedDataType){

    auto checkIfValueTypeIsCompatible = [&](Token& value, Keyword expectedType){
        if(value.m_tokenType.type == Type::NUMERIC_LITERAL){
            if(value.m_tokenType.isFloatingPointValue && expectedType != Keyword::FLOAT){
                m_errorHandler.reportError(error::INVALID_EXPR, value);
            }else if(!value.m_tokenType.isFloatingPointValue && expectedDataType != Keyword::INT){
                m_errorHandler.reportError(error::INVALID_EXPR, value);
            }
        }else if(value.m_tokenType.type == Type::STRING_LITERAL){
            if(expectedType != Keyword::CHAR){
                m_errorHandler.reportError(error::INVALID_EXPR, value);
            }
            if(value.m_valueSize > 1){
                m_errorHandler.reportError(error::CHAR_LENGTH_EXCEED, value);
            }
        }else if(value.m_tokenType == Type::IDENTIFIER){
            Keyword dataType = findVariableType(value);
            if(dataType != expectedDataType){
                m_errorHandler.reportError(error::INVALID_EXPR, value);
            }
        }
    };
    switch(factor.operandType){
        case ast::Factor::OperandType::VALUE:
            checkIfValueTypeIsCompatible(*factor.operand.value, expectedDataType);
            break;
        case ast::Factor::OperandType::EXPR:
            performTypeChecking(*factor.operand.expression, expectedDataType);
            break;
        case ast::Factor::OperandType::FUNCTION_CALL:
            Keyword functionReturnType = analyzeFunctionCallStatement(*factor.operand.functionCall);
            if(functionReturnType != expectedDataType){
                m_errorHandler.reportError(error::UNEXPECTED_RETURN, *factor.operand.functionCall->m_identifier);
            }
            break;
    }
}

Keyword Analyzer::findVariableType(Token& identifier){
    std::string_view varName(identifier.m_value, identifier.m_valueSize);
    auto symbolEntry = m_symbolTableHandler.findVariableSymbol(varName);
    if(symbolEntry.first == false){
        m_errorHandler.reportError(error::VARIABLE_NOT_FOUND, identifier);
    }
    return symbolEntry.second.dataType;
}

void Analyzer::performTypeChecking(ast::DeclarativeStatement& declarativeStatement){
    Token& dataTypeToken = *declarativeStatement.m_dataType;
    if(declarativeStatement.m_isInitialized){
        performTypeChecking(*declarativeStatement.m_expression, dataTypeToken.m_tokenType.keywordType);
    }
}

void Analyzer::performTypeChecking(ast::Expression& expression, Keyword expectedDataType){
    performTypeChecking(*expression.m_relational, expectedDataType);

    ast::ExpressionTail* exprTail = expression.m_expressionTail;
    while(exprTail != nullptr){
        performTypeChecking(*exprTail->m_relational, expectedDataType);
        exprTail = exprTail->m_expressionTail;
    }
}

void Analyzer::performTypeChecking(ast::Relational& relational, Keyword expectedDataType){
    performTypeChecking(*relational.m_additive, expectedDataType);

    ast::RelationalTail* relationalTail = relational.m_relationalTail;
    while(relationalTail != nullptr){
        performTypeChecking(*relationalTail->m_additive, expectedDataType);
        relationalTail = relationalTail->m_relationalTail;
    }
}

void Analyzer::performTypeChecking(ast::Additive& additive, Keyword expectedDataType){
    performTypeChecking(*additive.m_term, expectedDataType);

    ast::AdditiveTail* additiveTail = additive.m_additiveTail;
    while(additiveTail != nullptr){
        performTypeChecking(*additiveTail->m_term, expectedDataType);
        additiveTail = additiveTail->m_additiveTail;
    }
}

void Analyzer::performTypeChecking(ast::Term& term, Keyword expectedDataType){
    performTypeChecking(*term.m_factor, expectedDataType);

    ast::TermTail* termTail = term.m_termTail;
    while(termTail != nullptr){
        performTypeChecking(*termTail->m_factor, expectedDataType);
        termTail = termTail->m_termTail;
    }
}