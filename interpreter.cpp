#include "interpreter.h"
#include "expressions.h"
#include "token.h"
#include "statements.h"
#include "nativefunctions.h"
#include "function.h"
#include <assert.h>
#include <sstream>

EnvironmentPtr Interpreter::GetEnvironment(IExpressionVisitorContext& context)
{
    ExpressionVisitorContext* internalContext = static_cast<ExpressionVisitorContext*>(&context);
    return internalContext->m_environment;
}

EnvironmentPtr Interpreter::GetEnvironment(IStatementVisitorContext& context)
{
    StatementVisitorContext* internalContext = static_cast<StatementVisitorContext*>(&context);
    return internalContext->m_environment;
}

EnvironmentPtr Environment::CreateGlobalEnvironment(std::ostream& outputStream)
{
    return std::shared_ptr<Environment>(new Environment(outputStream));
}

EnvironmentPtr Environment::CreateLocalEnvironment(EnvironmentPtr outer)
{
    return std::shared_ptr<Environment>(new Environment(outer));
}

Environment::Environment(std::ostream& output)
    : m_outputStream(output)
{}

Environment::Environment(EnvironmentPtr outer)
    : m_outer(outer)
    , m_outputStream(outer->m_outputStream)
{
    assert(!m_outer->m_break);
}

Environment::~Environment()
{
    if (m_outer)
    {   
        m_outer->m_break = m_break;
    }
    else
    {
        assert(!m_break);
    }
}

void Environment::Define(std::string_view name, const Value& value)
{
     m_values.insert_or_assign(std::string(name), value);
}

void Environment::Assign(const Token& token, const Value& value)
{
    std::string name(token.m_lexeme);
    auto it = m_values.find(name);
    if (it != m_values.end())
    {
        it->second = value;
    }
    else if (m_outer)
    {
         m_outer->Assign(token, value);
    }
    else
    {
        throw Interpreter::InterpreterError(token, "Undefined variable '" + name + "'.");
    }
}

Value Environment::GetValue(const Token& token) const
{
    std::string name(token.m_lexeme);
    auto it = m_values.find(name);
    if (it != m_values.end())
    {
        return it->second;
    }
    else if (m_outer)
    {
        return m_outer->GetValue(token);
    }

    throw Interpreter::InterpreterError(token, "Undefined variable '" + name + "'.");
}

void Environment::RequestBreak()
{ 
    assert(!m_return);
    assert(!m_break);
    m_break = true;
}

void Environment::ClearBreak()
{
    assert(m_break);
    m_break = false;
}


bool Environment::BreakRequested() const
{
    return m_break;
}

void Environment::RequestReturn(Value returnValue)
{
    assert(!m_return);
    assert(!m_break);
    m_return = true;
    m_returnValue = returnValue;
}

void Environment::ClearReturn()
{
    assert(m_return);
    m_return = false;
    m_returnValue = Value();
}

bool Environment::ReturnRequested() const
{
    return m_return;
}

const Value& Environment::GetReturnValue() const
{
    assert(m_return);
    return m_returnValue;
}

EnvironmentPtr Environment::GetGlobalEnvironment()
{
    if (m_outer)
    {
        return m_outer->GetGlobalEnvironment();
    }

    return shared_from_this();
}

std::ostream& Environment::GetOutputStream()
{
    return m_outputStream;
}

Interpreter::Interpreter(EnvironmentPtr environment)
{
    RegisterNativeFunctions(environment);
}

bool Interpreter::AreEqual(const Token& token, const Value& lhs, const Value& rhs)
{
    if (!lhs.HasValue())
    {
        return !rhs.HasValue();
    }
    else if (const bool* leftExpr = lhs.GetBoolean())
    {
        if (const bool* rightExpr = rhs.GetBoolean())
        {
            return *leftExpr == *rightExpr;
        }
        else
        {
            throw InterpreterError(token, "Expecting boolean as right hand operand");
        }
    }
    else if (const double* leftExpr = lhs.GetNumber())
    {
        if (const double* rightExpr = rhs.GetNumber())
        {
            return *leftExpr == *rightExpr;
        }
        else
        {
            throw InterpreterError(token, "Expecting number as right hand operand");
        }
    }
    else if (const std::string* leftExpr = lhs.GetString())
    {
        if (const std::string* rightExpr = rhs.GetString())
        {
            return *leftExpr == *rightExpr;
        }
        else
        {
            throw InterpreterError(token, "Expecting string as right hand operand");
        }
    }

    throw InterpreterError(token, "Unsuported left operand type");
}

