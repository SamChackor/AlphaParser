#include "alphaparser/SemanticAnalyzer.hpp"
#include <algorithm>
#include <cctype>

namespace alphaparser {

SemanticAnalyzer::SemanticAnalyzer() = default;

static std::string toUpper(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

static std::string toLower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

std::string SemanticAnalyzer::mangleIndicatorName(const std::string& func, const std::string& series, int64_t period) {
    std::string base = toLower(func);
    if (series.empty() || series == "price") {
        return base + std::to_string(period);
    }
    return base + "_" + toLower(series) + "_" + std::to_string(period);
}

SemanticAnalysisResult SemanticAnalyzer::analyze(const ASTNode& root) {
    m_variableSet.clear();
    m_variableList.clear();
    m_indicatorSet.clear();
    m_indicatorList.clear();
    m_currentType = ExprType::Unknown;

    root.accept(*this);

    SemanticAnalysisResult result;
    result.rootType = m_currentType;
    result.variables = m_variableList;
    result.indicators = m_indicatorList;

    return result;
}

void SemanticAnalyzer::visit(const NumberNode& /*node*/) {
    m_currentType = ExprType::Double;
}

void SemanticAnalyzer::visit(const VariableNode& node) {
    m_currentType = ExprType::Double;
    if (m_variableSet.insert(node.name()).second) {
        m_variableList.push_back(node.name());
    }
}

void SemanticAnalyzer::visit(const UnaryOpNode& node) {
    node.operand().accept(*this);
    ExprType operandType = m_currentType;

    switch (node.op()) {
        case UnaryOp::Negate:
            if (operandType != ExprType::Double) {
                throw SemanticError("Unary negation '-' requires numeric operand, got " +
                                     std::string(exprTypeToString(operandType)), node.location());
            }
            m_currentType = ExprType::Double;
            break;

        case UnaryOp::LogicalNot:
            if (operandType != ExprType::Bool) {
                throw SemanticError("Logical NOT '!' requires boolean operand, got " +
                                     std::string(exprTypeToString(operandType)), node.location());
            }
            m_currentType = ExprType::Bool;
            break;
    }
}

void SemanticAnalyzer::visit(const BinaryOpNode& node) {
    node.left().accept(*this);
    ExprType leftType = m_currentType;

    node.right().accept(*this);
    ExprType rightType = m_currentType;

    switch (node.op()) {
        case BinaryOp::Add:
        case BinaryOp::Subtract:
        case BinaryOp::Multiply:
        case BinaryOp::Divide:
            if (leftType != ExprType::Double || rightType != ExprType::Double) {
                throw SemanticError("Arithmetic operator '" + std::string(binaryOpToString(node.op())) +
                                     "' requires numeric operands, got " +
                                     exprTypeToString(leftType) + " and " + exprTypeToString(rightType),
                                     node.location());
            }
            m_currentType = ExprType::Double;
            break;

        case BinaryOp::GreaterThan:
        case BinaryOp::LessThan:
        case BinaryOp::GreaterEqual:
        case BinaryOp::LessEqual:
            if (leftType != ExprType::Double || rightType != ExprType::Double) {
                throw SemanticError("Relational operator '" + std::string(binaryOpToString(node.op())) +
                                     "' requires numeric operands, got " +
                                     exprTypeToString(leftType) + " and " + exprTypeToString(rightType),
                                     node.location());
            }
            m_currentType = ExprType::Bool;
            break;

        case BinaryOp::Equal:
        case BinaryOp::NotEqual:
            if (leftType != rightType) {
                throw SemanticError("Equality operator '" + std::string(binaryOpToString(node.op())) +
                                     "' requires matching operand types, got " +
                                     exprTypeToString(leftType) + " and " + exprTypeToString(rightType),
                                     node.location());
            }
            m_currentType = ExprType::Bool;
            break;

        case BinaryOp::LogicalAnd:
        case BinaryOp::LogicalOr:
            if (leftType != ExprType::Bool || rightType != ExprType::Bool) {
                throw SemanticError("Logical operator '" + std::string(binaryOpToString(node.op())) +
                                     "' requires boolean operands, got " +
                                     exprTypeToString(leftType) + " and " + exprTypeToString(rightType),
                                     node.location());
            }
            m_currentType = ExprType::Bool;
            break;
    }
}

void SemanticAnalyzer::validateFunctionCall(const FunctionNode& node) {
    std::string upperName = toUpper(node.name());
    const auto& args = node.arguments();

    if (upperName == "SMA" || upperName == "EMA" || upperName == "RSI" || upperName == "STDDEV" ||
        upperName == "ATR") {
        if (args.empty() || args.size() > 2) {
            throw SemanticError("Indicator '" + node.name() + "' expects 1 or 2 arguments (e.g. " +
                                upperName + "(50) or " + upperName + "(price, 50)), got " +
                                std::to_string(args.size()), node.location());
        }

        std::string series = "price";
        const ASTNode* periodNode = nullptr;

        if (args.size() == 1) {
            periodNode = args[0].get();
        } else {
            const auto* var = dynamic_cast<const VariableNode*>(args[0].get());
            if (!var) {
                throw SemanticError("First argument of " + upperName + " must be a variable series name (e.g. price, close)",
                                    args[0]->location());
            }
            series = var->name();
            if (m_variableSet.insert(series).second) {
                m_variableList.push_back(series);
            }
            periodNode = args[1].get();
        }

        bool isNegative = false;
        const NumberNode* num = dynamic_cast<const NumberNode*>(periodNode);
        if (!num) {
            const auto* unary = dynamic_cast<const UnaryOpNode*>(periodNode);
            if (unary && unary->op() == UnaryOp::Negate) {
                num = dynamic_cast<const NumberNode*>(&unary->operand());
                isNegative = true;
            }
        }

        if (!num) {
            throw SemanticError("Function '" + node.name() + "' requires a constant integer period argument",
                                periodNode->location());
        }

        if (!num->isInteger()) {
            throw SemanticError("Function '" + node.name() + "' period argument must be an integer, got " +
                                std::to_string(num->value()), num->location());
        }

        int64_t period = isNegative ? -num->intValue() : num->intValue();
        if (period <= 0) {
            throw SemanticError("Function '" + node.name() + "' period argument must be greater than 0, got " +
                                std::to_string(period), num->location());
        }

        std::string mangled = mangleIndicatorName(upperName, series, period);

        IndicatorInfo info{upperName, series, period, mangled};
        if (m_indicatorSet.insert(info).second) {
            m_indicatorList.push_back(info);
        }
    } else {
        if (args.empty()) {
            throw SemanticError("Function '" + node.name() + "' expects at least 1 argument", node.location());
        }
        for (const auto& arg : args) {
            arg->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(const FunctionNode& node) {
    validateFunctionCall(node);
    m_currentType = ExprType::Double;
}

} // namespace alphaparser
