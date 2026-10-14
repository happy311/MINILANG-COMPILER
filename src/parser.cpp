#include "parser.h"

#include <stdexcept>

namespace {

ExprPtr makeNumber(Value value, int line) {
    ExprPtr expr = std::make_unique<Expr>();
    expr->kind = Expr::Kind::Number;
    expr->number = value;
    expr->line = line;
    return expr;
}

ExprPtr makeVariable(const std::string& name, int line) {
    ExprPtr expr = std::make_unique<Expr>();
    expr->kind = Expr::Kind::Variable;
    expr->name = name;
    expr->line = line;
    return expr;
}

ExprPtr makeBinary(BinaryOp op, ExprPtr lhs, ExprPtr rhs, int line) {
    ExprPtr expr = std::make_unique<Expr>();
    expr->kind = Expr::Kind::Binary;
    expr->op = op;
    expr->lhs = std::move(lhs);
    expr->rhs = std::move(rhs);
    expr->line = line;
    return expr;
}

}  // namespace

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

// ---------- token helpers ----------

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::advance() {
    const Token& token = tokens_[current_];
    if (token.type != TokenType::End) {
        current_++;
    }
    return token;
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::expect(TokenType type, const std::string& what) {
    if (!check(type)) {
        error("expected " + what);
    }
    return advance();
}

void Parser::error(const std::string& message) const {
    std::string found = peek().type == TokenType::End ? "end of file" : "'" + peek().text + "'";
    throw std::runtime_error("line " + std::to_string(peek().line) + ": " +
                             message + ", found " + found);
}

// ---------- statements ----------

std::vector<StmtPtr> Parser::parseProgram() {
    std::vector<StmtPtr> program;
    while (!check(TokenType::End)) {
        program.push_back(parseStatement());
    }
    return program;
}

std::vector<StmtPtr> Parser::parseBlock() {
    expect(TokenType::LBrace, "'{'");
    std::vector<StmtPtr> statements;
    while (!check(TokenType::RBrace) && !check(TokenType::End)) {
        statements.push_back(parseStatement());
    }
    expect(TokenType::RBrace, "'}'");
    return statements;
}

StmtPtr Parser::parseStatement() {
    StmtPtr stmt = std::make_unique<Stmt>();
    stmt->line = peek().line;

    if (match(TokenType::Let)) {
        stmt->kind = Stmt::Kind::Let;
        stmt->name = expect(TokenType::Identifier, "variable name").text;
        expect(TokenType::Assign, "'='");
        stmt->expr = parseExpression();
        expect(TokenType::Semicolon, "';'");
    } else if (match(TokenType::Print)) {
        stmt->kind = Stmt::Kind::Print;
        stmt->expr = parseExpression();
        expect(TokenType::Semicolon, "';'");
    } else if (match(TokenType::If)) {
        stmt->kind = Stmt::Kind::If;
        expect(TokenType::LParen, "'('");
        stmt->expr = parseExpression();
        expect(TokenType::RParen, "')'");
        stmt->body = parseBlock();
        if (match(TokenType::Else)) {
            stmt->hasElse = true;
            stmt->elseBody = parseBlock();
        }
    } else if (match(TokenType::While)) {
        stmt->kind = Stmt::Kind::While;
        expect(TokenType::LParen, "'('");
        stmt->expr = parseExpression();
        expect(TokenType::RParen, "')'");
        stmt->body = parseBlock();
    } else if (check(TokenType::Identifier)) {
        stmt->kind = Stmt::Kind::Assign;
        stmt->name = advance().text;
        expect(TokenType::Assign, "'='");
        stmt->expr = parseExpression();
        expect(TokenType::Semicolon, "';'");
    } else {
        error("expected a statement");
    }
    return stmt;
}

// ---------- expressions ----------

// Lowest precedence: comparisons (not chainable).
ExprPtr Parser::parseExpression() {
    ExprPtr left = parseAdditive();

    BinaryOp op;
    if (check(TokenType::Lt)) {
        op = BinaryOp::Lt;
    } else if (check(TokenType::Gt)) {
        op = BinaryOp::Gt;
    } else if (check(TokenType::Le)) {
        op = BinaryOp::Le;
    } else if (check(TokenType::Ge)) {
        op = BinaryOp::Ge;
    } else if (check(TokenType::EqEq)) {
        op = BinaryOp::Eq;
    } else if (check(TokenType::NotEq)) {
        op = BinaryOp::Ne;
    } else {
        return left;
    }

    int line = advance().line;
    ExprPtr right = parseAdditive();
    return makeBinary(op, std::move(left), std::move(right), line);
}

ExprPtr Parser::parseAdditive() {
    ExprPtr left = parseTerm();
    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        const Token& opToken = advance();
        BinaryOp op = (opToken.type == TokenType::Plus) ? BinaryOp::Add : BinaryOp::Sub;
        int line = opToken.line;
        ExprPtr right = parseTerm();
        left = makeBinary(op, std::move(left), std::move(right), line);
    }
    return left;
}

ExprPtr Parser::parseTerm() {
    ExprPtr left = parseUnary();
    while (check(TokenType::Star) || check(TokenType::Slash)) {
        const Token& opToken = advance();
        BinaryOp op = (opToken.type == TokenType::Star) ? BinaryOp::Mul : BinaryOp::Div;
        int line = opToken.line;
        ExprPtr right = parseUnary();
        left = makeBinary(op, std::move(left), std::move(right), line);
    }
    return left;
}

// Unary minus is rewritten as (0 - x), so the rest of the compiler
// only needs to know about binary operators.
ExprPtr Parser::parseUnary() {
    if (check(TokenType::Minus)) {
        int line = advance().line;
        ExprPtr operand = parseUnary();
        return makeBinary(BinaryOp::Sub, makeNumber(0, line), std::move(operand), line);
    }
    return parsePrimary();
}

ExprPtr Parser::parsePrimary() {
    if (check(TokenType::Number)) {
        const Token& token = advance();
        return makeNumber(token.number, token.line);
    }
    if (check(TokenType::Identifier)) {
        const Token& token = advance();
        return makeVariable(token.text, token.line);
    }
    if (match(TokenType::LParen)) {
        ExprPtr inner = parseExpression();
        expect(TokenType::RParen, "')'");
        return inner;
    }
    error("expected a number, variable or '('");
}
