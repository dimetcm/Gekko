#include "interpreter.h"
#include "expressions.h"
#include "token.h"
#include "statements.h"

Interpreter::Environment& Interpreter::GetEnvironment(IExpressionVisitorContext& context)
{
    ExpressionVisitorContext* internalContext = static_cast<ExpressionVisitorContext*>(&context);
    return internalContext->m_environment;
}

Interpreter::Environment& Interpreter::GetEnvironment(IStatementVisitorContext& context)
{
    StatementVisitorContext* internalContext = static_cast<StatementVisitorContext*>(&context);
    return internalContext->m_environment;
}

std::ostream& Interpreter::GetOutputStream(IStatementVisitorContext& context)
{
    StatementVisitorContext* internalContext = static_cast<StatementVisitorContext*>(&context);
    return internalContext->m_outputStream;
}

Interpreter::Environment::Environment(Environment* outer)
    : m_outer(outer)
{}

void Interpreter::Environment::Define(const Token& token, const Value& value)
{
     m_values.insert_or_assign(std::string(token.m_lexeme), value);
}

void Interpreter::Environment::Assign(const Token& token, const Value& value)
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
        throw InterpreterError(token, "Undefined variable '" + name + "'.");
    }
}

Value Interpreter::Environment::GetValue(const Token& token) const
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

    throw InterpreterError(token, "Undefined variable '" + name + "'.");
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

void Interpreter::Interpret(Environment& environment, const std::vector<IStatementPtr>& program, std::ostream& outputStream, std::ostream& errorsLog) const
{
    try
    {
        for (const IStatementPtr& statement : program)
        {
            Execute(*statement, environment, outputStream);
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
    Value value = Eval(*statement.m_expression, GetEnvironment(*context));
    GetOutputStream(*context) << value.ToString() << std::endl;
}

void Interpreter::VisitVariableDeclarationStatement(const VariableDeclarationStatement& statement, IStatementVisitorContext* context) const
{
    Environment& environment = GetEnvironment(*context);
    Value value;
    if (statement.m_initializer)
    {
        value = Eval(*statement.m_initializer, environment);
    }

    environment.Define(statement.m_name, value);
}

void Interpreter::VisitBlockStatement(const BlockStatement& statement, IStatementVisitorContext* context) const
{
    Environment environment(&GetEnvironment(*context));
    for (const IStatementPtr& statement : statement.m_block)
    {
        Execute(*statement, environment, GetOutputStream(*context));
    }
}

void Interpreter::VisitIfStatement(const IfStatement& statement, IStatementVisitorContext* context) const
{
    Environment& environment = GetEnvironment(*context);
    std::ostream& outputStream = GetOutputStream(*context);

    Value conditionResult = Eval(*statement.m_condition, environment);
    if (conditionResult.IsTruthy())
    {
        Execute(*statement.m_trueBranch, environment, outputStream);
    }
    else if (statement.m_falseBranch)
    {
        Execute(*statement.m_falseBranch, environment, outputStream);
    }
}

void Interpreter::VisitWhileStatement(const WhileStatement& statement, IStatementVisitorContext* context) const
{
    Environment& environment = GetEnvironment(*context);
    std::ostream& outputStream = GetOutputStream(*context);

    while (Eval(*statement.m_condition, environment).IsTruthy())
    {
        Execute(*statement.m_body, environment, outputStream);
    }
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
    Environment& environment = GetEnvironment(*context);
    
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
    result->m_result = GetEnvironment(*context).GetValue(variableExpression.m_name);
}

void Interpreter::VisitAssignmentExpression(const AssignmentExpression& assignmentExpression, IExpressionVisitorContext* context) const
{
    Environment& environment = GetEnvironment(*context);
    Value value = Eval(*assignmentExpression.m_expression, GetEnvironment(*context));
    environment.Assign(assignmentExpression.m_name, value);
    
    ExpressionVisitorContext* result = static_cast<ExpressionVisitorContext*>(context);
    result->m_result = value;
}

void Interpreter::VisitLogicalExpression(const LogicalExpression& logicalExpression, IExpressionVisitorContext* context) const
{
    Environment& environment = GetEnvironment(*context);
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

void Interpreter::Execute(const IStatement& statement, Environment& environment, std::ostream& outputStream) const
{
    StatementVisitorContext context(environment, outputStream);
    statement.Accept(*this, &context);
}

Value Interpreter::Eval(const IExpression& expression, Environment& environment) const
{
    ExpressionVisitorContext context(environment);
    expression.Accept(*this, &context);
    return context.m_result;
}