double Interpreter::GetNumberOperand(const Token& token, const Value& lhs)
{
    if (const double* val = lhs.GetNumber())
    {
        return *val;
    }

    throw InterpreterError(token, "Operand must be a number.");
}

void Interpreter::Interpret(EnvironmentPtr environment, const std::vector<IStatementPtr>& program, std::ostream& errorsLog) const
{
    try
    {
        for (const IStatementPtr& statement : program)
        {
            Execute(*statement, environment);
        }
    }
    catch(const InterpreterError& ie)
    {
        errorsLog << "[line " << ie.m_operator.m_line << "]: " <<  ie.m_message << "\n";
    }
}

void Interpreter::VisitExpressionStatement(const ExpressionStatement& statement, IStatementVisitorContext* context) const
{
    Eval(*statement.m_expression, GetEnvironment(*context));
}

void Interpreter::VisitPrintStatement(const PrintStatement& statement, IStatementVisitorContext* context) const
{
    EnvironmentPtr environment = GetEnvironment(*context);
    Value value = Eval(*statement.m_expression, environment);
    environment->GetOutputStream() << value.ToString() << std::endl;
}

void Interpreter::VisitVariableDeclarationStatement(const VariableDeclarationStatement& statement, IStatementVisitorContext* context) const
{
    EnvironmentPtr environment = GetEnvironment(*context);
    Value value;
    if (statement.m_initializer)
    {
        value = Eval(*statement.m_initializer, environment);
    }

    environment->Define(statement.m_name.m_lexeme, value);
}

void Interpreter::VisitFunctionDeclarationStatement(const FunctionDeclarationStatement& statement, IStatementVisitorContext* context) const
{
    EnvironmentPtr environment = GetEnvironment(*context);

    std::shared_ptr<const ICallable> valuePtr = std::make_shared<const Function>(statement, environment);
    environment->Define(statement.m_name.m_lexeme, Value(valuePtr));
}

void Interpreter::VisitBlockStatement(const BlockStatement& statement, IStatementVisitorContext* context) const
{
    EnvironmentPtr outer = GetEnvironment(*context);
    EnvironmentPtr inner = Environment::CreateLocalEnvironment(outer);
    for (const IStatementPtr& statement : statement.m_block)
    {
        Execute(*statement, inner);
        if (inner->BreakRequested())
        {
            break;
        }

        if (inner->ReturnRequested())
        {
            outer->RequestReturn(inner->GetReturnValue());
            break;
        }
    }
}

void Interpreter::VisitIfStatement(const IfStatement& statement, IStatementVisitorContext* context) const
{
    EnvironmentPtr environment = GetEnvironment(*context);

    Value conditionResult = Eval(*statement.m_condition, environment);
    if (conditionResult.IsTruthy())
    {
        Execute(*statement.m_trueBranch, environment);
    }
    else if (statement.m_falseBranch)
    {
        Execute(*statement.m_falseBranch, environment);
    }
}

void Interpreter::VisitWhileStatement(const WhileStatement& statement, IStatementVisitorContext* context) const
{
    EnvironmentPtr environment = GetEnvironment(*context);

    while (Eval(*statement.m_condition, environment).IsTruthy())
    {
        Execute(*statement.m_body, environment);
        if (environment->BreakRequested())
        {
            environment->ClearBreak();
            break;
        }

        if (environment->ReturnRequested())
        {
            break;
        }
    }
}

void Interpreter::VisitBreakStatement(const BreakStatement& statement, IStatementVisitorContext* context) const
{
    GetEnvironment(*context)->RequestBreak();
}

