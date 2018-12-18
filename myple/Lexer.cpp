#include <sstream>
#include <iostream>

#include "Lexer.h"

using namespace std;

Lexer::Lexer(std::istream &initIn): in(initIn) {
    // You shouldn't need to modify this function
    
    // If we are given an invalid input stream, throw an exception
    if (!in) {
        throw LexerException("Invalid input stream passed to Lexer");
    }
}

Lexeme Lexer::next() {
    eatWhitespace();
    char tmp = peek();
    int startLine = lineNum;
    int startCol = colNum;
    if (tmp == eof()) {
        return Lexeme(Token::EOS, "", lineNum, colNum);
    } else if ((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z')) {
        // TODO: This will be the beginning of an identifier or of some Token
        // that is represented by a series of letters (such as "while")
    	string str;
    	while ((peek() >= 'a' && peek() <= 'z') || (peek() >= 'A' && peek() <= 'Z')
    			|| (peek() >= '0' && peek() <= '9') || peek() == '_') {
    			str += read();
    	}
		if (str == "while") {
			return Lexeme(Token::WHILE, "while", startLine, startCol);
		} else if (str == "if") {
			return Lexeme(Token::IF, "if", startLine, startCol);
		} else if (str == "readint") {
			return Lexeme(Token::READINT, "readint", startLine, startCol);
		} else if (str == "readstr") {
			return Lexeme(Token::READSTR, "readstr", startLine, startCol);
		} else if (str == "println") {
			return Lexeme(Token::PRINTLN, "println", startLine, startCol);
		} else if (str == "print") {
			return Lexeme(Token::PRINT, "print", startLine, startCol);
		} else if (str == "elseif") {
			return Lexeme(Token::ELSEIF, "elseif", startLine, startCol);
		} else if (str == "else") {
			return Lexeme(Token::ELSE, "else", startLine, startCol);
		} else if (str == "then") {
			return Lexeme(Token::THEN, "then", startLine, startCol);
		} else if (str == "end") {
			return Lexeme(Token::END, "end", startLine, startCol);
		} else if (str == "do") {
			return Lexeme(Token::DO, "do", startLine, startCol);
		} else if (str == "true") {
			return Lexeme(Token::BOOL, "true", startLine, startCol);
		} else if (str == "false") {
			return Lexeme(Token::BOOL, "false", startLine, startCol);
		} else if (str == "and") {
			return Lexeme(Token::AND, "and", startLine, startCol);
		} else if (str == "or") {
			return Lexeme(Token::OR, "or", startLine, startCol);
		} else if (str == "not") {
			return Lexeme(Token::NOT,"not", startLine, startCol);
		}
		else {
			return Lexeme(Token::ID, str, startLine, startCol);
		}
    } else if (tmp == '+') {
        return makeSingleCharLexeme(Token::PLUS);
    } else if (tmp == '>') {
        read();
        if (peek() == '=') {
            read();
            return Lexeme(Token::GREATER_THAN_EQUAL, ">=", startLine, startCol);
        } else {
            return Lexeme(Token::GREATER_THAN, ">", startLine, startCol);
        }
    // TODO: there are a ton of other cases to consider.

    } else if (tmp == '<') {
    	read();
    	if (peek() == '=') {
    		read();
    		return Lexeme(Token::LESS_THAN_EQUAL, "<=", startLine, startCol);
    	} else {
    		return Lexeme(Token::LESS_THAN, "<", startLine, startCol);
    	}
    } else if (tmp == '=') {
    	read();
    	if (peek() == '=') {
    		read();
    		return Lexeme(Token::EQUAL, "==", startLine, startCol);
    	} else {
    		return Lexeme(Token::ASSIGN, "=", startLine, startCol);
    	}
    } else if (tmp == '(') {
    	return makeSingleCharLexeme(Token::LPAREN);
    } else if (tmp == ')') {
    	return makeSingleCharLexeme(Token::RPAREN);
    } else if (tmp == ']') {
    	return makeSingleCharLexeme(Token::RBRACKET);
    } else if (tmp == '[') {
    	return makeSingleCharLexeme(Token::LBRACKET);
    } else if (tmp == ';') {
    	return makeSingleCharLexeme(Token::SEMICOLON);
    } else if (tmp == '%') {
    	return makeSingleCharLexeme(Token::MODULUS);
    } else if (tmp == ',') {
    	return makeSingleCharLexeme(Token::COMMA);
    } else if (tmp == '/') {
    	return makeSingleCharLexeme(Token::DIVIDE);
    } else if (tmp == '*') {
    	return makeSingleCharLexeme(Token::MULTIPLY);
    } else if (tmp == '-') {
    	return makeSingleCharLexeme(Token::MINUS);
    } else if (tmp == '!') {
    	read();
    	if (peek() == '=') {
    		read();
    		return Lexeme(Token::NOT_EQUAL, "!=", startLine, startCol);
    	}
    	else {
    		ostringstream oss;
    		oss << "Unexpected symbol: '" << tmp << "'";
    		throw LexerException(debugMessage(oss.str()));
    	}
    } else if (tmp == '"') {
    	read();
    	string str;
    	while (peek() != '"' && peek() != '\n') {
    		str += read();
    	}
    	if (peek() == '"') {
    		read();
    		return Lexeme(Token::STRING, str, startLine, startCol);
    	}
    	else {
    		ostringstream oss;
    		oss << "Missing '\"' at end of string ";
    		throw LexerException(debugMessage(oss.str()));
    	}
    } else if (isdigit(tmp)) {
    	string str;
    	str += read();
    	if (!isalpha(peek())) {
			while (isdigit(peek())) {
				str += read();
			}
			return Lexeme(Token::INT, str, startLine, startCol);
    	}
    	else {
    		ostringstream oss;
    		oss << "unexpected symbol: '" << peek() << "'";
    		throw LexerException(debugMessage(oss.str()));
    	}
    }
    else {
        // You will not need to change the contents of this "else"
        ostringstream oss;
        oss << "Unexpected symbol: '" << peek() << "'";
        throw LexerException(debugMessage(oss.str()));
    }
    // Yes, you might get a warning about control reaching the end of a
    // non-void function.  Make sure you are returning a Lexeme from each
    // block (the code as distributed has one case with just a TODO).
    //
    // If this is an error in your compiler, you can put in a call to
    // abort() which will stop the program running if control reaches here.
}

bool Lexer::atEOF() {
    // You shouldn't need to modify this function
    return peek() == eof();
}

char Lexer::peek() {
    // You shouldn't need to modify this function
    return in.peek();
}

char Lexer::read() {
    // You shouldn't need to modify this function
    if (in.peek() == '\n') {
        colNum = 1;
        lineNum += 1;
    } else {
        colNum += 1;
    }
    return in.get();
}
void Lexer::eatWhitespace() {
    // TODO
    //
    // I would also suggest having this function detect comments and handle
    // them appropriately.

	while (peek() == ' ' || peek() == '\t' || peek() == '#' || peek() == '\n') {
		if (peek() == '#') {
			while (peek() != '\n')
				read();
			read();
		}
		else {
			read();
		}
	}
}

char Lexer::eof() {
    // You shouldn't need to modify this function
    return std::istream::traits_type::eof();
}

Lexeme Lexer::makeSingleCharLexeme(Token token) {
    // You shouldn't need to modify this function
	int outLine = lineNum;
	int outCol = colNum;
    string str(1, read());
    return Lexeme(token, str, outLine, outCol);
}

std::string Lexer::debugMessage(std::string msg) {
    // You shouldn't need to modify this function
    ostringstream oss;
    oss << msg << " " << lineNum << ":" << colNum;
    return oss.str();
}
