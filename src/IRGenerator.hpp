#pragma once
#include "AST.hpp"
#include "SymbolTableHandler.hpp"
#include <filesystem>
#include <llvm-18/llvm/IR/BasicBlock.h>
#include <llvm-18/llvm/IR/Function.h>
#include <llvm-18/llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <memory>
#include <unordered_map>

class IRGenerator{

public:
    virtual void generate(const ast::File& syntaxTree) = 0;
    virtual void saveToFile(const std::filesystem::path& outputFile) = 0;
};

class LlvmIRGenerator: public IRGenerator{

public:
    LlvmIRGenerator(const std::string& modulename);

    void generate(const ast::File& syntaxTree) override;
    void saveToFile(const std::filesystem::path& outputfile) override;

private:
    void init(const std::string& inputFile);
    void includeStandardLibFuncPrototype();
    llvm::Function* genFunction(ast::Function& function);
    llvm::Type* getType(Token& typeToken);
    llvm::Type* getType(Keyword type);
    llvm::Value* computeExpression(ast::Expression& expr);
    llvm::Value* computeRelational(ast::Relational& relational);
    llvm::Value* computeAdditive(ast::Additive& additive);
    llvm::Value* computeTerm(ast::Term& term);
    llvm::Value* getFactor(ast::Factor& factor);

    void genInstruction(ast::Statement& statement);
    void importFiles(std::list<Token*>& importPackages);
    void genInstruction(ast::DeclarativeStatement& declarativeStatement);
    void genInstruction(ast::AssignmentStatement& declarativeStatement);
    void genInstruction(ast::ReturnStatement& returnStatement);
    void genInstruction(ast::ConditionalStatement& conditionalStatement);
    void genInstruction(ast::WhileLoop& whileLoop);
    llvm::Value* genInstruction(ast::FunctionCallStatement& functionCallStatement);

    std::unordered_map<std::string_view, llvm::AllocaInst*> m_variables;    

    std::unique_ptr<llvm::Module> m_module;
    std::unique_ptr<llvm::IRBuilder<>> m_IRBuilder;

    static std::unique_ptr<llvm::LLVMContext> llvmContext;
    static llvm::Type* intType;
    static llvm::Type* charType;
    static llvm::Type* floatType;
    static llvm::Type* voidType;
};