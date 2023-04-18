#include "interpreter.h"
#include "expressions.h"
#include "token.h"

struct InterpreterError : std::exception
{
    InterpreterError(const Token& op, std::string message)
        : m_operator(op)
        , m_message(std::move(message))
    {}

    std::string m_message;
    const Token& m_operator;
};

static bool IsTruthy(const std::any& value)
{
    if (!value.has_value())
    {
        return false;
    }
    else if (const bool* result = std::any_cast<bool>(&value))
    {
        return *result;
    }

    return true;
}

static bool AreEqual(const Token& token, const std::any& lhs, const std::any& rhs)
{
    if (!lhs.has_value())
    {
        return !rhs.has_value();
    }
    else if (const bool* leftExpr = std::any_cast<bool>(&lhs))
    {
        if (const bool* rightExpr = std::any_cast<bool>(&rhs))
        {
            return *leftExpr == *rightExpr;
        }
        else
        {
            throw InterpreterError(token, "Expecting boolean as right hand operand");
        }
    }
    else if (const double* leftExpr = std::any_cast<double>(&lhs))
    {
        if (const double* rightExpr = std::any_cast<double>(&rhs))
        {
            return *leftExpr == *rightExpr;
        }
        else
        {
            throw InterpreterError(token, "Expecting number as right hand operand");
        }
    }
    else if (const std::string* leftExpr = std::any_cast<std::string>(&lhs))
    {
        if (const std::string* rightExpr = std::any_cast<std::string>(&rhs))
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

static double GetNumberOperand(const Token& token, const std::any& lhs)
{
    if (const double* val = std::any_cast<double>(&lhs))
    {
        return *val;
    }

    throw InterpreterError(token, "Operand must be a number.");
}

void Interpreter::VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const
{
    std::any expResult = Eval(*unaryExpression.m_expression);

    Context* interpreterContext = static_cast<Context*>(context);
    if (unaryExpression.m_operator.m_type == Token::Type::Minus)
    {
        interpreterContext->m_result = -GetNumberOperand(unaryExpression.m_operator, expResult);
    }
    else if (unaryExpression.m_operator.m_type == Token::Type::Plus)
    {
        interpreterContext->m_result = GetNumberOperand(unaryExpression.m_operator, expResult);
    }
    else if (unaryExpression.m_operator.m_type == Token::Type::Bang)
    {
        interpreterContext->m_result = std::make_any<bool>(!IsTruthy(expResult));
    }
    else
    {
        throw InterpreterError(unaryExpression.m_operator, "Unsupported unary operator.");
    }
}

void Interpreter::VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const
{
    Context* interpreterContext = static_cast<Context*>(context);

    std::any leftExprResult = Eval(*binaryExpression.m_left);

    Token::Type operatorType = binaryExpression.m_operator.m_type;

    switch (operatorType)
    {
    case Token::Type::EqualEqual:
    case Token::Type::BangEqual:
    {
        std::any rightExprResult = Eval(*binaryExpression.m_right);

        bool result = AreEqual(binaryExpression.m_operator, leftExprResult, rightExprResult);
        interpreterContext->m_result = std::make_any<bool>(operatorType == Token::Type::EqualEqual ? result : !result);        
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
            if (const std::string* lhs = std::any_cast<std::string>(&leftExprResult))
            {
                std::any rightExprResult = Eval(*binaryExpression.m_right);
                if (const std::string* rhs = std::any_cast<std::string>(&rightExprResult))
                {
                    interpreterContext->m_result = *lhs + *rhs;
                }
                else
                {
                    InterpreterError(binaryExpression.m_operator, "Expecting string as right hand operand.");
                }
            }
        }

        double lhs = GetNumberOperand(binaryExpression.m_operator, leftExprResult);

        std::any rightExprResult = Eval(*binaryExpression.m_right);

        double rhs = GetNumberOperand(binaryExpression.m_operator, rightExprResult);

        switch (operatorType)
        {
        case Token::Type::Slash:
        {
            if (rhs == 0.0)
            {
                InterpreterError(binaryExpression.m_operator, "Division by zero.");
            }
            interpreterContext->m_result = lhs / rhs; 
        } break;
        case Token::Type::Star:         interpreterContext->m_result = lhs * rhs; break;
        case Token::Type::Minus:        interpreterContext->m_result = lhs - rhs; break;
        case Token::Type::Plus:         interpreterContext->m_result = lhs + rhs; break;
        case Token::Type::Less:         interpreterContext->m_result = lhs < rhs; break;
        case Token::Type::LessEqual:    interpreterContext->m_result = lhs <= rhs; break;
        case Token::Type::Greater:      interpreterContext->m_result = lhs > rhs; break;
        case Token::Type::GreaterEqual: interpreterContext->m_result = lhs >= rhs; break;
        }
    }
    default: throw InterpreterError(binaryExpression.m_operator, "Unsuported binary operator"); break;
    }    
}

void Interpreter::VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const
{
    std::any conditionResult = Eval(*ternaryConditionalExpression.m_condition);
    if (IsTruthy(conditionResult))
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
    Context* interpreterContext = static_cast<Context*>(context);
    interpreterContext->m_result = literalExpression.m_value; 
}

std::any Interpreter::Eval(const IExpression& expression) const
{
    Context result;
    expression.Accept(*this, &result);
    return result.m_result;
}
