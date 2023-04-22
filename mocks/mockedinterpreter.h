#pragma once

#include "../interpreter.h"

struct MockedInterpreter : Interpreter
{
    Value Interpret(const IExpression& expression, std::ostream& logOutput)
    {
        try
        {
            return Eval(expression);
        }
        catch(const InterpreterError& ie)
        {
            logOutput << "[line " << ie.m_operator.m_line << "]: " <<  ie.m_message << "\n";
        }

        return Value();
    }
};

