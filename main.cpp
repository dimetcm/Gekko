#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <optional>
#include <assert.h>
#include <filesystem>
#include "scanner.h"
#include "parser.h"
#include "astprinter.h"
#include "statements.h"
#include "expressions.h"
#include "mocks/mockedinterpreter.h"
#include "mocks/mockedparser.h"

void run(EnvironmentPtr environment, std::string_view source)
{
    Scanner scanner(source);

    Parser parser(scanner.Tokens());
    std::vector<IStatementPtr> program = parser.Parse(std::cout);
    Interpreter interpreter(environment);
    interpreter.Interpret(environment, program, std::cerr);
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
        EnvironmentPtr environment = Environment::CreateGlobalEnvironment();
        run(environment, script.value());
    }
}

void runPrompt()
{
    EnvironmentPtr environment = Environment::CreateGlobalEnvironment();

    std::string line; 

    std::cout << "> ";
    while (std::getline(std::cin, line))
    {
        run(environment, line);
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
            
            EnvironmentPtr environment = Environment::CreateGlobalEnvironment();
            MockedInterpreter interpreter(environment);
            Value result = interpreter.Eval(*expression, environment);

            assert(result.GetNumber() && *result.GetNumber() == 22);
        }

        {
            Scanner scanner("(1 + 3) * 2 == 8");
            MockedParser parser(scanner.Tokens());
            IExpressionPtr expression = parser.ParseExpression(std::cerr);
            assert(expression);
            EnvironmentPtr environment = Environment::CreateGlobalEnvironment();
            MockedInterpreter interpreter(environment);
            Value result = interpreter.Eval(*expression, environment);

            assert(result.GetBoolean() && *result.GetBoolean());
        }

        {
            Scanner scanner("1/8 > 8");
            MockedParser parser(scanner.Tokens());
            IExpressionPtr expression = parser.ParseExpression(std::cerr);
            assert(expression);
            EnvironmentPtr environment = Environment::CreateGlobalEnvironment();
            MockedInterpreter interpreter(environment);
            Value result = interpreter.Eval(*expression, environment);

            assert(result.GetBoolean() && !*result.GetBoolean());
        }

        {   // assignment test
            Scanner scanner("var a = 3;\n a = 2;");
            MockedParser parser(scanner.Tokens());
            std::vector<IStatementPtr> programm = parser.Parse(std::cerr);
            assert(programm.size() == 2);
            MockedEnvironmentPtr environment = MockedEnvironment::Create();
            MockedInterpreter interpreter(environment);
            std::stringstream outputStream;
            for (const IStatementPtr& statement : programm)
            {
                interpreter.Execute(*statement, environment);
            }

            assert(environment->Hasvalue("a"));
            assert(environment->GetValue("a").GetNumber());
            assert(*environment->GetValue("a").GetNumber() == 2.0);
        }

        {   // block test
            Scanner scanner(
                "var a = 1;\n"
                "{\n"
                "var a = a + 2;"
                "print a;"
                "}"
                "print a;"
            );
            MockedParser parser(scanner.Tokens());
            std::vector<IStatementPtr> programm = parser.Parse(std::cerr);
            MockedEnvironmentPtr environment = MockedEnvironment::Create();
            MockedInterpreter interpreter(environment);
            std::stringstream outputStream;
            for (const IStatementPtr& statement : programm)
            {
                interpreter.Execute(*statement, environment);
            }

            assert(environment->Hasvalue("a"));
            assert(environment->GetValue("a").GetNumber() && *environment->GetValue("a").GetNumber() == 1.0);
        }

        {   // if test
            Scanner scanner(
                "if (true)"
                "{"
                "print true;"
                "}"
                "else"
                "{"
                "print false;"
                "}"
            );
            MockedParser parser(scanner.Tokens());
            std::stringstream outputStream;
            EnvironmentPtr environment = Environment::CreateGlobalEnvironment(outputStream);
            MockedInterpreter interpreter(environment);

            for (const IStatementPtr& statement : parser.Parse(std::cerr))
            {
                interpreter.Execute(*statement, environment);
            }
            assert(outputStream.str() == "true\n");
        }

        {   // logical test
            Scanner scanner(
                "print \"hi\" or 2;"
                "print nil or \"yes\";" 
            );
            MockedParser parser(scanner.Tokens());
            std::stringstream outputStream;
            EnvironmentPtr environment = Environment::CreateGlobalEnvironment(outputStream);
            MockedInterpreter interpreter(environment);
            
            for (const IStatementPtr& statement : parser.Parse(std::cerr))
            {
                interpreter.Execute(*statement, environment);
            }
            assert(outputStream.str() == "hi\nyes\n");
        }

        {   // while test
            Scanner scanner(
                "var i = 0;"
                "var a = \"a\";"
                "while (i < 3)"
                "{"
                "a = a + a;"
                "i = i + 1;"
                "}"
                "print a;" 
            );
            MockedParser parser(scanner.Tokens());
            std::stringstream outputStream;
            EnvironmentPtr environment = Environment::CreateGlobalEnvironment(outputStream);
            MockedInterpreter interpreter(environment);

            for (const IStatementPtr& statement : parser.Parse(std::cerr))
            {
                interpreter.Execute(*statement, environment);
            }
            assert(outputStream.str() == "aaaaaaaa\n");
        }

        { // parsing function declaration
            Scanner scanner(
                "fun TestFun(a, b)"
                "{"
                "print a + b;"
                "}"
            );
            MockedParser parser(scanner.Tokens());
            std::vector<IStatementPtr> programm = parser.Parse(std::cerr);
            assert(programm.size() == 1); // function declaration only
            const FunctionDeclarationStatement* functionDeclaration = dynamic_cast<const FunctionDeclarationStatement*>(programm[0].get());
            assert(functionDeclaration->m_name.m_lexeme == "TestFun");
            assert(functionDeclaration->m_parameters.size() == 2);
            assert(functionDeclaration->m_parameters[0].get().m_lexeme == "a");
            assert(functionDeclaration->m_parameters[1].get().m_lexeme == "b");
        }

        { // executing function test
            Scanner scanner(
                "fun TestFun(a, b)"
                "{"
                "print a + b;"
                "}"
                "TestFun(\"a\", \"b\");"
            );
            Parser parser(scanner.Tokens());
            std::stringstream outputStream;

            EnvironmentPtr environment = Environment::CreateGlobalEnvironment(outputStream);
            Interpreter interpreter(environment);

            for (const IStatementPtr& statement : parser.Parse(std::cerr))
            {
                interpreter.Execute(*statement, environment);
            }
            assert(outputStream.str() == "ab\n");
        }

        { // function return test
            Scanner scanner(
                "fun TestFun()"
                "{"
                    "return \"abc\";"
                "}"
                "print TestFun();"
            );
            Parser parser(scanner.Tokens());
            std::stringstream outputStream;
            EnvironmentPtr environment = Environment::CreateGlobalEnvironment(outputStream);
            Interpreter interpreter(environment);

            for (const IStatementPtr& statement : parser.Parse(std::cerr))
            {
                interpreter.Execute(*statement, environment);
            }
            assert(outputStream.str() == "abc\n");
        }
    }
}

int main(int argc, char* argv[])
{
    _CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

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