#include "interpreter.h"
#include "expressions.h"
#include "token.h"
#include "statements.h"

void Interpreter::Environment::Define(std::string_view name, const Value& value)
{
     m_values.insert_or_assign(name, value);
}

const Value* Interpreter::Environment::GetValue(std::string_view name) const
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

void Interpreter::Interpret(const std::vector<IStatementPtr>& program, std::ostream& logOutput) const
{
    try
    {
        Environment environment;
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
    Eval(*statement.m_expression);
}

void Interpreter::VisitPrintStatement(const PrintStatement& statement, IStatementVisitorContext* context) const
{
    Value value = Eval(*statement.m_expression);
    std::cout << value.ToString() << std::endl;
}

void Interpreter::VisitVariableDeclarationStatement(const VariableDeclarationStatement& statement, IStatementVisitorContext* context) const
{
    Value value;
    if (statement.m_initializer)
    {
        value = Eval(*statement.m_initializer);
    }

    Environment* environment = static_cast<Environment*>(context);
    environment->Define(statement.m_name.m_lexeme, value);
}


void Interpreter::VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const
{
    Value expResult = Eval(*unaryExpression.m_expression);

    ExpressionResult* result = static_cast<ExpressionResult*>(context);
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
    ExpressionResult* exprResult = static_cast<ExpressionResult*>(context);

    Value leftExprResult = Eval(*binaryExpression.m_left);

    Token::Type operatorType = binaryExpression.m_operator.m_type;

    switch (operatorType)
    {
    case Token::Type::EqualEqual:
    case Token::Type::BangEqual:
    {
        Value rightExprResult = Eval(*binaryExpression.m_right);

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
                Value rightExprResult = Eval(*binaryExpression.m_right);
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

        Value rightExprResult = Eval(*binaryExpression.m_right);

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
    Value conditionResult = Eval(*ternaryConditionalExpression.m_condition);
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
    ExpressionResult* result = static_cast<ExpressionResult*>(context);
    result->m_result = literalExpression.m_value; 
}

void Interpreter::Execute(const IStatement& statement, Environment& environment) const
{
    statement.Accept(*this, &environment);
}

Value Interpreter::Eval(const IExpression& expression) const
{
    ExpressionResult result;
    expression.Accept(*this, &result);
    return result.m_result;
}