void Interpreter::VisitReturnStatement(const ReturnStatement& statement, IStatementVisitorContext* context) const
{
    Value returnValue = Eval(*statement.m_returnValue, GetEnvironment(*context));
    GetEnvironment(*context)->RequestReturn(returnValue);
}

void Interpreter::VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const
{
    Value expResult = Eval(*unaryExpression.m_expression, GetEnvironment(*context));

    ExpressionVisitorContext* result = static_cast<ExpressionVisitorContext*>(context);
    if (unaryExpression.m_operator.m_type == Token::Type::Minus)
    {
        double number = GetNumberOperand(unaryExpression.m_operator, expResult);
        result->m_result = Value(-number);
    }
    else if (unaryExpression.m_operator.m_type == Token::Type::Plus)
    {
        double number = GetNumberOperand(unaryExpression.m_operator, expResult);
        result->m_result = Value(number);
    }
    else if (unaryExpression.m_operator.m_type == Token::Type::Bang)
    {
        result->m_result = Value(!expResult.IsTruthy());
    }
    else
    {
        throw InterpreterError(unaryExpression.m_operator, "Unsupported unary operator.");
    }
}

void Interpreter::VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const
{
    EnvironmentPtr environment = GetEnvironment(*context);
    
    ExpressionVisitorContext* exprResult = static_cast<ExpressionVisitorContext*>(context);

    Value leftExprResult = Eval(*binaryExpression.m_left, environment);

    Token::Type operatorType = binaryExpression.m_operator.m_type;

    switch (operatorType)
    {
    case Token::Type::EqualEqual:
    case Token::Type::BangEqual:
    {
        Value rightExprResult = Eval(*binaryExpression.m_right, environment);

        bool result = AreEqual(binaryExpression.m_operator, leftExprResult, rightExprResult);
        exprResult->m_result = Value(operatorType == Token::Type::EqualEqual ? result : !result);        
    } break;

    case Token::Type::Minus:
    case Token::Type::Plus:
    case Token::Type::Slash:
    case Token::Type::Star:
    case Token::Type::Less:
    case Token::Type::LessEqual:
    case Token::Type::Greater:
    case Token::Type::GreaterEqual:
    {
        if (Token::Type::Plus == operatorType)
        {
            if (const std::string* lhs = leftExprResult.GetString())
            {
                Value rightExprResult = Eval(*binaryExpression.m_right, environment);
                if (const std::string* rhs = rightExprResult.GetString())
                {
                    exprResult->m_result = Value(*lhs + *rhs);
                    return;
                }
                else
                {
                    throw InterpreterError(binaryExpression.m_operator, "Expecting string as right hand operand.");
                }
            }
        }

        double lhs = GetNumberOperand(binaryExpression.m_operator, leftExprResult);

        Value rightExprResult = Eval(*binaryExpression.m_right, environment);

        double rhs = GetNumberOperand(binaryExpression.m_operator, rightExprResult);

        switch (operatorType)
        {
        case Token::Type::Slash:
        {
            if (rhs == 0.0)
            {
                throw InterpreterError(binaryExpression.m_operator, "Division by zero.");
            }
            exprResult->m_result = Value(lhs / rhs); 
        } break;
        case Token::Type::Star:         exprResult->m_result = Value(lhs * rhs); break;
        case Token::Type::Minus:        exprResult->m_result = Value(lhs - rhs); break;
        case Token::Type::Plus:         exprResult->m_result = Value(lhs + rhs); break;
        case Token::Type::Less:         exprResult->m_result = Value(lhs < rhs); break;
        case Token::Type::LessEqual:    exprResult->m_result = Value(lhs <= rhs); break;
        case Token::Type::Greater:      exprResult->m_result = Value(lhs > rhs); break;
        case Token::Type::GreaterEqual: exprResult->m_result = Value(lhs >= rhs); break;
        }
    } break;
    default: throw InterpreterError(binaryExpression.m_operator, "Unsuported binary operator"); break;
    }    
}

