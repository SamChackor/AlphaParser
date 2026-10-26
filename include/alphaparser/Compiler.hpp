#pragma once

#include "Token.hpp"
#include "AST.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include "CodeGenerator.hpp"
#include <string>
#include <memory>

namespace alphaparser {

struct CompilationResult {
    bool success{false};
    std::string errorMessage{};
    SourceLocation errorLocation{};

    std::vector<Token> tokens;
    std::unique_ptr<ASTNode> ast;
    std::string astDump;
    SemanticAnalysisResult analysis;
    std::string generatedFunctionCode;
    std::string generatedModuleCode;
};

class Compiler {
public:
    [[nodiscard]] static CompilationResult compile(
        const std::string& dslSource,
        const std::string& functionName = "evaluateSignal"
    );
};

} // namespace alphaparser
