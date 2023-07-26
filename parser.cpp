#include "parser.h"
#include "statements.h"
#include "expressions.h"
#include "expressionvisitor.h"
#include "gekko.h"
#include <assert.h>

enum class FunctionType
{
    Function,
    ClassMethod
};

Parser::Parser(const std::vector<Token>& tokens)
    : m_tokens(tokens)
{}

std::vector<IStatementPtr> Parser::Parse(std::ostream& logOutput)
{
    std::vector<IStatementPtr> programme;

    try
    {
        while (!Match(Token::Type::EndOfFile))
        {
           programme.push_back(ParseDeclaration());
        }
    }
    catch(const ParseError& pe)
    {
        if (pe.m_token.m_type == Token::Type::EndOfFile)
        {
            Gekko::ReportError(pe.m_token.m_line, " at end", pe.m_message);
        }
        else
        {
            Gekko::ReportError(pe.m_token.m_line, " at '" + std::string(pe.m_token.m_lexeme) + "'", pe.m_message);
        }

        Synchronize();
    }
    catch(const std::exception& e)
    {
        logOutput << e.what() << '\n';
    }

    return programme;
}

IExpressionPtr Parser::ParseBinaryExpression(std::function<IExpressionPtr()> exprFunc, std::initializer_list<Token::Type> tokenTypes)
{
    for (Token::Type tokenType : tokenTypes)
    {
        if (!CanBeUnary(tokenType) && Match(tokenType))
        {
            const Token& op = m_tokens[m_current++];
            exprFunc(); // discard right handed expression in case of an error
            throw ParseError(op, "Binary operator appearing at the beginning of an expression");
        }
    }

    IExpressionPtr left = exprFunc();
    while (MatchAny(tokenTypes))
    {
        const Token& op = m_tokens[m_current++];

        IExpressionPtr right = exprFunc();
        left = std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
    }
    
    return left;
}

IStatementPtr Parser::ParseDeclaration()
{
    if (ConsumeIfMatch(Token::Type::Class))
    {
        return ParseClassDeclaration();
    }
    else if (ConsumeIfMatch(Token::Type::Var))
    {
        return ParseVariableDeclaration();
    }
    else if (ConsumeIfMatch(Token::Type::Fun))
    {
        return ParseFunctionDeclaration(FunctionType::Function);
    }

    return ParseStatement();
}

IStatementPtr Parser::ParseClassDeclaration()
{
    const Token& name = Consume(Token::Type::Identifier, "Expect class name.");

    Consume(Token::Type::OpeningBrace, "Expect '{' before class body.");

    std::vector<IStatementPtr> methods;
    while (CurrentToken().m_type != Token::Type::ClosingBrace && CurrentToken().m_type != Token::Type::EndOfFile)
    {
        methods.push_back(ParseFunctionDeclaration(FunctionType::ClassMethod));
    }
    

    Consume(Token::Type::ClosingBrace, "Expect '}' after class body.");

    return std::make_unique<ClassDeclarationStatement>(name, std::move(methods));
}

IStatementPtr Parser::ParseVariableDeclaration()
{
    const Token& name = Consume(Token::Type::Identifier, "Expect variable name.");

    IExpressionPtr initializer;
    if (ConsumeIfMatch(Token::Type::Equal))
    {
        initializer = ParseExpression();
    }

    Consume(Token::Type::Semicolon, "Expect ';' after variable declaration.");
    return std::make_unique<VariableDeclarationStatement>(name, std::move(initializer));
}

