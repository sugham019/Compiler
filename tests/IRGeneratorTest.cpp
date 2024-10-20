#include "Analyzer.hpp"
#include <Token.hpp>
#include <Tokenizer.hpp>
#include <gtest/gtest.h>
#include <Parser.hpp>
#include <AST.hpp>
#include <sys/types.h>
#include <IRGenerator.hpp>

class IRTestFixture : public testing::Test{

protected:
    ast::File syntaxTree;

public:
    IRTestFixture(){
        Tokenizer tokenizer("testfile");
        Parser parser(tokenizer);
        syntaxTree = parser.evaluate();
        Analyzer analyzer(syntaxTree);
        analyzer.analyze();
    }

};

TEST_F(IRTestFixture, main){
    LlvmIRGenerator llvmIRGenerator("main"); 
    llvmIRGenerator.generate(syntaxTree);
    llvmIRGenerator.saveToFile("outputfile");
}