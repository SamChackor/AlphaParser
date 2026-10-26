#pragma once

#include "Token.hpp"
#include <memory>
#include <string>
#include <vector>

namespace alphaparser {

// Forward declarations
class NumberNode;
class VariableNode;
class BinaryOpNode;
class UnaryOpNode;
class FunctionNode;

// Visitor Interface
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(const NumberNode& node) = 0;
    virtual void visit(const VariableNode& node) = 0;
    virtual void visit(const BinaryOpNode& node) = 0;
    virtual void visit(const UnaryOpNode& node) = 0;
    virtual void visit(const FunctionNode& node) = 0;
};

// Base AST Node
class ASTNode {
public:
    explicit ASTNode(SourceLocation loc) : m_location(loc) {}
    virtual ~ASTNode() = default;

    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;
    ASTNode(ASTNode&&) noexcept = default;
    ASTNode& operator=(ASTNode&&) noexcept = default;

    [[nodiscard]] const SourceLocation& location() const noexcept { return m_location; }

    virtual void accept(ASTVisitor& visitor) const = 0;
    [[nodiscard]] virtual std::string toString() const = 0;

private:
    SourceLocation m_location;
};

// 1. Number Literal Node
class NumberNode : public ASTNode {
public:
    NumberNode(double value, bool isInt, int64_t intVal, SourceLocation loc)
        : ASTNode(loc), m_value(value), m_isInteger(isInt), m_intValue(intVal) {}

    [[nodiscard]] double value() const noexcept { return m_value; }
    [[nodiscard]] bool isInteger() const noexcept { return m_isInteger; }
    [[nodiscard]] int64_t intValue() const noexcept { return m_intValue; }

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    [[nodiscard]] std::string toString() const override {
        if (m_isInteger) {
            return std::to_string(m_intValue);
        }
        return std::to_string(m_value);
    }

private:
    double m_value;
    bool m_isInteger;
    int64_t m_intValue;
};

// 2. Variable Node
class VariableNode : public ASTNode {
public:
    VariableNode(std::string name, SourceLocation loc)
        : ASTNode(loc), m_name(std::move(name)) {}

    [[nodiscard]] const std::string& name() const noexcept { return m_name; }

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    [[nodiscard]] std::string toString() const override {
        return m_name;
    }

private:
    std::string m_name;
};

// Binary Operator Types
enum class BinaryOp {
    Add, Subtract, Multiply, Divide,
    GreaterThan, LessThan, GreaterEqual, LessEqual,
    Equal, NotEqual, LogicalAnd, LogicalOr
};

inline const char* binaryOpToString(BinaryOp op) {
    switch (op) {
        case BinaryOp::Add:          return "+";
        case BinaryOp::Subtract:     return "-";
        case BinaryOp::Multiply:     return "*";
        case BinaryOp::Divide:       return "/";
        case BinaryOp::GreaterThan:  return ">";
        case BinaryOp::LessThan:     return "<";
        case BinaryOp::GreaterEqual: return ">=";
        case BinaryOp::LessEqual:    return "<=";
        case BinaryOp::Equal:        return "==";
        case BinaryOp::NotEqual:     return "!=";
        case BinaryOp::LogicalAnd:   return "&&";
        case BinaryOp::LogicalOr:    return "||";
        default:                     return "??";
    }
}

// 3. Binary Operation Node
class BinaryOpNode : public ASTNode {
public:
    BinaryOpNode(BinaryOp op,
                 std::unique_ptr<ASTNode> left,
                 std::unique_ptr<ASTNode> right,
                 SourceLocation loc)
        : ASTNode(loc),
          m_op(op),
          m_left(std::move(left)),
          m_right(std::move(right)) {}

    [[nodiscard]] BinaryOp op() const noexcept { return m_op; }
    [[nodiscard]] const ASTNode& left() const noexcept { return *m_left; }
    [[nodiscard]] const ASTNode& right() const noexcept { return *m_right; }

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    [[nodiscard]] std::string toString() const override {
        return "(" + m_left->toString() + " " + binaryOpToString(m_op) + " " + m_right->toString() + ")";
    }

private:
    BinaryOp m_op;
    std::unique_ptr<ASTNode> m_left;
    std::unique_ptr<ASTNode> m_right;
};

// Unary Operator Types
enum class UnaryOp {
    Negate,
    LogicalNot
};

inline const char* unaryOpToString(UnaryOp op) {
    switch (op) {
        case UnaryOp::Negate:     return "-";
        case UnaryOp::LogicalNot: return "!";
        default:                  return "??";
    }
}

// Unary Operation Node
class UnaryOpNode : public ASTNode {
public:
    UnaryOpNode(UnaryOp op, std::unique_ptr<ASTNode> operand, SourceLocation loc)
        : ASTNode(loc), m_op(op), m_operand(std::move(operand)) {}

    [[nodiscard]] UnaryOp op() const noexcept { return m_op; }
    [[nodiscard]] const ASTNode& operand() const noexcept { return *m_operand; }

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    [[nodiscard]] std::string toString() const override {
        return unaryOpToString(m_op) + m_operand->toString();
    }

private:
    UnaryOp m_op;
    std::unique_ptr<ASTNode> m_operand;
};

// 4. Function Call Node
class FunctionNode : public ASTNode {
public:
    FunctionNode(std::string name,
                 std::vector<std::unique_ptr<ASTNode>> args,
                 SourceLocation loc)
        : ASTNode(loc), m_name(std::move(name)), m_arguments(std::move(args)) {}

    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    [[nodiscard]] const std::vector<std::unique_ptr<ASTNode>>& arguments() const noexcept {
        return m_arguments;
    }

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    [[nodiscard]] std::string toString() const override {
        std::string res = m_name + "(";
        for (std::size_t i = 0; i < m_arguments.size(); ++i) {
            if (i > 0) res += ", ";
            res += m_arguments[i]->toString();
        }
        res += ")";
        return res;
    }

private:
    std::string m_name;
    std::vector<std::unique_ptr<ASTNode>> m_arguments;
};

} // namespace alphaparser
