#include "lexer.h"

#include <cctype>
#include <stdexcept>
#include <unordered_map>

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

char Lexer::peek(size_t offset) const {
    size_t index = pos_ + offset;
    if (index < source_.size()) {
        return source_[index];
    }
    return '\0';
}

void Lexer::skipWhitespaceAndComments() {
    while (pos_ < source_.size()) {
        char c = peek();
        if (c == '\n') {
            line_++;
            pos_++;
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            pos_++;
        } else if (c == '/' && peek(1) == '/') {
            // A comment runs until the end of the line.
            while (pos_ < source_.size() && peek() != '\n') {
                pos_++;
            }
        } else {
            break;
        }
    }
}

Token Lexer::makeToken(TokenType type, const std::string& text) const {
    Token token;
    token.type = type;
    token.text = text;
    token.line = line_;
    return token;
}

Token Lexer::readNumber() {
    Value value = 0;
    std::string text;
    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        text += peek();
        value = value * 10 + (peek() - '0');
        pos_++;
    }
    Token token = makeToken(TokenType::Number, text);
    token.number = value;
    return token;
}

Token Lexer::readWord() {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"let", TokenType::Let},
        {"print", TokenType::Print},
        {"if", TokenType::If},
        {"else", TokenType::Else},
        {"while", TokenType::While},
    };

    std::string text;
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        text += peek();
        pos_++;
    }

    auto it = keywords.find(text);
    if (it != keywords.end()) {
        return makeToken(it->second, text);
    }
    return makeToken(TokenType::Identifier, text);
}

Token Lexer::nextToken() {
    char c = peek();

    if (std::isdigit(static_cast<unsigned char>(c))) {
        return readNumber();
    }
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return readWord();
    }

    pos_++;  // consume the character; peek() now looks at the next one
    switch (c) {
        case '+': return makeToken(TokenType::Plus, "+");
        case '-': return makeToken(TokenType::Minus, "-");
        case '*': return makeToken(TokenType::Star, "*");
        case '/': return makeToken(TokenType::Slash, "/");
        case '(': return makeToken(TokenType::LParen, "(");
        case ')': return makeToken(TokenType::RParen, ")");
        case '{': return makeToken(TokenType::LBrace, "{");
        case '}': return makeToken(TokenType::RBrace, "}");
        case ';': return makeToken(TokenType::Semicolon, ";");
        case '<':
            if (peek() == '=') {
                pos_++;
                return makeToken(TokenType::Le, "<=");
            }
            return makeToken(TokenType::Lt, "<");
        case '>':
            if (peek() == '=') {
                pos_++;
                return makeToken(TokenType::Ge, ">=");
            }
            return makeToken(TokenType::Gt, ">");
        case '=':
            if (peek() == '=') {
                pos_++;
                return makeToken(TokenType::EqEq, "==");
            }
            return makeToken(TokenType::Assign, "=");
        case '!':
            if (peek() == '=') {
                pos_++;
                return makeToken(TokenType::NotEq, "!=");
            }
            break;
        default:
            break;
    }

    throw std::runtime_error("line " + std::to_string(line_) +
                             ": unexpected character '" + std::string(1, c) + "'");
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        skipWhitespaceAndComments();
        if (pos_ >= source_.size()) {
            break;
        }
        tokens.push_back(nextToken());
    }
    tokens.push_back(makeToken(TokenType::End, ""));
    return tokens;
}