IStatementPtr Parser::ParseFunctionDeclaration(FunctionType functionType)
{
    const std::string functionTypeName = functionType == FunctionType::Function ? "function" : "class method"; 
    const Token& name = Consume(Token::Type::Identifier, "Expect " + functionTypeName + " name.");
    Consume(Token::Type::OpeningParenthesis, "Expect '(' after " + functionTypeName + " name.");

    FunctionDeclarationStatement::ParametersType parameters;

    if (!Match(Token::Type::ClosingParenthesis))
    {
        do
        {
            parameters.emplace_back(Consume(Token::Type::Identifier, "Expect parameter name."));
            if (parameters.size() >= 255)
            {
                Gekko::ReportError(CurrentToken(), "Can't have more than 255 parameters.");
            }

        } while (ConsumeIfMatch(Token::Type::Comma));
        
    }

    Consume(Token::Type::ClosingParenthesis, "Expect ')' after " + functionTypeName + " parameters.");

    Consume(Token::Type::OpeningBrace, "Expect '{' before " + functionTypeName + " body.");

    FunctionDeclarationStatement::BodyType functionBody = ParseBlock();
    return std::make_unique<FunctionDeclarationStatement>(name, std::move(parameters), std::move(functionBody));
}

std::vector<IStatementPtr> Parser::ParseBlock()
{
    std::vector<IStatementPtr> block;
    while (!Match(Token::Type::ClosingBrace) && !Match(Token::Type::EndOfFile))
    {
        block.push_back(ParseDeclaration());
    }

    Consume(Token::Type::ClosingBrace, "Expect '}' after block.");

    return block;
}

IStatementPtr Parser::ParseStatement()
{
    if (ConsumeIfMatch(Token::Type::Print))
    {
        return ParsePrintStatement();
    }
    else if (ConsumeIfMatch(Token::Type::If))
    {
        return ParseIfStatement();
    }
    else if (ConsumeIfMatch(Token::Type::While))
    {
        return ParseWhileStatement();
    }
    else if (ConsumeIfMatch(Token::Type::For))
    {
        return ParseForStatement();
    } 
    else if (ConsumeIfMatch(Token::Type::Break))
    {
        return ParseBreakStatement();
    }
    else if (ConsumeIfMatch(Token::Type::Return))
    {
        return ParseReturnStatement();
    }
    else if (ConsumeIfMatch(Token::Type::OpeningBrace))
    {
        return ParseBlockStatement();
    }

    return ParseExpressionStatement();
}

IStatementPtr Parser::ParseIfStatement()
{
    Consume(Token::Type::OpeningParenthesis, "Expect '(' after 'if'.");
    IExpressionPtr condition = ParseExpression();
    Consume(Token::Type::ClosingParenthesis, "Expect ')' after if condition.");

    IStatementPtr trueBranch = ParseStatement();
    IStatementPtr falseBranch = nullptr;
    if (ConsumeIfMatch(Token::Type::Else))
    {
        falseBranch = ParseStatement();
    }
    
    return std::make_unique<IfStatement>(std::move(condition), std::move(trueBranch), std::move(falseBranch));
}

IStatementPtr Parser::ParseWhileStatement()
{
    Consume(Token::Type::OpeningParenthesis, "Expect '(' after 'while'.");
    IExpressionPtr condition = ParseExpression();
    Consume(Token::Type::ClosingParenthesis, "Expect ')' after while condition.");

    IStatementPtr statement = ParseStatement();
    return std::make_unique<WhileStatement>(std::move(condition), std::move(statement));
}

