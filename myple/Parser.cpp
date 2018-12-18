#include "Parser.h"

#include <iostream>
#include <sstream>

using namespace std;

const bool USE_CONTEXTLOG = false;

class ContextLog {
public:
    ContextLog(string p, const Lexeme &lex):
    part(p),
    l(lex.line),
    c(lex.col) {
        if (USE_CONTEXTLOG)
            cout << "Entering " << part << "(" << l << ":" << c << ")" << endl;
    }
    ~ContextLog() {
        if (USE_CONTEXTLOG)
            cout << "Exiting " << part << "(" << l << ":" << c << ")" << endl;
    }
private:
    string part;
    int l;
    int c;
};

Parser::Parser(Lexer &lex): lexer(lex) {
    advance();
    syntaxTree = stmts();
}

shared_ptr<ASTNode> Parser::getAST() {
    return syntaxTree;
}

void Parser::advance() {
    currentLexeme = lexer.next();
}

void Parser::eat(Token token, std::string message) {
    if (currentLexeme.token == token) {
        advance();
    } else {
        error(message);
    }
}

void Parser::error(std::string message) {
    cout << "Error found" << endl;
    ostringstream oss;
    oss << message << ", found " << currentLexeme << " instead";
    throw ParserException(oss.str());
}

std::shared_ptr<ASTStatementList> Parser::stmts(std::shared_ptr<ASTStatementList> lst) {
    ContextLog clog("stmts", currentLexeme);
    if (!lst) lst = make_shared<ASTStatementList>();
    switch (currentLexeme.token) {
        case Token::PRINT:
        case Token::PRINTLN:
        case Token::ID:
        case Token::IF:
        case Token::WHILE:
            lst->statements.push_back(stmt());
            stmts(lst);
            break;
        default:
            // May be empty
            break;
    }
    return lst;
}

std::shared_ptr<ASTStatement> Parser::stmt() {
    ContextLog clog("stmt", currentLexeme);
    switch (currentLexeme.token) {
        case Token::PRINT:
        case Token::PRINTLN:
            return output();
            break;
        case Token::ID:
            return assign();
            break;
        case Token::IF:
            return cond();
            break;
        case Token::WHILE:
            return loop();
            break;
        default:
            error("Expected statement");
    }
}

std::shared_ptr<ASTPrintStatement> Parser::output() {
    ContextLog clog("output", currentLexeme);
    auto ans = std::make_shared<ASTPrintStatement>();
    if (currentLexeme.token == Token::PRINT) {
        eat(Token::PRINT, "Expected print");
        ans->isPrintln = false;
    } else {
        eat(Token::PRINTLN, "Expected print or println");
        ans->isPrintln = true;
    }
    eat(Token::LPAREN, "Expected '('");
    ans->expression = expr();
    eat(Token::RPAREN, "Expected ')'");
    eat(Token::SEMICOLON, "Expected ';'");
    return ans;
}

std::shared_ptr<ASTReadExpression> Parser::input() {
    ContextLog clog("input", currentLexeme);
    auto ans = std::make_shared<ASTReadExpression>();
    if (currentLexeme.token == Token::READINT) {
    	eat(Token::READINT, "Expected readint");
    	ans->isReadInt = true;
    } else {
    	eat(Token::READSTR, "Expected readstr");
    	ans->isReadInt = false;
    }
    eat(Token::LPAREN, "Expected '('");
    if (currentLexeme.token == Token::STRING) {
        ans->message = currentLexeme.text;
    }
    eat(Token::STRING, "Expected a string");
    eat(Token::RPAREN, "Expected ')'");
    return ans;
}

std::shared_ptr<ASTAssignmentStatement> Parser::assign() {
    ContextLog clog("assign", currentLexeme);
    auto ans = std::make_shared<ASTAssignmentStatement>();
    auto id = std::make_shared<ASTIdentifier>();
    if (currentLexeme.token == Token::ID) {
    	id->name = currentLexeme.text;
    }
    eat(Token::ID, "Expected an identifier");
    id->indexExpression = listindex();
    eat(Token::ASSIGN, "Expected '='");
    ans->identifier = id;
    ans->rhs = expr();
    eat(Token::SEMICOLON, "Expected ';'");
    return ans;
}

