// lexer.h - Stage 1: turns source text into a list of tokens.
#pragma once

#include <string>
#include <vector>

#include "ast.h"

enum class TokenType {
    // literals and names
    Number, Identifier,
    // keywords
    Let, Print, If, Else, While,
    // arithmetic
    Plus, Minus, Star, Slash,
    // punctuation
    LParen, RParen, LBrace, RBrace, Semicolon,
    // assignment and comparisons
    Assign, Lt, Gt, Le, Ge, EqEq, NotEq,
    // end of input
    End
};

struct Token {
    TokenType type = TokenType::End;
    std::string text;
    Value number = 0;  // only for Number tokens
    int line = 1;
};

class Lexer {
public:
    explicit Lexer(std::string source);

    // Converts the whole source into tokens (last token is always End).
    // Throws std::runtime_error on an unknown character.
    std::vector<Token> tokenize();

private:
    char peek(size_t offset = 0) const;
    void skipWhitespaceAndComments();
    Token nextToken();
    Token readNumber();
    Token readWord();
    Token makeToken(TokenType type, const std::string& text) const;

    std::string source_;
    size_t pos_ = 0;
    int line_ = 1;
};
