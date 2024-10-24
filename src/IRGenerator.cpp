#include "IRGenerator.hpp"
#include "AST.hpp"
#include "ErrorHandler.hpp"
#include "Token.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Argument.h>
#include <string>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LLVMContext.h>
#include <memory>
#include <string>

std::unique_ptr<llvm::LLVMContext> LlvmIRGenerator::llvmContext;
llvm::Type* LlvmIRGenerator::intType;
llvm::Type* LlvmIRGenerator::charType;
llvm::Type* LlvmIRGenerator::floatType;
llvm::Type* LlvmIRGenerator::voidType;

LlvmIRGenerator::LlvmIRGenerator(const std::string& moduleName){
    init(moduleName);
    includeStandardLibFuncPrototype();
}

void LlvmIRGenerator::init(const std::string& moduleName){
    static bool isInitialized = false;
    if(!isInitialized){
        llvmContext = std::make_unique<llvm::LLVMContext>();
        intType = llvm::Type::getInt32Ty(*llvmContext);
        charType = llvm::Type::getInt8Ty(*llvmContext);
        floatType = llvm::Type::getFloatTy(*llvmContext);
        voidType = llvm::Type::getVoidTy(*llvmContext);
        isInitialized = true;
    }
    m_module =  std::make_unique<llvm::Module>(moduleName, *llvmContext);
    m_IRBuilder = std::make_unique<llvm::IRBuilder<>>(*llvmContext);
}

void LlvmIRGenerator::includeStandardLibFuncPrototype(){
    for(const auto& pair: SymbolTableHandler::standardLibFuncSymbols){
        const SymbolTableEntry& symbol = pair.second;
        llvm::Type* returnType = getType(symbol.dataType);
        std::vector<llvm::Type*> paramsType;
        for(Keyword paramType : symbol.paramTypes){
            llvm::Type* type = getType(paramType);
            paramsType.push_back(type);
        }
        llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramsType, false);
        llvm::Function* functionPrototype = llvm::Function::Create(funcType, llvm::GlobalValue::ExternalLinkage, pair.first, *m_module);
    }
}