// returns ASTListLiteral if there is a list
// returns nullptr otherwise
std::shared_ptr<ASTExpression> Parser::listindex() {
    ContextLog clog("listindex", currentLexeme);
    if (currentLexeme.token == Token::LBRACKET) {
    	auto ans = std::make_shared<ASTListLiteral>();
    	advance();
    	ans->expressions.push_back(expr());
    	eat(Token::RBRACKET, "Expected ']'");
    	return ans;
    } else {
    	return nullptr;
    }
}

// returns ASTComplexExpression pointer if expression involves
// arthmetic. returns only the first operand if no arithmetic involved
std::shared_ptr<ASTExpression> Parser::expr() {
    ContextLog clog("expr", currentLexeme);
    auto ans = std::make_shared<ASTComplexExpression>();
    ans->firstOperand = value();
    exprt(ans);
    if (!ans->rest) {
    	return ans->firstOperand;
    }
    else {
    	return ans;
    }
}

// sets the operation and other operand of a comlex expression
void Parser::exprt(std::shared_ptr<ASTComplexExpression> expression) {
    ContextLog clog("exprt", currentLexeme);
	switch (currentLexeme.token) {
	case Token::PLUS:
	{
		expression->operation = Token::PLUS;
		eat(Token::PLUS, "Expected '+'");
		expression->rest = expr();
		break;
	}
	case Token::MINUS:
	{
		expression->operation = Token::MINUS;
		eat(Token::MINUS, "Expected '-'");
		expression->rest = expr();
		break;
	}
	case Token::DIVIDE:
	{
		expression->operation = Token::DIVIDE;
		eat(Token::DIVIDE, "Expected '/'");
		expression->rest = expr();
		break;
	}
	case Token::MULTIPLY:
	{
		expression->operation = Token::MULTIPLY;
		eat(Token::MULTIPLY, "Expected '*'");
		expression->rest = expr();
		break;
	}
	case Token::MODULUS:
	{
		expression->operation = Token::MODULUS;
		eat(Token::MODULUS, "Expected '%'");
		expression->rest = expr();
		break;
	}
	default:
		break;
	}
}

bool isTokenAValue(Token token) {
    switch (token) {
        case Token::ID:
        case Token::STRING:
        case Token::INT:
        case Token::BOOL:
        case Token::READINT:
        case Token::READSTR:
        case Token::LBRACKET:
            return true;
            break;
        default:
            return false;
    }
}

// returns an AST pointer whose type is determined by the kind
// of token 
std::shared_ptr<ASTExpression> Parser::value() {
    ContextLog clog("value", currentLexeme);
    switch (currentLexeme.token) {
        case Token::ID:
        {
            auto ans = std::make_shared<ASTIdentifier>();
            ans->name = currentLexeme.text;
            advance();
            ans->indexExpression = listindex();
            return ans;
            break;
        }
        case Token::STRING:
        {
            auto ans = std::make_shared<ASTLiteral>();
            ans->type = MPLType::STRING;
            ans->value = currentLexeme.text;
            advance();
            return ans;
            break;
        }
        case Token::INT:
        {
            auto ans = std::make_shared<ASTLiteral>();
            ans->type = MPLType::INT;
            ans->value = currentLexeme.text;
            advance();
            return ans;
            break;
        }
        case Token::BOOL:
        {
        	auto ans = std::make_shared<ASTLiteral>();
        	ans->type = MPLType::BOOL;
        	ans->value = currentLexeme.text;
        	advance();
        	return ans;
        	break;
        }
        case Token::READSTR:
        case Token::READINT:
        {
        	return input();
        	break;
        }
        case Token::LBRACKET:
        {
        	auto ans = std::make_shared<ASTListLiteral>();
        	eat(Token::LBRACKET, "Expected '['");
        	ans->expressions.push_back(exprlist());
        	eat(Token::RBRACKET, "Expected ']'");
        	return ans;
        	break;
        }
        default:
            error("Expected a value");
    }
}

