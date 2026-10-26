#include "alphaparser/Lexer.hpp"
#include <cctype>
#include <cstdlib>

namespace alphaparser {

Lexer::Lexer(std::string source)
    : m_source(std::move(source)),
      m_cursor(0),
      m_line(1),
      m_column(1),
      m_knownFunctions{"SMA", "EMA", "RSI", "MACD", "STDDEV", "MIN", "MAX", "ATR", "VWAP"} {}

bool Lexer::isAtEnd() const noexcept {
    return m_cursor >= m_source.size();
}

char Lexer::peek() const noexcept {
    if (isAtEnd()) return '\0';
    return m_source[m_cursor];
}

char Lexer::peekNext() const noexcept {
    if (m_cursor + 1 >= m_source.size()) return '\0';
    return m_source[m_cursor + 1];
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    char c = m_source[m_cursor++];
    if (c == '\n') {
        m_line++;
        m_column = 1;
    } else {
        m_column++;
    }
    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (m_source[m_cursor] != expected) return false;
    advance();
    return true;
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && peekNext() == '/') {
            advance();
            advance();
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else {
            break;
        }
    }
}

bool Lexer::isKnownFunction(const std::string& name) const noexcept {
    return m_knownFunctions.find(name) != m_knownFunctions.end();
}

Token Lexer::scanNumber() {
    SourceLocation loc{m_line, m_column, m_cursor};
    std::size_t start = m_cursor;

    while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    if (!isAtEnd() && peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        advance();
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    if (!isAtEnd() && (peek() == 'e' || peek() == 'E')) {
        advance();
        if (!isAtEnd() && (peek() == '+' || peek() == '-')) {
            advance();
        }
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    std::string text = m_source.substr(start, m_cursor - start);
    char* endPtr = nullptr;
    double val = std::strtod(text.c_str(), &endPtr);

    return Token(TokenType::TOKEN_NUMBER, text, val, loc);
}

Token Lexer::scanIdentifierOrFunction() {
    SourceLocation loc{m_line, m_column, m_cursor};
    std::size_t start = m_cursor;

    while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        advance();
    }

    std::string text = m_source.substr(start, m_cursor - start);

    std::size_t lookahead = m_cursor;
    while (lookahead < m_source.size() &&
           (m_source[lookahead] == ' ' || m_source[lookahead] == '\t' ||
            m_source[lookahead] == '\r' || m_source[lookahead] == '\n')) {
        lookahead++;
    }

    bool followedByParen = (lookahead < m_source.size() && m_source[lookahead] == '(');

    if (isKnownFunction(text) || followedByParen) {
        return Token(TokenType::TOKEN_FUNC, text, loc);
    }

    return Token(TokenType::TOKEN_VAR, text, loc);
}

Token Lexer::scanToken() {
    skipWhitespaceAndComments();

    SourceLocation loc{m_line, m_column, m_cursor};

    if (isAtEnd()) {
        return Token(TokenType::TOKEN_EOF, "", loc);
    }

    char c = peek();

    if (std::isdigit(static_cast<unsigned char>(c)) ||
        (c == '.' && std::isdigit(static_cast<unsigned char>(peekNext())))) {
        return scanNumber();
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return scanIdentifierOrFunction();
    }

    advance();
    switch (c) {
        case '(': return Token(TokenType::TOKEN_LPAREN, "(", loc);
        case ')': return Token(TokenType::TOKEN_RPAREN, ")", loc);
        case ',': return Token(TokenType::TOKEN_COMMA, ",", loc);
        case '+': return Token(TokenType::TOKEN_PLUS, "+", loc);
        case '-': return Token(TokenType::TOKEN_MINUS, "-", loc);
        case '*': return Token(TokenType::TOKEN_STAR, "*", loc);
        case '/': return Token(TokenType::TOKEN_SLASH, "/", loc);

        case '>':
            if (match('=')) return Token(TokenType::TOKEN_GREATER_EQUAL, ">=", loc);
            return Token(TokenType::TOKEN_GREATER, ">", loc);

        case '<':
            if (match('=')) return Token(TokenType::TOKEN_LESS_EQUAL, "<=", loc);
            return Token(TokenType::TOKEN_LESS, "<", loc);

        case '=':
            if (match('=')) return Token(TokenType::TOKEN_EQUAL_EQUAL, "==", loc);
            throw LexerError("Unexpected single '=' assignment operator. Did you mean '=='?", loc);

        case '!':
            if (match('=')) return Token(TokenType::TOKEN_NOT_EQUAL, "!=", loc);
            return Token(TokenType::TOKEN_LOGICAL_NOT, "!", loc);

        case '&':
            if (match('&')) return Token(TokenType::TOKEN_LOGICAL_AND, "&&", loc);
            throw LexerError("Unexpected bitwise '&'. AlphaParser expects '&&' for logical AND.", loc);

        case '|':
            if (match('|')) return Token(TokenType::TOKEN_LOGICAL_OR, "||", loc);
            throw LexerError("Unexpected bitwise '|'. AlphaParser expects '||' for logical OR.", loc);

        default:
            throw LexerError(std::string("Unexpected character: '") + c + "'", loc);
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token tok = scanToken();
        tokens.push_back(tok);
        if (tok.type == TokenType::TOKEN_EOF) {
            break;
        }
    }
    return tokens;
}

} // namespace alphaparser
