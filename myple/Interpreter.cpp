#include <iostream>
#include <sstream>
#include <limits>

#include "Interpreter.h"
#include "ASTNodes.h"

using namespace std;

void Interpreter::visit(ASTSimpleBoolExpression& simpleBoolExpr) {
    simpleBoolExpr.expression->accept(*this);
    // Note: after visiting the above expression, (assuming it's boolean)
    // it will set currentBool to be whatever boolean value it represents
    if (currentType != MPLType::BOOL) {
        throw InterpreterException("Expected boolean expression");
    }
    if (simpleBoolExpr.negated) {
        currentBool = !currentBool;
    }
    currentType = MPLType::BOOL;
}

void Interpreter::visit(ASTComplexBoolExpression& complexBoolExpr) {
    complexBoolExpr.first->accept(*this);
    auto lhsType = currentType;
    auto lhsInt = currentInt;
    auto lhsString = currentString;
    auto lhsBool = currentBool;
    complexBoolExpr.second->accept(*this);
    bool myResult;
    
    // TODO: figure out what comparison to make, do that comparison,
    // and store the result in myResult.
    
    if (currentType == lhsType) {
    	auto relation = complexBoolExpr.relation;
    	switch(currentType) {
    	case MPLType::BOOL:
    		if (relation == Token::EQUAL) {
    			myResult = lhsBool && currentBool;
    		} else if (relation == Token::NOT_EQUAL) {
    			myResult = !(lhsBool && currentBool);
    		} else {
    			InterpreterException("Cannot use '" + toString(relation) + "' to compare boolean expressions");
    		}
    		break;
    	case MPLType::INT:
    		if (relation == Token::EQUAL) {
    			myResult = lhsInt == currentInt;
    		} else if (relation == Token::NOT) {
    			myResult = lhsInt != currentInt;
    		} else if (relation == Token::GREATER_THAN) {
    			myResult = lhsInt > currentInt;
    		} else if (relation == Token::GREATER_THAN_EQUAL) {
    			myResult = lhsInt >= currentInt;
    		} else if (relation == Token::LESS_THAN) {
    			myResult = lhsInt < currentInt;
    		} else if (relation == Token::LESS_THAN_EQUAL) {
    			myResult = lhsInt <= currentInt;
    		} else {
    			InterpreterException("Cannot use '" + toString(relation) + "' when comparing integers");
    		}
    		break;
    	case MPLType::STRING:
    		if (relation == Token::EQUAL) {
    			myResult = lhsString == currentString;
    		} else if (relation == Token::NOT) {
    			myResult = lhsString != currentString;
    		} else if (relation == Token::GREATER_THAN) {
    			myResult = lhsString > currentString;
    		} else if (relation == Token::GREATER_THAN_EQUAL) {
    			myResult = lhsString >= currentString;
    		} else if (relation == Token::LESS_THAN) {
    			myResult = lhsString < currentString;
    		} else if (relation == Token::LESS_THAN_EQUAL) {
    			myResult = lhsString <= currentString;
    		} else {
    			InterpreterException("Cannot use '" + toString(relation) + "' when comparing strings");
    		}
    		break;
    	default:
    		InterpreterException("Invalid comparison");
    	}
    }

    if (complexBoolExpr.hasConjunction) {
        complexBoolExpr.remainder->accept(*this);
        if (complexBoolExpr.conjunction == Token::AND) {
            currentBool = myResult && currentBool;
        } else {
            currentBool = myResult || currentBool;
        }
    } else {
        currentBool = myResult;
    }
    if (complexBoolExpr.negated) {
        currentBool = !currentBool;
    }
    currentType = MPLType::BOOL;
}

void Interpreter::visit(ASTStatementList& statementList) {
    table.pushTable();
    for (auto &statement : statementList.statements) {
        statement->accept(*this);
    }
    table.popTable();
}

void Interpreter::visit(ASTBasicIf& basicIf) {
    // I strongly suggest not using this function and instead having
    // visit(ASTIfStatement) explicitly look inside the various baseIfs.
    // This is because once you enter a certain if statementList, you will
    // not want to enter any other statementLists.
    abort();
}

