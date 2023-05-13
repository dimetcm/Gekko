#include "parser.h"
#include "statements.h"
#include "expressions.h"
#include "expressionvisitor.h"
#include "gekko.h"
#include <assert.h>

struct ParsingContext
{
    ParsingContext(bool isInsideLoop = false) : m_isInsideLoop(isInsideLoop) {}

    bool m_isInsideLoop = false;
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
           programme.push_back(ParseDeclaration(ParsingContext()));
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

IStatementPtr Parser::ParseDeclaration(const ParsingContext& context)
{
    if (ConsumeIfMatch(Token::Type::Var))
    {
        return ParseVariableDeclaration();
    }

    return ParseStatement(context);
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

std::vector<IStatementPtr> Parser::ParseBlock(const ParsingContext& context)
{
    std::vector<IStatementPtr> block;
    while (!Match(Token::Type::ClosingBrace) && !Match(Token::Type::EndOfFile))
    {
        block.push_back(ParseDeclaration(context));
    }

    Consume(Token::Type::ClosingBrace, "Expect '}' after block.");

    return block;
}

IStatementPtr Parser::ParseStatement(const ParsingContext& context)
{
    if (ConsumeIfMatch(Token::Type::Print))
    {
        return ParsePrintStatement();
    }

    if (ConsumeIfMatch(Token::Type::If))
    {
        return ParseIfStatement(context);
    }

    if (ConsumeIfMatch(Token::Type::While))
    {
        return ParseWhileStatement(context);
    }

    if (ConsumeIfMatch(Token::Type::For))
    {
        return ParseForStatement(context);
    }

    if (ConsumeIfMatch(Token::Type::Break))
    {
        if (!context.m_isInsideLoop)
        {
            throw ParseError(PreviousToken(), "break encountered outside of a loop body.");
        }

        return ParseBreakStatement(context);
    }

    if (ConsumeIfMatch(Token::Type::OpeningBrace))
    {
        return ParseBlockStatement(context);
    }

    return ParseExpressionStatement();
}

IStatementPtr Parser::ParseIfStatement(const ParsingContext& context)
{
    Consume(Token::Type::OpeningParenthesis, "Expect '(' after 'if'.");
    IExpressionPtr condition = ParseExpression();
    Consume(Token::Type::ClosingParenthesis, "Expect ')' after if condition.");

    IStatementPtr trueBranch = ParseStatement(context);
    IStatementPtr falseBranch = nullptr;
    if (ConsumeIfMatch(Token::Type::Else))
    {
        falseBranch = ParseStatement(context);
    }
    
    return std::make_unique<IfStatement>(std::move(condition), std::move(trueBranch), std::move(falseBranch));
}

IStatementPtr Parser::ParseWhileStatement(const ParsingContext& context)
{
    Consume(Token::Type::OpeningParenthesis, "Expect '(' after 'while'.");
    IExpressionPtr condition = ParseExpression();
    Consume(Token::Type::ClosingParenthesis, "Expect ')' after while condition.");

    IStatementPtr statement = ParseStatement(ParsingContext(true));
    return std::make_unique<WhileStatement>(std::move(condition), std::move(statement));
}

IStatementPtr Parser::ParseForStatement(const ParsingContext& context)
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

    IStatementPtr body = ParseStatement(ParsingContext(true));

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

IStatementPtr Parser::ParseBreakStatement(const ParsingContext& context)
{
    Consume(Token::Type::Semicolon, "Expect ';' after break.");
    return std::make_unique<BreakStatement>();
}

IStatementPtr Parser::ParseBlockStatement(const ParsingContext& context)
{
    return std::make_unique<BlockStatement>(ParseBlock(context));
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
    return ParseAssignment();
}

IExpressionPtr Parser::ParseAssignment()
{
   IExpressionPtr expression = ParseComma(); 
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
        } visitor;

        expression->Accept(visitor, &visitor);

        if (visitor.m_result)
        {
            IExpressionPtr value = ParseAssignment();
            return std::make_unique<AssignmentExpression>(*visitor.m_result, std::move(value));
        }
        else
        {
            ParseError(m_tokens[m_current], "Invalid assignment target.");
        }
    }

    return expression;
 }

IExpressionPtr Parser::ParseComma()
{
    return ParseBinaryExpression(std::bind(&Parser::ParseTernaryConditional, this), {Token::Type::Comma});
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
        Token token = m_tokens[m_current++];
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

    return ParsePrimary();
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