void LlvmIRGenerator::saveToFile(const std::filesystem::path& outputFile){

    std::error_code error;
    llvm::raw_fd_ostream output(outputFile.string(), error);
    m_module->print(output, nullptr);
    if(error){
        std::cerr << "Could not open file : "+ outputFile.string() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void LlvmIRGenerator::generate(const ast::File& syntaxTree){
    for(ast::Function* function: syntaxTree.functions){
        genFunction(*function);
    }
}

llvm::Type* LlvmIRGenerator::getType(Token& typeToken){
    Keyword type = typeToken.m_tokenType.keywordType;
    return getType(type);
}

llvm::Type* LlvmIRGenerator::getType(Keyword type){
    switch(type){
        case Keyword::INT:
            return intType;
        case Keyword::FLOAT:
            return floatType;
        case Keyword::CHAR:
            return charType;
        default:
            return voidType;
    }
    return nullptr;
}

llvm::Value* LlvmIRGenerator::getFactor(ast::Factor& factor){
    
    auto fetchLiteralValue= [&](Token& token)->llvm::Value*{
        llvm::Type* type = getType(token);  
        if(token.m_tokenType == Type::STRING_LITERAL){
            return llvm::ConstantInt::get(charType, token.m_value[0]);
        }else if(token.m_tokenType == Type::NUMERIC_LITERAL){
            std::string value(token.m_value, token.m_valueSize);
            if(token.m_tokenType.isFloatingPointValue){
                float convertedValue = std::stof(value);
                return llvm::ConstantFP::get(floatType, convertedValue);
            }
            int convertedValue = std::stoi(value);
            return llvm::ConstantInt::get(intType, convertedValue);
        }
        return nullptr;
    };

    switch(factor.operandType){

        case ast::Factor::OperandType::VALUE:
            {
                Token& valueToken = *factor.operand.value;
                if(valueToken.m_tokenType == Type::IDENTIFIER){
                    std::string_view varName(valueToken.m_value, valueToken.m_valueSize);
                    auto it = m_variables.find(varName);
                    llvm::Type* type = it->second->getAllocatedType();
                    llvm::LoadInst* loadValue = m_IRBuilder->CreateLoad(type, it->second, "loadValue");
                    return loadValue;
                }
                llvm::Value* value = fetchLiteralValue(valueToken);
                return fetchLiteralValue(valueToken);
            }
        case ast::Factor::OperandType::EXPR:
            return computeExpression(*factor.operand.expression);
        case ast::Factor::OperandType::FUNCTION_CALL:
            return genInstruction(*factor.operand.functionCall);
    }
    return nullptr;
}

llvm::Value* LlvmIRGenerator::computeTerm(ast::Term& term){
    llvm::Value* lhs = getFactor(*term.m_factor);
    ast::TermTail* termTail = term.m_termTail;
    bool isFloat = false;
    if(lhs->getType()->isFloatingPointTy()){
        isFloat = true;
    }
    while(termTail != nullptr){
        llvm::Value* rhs = getFactor(*termTail->m_factor);
        ast::Opcode opcode = termTail->m_opcode;
        if(opcode == ast::Opcode::MULTIPLICATION){
            if(isFloat)
                lhs = m_IRBuilder->CreateFMul(lhs, rhs);
            else
                lhs = m_IRBuilder->CreateSub(lhs, rhs);
        }else if(opcode == ast::Opcode::DIVISION){
            if(isFloat)
                lhs = m_IRBuilder->CreateFDiv(lhs, rhs);
            else
                lhs = m_IRBuilder->CreateSDiv(lhs, rhs);
        }
        termTail = termTail->m_termTail;
    }
    return lhs;
}

llvm::Value* LlvmIRGenerator::computeAdditive(ast::Additive& additive){
    llvm::Value* lhs = computeTerm(*additive.m_term);
    ast::AdditiveTail* additiveTail = additive.m_additiveTail;
    bool isFloat = false;
    if(lhs->getType()->isFloatingPointTy()){
        isFloat = true;
    }
    while(additiveTail != nullptr){
        llvm::Value* rhs = computeTerm(*additiveTail->m_term);
        ast::Opcode opcode = additiveTail->m_opcode;
        if(opcode == ast::Opcode::ADDITION){
            if(isFloat)
                lhs = m_IRBuilder->CreateFAdd(lhs, rhs);
            else
                lhs = m_IRBuilder->CreateAdd(lhs, rhs);
            
        }else if(opcode == ast::Opcode::SUBTRACTION){
            if(isFloat)
                lhs = m_IRBuilder->CreateFSub(lhs, rhs);
            else
                lhs = m_IRBuilder->CreateSub(lhs, rhs);
            
        }
        additiveTail = additiveTail->m_additiveTail;
    }
    return lhs;
}

llvm::Value* LlvmIRGenerator::computeRelational(ast::Relational& relational){
    llvm::Value* lhs = computeAdditive(*relational.m_additive);
    ast::RelationalTail* relationalTail = relational.m_relationalTail;
    while(relationalTail != nullptr){
        llvm::Value* rhs = computeAdditive(*relationalTail->m_additive);
        ast::Opcode opcode = relationalTail->m_opcode;
        switch (opcode) {
            case ast::Opcode::GREATER_THAN:
                lhs = m_IRBuilder->CreateICmpSGT(lhs, rhs);
                break;
            case ast::Opcode::SMALLER_THAN:
                lhs = m_IRBuilder->CreateICmpSLT(lhs, rhs);
                break;
            case ast::Opcode::GREATER_OR_EQUAL:
                lhs = m_IRBuilder->CreateICmpSGE(lhs, rhs);
                break;
            case ast::Opcode::SMALLER_OR_EQUAL:
                lhs = m_IRBuilder->CreateICmpSLE(lhs, rhs);
                break;
            case ast::Opcode::EQUAL_TO:
                lhs = m_IRBuilder->CreateICmpEQ(lhs, rhs);
                break;
            default:
                break;
        }
        relationalTail = relationalTail->m_relationalTail;
    }
    return lhs;
}

llvm::Value* LlvmIRGenerator::computeExpression(ast::Expression& expression){
    llvm::Value* lhs = computeRelational(*expression.m_relational);
    ast::ExpressionTail* exprTail = expression.m_expressionTail;
    while(exprTail != nullptr){
        llvm::Value* rhs = computeRelational(*exprTail->m_relational);
        ast::Opcode opcode = exprTail->m_opcode;
        if(opcode == ast::Opcode::LOGICAL_AND){
            lhs = m_IRBuilder->CreateAnd(lhs, rhs);
        }else if(opcode == ast::Opcode::LOGICAL_OR){
            lhs = m_IRBuilder->CreateOr(lhs, rhs);
        }
        exprTail = exprTail->m_expressionTail;
    }
    return lhs;
}


void LlvmIRGenerator::genInstruction(ast::DeclarativeStatement& declarativeStatement){
    llvm::Type* dataType = getType(*declarativeStatement.m_dataType);
    std::string_view varIdentifier(declarativeStatement.m_identifier->m_value, declarativeStatement.m_identifier->m_valueSize);
    llvm::AllocaInst* variable = m_IRBuilder->CreateAlloca(dataType, nullptr, varIdentifier);
    if(declarativeStatement.m_isInitialized){
        llvm::Value* value = computeExpression(*declarativeStatement.m_expression);
        m_IRBuilder->CreateStore(value, variable);
    }
    m_variables.insert({varIdentifier, variable});
}

void LlvmIRGenerator::genInstruction(ast::AssignmentStatement& assignmentStatement){
    std::string_view varIdentifier(assignmentStatement.m_identifier->m_value, assignmentStatement.m_identifier->m_valueSize);
    auto var = m_variables.find(varIdentifier);
    llvm::Value* value = computeExpression(*assignmentStatement.m_expression);
    m_IRBuilder->CreateStore(value, var->second);
}

void LlvmIRGenerator::genInstruction(ast::ReturnStatement& returnStatment){
    if(returnStatment.m_expr == nullptr){
        m_IRBuilder->CreateRetVoid();
        return;
    }
    llvm::Value* value = computeExpression(*returnStatment.m_expr);
    m_IRBuilder->CreateRet(value);
}

 llvm::Value* LlvmIRGenerator::genInstruction(ast::FunctionCallStatement& functionCallStatement){
    std::string_view functionName(functionCallStatement.m_identifier->m_value, functionCallStatement.m_identifier->m_valueSize);
    llvm::Function* function = m_module->getFunction(functionName);
    if(functionCallStatement.m_args.empty()){
        return m_IRBuilder->CreateCall(function);
    }
    std::vector<llvm::Value*> params;
    for(ast::Expression* arg: functionCallStatement.m_args){
        llvm::Value* argValue = computeExpression(*arg);
        params.push_back(argValue);
    }
    llvm::Value* value = m_IRBuilder->CreateCall(function, params);
    return value;
 }

 void LlvmIRGenerator::genInstruction(ast::ConditionalStatement& conditionalStatement, llvm::BasicBlock* finalBlock){
    llvm::Value* conditionExpr = computeExpression(*conditionalStatement.m_expr);
    llvm::BasicBlock* initialBlock = m_IRBuilder->GetInsertBlock();
    llvm::Function* currentFunc = initialBlock->getParent();
    llvm::BasicBlock* onTrue = llvm::BasicBlock::Create(*llvmContext, "onTrue", currentFunc);
    if(finalBlock== nullptr)
        finalBlock = llvm::BasicBlock::Create(*llvmContext, "finalBlock", currentFunc);

    auto fillInstructions = [&](std::list<ast::Statement*>& stmnts){
        bool hasReturnStatement = false;
        for(ast::Statement* stmnt: stmnts){
            genInstruction(*stmnt);
            if(stmnt->m_type == ast::Statement::Type::RETURN){
                hasReturnStatement = true;
                break;
            };
        }
        if(!hasReturnStatement){
            m_IRBuilder->CreateBr(finalBlock);
        }
    };
    m_IRBuilder->SetInsertPoint(onTrue);
    fillInstructions(conditionalStatement.m_stmnts);

    m_IRBuilder->SetInsertPoint(initialBlock);
    if(conditionalStatement.m_else != nullptr){
        llvm::BasicBlock* onFalse = llvm::BasicBlock::Create(*llvmContext, "onFalse", currentFunc);
        m_IRBuilder->CreateCondBr(conditionExpr, onTrue, onFalse);
        m_IRBuilder->SetInsertPoint(onFalse);
        if(conditionalStatement.m_else->m_expr != nullptr){
            genInstruction(*conditionalStatement.m_else, finalBlock);
        }else{
            fillInstructions(conditionalStatement.m_else->m_stmnts);
        }
        m_IRBuilder->SetInsertPoint(finalBlock);
        return;

    }
    m_IRBuilder->CreateCondBr(conditionExpr, onTrue, finalBlock);
    m_IRBuilder->SetInsertPoint(finalBlock);
}

void LlvmIRGenerator::genInstruction(ast::WhileLoop& whileLoop){
    llvm::Value* conditionalExpr = computeExpression(*whileLoop.m_expr);
    llvm::BasicBlock* currentBlock = m_IRBuilder->GetInsertBlock();
    llvm::Function* currentFunc = currentBlock->getParent();
    llvm::BasicBlock* loopBlock = llvm::BasicBlock::Create(*llvmContext, "loop", currentFunc);
    llvm::BasicBlock* finalBlock = llvm::BasicBlock::Create(*llvmContext, "final", currentFunc);

    m_IRBuilder->CreateCondBr(conditionalExpr, loopBlock, finalBlock);

    m_IRBuilder->SetInsertPoint(loopBlock);
    bool hasReturnStatement = false;
    for(ast::Statement* stmnt: whileLoop.m_stmnts){
        genInstruction(*stmnt);
        if(stmnt->m_type == ast::Statement::Type::RETURN){
            hasReturnStatement = true;
            break;
        }
    }
    if(!hasReturnStatement){
        conditionalExpr = computeExpression(*whileLoop.m_expr);
        m_IRBuilder->CreateCondBr(conditionalExpr, loopBlock, finalBlock);
    }
    m_IRBuilder->SetInsertPoint(finalBlock); 
}

void LlvmIRGenerator::genInstruction(ast::Statement& statement){
    switch (statement.m_type) {

        case ast::Statement::Type::DECLARATIVE:
            genInstruction(*statement.m_data.declarativeStatement);
            break;
        case ast::Statement::Type::ASSIGNMENT:
            genInstruction(*statement.m_data.assignmentStatement);
            break;
        case ast::Statement::Type::CONDITIONAL:
            genInstruction(*statement.m_data.conditionalStatement, nullptr);
            break;
        case ast::Statement::Type::FUNCTION_CALL:
            genInstruction(*statement.m_data.functionalCallStatement);
            break;
        case ast::Statement::Type::RETURN:
            genInstruction(*statement.m_data.returnStatement);
            break;
        case ast::Statement::Type::WHILE_LOOP:
            genInstruction(*statement.m_data.whileLoop);
            break;
    };
}

llvm::Function* LlvmIRGenerator::genFunction(ast::Function& function){
    const std::string identifier(function.m_identifier->m_value, function.m_identifier->m_valueSize);
    llvm::Type* returnType = getType(*function.m_returnType);
    llvm::FunctionType* funcType;
    if(function.m_parameters.empty()){
        funcType = llvm::FunctionType::get(returnType, false);
    }else{
        std::vector<llvm::Type*> paramsType;
        for(ast::Parameter param : function.m_parameters){
            llvm::Type* paramType = getType(*param.m_dataType);
            paramsType.push_back(paramType);
        }
        funcType = llvm::FunctionType::get(returnType, llvm::ArrayRef<llvm::Type*>(paramsType), false);
    }
    llvm::Function* func = llvm::Function::Create(funcType, llvm::GlobalValue::ExternalLinkage, identifier, *m_module);

    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*llvmContext, "entry", func);
    m_IRBuilder->SetInsertPoint(entryBlock);
    std::list<ast::Parameter>::iterator param = function.m_parameters.begin();
    for(llvm::Argument& arg: func->args()){
        std::string_view argName(param->m_identifier->m_value, param->m_identifier->m_valueSize);
        llvm::Type* type = arg.getType();
        llvm::AllocaInst* variable = m_IRBuilder->CreateAlloca(type, nullptr, argName); 
        
        m_IRBuilder->CreateStore(&arg, variable); 
        m_variables.insert({argName, variable});
        param++;
    }
    for(ast::Statement* statement: function.m_statements){
        genInstruction(*statement);
        if(statement->m_type == ast::Statement::Type::RETURN){
            break;
        }
    }
    m_variables.clear();
    return func;
}