void Interpreter::visit(ASTIfStatement& ifStatement) {
    // TODO
	bool if_true = false;
	bool elseif_true = false;
	ifStatement.baseIf.expression->accept(*this);
	if (currentType != MPLType::BOOL) {
		InterpreterException("Expected boolean expression");
	}
	if (currentBool) {
		ifStatement.baseIf.statementList->accept(*this);
		if_true = true;
	} else {
//		table.pushTable();
		for (auto &statement : ifStatement.elseifs) {
			if (!elseif_true) {
				statement.expression->accept(*this);
				if (currentBool) {
					statement.statementList->accept(*this);
					elseif_true = true;
				}
			}
		}
	}
	if(ifStatement.elseList) {
		if (!if_true && !elseif_true) {
			ifStatement.elseList->accept(*this);
		}
	}
//	table.popTable();
}

void Interpreter::visit(ASTWhileStatement& whileStatement) {
    whileStatement.condition->accept(*this);
    while (currentBool) {
        whileStatement.statements->accept(*this);
        whileStatement.condition->accept(*this);
    }
}

void Interpreter::visit(ASTPrintStatement& printStatement) {
    // TODO
	printStatement.expression->accept(*this);
	switch(currentType) {
	case MPLType::BOOL:
		cout << currentBool;
		break;
	case MPLType::INT:
		cout << currentInt;
		break;
	case MPLType::STRING:
		cout << currentString;
		break;
	default:
		break;
	}
	if (printStatement.isPrintln) {
		cout << endl;
	}
}

void Interpreter::visit(ASTAssignmentStatement& assignmentStatement) {
    // TODO
	string name = assignmentStatement.identifier->name;
	std::vector<Symbol> empty;
	assignmentStatement.rhs->accept(*this);
	auto rhsType = currentType;
	auto rhsInt = currentInt;
	auto rhsString = currentString;
	auto rhsBool = currentBool;
	if (!table.doesSymbolExist(assignmentStatement.identifier->name)) {
		switch(rhsType) {
		case MPLType::BOOL:
			table.storeBoolVal(assignmentStatement.identifier->name,currentBool);
			break;
		case MPLType::INT:
			table.storeIntVal(assignmentStatement.identifier->name,currentInt);
			break;
		case MPLType::STRING:
			table.storeStringVal(assignmentStatement.identifier->name,currentString);
			break;
		case MPLType::ARRAY:
			table.storeVector(assignmentStatement.identifier->name,empty);
			break;
		default:
			break;
		}
	}
	else {
		assignmentStatement.identifier->accept(*this);
		auto id_type = currentType;
		switch(id_type) {
			case MPLType::BOOL:
				if (rhsType != MPLType::BOOL) {
					throw InterpreterException("can't change '" + name + "' from bool to " + toString(currentType));
				}
				else {
					table.storeBoolVal(assignmentStatement.identifier->name,rhsBool);
				}
				break;
			case MPLType::INT:
				if (rhsType != MPLType::INT) {
					throw InterpreterException("can't change '" + name + "' from int to " + toString(currentType));
				}
				else {
					table.storeIntVal(assignmentStatement.identifier->name,rhsInt);
				}
				break;
			case MPLType::STRING:
				if (rhsType != MPLType::STRING) {
					throw InterpreterException("can't change '" + name + "' from string to " + toString(currentType));
				}
				else {
					table.storeStringVal(assignmentStatement.identifier->name,rhsString);
				}
				break;
			case MPLType::ARRAY:
				if (rhsType != MPLType::ARRAY) {
					throw InterpreterException("can't change '" + name + "' from array to " + toString(currentType));
				}
				else {
					table.storeVector(assignmentStatement.identifier->name,empty);
				}
				break;
			default:
				//assignmentStatement.rhs->accept(*this);
				break;
		}
	}
}

void Interpreter::visit(ASTIdentifier& identifier) {
    // TODO
    if (table.doesSymbolExist(identifier.name)) {
        currentType = table.getSymbolType(identifier.name);
        switch(currentType) {
        case MPLType::BOOL:
        	currentBool = table.getBoolVal(identifier.name);
        	break;
        case MPLType::INT:
        	currentInt = table.getIntVal(identifier.name);
        	break;
        case MPLType::STRING:
        	currentString = table.getStringVal(identifier.name);
        	break;
        default:
        	break;
        }
    } else {
        throw InterpreterException("Identifier " + identifier.name + " used before defined");
    }
    if (identifier.indexExpression && currentType != MPLType::ARRAY) {
        throw InterpreterException("Identifier " + identifier.name + " given an index when not an array");
    }
}

