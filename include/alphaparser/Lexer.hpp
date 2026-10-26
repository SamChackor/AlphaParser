#pragma once

#include "Token.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_set>

namespace alphaparser {

class LexerError : public std::runtime_error {
public:
    LexerError(const std::string& message, SourceLocation loc)
        : std::runtime_error("Lexer Error at " + loc.toString() + ": " + message),
          m_location(loc) {}

    [[nodiscard]] const SourceLocation& location() const noexcept { return m_location; }

private:
    SourceLocation m_location;
};

class Lexer {
public:
    explicit Lexer(std::string source);

    [[nodiscard]] std::vector<Token> tokenize();

private:
    [[nodiscard]] bool isAtEnd() const noexcept;
    [[nodiscard]] char peek() const noexcept;
    [[nodiscard]] char peekNext() const noexcept;
    char advance();
    bool match(char expected);

    void skipWhitespaceAndComments();
    Token scanToken();
    Token scanNumber();
    Token scanIdentifierOrFunction();

    [[nodiscard]] bool isKnownFunction(const std::string& name) const noexcept;

private:
    std::string m_source;
    std::size_t m_cursor{0};
    std::size_t m_line{1};
    std::size_t m_column{1};

    std::unordered_set<std::string> m_knownFunctions;
};

} // namespace alphaparser
