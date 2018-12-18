#include "Typechecker.h"
#include "ASTNodes.h"
#include <iostream>
#include <cctype>

using namespace std;

void Typechecker::visit(ASTSimpleBoolExpression& simpleBoolExpr) {
    simpleBoolExpr.expression->accept(*this);
    if (currentType != MPLType::BOOL) {
        throw TypecheckerException("Expected boolean expression");
    }
    currentType = MPLType::BOOL;
}

void Typechecker::visit(ASTComplexBoolExpression& complexBoolExpr) {
	complexBoolExpr.first->accept(*this);
	switch(currentType) {
	case MPLType::BOOL:
		complexBoolExpr.second->accept(*this);
		if (currentType != MPLType::BOOL) {
			throw TypecheckerException("can't compare bool to " + toString(currentType));
		}
		break;
	case MPLType::INT:
		complexBoolExpr.second->accept(*this);
		if (currentType != MPLType::INT) {
			throw TypecheckerException("can't compare int to " + toString(currentType));
		}
		break;
	case MPLType::STRING:
		complexBoolExpr.second->accept(*this);
		if (currentType != MPLType::STRING) {
			throw TypecheckerException("can't compare string to " + toString(currentType));
		}
		break;
	case MPLType::ARRAY:
		complexBoolExpr.second->accept(*this);
		if (currentType != MPLType::ARRAY) {
			throw TypecheckerException("can't compare array to " + toString(currentType));
		}
		break;
	default:
		complexBoolExpr.second->accept(*this);
	}
	if (complexBoolExpr.remainder) {
		complexBoolExpr.remainder->accept(*this);
	}
    currentType = MPLType::BOOL;
}

void Typechecker::visit(ASTStatementList& statementList) {
    table.pushTable();
    for (auto &statement : statementList.statements) {
        statement->accept(*this);
    }
    table.popTable();
}

void Typechecker::visit(ASTBasicIf& basicIf) {
	basicIf.expression->accept(*this);
	if (currentType != MPLType::BOOL) {
		TypecheckerException("Expected boolean expression");
	}
	basicIf.statementList->accept(*this);
}

void Typechecker::visit(ASTIfStatement& ifStatement) {
	ifStatement.baseIf.expression->accept(*this);
	if (currentType != MPLType::BOOL) {
		TypecheckerException("Expected boolean expression");
	}
	ifStatement.baseIf.statementList->accept(*this);
//	table.pushTable();
	for (auto &statement : ifStatement.elseifs) {
		statement.expression->accept(*this);
		statement.statementList->accept(*this);
	}
	if(ifStatement.elseList) {
		ifStatement.elseList->accept(*this);
	}
//	table.popTable();
}

void Typechecker::visit(ASTWhileStatement& whileStatement) {
	whileStatement.condition->accept(*this);
	if (currentType != MPLType::BOOL) {
		TypecheckerException("Expected boolean expression");
	}
//	table.pushTable();
	whileStatement.statements->accept(*this);
//	table.popTable();
}

void Typechecker::visit(ASTPrintStatement& printStatement) {
    printStatement.expression->accept(*this);
}

void Typechecker::visit(ASTAssignmentStatement& assignmentStatement) {
	string name = assignmentStatement.identifier->name;
	assignmentStatement.rhs->accept(*this);
	auto rhsType = currentType;
	std::vector<Symbol> empty;
	if (!table.doesSymbolExist(assignmentStatement.identifier->name)) {
		switch(rhsType) {
		case MPLType::BOOL:
			table.storeBoolVal(assignmentStatement.identifier->name,false);
			break;
		case MPLType::INT:
			table.storeIntVal(assignmentStatement.identifier->name,0);
			break;
		case MPLType::STRING:
			table.storeStringVal(assignmentStatement.identifier->name,"");
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
		switch(currentType) {
			case MPLType::BOOL:
				if (rhsType != MPLType::BOOL) {
					throw TypecheckerException("can't change '" + name + "' from bool to " + toString(currentType));
				}
				else {
					table.storeBoolVal(assignmentStatement.identifier->name,false);
				}
				break;
			case MPLType::INT:
				if (rhsType != MPLType::INT) {
					throw TypecheckerException("can't change '" + name + "' from int to " + toString(currentType));
				}
				else {
					table.storeIntVal(assignmentStatement.identifier->name,0);
				}
				break;
			case MPLType::STRING:
				if (rhsType != MPLType::STRING) {
					throw TypecheckerException("can't change '" + name + "' from string to " + toString(currentType));
				}
				else {
					table.storeStringVal(assignmentStatement.identifier->name,"");
				}
				break;
			case MPLType::ARRAY:
				if (rhsType != MPLType::ARRAY) {
					throw TypecheckerException("can't change '" + name + "' from array to " + toString(currentType));
				}
				else {
				//	std::vector<Symbol> empty;
					table.storeVector(assignmentStatement.identifier->name,empty);
				}
				break;
			default:
			//	assignmentStatement.rhs->accept(*this);
				break;
		}
	}
}

void Typechecker::visit(ASTIdentifier& identifier) {
    if (table.doesSymbolExist(identifier.name)) {
        currentType = table.getSymbolType(identifier.name);
    } else {
        throw TypecheckerException("Identifier " + identifier.name + " used before defined");
    }
    if (identifier.indexExpression && currentType != MPLType::ARRAY) {
        throw TypecheckerException("Identifier " + identifier.name + " given an index when not an array");
    }
}

void Typechecker::visit(ASTLiteral& literal) {
    currentType = literal.type;
}

void Typechecker::visit(ASTListLiteral& listLiteral) {
	listLiteral.expressions.front()->accept(*this);
	MPLType listType = currentType;
    for (auto &expression : listLiteral.expressions) {
        expression->accept(*this);
        if (currentType != listType) {
        	throw TypecheckerException("elements of array must be of same type");
        }
    }
    currentType = MPLType::ARRAY;
}

void Typechecker::visit(ASTReadExpression& readExpression) {
    if (readExpression.isReadInt) {
        currentType = MPLType::INT;
    } else {
        currentType = MPLType::STRING;
    }
}

void Typechecker::visit(ASTComplexExpression& complexExpression) {
	complexExpression.firstOperand->accept(*this);
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
	switch(currentType) {
	case MPLType::BOOL:
		complexExpression.rest->accept(*this);
		if (currentType != MPLType::BOOL) {
			throw TypecheckerException("can't perform arithmetic on boolean");
		}
		break;
	case MPLType::INT:
		complexExpression.rest->accept(*this);
		if (currentType != MPLType::INT) {
			throw TypecheckerException("can't " + op + " int with " + toString(currentType));
		}
		break;
	case MPLType::STRING:
		complexExpression.rest->accept(*this);
		if (complexExpression.operation != Token::PLUS) {
			throw TypecheckerException("Cannot " + op + " with strings");
		}
		else {
			if (currentType != MPLType::STRING) {
				throw TypecheckerException("can't " + op + " strings with " + toString(currentType));
			}
		}
		break;
	case MPLType::ARRAY:
		complexExpression.rest->accept(*this);
		if (currentType != MPLType::ARRAY) {
			throw TypecheckerException("can't " + op + " arrays with " + toString(currentType));
		}
		break;
	default:
		complexExpression.rest->accept(*this);
	}
}



