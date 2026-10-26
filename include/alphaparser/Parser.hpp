#pragma once

#include "Token.hpp"
#include "AST.hpp"
#include <vector>
#include <memory>
#include <stdexcept>

namespace alphaparser {

class ParserError : public std::runtime_error {
public:
    ParserError(const std::string& message, SourceLocation loc)
        : std::runtime_error("Parser Error at " + loc.toString() + ": " + message),
          m_location(loc) {}

    [[nodiscard]] const SourceLocation& location() const noexcept { return m_location; }

private:
    SourceLocation m_location;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    [[nodiscard]] std::unique_ptr<ASTNode> parse();

private:
    [[nodiscard]] std::unique_ptr<ASTNode> parseExpression();
    [[nodiscard]] std::unique_ptr<ASTNode> parseBinaryOp(int minPrecedence);
    [[nodiscard]] std::unique_ptr<ASTNode> parseUnary();
    [[nodiscard]] std::unique_ptr<ASTNode> parsePrimary();
    [[nodiscard]] std::unique_ptr<ASTNode> parseFunctionCall(const Token& funcToken);

    [[nodiscard]] const Token& peek() const;
    [[nodiscard]] const Token& previous() const;
    [[nodiscard]] bool isAtEnd() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& consume(TokenType type, const std::string& errorMessage);

    [[nodiscard]] static int getPrecedence(TokenType type);
    [[nodiscard]] static BinaryOp tokenToBinaryOp(TokenType type);

private:
    std::vector<Token> m_tokens;
    std::size_t m_current{0};
};

} // namespace alphaparser
