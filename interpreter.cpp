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

void Interpreter::Environment::Define(const std::string& name, const Value& value)
{
     m_values.insert_or_assign(name, value);
}

const Value* Interpreter::Environment::GetValue(const std::string& name) const
{
    auto it = m_values.find(name);
    if (it != m_values.end())
    {
        return &it->second;
    }

    return nullptr;
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

void Interpreter::Interpret(Environment& environment, const std::vector<IStatementPtr>& program, std::ostream& logOutput) const
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
        logOutput << "[line " << ie.m_operator.m_line << "]: " <<  ie.m_message << "\n";
    }
}

void Interpreter::VisitExpressionStatement(const ExpressionStatement& statement, IStatementVisitorContext* context) const
{
    Eval(*statement.m_expression, GetEnvironment(*context));
}

void Interpreter::VisitPrintStatement(const PrintStatement& statement, IStatementVisitorContext* context) const
{
    Value value = Eval(*statement.m_expression, GetEnvironment(*context));
    std::cout << value.ToString() << std::endl;
}

void Interpreter::VisitVariableDeclarationStatement(const VariableDeclarationStatement& statement, IStatementVisitorContext* context) const
{
    Environment& environment = GetEnvironment(*context);
    Value value;
    if (statement.m_initializer)
    {
        value = Eval(*statement.m_initializer, environment);
    }

    environment.Define(std::string(statement.m_name.m_lexeme), value);
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
    if (const Value* value = result->m_environment.GetValue(std::string(variableExpression.m_name.m_lexeme)))
    {
        result->m_result = *value;
    }
    else
    {
        throw InterpreterError(variableExpression.m_name, "Undefined variable '" + std::string(variableExpression.m_name.m_lexeme) + "'.");
    }
}

void Interpreter::VisitAssignmentExpression(const AssignmentExpression& assignmentExpression, IExpressionVisitorContext* context) const
{
        throw InterpreterError(assignmentExpression.m_name, "NOT IMPLEMENTED" );
}

void Interpreter::Execute(const IStatement& statement, Environment& environment) const
{
    StatementVisitorContext context(environment);
    statement.Accept(*this, &context);
}

Value Interpreter::Eval(const IExpression& expression, Environment& environment) const
{
    ExpressionVisitorContext context(environment);
    expression.Accept(*this, &context);
    return context.m_result;
}
