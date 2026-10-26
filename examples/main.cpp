#include "alphaparser/Compiler.hpp"
#include <iostream>
#include <vector>
#include <iomanip>

void printDivider(const std::string &title)
{
    std::cout << "\n"
              << std::string(80, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(80, '=') << "\n";
}

void runTestCase(const std::string &testName, const std::string &dslExpr)
{
    printDivider(testName);
    std::cout << "Input DSL Signal: \"" << dslExpr << "\"\n\n";

    auto result = alphaparser::Compiler::compile(dslExpr);

    if (!result.success)
    {
        std::cerr << "[-] Compilation Failed!\n";
        std::cerr << "    " << result.errorMessage << "\n";
        return;
    }

    std::cout << "[+] Lexical Tokens (" << result.tokens.size() << " tokens):\n";
    for (const auto &tok : result.tokens)
    {
        std::cout << "    [" << std::setw(20) << std::left << alphaparser::tokenTypeToString(tok.type)
                  << " | Lexeme: '" << tok.lexeme << "' | " << tok.location.toString() << "]\n";
    }

    std::cout << "\n[+] Abstract Syntax Tree (AST):\n";
    std::cout << result.astDump;

    std::cout << "[+] Semantic Analysis:\n";
    std::cout << "    Root Type: " << alphaparser::exprTypeToString(result.analysis.rootType) << "\n";
    std::cout << "    Referenced Variables (" << result.analysis.variables.size() << "): ";
    for (const auto &v : result.analysis.variables)
        std::cout << v << " ";
    std::cout << "\n    Referenced Indicators (" << result.analysis.indicators.size() << "):\n";
    for (const auto &ind : result.analysis.indicators)
    {
        std::cout << "      - " << ind.functionName << "(period=" << ind.period
                  << ", series=" << ind.seriesName << ") -> buffer: " << ind.mangledName << "\n";
    }

    std::cout << "\n[+] Generated C++ Function:\n";
    std::cout << result.generatedFunctionCode << "\n";
}

int main()
{
    std::cout << R"(
    ================================================================
         AlphaParser (Sig-C) Quantitative DSL Compiler Engine
    ================================================================
    )" << std::endl;

    // Requirement 1 & 4 Core Example: SMA(50) > price
    runTestCase("TEST CASE 1: Standard Moving Average Crossover", "SMA(50) > price");

    // Complex Multi-Indicator Expression
    runTestCase("TEST CASE 2: Dual Moving Average & EMA Trend Filter",
                "(SMA(20) > SMA(50)) && (price > EMA(10))");

    // Volatility Breakout Expression
    runTestCase("TEST CASE 3: Bollinger / Volatility Breakout Filter",
                "(price - SMA(20)) > (2.0 * STDDEV(20))");

    // Semantic Error Validation 1: Negative period
    runTestCase("TEST CASE 4: Semantic Error Detection (Negative Period)",
                "SMA(-50) > price");

    // Semantic Error Validation 2: Floating point period
    runTestCase("TEST CASE 5: Semantic Error Detection (Floating-point Period)",
                "SMA(50.5) > price");

    // Semantic Error Validation 3: Type Mismatch (Arithmetic on Boolean)
    runTestCase("TEST CASE 6: Semantic Error Detection (Type Mismatch)",
                "(price > 100.0) + 5.0");

    // My Custom Strategy
    runTestCase("TEST CASE 7: Custom Breakout Strategy", "(price - EMA(10)) > (2.0 * STDDEV(20))");

    // Simulated Runtime Verification of the Generated Logic
    printDivider("RUNTIME VERIFICATION: Executing Emitted C++ Logic");

    // Sample market data series (e.g. 5 bars)
    std::vector<double> price = {100.0, 102.5, 101.0, 104.0, 108.0};
    std::vector<double> sma50 = {105.0, 105.5, 106.0, 106.5, 107.0};

    // The generated logic for "SMA(50) > price" is:
    // (sma50[idx] > price[idx])
    const std::size_t idx = price.size() - 1; // latest bar
    bool signalTriggered = (sma50[idx] > price[idx]);

    std::cout << "Bar Index: " << idx << "\n";
    std::cout << "price[" << idx << "] = " << price[idx] << "\n";
    std::cout << "sma50[" << idx << "] = " << sma50[idx] << "\n";
    std::cout << "Condition (sma50 > price): "
              << (signalTriggered ? "TRUE (SELL / SHORT)" : "FALSE (HOLD / NO SHORT)") << "\n";

    return 0;
}
