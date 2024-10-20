#include <gtest/gtest.h>
#include <Tokenizer.hpp>
#include <Token.hpp>

Tokenizer tokenizer("sugham");

// TEST(TokenizerTest, checkIdentifierToken){
//     Token token = tokenizer.nextToken();
//     for(int i=0;i<token.valueSize;i++){
//         std::cout << token.value[i];
//     }
//     EXPECT_EQ(TokenType(TokenType::IDENTIFIER), token.tokenType);
// }

// TEST(TokenizerTest, checkKeywordToken){
//     TokenData tokenData = tokenizer.nextToken();
//     EXPECT_EQ(TokenType(TokenType::KEYWORD, TokenType::INT), tokenData.token.tokenType);
// }

// TEST(TokenizerTest, checkStringToken){
//     TokenData tokenData = tokenizer.nextToken();
//     for(int i=0;i<tokenData.token.valueSize;i++){
//         std::cout << tokenData.token.value[i];
//     }
//     EXPECT_EQ(TokenType(TokenType::STRING_LITERAL), tokenData.token.tokenType);
// }


// TEST(TokenizerTest, checkSymbolToken){
//     TokenData tokenData = tokenizer.nextToken();
//     EXPECT_EQ(TokenType(TokenType::SYMBOL, '*'), tokenData.token.tokenType);
// }