IStatementPtr Parser::ParseForStatement()
{
    Consume(Token::Type::OpeningParenthesis, "Expect '(' after 'for'.");
    IStatementPtr initStatement;

    if (!ConsumeIfMatch(Token::Type::Semicolon))
    {
        initStatement = ConsumeIfMatch(Token::Type::Var) ? ParseVariableDeclaration() : ParseExpressionStatement();
    }

    IExpressionPtr condition;
    if (!Match(Token::Type::Semicolon))
    {
        condition = ParseExpression();
    }
    
    Consume(Token::Type::Semicolon, "Expect ';' after loop condition.");
 
    IExpressionPtr increment;
    if (!Match(Token::Type::Semicolon))
    {
        increment = ParseExpression();
    }

    Consume(Token::Type::ClosingParenthesis, "Expect ')' after loop increment.");

    IStatementPtr body = ParseStatement();

    std::vector<IStatementPtr> bodyWithIncrement;
    bodyWithIncrement.push_back(std::move(body));
    if (increment)
    {
        IStatementPtr incrementStatement = std::make_unique<ExpressionStatement>(std::move(increment));
        bodyWithIncrement.push_back(std::move(incrementStatement));
    }

    IStatementPtr whileBody = std::make_unique<BlockStatement>(std::move(bodyWithIncrement));
    IStatementPtr whileStatement = std::make_unique<WhileStatement>(std::move(condition), std::move(whileBody));

    std::vector<IStatementPtr> InitializerWithWhile;
    if (initStatement)    
    {
        InitializerWithWhile.push_back(std::move(initStatement));
    }

    InitializerWithWhile.push_back(std::move(whileStatement));

    return std::make_unique<BlockStatement>(std::move(InitializerWithWhile));
}

IStatementPtr Parser::ParseBreakStatement()
{
    const Token& keyword = PreviousToken();
    Consume(Token::Type::Semicolon, "Expect ';' after break.");
    return std::make_unique<BreakStatement>(keyword);
}

IStatementPtr Parser::ParseReturnStatement()
{
    const Token& returnKeyword = PreviousToken();

    IExpressionPtr returnValue;
    if (!Match(Token::Type::Semicolon))
    {
        returnValue = ParseExpression();
    }

    Consume(Token::Type::Semicolon, "Expect ';' after return value.");
    return std::make_unique<ReturnStatement>(std::move(returnValue), returnKeyword);
}

IStatementPtr Parser::ParseBlockStatement()
{
    return std::make_unique<BlockStatement>(ParseBlock());
}

IStatementPtr Parser::ParsePrintStatement()
{
    IExpressionPtr value = ParseExpression();
    Consume(Token::Type::Semicolon, "Expect ';' after value.");

    return std::make_unique<PrintStatement>(std::move(value));
}

IStatementPtr Parser::ParseExpressionStatement()
{
    IExpressionPtr expression = ParseExpression();
    Consume(Token::Type::Semicolon, "Expect ';' after expression.");
    
    return std::make_unique<ExpressionStatement>(std::move(expression));
}

IExpressionPtr Parser::ParseExpression()
{
    return ParseComma();
}

IExpressionPtr Parser::ParseComma()
{
    return ParseBinaryExpression(std::bind(&Parser::ParseAssignment, this), {Token::Type::Comma});
}

IExpressionPtr Parser::ParseAssignment()
{
   IExpressionPtr expression = ParseTernaryConditional(); 
    if (ConsumeIfMatch(Token::Type::Equal))
    {
        struct VariableGetter : IExpressionVisitor, IExpressionVisitorContext
        {
            void VisitVariableExpression(const VariableExpression& variableExpression, IExpressionVisitorContext* context) const
            {
                VariableGetter* getter = static_cast<VariableGetter*>(context);
                getter->m_result = &variableExpression.m_name;
            }

            const Token* m_result = nullptr;
        } getVariableVisitor;

        expression->Accept(getVariableVisitor, &getVariableVisitor);

        if (getVariableVisitor.m_result)
        {
            IExpressionPtr value = ParseAssignment();
            return std::make_unique<AssignmentExpression>(*getVariableVisitor.m_result, std::move(value));
        }
        else
        {
            struct IsGetExpression : IExpressionVisitor, IExpressionVisitorContext
            {
                void VisitGetExpression(const GetExpression& variableExpression, IExpressionVisitorContext* context) const
                {
                    IsGetExpression* me = static_cast<IsGetExpression*>(context);
                    me->m_result = true;
                    me->m_expressionName = &variableExpression.m_name;
                }

                bool m_result = false;
                const Token* m_expressionName = nullptr;
            } isGetExpressionVisitor;

            if (isGetExpressionVisitor.m_result)
            {
                IExpressionPtr value = ParseAssignment();
                return std::make_unique<SetExpression>(std::move(expression), *isGetExpressionVisitor.m_expressionName, std::move(value));
            }
            else
            {
                ParseError(m_tokens[m_current], "Invalid assignment target.");
            }
        }
    }

    return expression;
 }

