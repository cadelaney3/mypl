#include <iostream>
#include <string>

#include "Token.h"
#include "Lexeme.h"
#include "Lexer.h"
#include "Parser.h"
#include "ASTPrinter.h"
#include "Typechecker.h"
#include "Interpreter.h"

using namespace std;

int main(int argc, const char * argv[]) {
/*    string code = R"code(
x = readint("Enter an int: ");
y = readint("Enter an int: ");
t = "test";
println(t);
t = "test" + "fjl";
println(t);
if x > y then
    println("The first int was bigger than the second!");
    z = 1;
elseif y > x then
    println("The second int was bigger than the first!");
    z = "test";
else
    println("You entered the same value twice!");
    z = false;
end
y = 21;
y = y + 1;
println(y);
while x > 0 do
	j = 0;
    print(".");
    x = x - 1;
	if j < 10 then
		j = j + 1;
		println(j);
	end
end
z = [[1,2], [2,3], [4,5]];
    )code"; */
	string code = R"code(
# Welcome message
println("Welcome to an ad-lib");

# create variable for the loop
playAgain = true;

# loop for the game
while playAgain == true do
	noun1 = readstr("Enter a noun: ");
	adj1 = readstr("Enter an adjective: ");
	verb1 = readstr("Enter a verb: ");
	num1 = readint("Enter a number: ");
	noun2 = readstr("Enter a noun: ");
	adj2 = readstr("Enter an adjective: ");
	verb2 = readstr("Enter a past tense verb: ");
	
	println("");
	print("There was a ");
	print(adj1);
	print(" ");
	print(noun1);
	print(" running through the woods. It was on its way to ");
	print(verb1);
	print(" as fast as it could so it could take a nice long nap. ");
	print("Along the way, it saw ");
	print(num1);
	print(" aliens. So it grabbed a ");
	print(noun2);
	print(" and started throwing it at the aliens. ");
	print("Well, the aliens weren't happy about it, so they made a ");
	print(adj2);
	print(" mega-blaster and ");
	print(verb2);
	print(" the ");
	print(noun1);
	print(". It goes to show what happens when you throw a ");
	print(noun2);
	print(" at aliens.");
	println("");
	
	# see if user wants to play again
	again = readstr("Would you like to play again? (Y/n): ");
	
	if again == "n" or again == "N" then
		playAgain = false;
	end
end

)code";
    try {
        istringstream ins(code);
        Lexer lexer(ins);
        Parser parser(lexer);
        ASTPrinter printer;
   //     parser.getAST()->accept(printer);
        Typechecker checker;
        parser.getAST()->accept(checker);
        Interpreter interpreter;
        parser.getAST()->accept(interpreter);
    } catch (MyPLException &e) {
        cout << "Error encountered: " << e.what() << endl;
    }
    return 0;
}
