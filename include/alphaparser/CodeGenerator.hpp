#pragma once

#include "AST.hpp"
#include "SemanticAnalyzer.hpp"
#include <string>
#include <sstream>

namespace alphaparser {

enum class CodegenMode {
    VectorLatest, // evaluateSignal(const std::vector<double>& price, ...) at latest bar
    VectorIndex,  // evaluateSignalAt(size_t idx, const std::vector<double>& price, ...)
    ScalarTick    // evaluateSignalTick(double price, double sma50)
};

class CodeGenerator : public ASTVisitor {
public:
    explicit CodeGenerator(const SemanticAnalysisResult& analysis);

    [[nodiscard]] std::string generateFunction(const ASTNode& root, const std::string& functionName = "evaluateSignal");
    [[nodiscard]] std::string generateScalarFunction(const ASTNode& root, const std::string& functionName = "evaluateSignalTick");
    [[nodiscard]] std::string generateModule(const ASTNode& root, const std::string& functionName = "evaluateSignal");

    void visit(const NumberNode& node) override;
    void visit(const VariableNode& node) override;
    void visit(const BinaryOpNode& node) override;
    void visit(const UnaryOpNode& node) override;
    void visit(const FunctionNode& node) override;

private:
    [[nodiscard]] std::string buildParameterList(bool isVector) const;
    [[nodiscard]] std::string buildBoundsCheck() const;

private:
    SemanticAnalysisResult m_analysis;
    std::ostringstream m_out;
    CodegenMode m_mode{CodegenMode::VectorLatest};
};

class ASTPrinter : public ASTVisitor {
public:
    [[nodiscard]] std::string print(const ASTNode& node);

    void visit(const NumberNode& node) override;
    void visit(const VariableNode& node) override;
    void visit(const BinaryOpNode& node) override;
    void visit(const UnaryOpNode& node) override;
    void visit(const FunctionNode& node) override;

private:
    void indent();

private:
    std::ostringstream m_out;
    int m_indentLevel{0};
};

} // namespace alphaparser
