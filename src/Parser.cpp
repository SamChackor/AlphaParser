#include "alphaparser/Parser.hpp"

namespace alphaparser {

Parser::Parser(std::vector<Token> tokens)
    : m_tokens(std::move(tokens)), m_current(0) {}

const Token& Parser::peek() const {
    if (m_current >= m_tokens.size()) {
        return m_tokens.back();
    }
    return m_tokens[m_current];
}

const Token& Parser::previous() const {
    if (m_current == 0) {
        return m_tokens.front();
    }
    return m_tokens[m_current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::TOKEN_EOF;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        m_current++;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::consume(TokenType type, const std::string& errorMessage) {
    if (check(type)) {
        return advance();
    }
    throw ParserError(errorMessage + " (found '" + peek().lexeme + "')", peek().location);
}

int Parser::getPrecedence(TokenType type) {
    switch (type) {
        case TokenType::TOKEN_LOGICAL_OR:    return 1;
        case TokenType::TOKEN_LOGICAL_AND:   return 2;
        case TokenType::TOKEN_EQUAL_EQUAL:
        case TokenType::TOKEN_NOT_EQUAL:     return 3;
        case TokenType::TOKEN_GREATER:
        case TokenType::TOKEN_LESS:
        case TokenType::TOKEN_GREATER_EQUAL:
        case TokenType::TOKEN_LESS_EQUAL:    return 4;
        case TokenType::TOKEN_PLUS:
        case TokenType::TOKEN_MINUS:         return 5;
        case TokenType::TOKEN_STAR:
        case TokenType::TOKEN_SLASH:         return 6;
        default:                             return 0;
    }
}

BinaryOp Parser::tokenToBinaryOp(TokenType type) {
    switch (type) {
        case TokenType::TOKEN_PLUS:          return BinaryOp::Add;
        case TokenType::TOKEN_MINUS:         return BinaryOp::Subtract;
        case TokenType::TOKEN_STAR:          return BinaryOp::Multiply;
        case TokenType::TOKEN_SLASH:         return BinaryOp::Divide;
        case TokenType::TOKEN_GREATER:       return BinaryOp::GreaterThan;
        case TokenType::TOKEN_LESS:          return BinaryOp::LessThan;
        case TokenType::TOKEN_GREATER_EQUAL: return BinaryOp::GreaterEqual;
        case TokenType::TOKEN_LESS_EQUAL:    return BinaryOp::LessEqual;
        case TokenType::TOKEN_EQUAL_EQUAL:   return BinaryOp::Equal;
        case TokenType::TOKEN_NOT_EQUAL:     return BinaryOp::NotEqual;
        case TokenType::TOKEN_LOGICAL_AND:   return BinaryOp::LogicalAnd;
        case TokenType::TOKEN_LOGICAL_OR:    return BinaryOp::LogicalOr;
        default:
            throw std::invalid_argument("Token is not a binary operator");
    }
}

std::unique_ptr<ASTNode> Parser::parse() {
    if (m_tokens.empty() || m_tokens.front().type == TokenType::TOKEN_EOF) {
        throw ParserError("Empty input expression", SourceLocation{1, 1, 0});
    }

    auto root = parseExpression();

    if (!isAtEnd()) {
        throw ParserError("Unexpected trailing token '" + peek().lexeme + "' after expression", peek().location);
    }

    return root;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    return parseBinaryOp(1);
}

std::unique_ptr<ASTNode> Parser::parseBinaryOp(int minPrecedence) {
    auto left = parseUnary();

    while (true) {
        TokenType currentType = peek().type;
        int precedence = getPrecedence(currentType);

        if (precedence < minPrecedence || precedence == 0) {
            break;
        }

        Token opToken = advance();
        BinaryOp op = tokenToBinaryOp(opToken.type);

        auto right = parseBinaryOp(precedence + 1);

        SourceLocation loc = opToken.location;
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), loc);
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseUnary() {
    if (match(TokenType::TOKEN_MINUS)) {
        Token opToken = previous();
        auto operand = parseUnary();
        return std::make_unique<UnaryOpNode>(UnaryOp::Negate, std::move(operand), opToken.location);
    }

    if (match(TokenType::TOKEN_LOGICAL_NOT)) {
        Token opToken = previous();
        auto operand = parseUnary();
        return std::make_unique<UnaryOpNode>(UnaryOp::LogicalNot, std::move(operand), opToken.location);
    }

    return parsePrimary();
}

std::unique_ptr<ASTNode> Parser::parseFunctionCall(const Token& funcToken) {
    consume(TokenType::TOKEN_LPAREN, "Expected '(' after function name '" + funcToken.lexeme + "'");

    std::vector<std::unique_ptr<ASTNode>> args;

    if (!check(TokenType::TOKEN_RPAREN)) {
        args.push_back(parseExpression());
        while (match(TokenType::TOKEN_COMMA)) {
            args.push_back(parseExpression());
        }
    }

    consume(TokenType::TOKEN_RPAREN, "Expected ')' after arguments for function '" + funcToken.lexeme + "'");

    return std::make_unique<FunctionNode>(funcToken.lexeme, std::move(args), funcToken.location);
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    if (match(TokenType::TOKEN_NUMBER)) {
        Token tok = previous();
        bool isInt = (tok.lexeme.find('.') == std::string::npos &&
                      tok.lexeme.find('e') == std::string::npos &&
                      tok.lexeme.find('E') == std::string::npos);
        int64_t intVal = 0;
        if (isInt) {
            try {
                intVal = std::stoll(tok.lexeme);
            } catch (...) {
                isInt = false;
            }
        }
        return std::make_unique<NumberNode>(tok.numberValue, isInt, intVal, tok.location);
    }

    if (match(TokenType::TOKEN_FUNC)) {
        Token funcTok = previous();
        return parseFunctionCall(funcTok);
    }

    if (match(TokenType::TOKEN_VAR)) {
        Token varTok = previous();
        if (check(TokenType::TOKEN_LPAREN)) {
            return parseFunctionCall(varTok);
        }
        return std::make_unique<VariableNode>(varTok.lexeme, varTok.location);
    }

    if (match(TokenType::TOKEN_LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::TOKEN_RPAREN, "Expected ')' after parenthesized expression");
        return expr;
    }

    throw ParserError("Unexpected token '" + peek().lexeme + "' where expression was expected", peek().location);
}

} // namespace alphaparser
