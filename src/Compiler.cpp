#include "Compiler.hpp"
#include "AST.hpp"
#include "ErrorHandler.hpp"
#include "IRGenerator.hpp"
#include "Parser.hpp"
#include "Tokenizer.hpp"

Compiler::Compiler(IRGenerator& irGenerator) : m_irGenerator(irGenerator){

}

void Compiler::compile(const std::filesystem::path& srcFilepath, const std::filesystem::path& outputFilepath){
    ast::File syntaxTree = generateAST(srcFilepath);
    performIRGeneration(syntaxTree, srcFilepath.filename().string());
    m_irGenerator.saveToFile(outputFilepath);
}

ast::File Compiler::generateAST(const std::filesystem::path& srcFilepath){
    Tokenizer tokenizer(srcFilepath.string());
    Parser parser(tokenizer);
    return parser.evaluate();
}

void Compiler::performIRGeneration(const ast::File& file, const std::string& filename){
    m_files.push_back(filename);
    for(auto package: file.importPackages){
        std::string packagePathStr(package->m_value, package->m_valueSize);
        std::filesystem::path packagePath(packagePathStr);
        std::string packageName = packagePath.filename().string();

        auto searchResult = std::find(m_files.begin(), m_files.end(), packageName);
        if(searchResult != m_files.end()){
            reportError("Cross dependency among files is not allowed", *package);
        }
        ast::File syntaxTree = generateAST(packagePathStr);
        performIRGeneration(syntaxTree, packageName);
    }
    m_irGenerator.generate(file);
}