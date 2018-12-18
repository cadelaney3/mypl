CC = g++

default: mypl

mypl: main.cpp Interpreter.cpp Token.cpp Parser.cpp Lexeme.cpp Lexer.cpp SymbolTable.cpp Typechecker.cpp common.cpp ASTPrinter.cpp
	$(CC) -o mypl main.cpp Interpreter.cpp Token.cpp Parser.cpp Lexeme.cpp Lexer.cpp SymbolTable.cpp Typechecker.cpp common.cpp ASTPrinter.cpp

clean:
	$(RM) mypl *.o *~

