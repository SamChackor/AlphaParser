#pragma once

#include "AST.hpp"
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>

namespace alphaparser {

class SemanticError : public std::runtime_error {
public:
    SemanticError(const std::string& message, SourceLocation loc)
        : std::runtime_error("Semantic Error at " + loc.toString() + ": " + message),
          m_location(loc) {}

    [[nodiscard]] const SourceLocation& location() const noexcept { return m_location; }

private:
    SourceLocation m_location;
};

enum class ExprType {
    Unknown,
    Double,
    Bool
};

inline const char* exprTypeToString(ExprType type) {
    switch (type) {
        case ExprType::Double:  return "double";
        case ExprType::Bool:    return "bool";
        case ExprType::Unknown: return "unknown";
        default:                return "invalid";
    }
}

struct IndicatorInfo {
    std::string functionName;     // e.g., "SMA"
    std::string seriesName;       // e.g., "price"
    int64_t period{0};            // e.g., 50
    std::string mangledName;      // e.g., "sma50"

    bool operator<(const IndicatorInfo& other) const {
        if (functionName != other.functionName) return functionName < other.functionName;
        if (seriesName != other.seriesName) return seriesName < other.seriesName;
        return period < other.period;
    }
};

struct SemanticAnalysisResult {
    ExprType rootType{ExprType::Unknown};
    std::vector<std::string> variables;
    std::vector<IndicatorInfo> indicators;
};

class SemanticAnalyzer : public ASTVisitor {
public:
    SemanticAnalyzer();

    [[nodiscard]] SemanticAnalysisResult analyze(const ASTNode& root);

    void visit(const NumberNode& node) override;
    void visit(const VariableNode& node) override;
    void visit(const BinaryOpNode& node) override;
    void visit(const UnaryOpNode& node) override;
    void visit(const FunctionNode& node) override;

    [[nodiscard]] static std::string mangleIndicatorName(const std::string& func, const std::string& series, int64_t period);

private:
    void validateFunctionCall(const FunctionNode& node);

private:
    ExprType m_currentType{ExprType::Unknown};
    std::set<std::string> m_variableSet;
    std::vector<std::string> m_variableList;

    std::set<IndicatorInfo> m_indicatorSet;
    std::vector<IndicatorInfo> m_indicatorList;
};

} // namespace alphaparser
