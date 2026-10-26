#include "alphaparser/Compiler.hpp"

namespace alphaparser {

CompilationResult Compiler::compile(const std::string& dslSource, const std::string& functionName) {
    CompilationResult result;

    try {
        // Stage 1: Lexical Analysis
        Lexer lexer(dslSource);
        result.tokens = lexer.tokenize();

        // Stage 2: Parsing & AST Construction
        Parser parser(result.tokens);
        result.ast = parser.parse();

        // Dump AST representation
        ASTPrinter printer;
        result.astDump = printer.print(*result.ast);

        // Stage 3: Semantic Analysis & Type/Parameter Validation
        SemanticAnalyzer analyzer;
        result.analysis = analyzer.analyze(*result.ast);

        // Stage 4: Code Generation (Transpilation)
        CodeGenerator codegen(result.analysis);
        result.generatedFunctionCode = codegen.generateFunction(*result.ast, functionName);
        result.generatedModuleCode = codegen.generateModule(*result.ast, functionName);

        result.success = true;
    } catch (const LexerError& err) {
        result.success = false;
        result.errorMessage = err.what();
        result.errorLocation = err.location();
    } catch (const ParserError& err) {
        result.success = false;
        result.errorMessage = err.what();
        result.errorLocation = err.location();
    } catch (const SemanticError& err) {
        result.success = false;
        result.errorMessage = err.what();
        result.errorLocation = err.location();
    } catch (const std::exception& err) {
        result.success = false;
        result.errorMessage = std::string("Unexpected compiler exception: ") + err.what();
    }

    return result;
}

} // namespace alphaparser