IExpressionPtr Parser::ParseTernaryConditional()
{
    IExpressionPtr expression = ParseOr();

    while (ConsumeIfMatch(Token::Type::Questionmark))
    {
        IExpressionPtr trueBranch = ParseExpression();

        if (ConsumeIfMatch(Token::Type::Colon))
        {
            IExpressionPtr falseBranch = ParseExpression();
            expression = std::make_unique<TernaryConditionalExpression>(std::move(expression), std::move(trueBranch), std::move(falseBranch));
        }
        else
        {
            throw ParseError(m_tokens[m_current], "missing colon ':' after questionmark '?' in ternary conditional operator.");
        }
    }

    return expression;
}

IExpressionPtr Parser::ParseLogicalExpression(std::function<IExpressionPtr()> exprFunc, Token::Type tokenType)
{
    IExpressionPtr expression = exprFunc();
    if (Match(tokenType))
    {
        const Token& token = m_tokens[m_current++];
        IExpressionPtr right = exprFunc();
        return std::make_unique<LogicalExpression>(std::move(expression), token, std::move(right));
    }

    return expression;    
}

IExpressionPtr Parser::ParseOr()
{
    return ParseLogicalExpression(std::bind(&Parser::ParseAnd, this), Token::Type::Or);
}

IExpressionPtr Parser::ParseAnd()
{
    return ParseLogicalExpression(std::bind(&Parser::ParseEquality, this), Token::Type::And);
}

IExpressionPtr Parser::ParseEquality()
{
    return ParseBinaryExpression(std::bind(&Parser::ParseComparison, this), {Token::Type::BangEqual, Token::Type::EqualEqual});
} 

IExpressionPtr Parser::ParseComparison()
{    
    return ParseBinaryExpression(std::bind(&Parser::ParseTerm, this), {Token::Type::Greater, Token::Type::GreaterEqual, Token::Type::LessEqual, Token::Type::Less});
} 

IExpressionPtr Parser::ParseTerm()
{
    return ParseBinaryExpression(std::bind(&Parser::ParseFactor, this), {Token::Type::Minus, Token::Type::Plus});
}

IExpressionPtr Parser::ParseFactor()
{
    return ParseBinaryExpression(std::bind(&Parser::ParseUnary, this), {Token::Type::Slash, Token::Type::Star});
}

IExpressionPtr Parser::ParseUnary()
{
    if (MatchAny({Token::Type::Bang, Token::Type::Minus}))
    {
        const Token& op = m_tokens[m_current++];
        return std::make_unique<UnaryExpression>(op, ParseUnary());
    }

    return ParseCall();
}

IExpressionPtr Parser::ParseCall()
{
    IExpressionPtr expression = ParsePrimary();
    while (true)
    {
        if (ConsumeIfMatch(Token::Type::OpeningParenthesis))
        {
            std::vector<IExpressionPtr> argumets;
            if (!Match(Token::Type::ClosingParenthesis))
            {
                do
                {
                    argumets.emplace_back(ParseAssignment());
                    if (argumets.size() >= 255)
                    {
                        Gekko::ReportError(CurrentToken(), "Can't have more than 255 arguments.");
                    }
                } while (ConsumeIfMatch(Token::Type::Comma));
            }

            Consume(Token::Type::ClosingParenthesis, "Expect ')' after arguments.");
            expression = std::make_unique<CallExpression>(std::move(expression), PreviousToken(), std::move(argumets));
        }
        else if (ConsumeIfMatch(Token::Type::Dot))
        {
            const Token& name = Consume(Token::Type::Identifier, "Expect property name after '.'.");
            expression = std::make_unique<GetExpression>(std::move(expression),  name);   
        }
        else
        {
            break;
        }
    }

    return expression;
}

