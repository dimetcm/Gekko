#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "scanner.h"

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

void runFile(char* filename)
{
    std::cout << "running file: " << filename << std::endl;

    std::ifstream script(filename);
    if (script.is_open())
    {
        std::stringstream buffer;
        buffer << script.rdbuf();

        run(buffer.str());
        script.close();
    }
    else
    {
        std::cout << "can't open file: " << filename << std::endl;
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

int main(int argc, char* argv[])
{
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