std::shared_ptr<ASTListLiteral> Parser::exprlist() {
    ContextLog clog("exprlist", currentLexeme);
	auto ans = std::make_shared<ASTListLiteral>();
    if (isTokenAValue(currentLexeme.token)) {
    	ans->expressions.push_back(expr());
    	exprtail(ans);
    }
    return ans;
}

void Parser::exprtail(std::shared_ptr<ASTListLiteral> list) {
    ContextLog clog("exprtail", currentLexeme);
    if (currentLexeme.token == Token::COMMA) {
    	eat(Token::COMMA, "Expected ','");
    	list->expressions.push_back(expr());
    	exprtail(list);
    }
}

std::shared_ptr<ASTIfStatement> Parser::cond() {
    ContextLog clog("cond", currentLexeme);
    auto ans = std::make_shared<ASTIfStatement>();
    eat(Token::IF, "Expected if");
    ans->baseIf.expression = bexpr();
    eat(Token::THEN, "Expected then");
    ans->baseIf.statementList = stmts();
    condt(ans);
    eat(Token::END, "Expected end");
    return ans;
}

void Parser::condt(std::shared_ptr<ASTIfStatement> statement) {
    ContextLog clog("condt", currentLexeme);
    if (currentLexeme.token == Token::ELSEIF) {
    	eat(Token::ELSEIF, "Expected elseif");
    	ASTBasicIf ans;
    	statement->elseifs.push_back(ans);
    	statement->elseifs.back().expression = bexpr();
    	eat(Token::THEN, "Expected then");
    	statement->elseifs.back().statementList = stmts();
    	condt(statement);
    }
    if (currentLexeme.token == Token::ELSE) {
    	eat(Token::ELSE, "Expected else");
    	statement->elseList = stmts();
    }
}

// returns an ASTComplexBoolExpression pointer if boolean expression
// involves a boolean relation. returns an ASTSimpleBoolExpression
// pointer otherwise
std::shared_ptr<ASTBoolExpression> Parser::bexpr() {
    ContextLog clog("bexpr", currentLexeme);
    auto ans = std::make_shared<ASTComplexBoolExpression>();
    if (currentLexeme.token == Token::NOT) {
    	eat(Token::NOT, "Expected not");
    	ans->negated = true;
    }
    ans->first = expr();
    bexprt(ans);
    if (!ans->second) {
    	auto simple = ::make_shared<ASTSimpleBoolExpression>();
    	simple->negated = ans->negated;
    	simple->expression = ans->first;
    	return simple;
    }
    else {
    	return ans;
    }
}

// checks if boolean expression involves a relation
void Parser::bexprt(std::shared_ptr<ASTComplexBoolExpression> expression) {
    ContextLog clog("bexprt", currentLexeme);
    switch (currentLexeme.token) {
        case Token::EQUAL:
        case Token::LESS_THAN:
        case Token::GREATER_THAN:
        case Token::LESS_THAN_EQUAL:
        case Token::GREATER_THAN_EQUAL:
        case Token::NOT_EQUAL:
        {
            expression->relation = currentLexeme.token;
            advance();
            expression->second = expr();
            bconnect(expression);
            break;
        }
        default:
         //  error("Internal error: expected relational operator");
           break;
    }
}

void Parser::bconnect(std::shared_ptr<ASTComplexBoolExpression> expression) {
    ContextLog clog("bconnect", currentLexeme);
    if (currentLexeme.token == Token::AND) {
    	expression->hasConjunction = true;
    	expression->conjunction = currentLexeme.token;
    	eat(Token::AND, "Expected and");
        expression->remainder = bexpr();
    }
    if (currentLexeme.token == Token::OR) {
    	expression->hasConjunction = true;
    	expression->conjunction = currentLexeme.token;
    	eat(Token::OR, "Expected or");
        expression->remainder = bexpr();
    }
}

std::shared_ptr<ASTWhileStatement> Parser::loop() {
    ContextLog clog("loop", currentLexeme);
    auto ans = std::make_shared<ASTWhileStatement>();
    eat(Token::WHILE, "Expected while");
    ans->condition = bexpr();
    eat(Token::DO, "Expected do");
    ans->statements = stmts();
    eat(Token::END, "Expected end");
    return ans;
}
