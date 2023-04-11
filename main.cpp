#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <optional>
#include <assert.h>
#include "scanner.h"
#include "astprinter.h"

class ErrorReporter : public Scanner::IErrorReporter
{
public:
    static ErrorReporter& Instance()
    {
        static ErrorReporter errorReporter;
        return errorReporter;
    }

private:
    virtual void OnError(int line, std::string_view message) override
    {
        Report(line, "", message);
    }

    void Report(int line, std::string_view where, std::string_view message)
    {
        std::cerr << "[line " << line << "] Error " << where << ": " << message << std::endl; 
    }
};

void run(std::string_view source)
{
    Scanner scanner(source, ErrorReporter::Instance());
    
    for (const Token& token : scanner.Tokens())
    {
        std::cout << token << std::endl;
    }

}

std::optional<std::string> GetFileContent(const char* filename)
{
    std::ifstream script(filename);
    if (script.is_open())
    {
        std::stringstream buffer;
        buffer << script.rdbuf();
        script.close();

        return buffer.str();
    }
    else
    {
        std::cout << "can't open file: " << filename << std::endl;
        return std::nullopt;
    }
}

void runFile(const char* filename)
{
    std::cout << "running file: " << filename << std::endl;

    std::optional<std::string> script = GetFileContent(filename);
    if (script.has_value())
    {
        run(script.value());
    }
}

void runPrompt()
{
    std::string line; 

    std::cout << "> ";
    while (std::getline(std::cin, line))
    {
        run(line);
        std::cout << "> ";
    }
}

void runTests()
{
    ErrorReporter errorReporter;

    // empty source
    {
        Scanner scanner("", errorReporter);
        assert(scanner.Tokens().size() == 1);
        assert(scanner.Tokens()[0].m_type == Token::Type::EndOfFile);
    }

    // declaring number variable
    {
        Scanner scanner("var test = 42.7", errorReporter);
        assert(scanner.Tokens().size() == 5);
        assert(scanner.Tokens()[0].m_type == Token::Type::Var);
        assert(scanner.Tokens()[1].m_type == Token::Type::Identifier);
        assert(scanner.Tokens()[1].m_lexeme == "test");
        assert(scanner.Tokens()[2].m_type == Token::Type::Equal);
        assert(scanner.Tokens()[3].m_type == Token::Type::Number);
        double val = std::any_cast<double>(scanner.Tokens()[3].m_literalvalue);
        assert(val == 42.7);
        assert(scanner.Tokens()[4].m_type == Token::Type::EndOfFile);
    }

    // multiline script
    {
        std::optional<std::string> script = GetFileContent("../../scripts/unit_test_1.gk");

        assert(script.has_value());

        Scanner scanner(script.value(), errorReporter);

        assert(scanner.Tokens().size() == 5);
        assert(scanner.Tokens()[0].m_type == Token::Type::Var);
        assert(scanner.Tokens()[1].m_type == Token::Type::Identifier);
        assert(scanner.Tokens()[1].m_lexeme == "someString");
        assert(scanner.Tokens()[2].m_type == Token::Type::Equal);
        assert(scanner.Tokens()[3].m_type == Token::Type::String);
        std::string_view val = std::any_cast<std::string_view>(scanner.Tokens()[3].m_literalvalue);
        assert(val == "TestString");
        assert(scanner.Tokens()[4].m_type == Token::Type::EndOfFile);
    }
}

int main(int argc, char* argv[])
{
    runTests();

    std::any l1 = std::make_any<double>(123.0);
    LiteralExpression l1Expression(l1);
    Token minusToken(Token::Type::Minus, "-", std::any(), 1);
    UnaryExpression unaryExpression(minusToken, l1Expression);
    std::any l2 = std::make_any<double>(45.67);
    LiteralExpression l2Expression(l2);
    GroupingExpression groupingExpression(l2Expression);
    Token starToken(Token::Type::Star, "*", std::any(), 1);
    BinaryExpression binaryExpression(
        unaryExpression,
        starToken,
        groupingExpression
    );

    ASTPrinter printer;
    std::cout << printer.ToString(binaryExpression) << std::endl;

    if (argc > 2)
    {
        std::cout << "Usage: Gekko [script]" << std::endl;
    }
    else if (argc == 2)
    {
        runFile(argv[1]);
    }
    else
    {
        runPrompt();
    }

    return 0;
}