void Interpreter::visit(ASTLiteral& literal) {
    currentType = literal.type;
    if (currentType == MPLType::INT) {
        // istringstream allows us to create something like "cin" but instead
        // of reading from the keyboard, it reads from the string that we give
        // it
        istringstream iss(literal.value);
        iss >> currentInt;
        // If there was an error reading the integer out of the istringstream,
        // it will evaluate to false when used in a boolean context.
        if (!iss) {
            throw InterpreterException("Bad integer literal: " + literal.value);
        }
    } else if (currentType == MPLType::BOOL) {
        // also could be written as
        // currentBool = literal.value == "true", but that looks confusing
        if (literal.value == "true") {
            currentBool = true;
        } else {
            currentBool = false;
        }
    } else {
        currentString = literal.value;
    }
}

void Interpreter::visit(ASTListLiteral& listLiteral) {
    // TODO? (only needed for extra credit)
}

void Interpreter::visit(ASTReadExpression& readExpression) {
    cout << readExpression.message;
    if (readExpression.isReadInt) {
        cin >> currentInt;
        currentType = MPLType::INT;
    } else {
        // getline will read everything up until a newline character, allowing
        // us to read in a string containing spaces
        getline(cin, currentString);
        currentType = MPLType::STRING;
    }
    // If there was an input problem (say, a user entering 'test' to readint),
    // cin will be false when used in a boolean context.  This way, if the
    // user enters something silly, we can throw an error.
    if (!cin) {
        throw InterpreterException("Invalid user input");
    }
    // If we read in a number, ignore everything else on the current line
    if (readExpression.isReadInt) {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void Interpreter::visit(ASTComplexExpression& complexExpression) {
    // TODO
	complexExpression.firstOperand->accept(*this);
    auto lhsType = currentType;
    auto lhsInt = currentInt;
    auto lhsString = currentString;
    auto lhsBool = currentBool;
    complexExpression.rest->accept(*this);
	string op;
	switch(complexExpression.operation) {
	case Token::PLUS:
		op = "add";
		break;
	case Token::MINUS:
		op = "subtract";
		break;
	case Token::MULTIPLY:
		op = "multiply";
		break;
	case Token::DIVIDE:
		op = "divide";
		break;
	case Token::MODULUS:
		op = "mod";
		break;
	default:
		break;
	}
	switch(lhsType) {
	case MPLType::BOOL:
		throw InterpreterException("can't perform arithmetic on boolean");
		break;
	case MPLType::INT:
		if (currentType != MPLType::INT) {
			throw InterpreterException("can't " + op + " int with " + toString(currentType));
		}
		if (complexExpression.operation == Token::PLUS) {
			currentInt = currentInt + lhsInt;
		} else if (complexExpression.operation == Token::MULTIPLY) {
			currentInt = currentInt * lhsInt;
		} else if (complexExpression.operation == Token::MINUS) {
			currentInt = lhsInt - currentInt;
		} else if (complexExpression.operation == Token::DIVIDE) {
			if (currentInt == 0) {
				throw InterpreterException("Divide by zero error");
			}
			currentInt = lhsInt / currentInt;
		} else if (complexExpression.operation == Token::MODULUS) {
			currentInt = lhsInt % currentInt;
		} else {
			throw InterpreterException("Invalid operation on integers");
		}
		break;
	case MPLType::STRING:
		if (complexExpression.operation != Token::PLUS) {
			throw InterpreterException("Cannot " + op + " with strings");
		}
		else {
			if (currentType != MPLType::STRING) {
				throw InterpreterException("can't " + op + " strings with " + toString(currentType));
			}
			currentString = lhsString + currentString;
		}
		break;
	case MPLType::ARRAY:
		if (currentType != MPLType::ARRAY) {
			throw InterpreterException("can't " + op + " arrays with " + toString(currentType));
		}
		break;
	default:
		break;
	}
}