void Interpreter::VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const
{
    Value conditionResult = Eval(*ternaryConditionalExpression.m_condition, GetEnvironment(*context));
    if (conditionResult.IsTruthy())
    {
        ternaryConditionalExpression.m_trueBranch->Accept(*this, context);
    }
    else
    {
        ternaryConditionalExpression.m_falseBranch->Accept(*this, context);
    }
}

void Interpreter::VisitGroupingExpression(const GroupingExpression& groupingExpression, IExpressionVisitorContext* context) const
{
    groupingExpression.m_expression->Accept(*this, context);
}

void Interpreter::VisitLiteralExpression(const LiteralExpression& literalExpression, IExpressionVisitorContext* context) const
{
    ExpressionVisitorContext* result = static_cast<ExpressionVisitorContext*>(context);
    result->m_result = literalExpression.m_value; 
}

void Interpreter::VisitVariableExpression(const VariableExpression& variableExpression, IExpressionVisitorContext* context) const
{
    ExpressionVisitorContext* result = static_cast<ExpressionVisitorContext*>(context);
    result->m_result = GetEnvironment(*context)->GetValue(variableExpression.m_name);
}

void Interpreter::VisitAssignmentExpression(const AssignmentExpression& assignmentExpression, IExpressionVisitorContext* context) const
{
    EnvironmentPtr environment = GetEnvironment(*context);
    Value value = Eval(*assignmentExpression.m_expression, GetEnvironment(*context));
    environment->Assign(assignmentExpression.m_name, value);
    
    ExpressionVisitorContext* result = static_cast<ExpressionVisitorContext*>(context);
    result->m_result = value;
}

void Interpreter::VisitLogicalExpression(const LogicalExpression& logicalExpression, IExpressionVisitorContext* context) const
{
    EnvironmentPtr environment = GetEnvironment(*context);
    ExpressionVisitorContext* result = static_cast<ExpressionVisitorContext*>(context);

    Value left = Eval(*logicalExpression.m_left, environment);
    if (logicalExpression.m_operator.m_type == Token::Type::Or)
    {
        result->m_result = left.IsTruthy() ? left : Eval(*logicalExpression.m_right, environment);
    }
    else if (logicalExpression.m_operator.m_type == Token::Type::And)
    {
        result->m_result = !left.IsTruthy() ? left : Eval(*logicalExpression.m_right, environment);
    }
    else
    {
        throw InterpreterError(logicalExpression.m_operator, "unsupported logical operator");
    }
}

void Interpreter::VisitCallExpression(const CallExpression& callExpression, IExpressionVisitorContext* context) const
{
    Value calle = Eval(*callExpression.m_calle, GetEnvironment(*context));

    if (!calle.GetCallable())
    {
        throw InterpreterError(callExpression.m_token, "Can only call functions and classes.");
    }

    const ICallable* callable = calle.GetCallable();

    assert(callable);

    if (callable->Arity() != callExpression.m_arguments.size())
    {
        std::stringstream message;
        message << "Expected " << callable->Arity() << " arguments, but got " << callExpression.m_arguments.size() << '.';   
        throw InterpreterError(callExpression.m_token, message.str());
    }

    std::vector<Value> arguments;
    arguments.reserve(callExpression.m_arguments.size());

    for (const IExpressionPtr& expression : callExpression.m_arguments)
    {
        arguments.emplace_back(Eval(*expression, GetEnvironment(*context)));
    }

    ExpressionVisitorContext* result = static_cast<ExpressionVisitorContext*>(context);
    result->m_result = callable->Call(*this, GetEnvironment(*context)->GetGlobalEnvironment(), arguments);
}

void Interpreter::Execute(const IStatement& statement, EnvironmentPtr environment) const
{
    StatementVisitorContext context(environment);
    statement.Accept(*this, &context);
}

Value Interpreter::Eval(const IExpression& expression, EnvironmentPtr environment) const
{
    ExpressionVisitorContext context(environment);
    expression.Accept(*this, &context);
    return context.m_result;
}

void Interpreter::RegisterNativeFunctions(EnvironmentPtr environment) const
{
    std::shared_ptr<const ICallable> clockCallable = std::make_shared<const ClockCallable>();
    environment->Define("clock", Value(clockCallable));
}