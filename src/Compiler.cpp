#include "Compiler.hpp"
#include "AST.hpp"
#include "Analyzer.hpp"
#include "ErrorHandler.hpp"
#include <cstdio>
#include "IRGenerator.hpp"
#include "Parser.hpp"
#include "Tokenizer.hpp"

Compiler::Compiler(IRGenerator& irGenerator) : m_irGenerator(irGenerator){

}

void Compiler::compileToIR(const std::filesystem::path& srcFilepath, const std::filesystem::path& outputFilepath){

    ast::File syntaxTree = generateAST(srcFilepath);
    performIRGeneration(syntaxTree, srcFilepath.filename().string());
    m_irGenerator.saveToFile(outputFilepath);
}

void Compiler::buildExec(const std::string& irFilePath, const std::string& outputfile,  Platform platform){
    std::string tempObj = outputfile+".o";
    std::string compileToObjCommand = "llc -filetype=obj "+irFilePath+" -o "+tempObj;
    std::string linkCommand;
    
    if(platform == Platform::WIN){
        linkCommand = "lld-link "+tempObj+" "+commonLibs+" "+winLibs+" -o "+outputfile;
    }else if(platform == Platform::LINUX){
        linkCommand = "ld.lld "+tempObj+" "+commonLibs+" "+linuxLibs+" -o "+outputfile;
    }
    system(compileToObjCommand.c_str());
    system(linkCommand.c_str());
    remove(tempObj.c_str());
}

ast::File Compiler::generateAST(const std::filesystem::path& srcFilepath){

    std::ifstream srcFile(srcFilepath.string());

    if(!srcFile){
        std::cerr << "Could not open file : "+ srcFilepath.string() << std::endl;
    }
    const ErrorHandler errorHandler(srcFile);
    Tokenizer tokenizer(srcFile, errorHandler);
    Parser parser(tokenizer, errorHandler);
    ast::File syntaxTree = parser.evaluate();
    Analyzer analyzer(syntaxTree, errorHandler);
    analyzer.analyze();
    return syntaxTree;
}

void Compiler::performIRGeneration(const ast::File& file, const std::string& filename){
    m_files.push_back(filename);
    for(auto package: file.importPackages){
        std::string packagePathStr(package->m_value, package->m_valueSize);
        std::filesystem::path packagePath(packagePathStr);
        std::string packageName = packagePath.filename().string();

        auto searchResult = std::find(m_files.begin(), m_files.end(), packageName);
        if(searchResult != m_files.end()){
            continue;
        }
        ast::File syntaxTree = generateAST(packagePathStr);
        performIRGeneration(syntaxTree, packageName);
    }
    m_irGenerator.generate(file);
}