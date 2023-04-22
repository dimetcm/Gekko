#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <optional>
#include <assert.h>
#include "scanner.h"
#include "parser.h"
#include "astprinter.h"
#include "statements.h"
#include "expressions.h"
#include "mocks/mockedinterpreter.h"
#include "mocks/mockedparser.h"

void run(std::string_view source)
{
    Scanner scanner(source);

    Parser parser(scanner.Tokens());
    std::vector<IStatementPtr> program = parser.Parse(std::cout);
    Interpreter interpreter;
    interpreter.Interpret(program, std::cout);
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
    // empty source
    {
        Scanner scanner("");
        assert(scanner.Tokens().size() == 1);
        assert(scanner.Tokens()[0].m_type == Token::Type::EndOfFile);
    }

    // declaring number variable
    {
        Scanner scanner("var test = 42.7");
        assert(scanner.Tokens().size() == 5);
        assert(scanner.Tokens()[0].m_type == Token::Type::Var);
        assert(scanner.Tokens()[1].m_type == Token::Type::Identifier);
        assert(scanner.Tokens()[1].m_lexeme == "test");
        assert(scanner.Tokens()[2].m_type == Token::Type::Equal);
        assert(scanner.Tokens()[3].m_type == Token::Type::Number);
        const double* val = scanner.Tokens()[3].m_literalvalue.GetNumber();
        assert(val && *val == 42.7);
        assert(scanner.Tokens()[4].m_type == Token::Type::EndOfFile);
    }

    // multiline script
    {
        std::optional<std::string> script = GetFileContent("../../scripts/unit_test_1.gk");

        assert(script.has_value());

        Scanner scanner(script.value());

        assert(scanner.Tokens().size() == 5);
        assert(scanner.Tokens()[0].m_type == Token::Type::Var);
        assert(scanner.Tokens()[1].m_type == Token::Type::Identifier);
        assert(scanner.Tokens()[1].m_lexeme == "someString");
        assert(scanner.Tokens()[2].m_type == Token::Type::Equal);
        assert(scanner.Tokens()[3].m_type == Token::Type::String);
        const std::string* val = scanner.Tokens()[3].m_literalvalue.GetString();
        assert(val && *val == "TestString");
        assert(scanner.Tokens()[4].m_type == Token::Type::EndOfFile);
    }

    { // interpreter tests
        {
            Scanner scanner("2 * 10 - 1 + 3");
            MockedParser parser(scanner.Tokens());
            IExpressionPtr expression = parser.ParseExpression(std::cerr);
            assert(expression);
            MockedInterpreter interpreter;
            Value result = interpreter.Interpret(*expression, std::cerr);

            assert(result.GetNumber() && *result.GetNumber() == 22);
        }

        {
            Scanner scanner("(1 + 3) * 2 == 8");
            MockedParser parser(scanner.Tokens());
            IExpressionPtr expression = parser.ParseExpression(std::cerr);
            assert(expression);
            MockedInterpreter interpreter;
            Value result = interpreter.Interpret(*expression, std::cerr);

            assert(result.GetBoolean() && *result.GetBoolean());
        }

        {
            Scanner scanner("1/8 > 8");
            MockedParser parser(scanner.Tokens());
            IExpressionPtr expression = parser.ParseExpression(std::cerr);
            assert(expression);
            MockedInterpreter interpreter;
            Value result = interpreter.Interpret(*expression, std::cerr);

            assert(result.GetBoolean() && !*result.GetBoolean());
        }
    }
}

int main(int argc, char* argv[])
{
    runTests();

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