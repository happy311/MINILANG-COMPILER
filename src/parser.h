// parser.h - Stage 2: turns tokens into an AST (recursive descent parser).
//
// Grammar:
//   program    := statement*
//   statement  := "let" IDENT "=" expr ";"
//               | IDENT "=" expr ";"
//               | "print" expr ";"
//               | "if" "(" expr ")" block ( "else" block )?
//               | "while" "(" expr ")" block
//   block      := "{" statement* "}"
//   expr       := additive ( ("<"|">"|"<="|">="|"=="|"!=") additive )?
//   additive   := term ( ("+"|"-") term )*
//   term       := unary ( ("*"|"/") unary )*
//   unary      := "-" unary | primary
//   primary    := NUMBER | IDENT | "(" expr ")"
#pragma once

#include <vector>

#include "ast.h"
#include "lexer.h"

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    // Parses the whole program. Throws std::runtime_error on syntax errors.
    std::vector<StmtPtr> parseProgram();

private:
    // token helpers
    const Token& peek() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& expect(TokenType type, const std::string& what);
    [[noreturn]] void error(const std::string& message) const;

    // statements
    StmtPtr parseStatement();
    std::vector<StmtPtr> parseBlock();

    // expressions (one function per precedence level)
    ExprPtr parseExpression();
    ExprPtr parseAdditive();
    ExprPtr parseTerm();
    ExprPtr parseUnary();
    ExprPtr parsePrimary();

    std::vector<Token> tokens_;
    size_t current_ = 0;
};