IExpressionPtr Parser::ParsePrimary()
{
    const Token& token = m_tokens[m_current++];
    switch (token.m_type)
    {
    case Token::Type::False:    return std::make_unique<LiteralExpression>(Value(false)); 
    case Token::Type::True:     return std::make_unique<LiteralExpression>(Value(true));
    case Token::Type::Nil:      return std::make_unique<LiteralExpression>();
    case Token::Type::Number:
    case Token::Type::String:   return std::make_unique<LiteralExpression>(Value(token.m_literalvalue));
    case Token::Type::OpeningParenthesis:
    {
        IExpressionPtr expression = ParseExpression();
        if (ConsumeIfMatch(Token::Type::ClosingParenthesis))
        {
            return std::make_unique<GroupingExpression>(std::move(expression));
        }
        else
        {
            throw ParseError(token, "Expect ')' after expression.");
        }
    }
    case Token::Type::Identifier:   return std::make_unique<VariableExpression>(token);
    case Token::Type::Fun:
    {
        Consume(Token::Type::OpeningParenthesis, "Expect '(' after lambda.");
        LambdaExpression::ParametersType parameters;

        if (!Match(Token::Type::ClosingParenthesis))
        {
            do
            {
                parameters.emplace_back(Consume(Token::Type::Identifier, "Expect parameter name."));
                if (parameters.size() >= 255)
                {
                    Gekko::ReportError(CurrentToken(), "Can't have more than 255 parameters.");
                }

            } while (ConsumeIfMatch(Token::Type::Comma));
            
        }

        Consume(Token::Type::ClosingParenthesis, "Expect ')' after function parameters.");

        Consume(Token::Type::OpeningBrace, "Expect '{' before function body.");

        LambdaExpression::BodyType functionBody = ParseBlock();
        return std::make_unique<LambdaExpression>(std::move(parameters), std::move(functionBody));
    }
    default: throw ParseError(token, "Unhandled primary token type.");
    }
}

bool Parser::MatchAny(std::initializer_list<Token::Type> tokenTypes) const
{
    for (Token::Type tokenType : tokenTypes)
    {
        if (m_tokens[m_current].m_type == tokenType)
        {
            return true;
        }            
    } 

    return false;   
}

bool Parser::Match(Token::Type tokenType) const
{
    return m_tokens[m_current].m_type == tokenType;
}

bool Parser::ConsumeIfMatch(Token::Type tokenType)
{
    if (Match(tokenType))
    {
        ++m_current;
        return true;
    }
    return false;
}

const Token& Parser::Consume(Token::Type tokenType, std::string&& errorMessage)
{
    if (Match(tokenType))
    {
        return m_tokens[m_current++];
    }
    else
    {
        throw ParseError(m_tokens[m_current], errorMessage);
    }
}

bool Parser::CanBeUnary(Token::Type tokenType) const
{
    return tokenType == Token::Type::Minus || tokenType == Token::Type::Plus;
}

const Token& Parser::CurrentToken() const
{
    assert(m_current < m_tokens.size());
    return m_tokens[m_current];
}

const Token& Parser::PreviousToken() const
{
    assert(m_current > 0 && m_current <= m_tokens.size());
    return m_tokens[m_current-1];
}

void Parser::Synchronize()
{
    while (m_tokens[m_current].m_type != Token::Type::EndOfFile) 
    {
        if (m_tokens[m_current++].m_type == Token::Type::Semicolon)
        {
            return;
        }

        switch (m_tokens[m_current].m_type)
        {
        case Token::Type::Class:
        case Token::Type::Fun:
        case Token::Type::Var:
        case Token::Type::For:
        case Token::Type::If:
        case Token::Type::While:
        case Token::Type::Print:
        case Token::Type::Return: return;
        }
    }
}
