#pragma once

#include <string>
#include <cstddef>

namespace alphaparser {

enum class TokenType {
    TOKEN_EOF = 0,

    // Literals & Identifiers
    TOKEN_NUMBER,     // e.g. 50, 3.14159, 0.05
    TOKEN_VAR,        // e.g. price, close, volume, open
    TOKEN_FUNC,       // e.g. SMA, EMA, RSI, STDDEV

    // Binary & Unary Operators
    TOKEN_OPERATOR,
    TOKEN_PLUS,       // +
    TOKEN_MINUS,      // -
    TOKEN_STAR,       // *
    TOKEN_SLASH,      // /
    
    // Comparisons
    TOKEN_GREATER,       // >
    TOKEN_LESS,          // <
    TOKEN_GREATER_EQUAL, // >=
    TOKEN_LESS_EQUAL,    // <=
    TOKEN_EQUAL_EQUAL,   // ==
    TOKEN_NOT_EQUAL,     // !=

    // Logical Operators
    TOKEN_LOGICAL_AND,   // &&
    TOKEN_LOGICAL_OR,    // ||
    TOKEN_LOGICAL_NOT,   // !

    // Delimiters
    TOKEN_LPAREN,     // (
    TOKEN_RPAREN,     // )
    TOKEN_COMMA       // ,
};

struct SourceLocation {
    std::size_t line{1};
    std::size_t column{1};
    std::size_t offset{0};

    [[nodiscard]] std::string toString() const {
        return "line " + std::to_string(line) + ", column " + std::to_string(column);
    }
};

struct Token {
    TokenType type{TokenType::TOKEN_EOF};
    std::string lexeme{};
    double numberValue{0.0};
    SourceLocation location{};

    Token() = default;

    Token(TokenType t, std::string lex, SourceLocation loc)
        : type(t), lexeme(std::move(lex)), numberValue(0.0), location(loc) {}

    Token(TokenType t, std::string lex, double numVal, SourceLocation loc)
        : type(t), lexeme(std::move(lex)), numberValue(numVal), location(loc) {}

    [[nodiscard]] bool isOperator() const noexcept {
        return type == TokenType::TOKEN_PLUS ||
               type == TokenType::TOKEN_MINUS ||
               type == TokenType::TOKEN_STAR ||
               type == TokenType::TOKEN_SLASH ||
               type == TokenType::TOKEN_GREATER ||
               type == TokenType::TOKEN_LESS ||
               type == TokenType::TOKEN_GREATER_EQUAL ||
               type == TokenType::TOKEN_LESS_EQUAL ||
               type == TokenType::TOKEN_EQUAL_EQUAL ||
               type == TokenType::TOKEN_NOT_EQUAL ||
               type == TokenType::TOKEN_LOGICAL_AND ||
               type == TokenType::TOKEN_LOGICAL_OR ||
               type == TokenType::TOKEN_LOGICAL_NOT;
    }
};

inline const char* tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::TOKEN_EOF:           return "TOKEN_EOF";
        case TokenType::TOKEN_NUMBER:        return "TOKEN_NUMBER";
        case TokenType::TOKEN_VAR:           return "TOKEN_VAR";
        case TokenType::TOKEN_FUNC:          return "TOKEN_FUNC";
        case TokenType::TOKEN_OPERATOR:      return "TOKEN_OPERATOR";
        case TokenType::TOKEN_PLUS:          return "TOKEN_PLUS";
        case TokenType::TOKEN_MINUS:         return "TOKEN_MINUS";
        case TokenType::TOKEN_STAR:          return "TOKEN_STAR";
        case TokenType::TOKEN_SLASH:         return "TOKEN_SLASH";
        case TokenType::TOKEN_GREATER:       return "TOKEN_GREATER";
        case TokenType::TOKEN_LESS:          return "TOKEN_LESS";
        case TokenType::TOKEN_GREATER_EQUAL: return "TOKEN_GREATER_EQUAL";
        case TokenType::TOKEN_LESS_EQUAL:    return "TOKEN_LESS_EQUAL";
        case TokenType::TOKEN_EQUAL_EQUAL:   return "TOKEN_EQUAL_EQUAL";
        case TokenType::TOKEN_NOT_EQUAL:     return "TOKEN_NOT_EQUAL";
        case TokenType::TOKEN_LOGICAL_AND:   return "TOKEN_LOGICAL_AND";
        case TokenType::TOKEN_LOGICAL_OR:    return "TOKEN_LOGICAL_OR";
        case TokenType::TOKEN_LOGICAL_NOT:   return "TOKEN_LOGICAL_NOT";
        case TokenType::TOKEN_LPAREN:        return "TOKEN_LPAREN";
        case TokenType::TOKEN_RPAREN:        return "TOKEN_RPAREN";
        case TokenType::TOKEN_COMMA:         return "TOKEN_COMMA";
        default:                             return "UNKNOWN_TOKEN";
    }
}

} // namespace alphaparser
