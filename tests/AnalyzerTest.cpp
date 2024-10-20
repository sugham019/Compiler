#include <Token.hpp>
#include <Tokenizer.hpp>
#include <gtest/gtest.h>
#include <Parser.hpp>
#include <AST.hpp>
#include <iterator>
#include <sys/types.h>
#include <Analyzer.hpp>

Tokenizer tokenizer("testfile");
Parser parser(tokenizer);

TEST(AnalyzerTest, sample){
    ast::File syntaxTree = parser.evaluate();
    Analyzer analyzer(syntaxTree);
    analyzer.analyze();
}