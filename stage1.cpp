//Hoang Long Nguyen & Minh Ha Le
//CS 4301
//Stage 1

#include <stage1.h>
#include <ctime>
#include <iomanip>

//member function

Compiler::Compiler(char** argv) { // constructor
	sourceFile.open(argv[1]);
	listingFile.open(argv[2]);
	objectFile.open(argv[3]);
}

Compiler::~Compiler() { // destructor
	sourceFile.close();
	listingFile.close();
	objectFile.close();
}

void Compiler::createListingHeader() {
	time_t timeNow = time(NULL);
	listingFile << "STAGE1:  Hoang Long Nguyen & Minh Ha Le       " << ctime(&timeNow) << endl;
	listingFile << "LINE NO.              SOURCE STATEMENT" << endl << endl;
	//line numbers and source statements should be aligned under the headings
}

void Compiler::parser() {
	nextChar();
	//ch must be initialized to the first character of the source file
	if (nextToken() != "program")
		processError("keyword \"program\" expected");
	//a call to nextToken() has two effects
	// (1) the variable, token, is assigned the value of the next token
	// (2) the next token is read from the source file in order to make
	// the assignment. The value returned by nextToken() is also
	// the next token.
	prog();
	//parser implements the grammar rules, calling first rule
}

void Compiler::createListingTrailer() {
	//print "COMPILATION TERMINATED", " ERRORS ENCOUNTERED"
	listingFile << "\nCOMPILATION TERMINATED      " << errorCount << " ERRORS ENCOUNTERED" << endl;
}

// stage 0 production 1

void Compiler::prog() { //token should be "program"
	if (token != "program")
		processError("keyword \"program\" expected");
	progStmt();
	if (token == "const")
		consts();
	if (token == "var")
		vars();
	if (token != "begin")
		processError("keyword \"begin\" expected");
	beginEndStmt();
	if (token[0] != END_OF_FILE)
		processError("no text may follow \"end\"");
}

// stage 0 production 2

void Compiler::progStmt() { //token should be "program"
	string x;
	if (token != "program")
		processError("keyword \"program\" expected");
	x = nextToken();
	if (!isNonKeyId(token))
		processError("program name expected");
	if (nextToken() != ";")
		processError("semicolon expected");
	nextToken();
	code("program", x);
	insert(x, storeTypes::PROG_NAME, modes::CONSTANT, x, allocation::NO, 0);
}

// stage 0 production 3

void Compiler::consts() { //token should be "const"
	if (token != "const")
		processError("keyword \"const\" expected");
	if (!isNonKeyId(nextToken()))
		processError("non - keyword identifier must follow \"const\"");
	constStmts();
}

// stage 0 production 4

void Compiler::vars() { //token should be "var"
	if (token != "var")
		processError("keyword \"var\" expected");
	if (!isNonKeyId(nextToken()))
		processError("non - keyword identifier must follow \"var\"");
	varStmts();
}

// stage 0 production 5

void Compiler::beginEndStmt() { //token should be "begin"
	if (token != "begin")
		processError("keyword \"begin\" expected");

	// change as of stage 1
	nextToken();
	if (isNonKeyId(token) || token == "read" || token == "write" || token == ";" || token == "begin") {
		execStmts();
	}
	
	if (token != "end")
		processError("keyword \"end\" expected");
	if (nextToken() != ".")
		processError("period expected");
	nextToken();
	code("end", ".");
}

// stage 0 production 6

void Compiler::constStmts() { //token should be NON_KEY_ID
	string x, y;
	if (!isNonKeyId(token))
		processError("non - keyword identifier expected");
	x = token;
	if (nextToken() != "=")
		processError("\"=\" expected");
	y = nextToken();
	if (y != "+" && y != "-" && y != "not" && !isNonKeyId(y) && !isBoolean(y) && !isInteger(y))
		processError("token to right of \"=\" illegal");
	if (y == "+" || y == "-")
	{
		if (!isInteger(nextToken()))
			processError("integer expected after sign");
		y = y + token;
	}
	if (y == "not")
	{
		if (!isBoolean(nextToken()))
			processError("boolean expected after \"not\"");
		if (token == "true")
			y = "false";
		else
			y = "true";
	}
	if (nextToken() != ";")
		processError("semicolon expected");
	if (whichType(y) != storeTypes::INTEGER && whichType(y) != storeTypes::BOOLEAN)
		processError("data type of token on the right - hand side must be INTEGER or BOOLEAN");
	insert(x, whichType(y), modes::CONSTANT, whichValue(y), allocation::YES, 1);
	x = nextToken();
	if (x != "begin" && x != "var" && !isNonKeyId(x))
		processError("non - keyword identifier, \"begin\", or \"var\" expected");
	if (isNonKeyId(x))
		constStmts();
}

// stage 0 production 7

void Compiler::varStmts() { //token should be NON_KEY_ID
	string x, y;
	if (!isNonKeyId(token))
		processError("non - keyword identifier expected");
	x = ids();
	if (token != ":")
		processError("\":\" expected; found" + token);
	if (nextToken() != "integer" && token != "boolean")
		processError("illegal type follows \":\"");
	y = token;
	if (nextToken() != ";")
		processError("semicolon expected");

	if (y == "integer") insert(x, storeTypes::INTEGER, modes::VARIABLE, "1", allocation::YES, 1);
	else insert(x, storeTypes::BOOLEAN, modes::VARIABLE, "1", allocation::YES, 1);
	if (nextToken() != "begin" && !isNonKeyId(token))
		processError("non - keyword identifier or \"begin\" expected");
	if (isNonKeyId(token))
		varStmts();
}

// stage 0 production 8

string Compiler::ids() { //token should be NON_KEY_ID
	string temp, tempString;
	if (!isNonKeyId(token))
		processError("non - keyword identifier expected");
	tempString = token;
	temp = token;
	if (nextToken() == ",")
	{
		if (!isNonKeyId(nextToken()))
			processError("non - keyword identifier expected");
		tempString = temp + "," + ids();
	}
	return tempString;
}

void Compiler::execStmts() {      // stage 1, production 2
	if (isNonKeyId(token) || token == "read" || token == "write" || token == ";" ||  token == "begin") {
		execStmt();
		nextToken();
		execStmts();
	}
	else if (token == "end");

	else processError("non - keyword identifier, \"read\", \"write\", or \"begin\" expected");
}

void Compiler::execStmt() {       // stage 1, production 3

	if (isNonKeyId(token)) {
		assignStmt();
	}

	else if (token == "read") {
		readStmt();
	}

	else if (token == "write") {
		writeStmt();
	}

	else processError("non-keyword id, \"read\", or \"write\" expected");
}

void Compiler::assignStmt() {     // stage 1, production 4
	string secondOperand, firstOperand;
	if (!isNonKeyId(token))
		processError("non - keyword identifier expected");

	//Token must be already defined
	if (symbolTable.count(token) == 0) processError("reference to undefined variable");

	pushOperand(token);
	nextToken();

	if (token != ":=") processError("':=' expected; found " + token);
	else pushOperator(":=");

	nextToken();

	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
		&& token != "-" && !isInteger(token) && !isNonKeyId(token))
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	else express();

	if (token != ";") processError("';' expected; found " + token);

	secondOperand = popOperand();
	firstOperand = popOperand();
	code(popOperator(), secondOperand, firstOperand);
}

void Compiler::readStmt() {       // stage 1, production 5
	if (token != "read") processError("\"read\" expected");

	nextToken();

	if (token != "(") processError("expected '('; found " + token);

	//read_list
	string temp = "";
		
	nextToken();

	if (symbolTable.count(token) == 0 || isInteger(token))
	{
		if (symbolTable.count(token) == 0 && !isInteger(token))
			processError("reference to undefined variable");
		else
			processError("non-keyword identifier expected");
	}

	temp = ids();
	if (token != ")")
		processError("expected ')'; found " + token);
	else
		code("read", temp);

	if (nextToken() != ";") processError("';' expected; found " + token);
}

void Compiler::writeStmt() {      // stage 1, production 7
	if (token != "write")
		processError("\"write\" expected");

	nextToken();

	if (token != "(") processError("'(' expected; found " + token);
	
	//write_list
	string temp = "";
	string idsPart = "";

	nextToken();

	if (symbolTable.count(token) == 0 || isInteger(token))
	{
		if (symbolTable.count(token) == 0 && !isInteger(token))
			processError("reference to undefined variable");
		else
			processError("non-keyword identifier expected");
	}

	temp = ids();
	if (token != ")")
		processError("')' expected; found " + token);

	else 
		code("write", temp);

	if (nextToken() != ";")
		processError("';' expected; found " + token);
}

void Compiler::express() {        // stage 1, production 9
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
		&& token != "-" && !isInteger(token) && !isNonKeyId(token))
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", non - keyword identifier or integer expected");

	term();

	if (token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">")
		expresses();
}

void Compiler::expresses() {      // stage 1, production 10
	string x = "";
	string operand1, operand2;
	if (token != "=" && token != "<>" && token != "<=" && token != ">=" && token != "<" && token != ">")
		processError("\"=\", \"<>\", \"<=\", \">=\", \"<\", or \">\" expected");

	pushOperator(token);
	nextToken();

	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
		&& token != "-" && !isInteger(token) && !isNonKeyId(token))
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	else term();

	operand1 = popOperand();
	operand2 = popOperand();

	code(popOperator(), operand1, operand2);

	if (token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">")
		expresses();
}

void Compiler::term() {           // stage 1, production 11
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
		&& token != "-" && !isInteger(token) && !isNonKeyId(token))
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");

	factor();

	if (token == "-" || token == "+" || token == "or") terms();
}

void Compiler::terms() {          // stage 1, production 12
	string x = "";
	string operand1, operand2;

	if (token != "+" && token != "-" && token != "or")
		processError("\"+\", \"-\", or \"or\" expected");

	pushOperator(token);
	nextToken();

	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
		&& token != "-" && !isInteger(token) && !isNonKeyId(token))
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	else factor();

	operand1 = popOperand();
	operand2 = popOperand();
	code(popOperator(), operand1, operand2);

	if (token == "+" || token == "-" || token == "or") terms();
}

void Compiler::factor() {         // stage 1, production 13
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
		&& token != "-" && !isInteger(token) && !isNonKeyId(token))
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");

	part();

	if (token == "*" || token == "div" || token == "mod" || token == "and")
		factors();

	else if (token == "=" || token == "<>" || token == "<=" || token == ">=" || token == "<" || token == ">" ||
		token == ")" || token == ";" || token == "-" || token == "+" || token == "or" || token == "begin");
	else processError("invalid expression");
}

void Compiler::factors() {        // stage 1, production 14
	string x = "";
	string operand1, operand2;
	if (token != "*" && token != "div" && token != "mod" && token != "and")
		processError("\"*\", \"div\", \"mod\", or \"and\" expected");

	pushOperator(token);
	nextToken();

	if (token != "not" && token != "+" && token != "-" && token != "(" && token != "true" && token != "false"
		&& !isInteger(token) && !isNonKeyId(token))
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	else part();

	operand1 = popOperand();
	operand2 = popOperand();
	code(popOperator(), operand1, operand2);
	if (token == "*" || token == "div" || token == "mod" || token == "and")
		factors();
}

void Compiler::part() {           // stage 1, production 15
	string x = "";
	if (token == "not")
	{
		nextToken();
		if (token == "(") {
			nextToken();
			if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
				&& token != "-" && !isInteger(token) && !isNonKeyId(token))
				processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
			express();
			if (token != ")")
				processError(") expected; found " + token);
			nextToken();
			code("not", popOperand());
		}

		else if (isBoolean(token)) {
			if (token == "true") {
				pushOperand("false");
				nextToken();
			}
			else {
				pushOperand("true");
				nextToken();
			}
		}

		else if (isNonKeyId(token)) {
			code("not", token);
			nextToken();
		}
	}

	else if (token == "+")
	{
		nextToken();
		if (token == "(") {
			nextToken();
			if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
				&& token != "-" && !isInteger(token) && !isNonKeyId(token))
				processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
			express();
			if (token != ")")
				processError("expected ')'; found " + token);
			nextToken();
		}
		else if (isInteger(token) || isNonKeyId(token)) {
			pushOperand(token);
			nextToken();
		}

		else processError("expected '(', integer, or non-keyword id; found " + token);
	}

	else if (token == "-")
	{
		nextToken();
		if (token == "(") {
			nextToken();
			if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
				&& token != "-" && !isInteger(token) && !isNonKeyId(token))
				processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
			express();
			if (token != ")")
				processError("expected ')'; found " + token);
			nextToken();
			code("neg", popOperand());
		}
		else if (isInteger(token)) {
			pushOperand("-" + token);
			nextToken();
		}
		else if (isNonKeyId(token)) {
			code("neg", token);
			nextToken();
		}
	}

	else if (token == "(") {
		nextToken();
		if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
			&& token != "-" && !isInteger(token) && !isNonKeyId(token))
			processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
		express();
		if (token != ")") processError(") expected; found " + token);
		nextToken();
	}

	else if (isInteger(token) || isBoolean(token) || isNonKeyId(token)) {
		pushOperand(token);
		nextToken();
	}

	else processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, boolean, or non - keyword identifier expected");
}

// Helper functions for the Pascallite lexicon
bool Compiler::isKeyword(string s) const { // determines if s is a keyword
	if (s == "program"
		|| s == "const"
		|| s == "var"
		|| s == "integer"
		|| s == "boolean"
		|| s == "begin"
		|| s == "end"
		|| s == "true"
		|| s == "false"
		|| s == "not"
		|| s == "mod"
		|| s == "div"
		|| s == "and"
		|| s == "or"
		|| s == "read"
		|| s == "write") {
		return true;
	}
	return false;
}

bool Compiler::isSpecialSymbol(char c) const { // determines if c is a special symbol
	if (c == '='
		|| c == ':'
		|| c == ','
		|| c == ';'
		|| c == '.'
		|| c == '+'
		|| c == '-'
		|| c == '*'
		|| c == '<'
		|| c == '>'
		|| c == '('
		|| c == ')') {
		return true;
	}
	return false;
}

bool Compiler::isNonKeyId(string s) const { // determines if s is a non_key_id
	return !(isKeyword(s) || isSpecialSymbol(s[0]) || isInteger(s));
}

bool Compiler::isInteger(string s) const { // determines if s is an integer
	try {
		stoi(s);
	}
	catch (const invalid_argument& ia) {
		return false;
	}
	return true;
}

bool Compiler::isBoolean(string s) const { // determines if s is a boolean
	if (s == "true" || s == "false") {
		return true;
	}
	return false;
}

bool Compiler::isLiteral(string s) const { // determines if s is a literal
	if (isBoolean(s) || s.front() == '+' || s.front() == '-' || isInteger(s))
		return true;

	return false;
}

// Action routines

void Compiler::insert(string externalName, storeTypes inType, modes inMode, string inValue, allocation inAlloc, int inUnits) {
//create symbol table entry for each identifier in list of external names
//Multiply inserted names are illegal

	string name;

	auto itr = externalName.begin();

	while (itr < externalName.end()) {
		name = "";
		while (itr < externalName.end() && *itr != ',') {
			name += *itr;
			++itr;
		}

		if (name != "")	{
			if (symbolTable.count(name) > 0)
				processError("multiple name definition");
			else if (isKeyword(name) && name != "true" && name != "false")
				processError("illegal use of keyword");
			else //create table entry
			{
				if (isupper(name[0]) || name == "true" || name == "false")
					if (name == "true")
						symbolTable.insert(pair<string, SymbolTableEntry>(name.substr(0, 15),
							SymbolTableEntry("TRUE", inType, inMode, inValue, inAlloc, inUnits)));
					else if (name == "false")
						symbolTable.insert(pair<string, SymbolTableEntry>(name.substr(0, 15),
							SymbolTableEntry("FALSE", inType, inMode, inValue, inAlloc, inUnits)));
					else symbolTable.insert(pair<string, SymbolTableEntry>(name.substr(0, 15),
						SymbolTableEntry(name, inType, inMode, inValue, inAlloc, inUnits)));
				else
					symbolTable.insert(pair<string, SymbolTableEntry>(name.substr(0, 15),
						SymbolTableEntry(genInternalName(inType), inType, inMode, inValue, inAlloc, inUnits)));
			}
		}

		if (itr == externalName.end()) break;
		else ++itr;
	}
}

storeTypes Compiler::whichType(string name) { //tells which data type a name has
	storeTypes dataType;
	if (isLiteral(name))
		if (isBoolean(name))
			dataType = storeTypes::BOOLEAN;
		else
			dataType = storeTypes::INTEGER;
	else //name is an identifier and hopefully a constant
		if (symbolTable.count(name) > 0)
			dataType = symbolTable.at(name).getDataType();
		else
			processError("reference to undefined constant");
	return dataType;
}

string Compiler::whichValue(string name) { //tells which value a name has
	string value;

	if (isLiteral(name)) {
		if (name == "true")
			value = "-1";
		else if (name == "false")
			value = "0";
		else value = name;
	}

	else //name is an identifier and hopefully a constant
		if (symbolTable.count(name) > 0 && symbolTable.at(name).getValue() != "")
			value = symbolTable.at(name).getValue();
		else
			processError("reference to undefined constant");
	return value;
}

void Compiler::code(string op, string operand1, string operand2) {
	if (op == "program")
		emitPrologue(operand1);
	else if (op == "end")
		emitEpilogue();
	else if (op == "read")
		emitReadCode(operand1, "");
	else if (op == "write")
		emitWriteCode(operand1, "");
	else if (op == "+") // this must be binary '+'
		emitAdditionCode(operand1, operand2);
	else if (op == "-") // this must be binary '-'
		emitSubtractionCode(operand1, operand2);
	else if (op == "neg") // this must be unary '-'
		emitNegationCode(operand1, "");
	else if (op == "not")
		emitNotCode(operand1, "");
	else if (op == "*")
		emitMultiplicationCode(operand1, operand2);
	else if (op == "div")
		emitDivisionCode(operand1, operand2);
	else if (op == "mod")
		emitModuloCode(operand1, operand2);
	else if (op == "and")
		emitAndCode(operand1, operand2);
	else if (op == "<>")
		emitInequalityCode(operand1, operand2);
	else if (op == "or")
		emitOrCode(operand1, operand2);
	else if (op == "<")
		emitLessThanCode(operand1, operand2);
	else if (op == ">")
		emitGreaterThanCode(operand1, operand2);
	else if (op == "<=")
		emitLessThanOrEqualToCode(operand1, operand2);
	else if (op == ">=")
		emitGreaterThanOrEqualToCode(operand1, operand2);
	else if (op == "=")
		emitEqualityCode(operand1, operand2);
	else if (op == ":=")
		emitAssignCode(operand1, operand2);
	else
		processError("compiler error since function code should not be called with illegal arguments");
}

void Compiler::pushOperator(string name) { // Push name onto opertorStk
	operatorStk.push(name);
}

string Compiler::popOperator() { // pop name from operandStk

	string temp;

	if (!operatorStk.empty()) {
		temp = operatorStk.top();
		operatorStk.pop();
	}

	else processError("compiler error; operator stack underflow");

	return temp;
}

void Compiler::pushOperand(string operand) { // Push name onto operandStk
	if (symbolTable.count(operand) == 0) {
		if (isInteger(operand) || operand == "true" || operand == "false")
			insert(operand, whichType(operand), modes::CONSTANT, whichValue(operand), allocation::YES, 1);
	}

	operandStk.push(operand);
}
string Compiler::popOperand() { //pop name from operandStk
	string temp;

	if (!operandStk.empty()) {
		temp = operandStk.top();
		operandStk.pop();
	}

	else processError("compiler error; operand stack underflow");

	return temp;
}

void Compiler::emit(string label, string instruction, string operands, string comment)
{
	//Turn on left justification in objectFile
	objectFile.setf(ios_base::left);

	//Output label in a field of width 8
	objectFile << setw(8) << label;

	//Output instruction in a field of width 8
	objectFile << setw(8) << instruction;

	//Output the operands in a field of width 24
	objectFile << setw(24) << operands;

	//Output the comment
	objectFile << comment << endl;
}

void Compiler::emitPrologue(string progName, string operand2)
{
	time_t timeNow = time(NULL);
	objectFile << "; Hoang Long Nguyen & Minh Ha Le      " << ctime(&timeNow);
	objectFile << "%INCLUDE \"Along32.inc\"\n"
		"%INCLUDE \"Macros_Along.inc\"\n\n";

	emit("SECTION", ".text");
	emit("global", "_start", "", "; program " + progName.substr(0, 15));
	objectFile << endl;
	emit("_start:");
}

void Compiler::emitEpilogue(string operand1, string operand2)
{
	emit("", "Exit", "{0}");
	objectFile << endl;
	emitStorage();
}

void Compiler::emitStorage()
{
	emit("SECTION", ".data");
	//for those entries in the symbolTable that have
				//an allocation of YES and a storage mode of CONSTANT
			//{ call emit to output a line to objectFile }

	for (auto data : symbolTable)
		if (data.second.getAlloc() == allocation::YES && data.second.getMode() == modes::CONSTANT)
			emit(data.second.getInternalName(), "dd", data.second.getValue(), "; " + data.first);
	objectFile << endl;

	emit("SECTION", ".bss");
	//for those entries in the symbolTable that have
				//an allocation of YES and a storage mode of VARIABLE
			//{ call emit to output a line to objectFile }

	for (auto data : symbolTable)
		if (data.second.getAlloc() == allocation::YES && data.second.getMode() == modes::VARIABLE)
			emit(data.second.getInternalName(), "resd", data.second.getValue(), "; " + data.first);
}

void Compiler::emitReadCode(string operand, string) {
	string name;

	for (uint i = 0; i < operand.size(); ++i) {
				
		if (operand[i] != ',' && i < operand.size()) {
			name += operand[i];
			continue;
		}

		if (name != "") {

			if (symbolTable.count(name) == 0)
				processError("reference to undefined symbol " + name);
			if (symbolTable.at(name).getDataType() != storeTypes::INTEGER)
				processError("can't read variables of this type");
			if (symbolTable.at(name).getMode() != modes::VARIABLE)
				processError("attempting to read to a read-only location");
			emit("", "call", "ReadInt", "; read int; value placed in eax");
			emit("", "mov", "[" + symbolTable.at(name).getInternalName() + "],eax", "; store eax at " + name);
			contentsOfAReg = symbolTable.at(name).getInternalName();
		}

		name = "";
	}

	if (name != "") {

		if (symbolTable.count(name) == 0)
			processError("reference to undefined symbol " + name);
		if (symbolTable.at(name).getDataType() != storeTypes::INTEGER)
			processError("can't read variables of this type");
		if (symbolTable.at(name).getMode() != modes::VARIABLE)
			processError("attempting to read to a read-only location");
		emit("", "call", "ReadInt", "; read int; value placed in eax");
		emit("", "mov", "[" + symbolTable.at(name).getInternalName() + "],eax", "; store eax at " + name);
		contentsOfAReg = symbolTable.at(name).getInternalName();
	}
}

void Compiler::emitWriteCode(string operand, string)
{
	string name;
	static bool definedStorage = false;

	for (uint i = 0; i < operand.size(); ++i) {

		if (operand[i] != ',' && i < operand.size()) {
			name += operand[i];
			continue;
		}

		if (name != "") {
			if (symbolTable.count(name) == 0)
				processError("reference to undefined symbol " + name);
			if (symbolTable.at(name).getInternalName() != contentsOfAReg) {
				emit("", "mov", "eax,[" + symbolTable.at(name).getInternalName() + "]", "; load " + name + " in eax");
				contentsOfAReg = symbolTable.at(name).getInternalName();
			}
			if (symbolTable.at(name).getDataType() == storeTypes::INTEGER)
				emit("", "call", "WriteInt", "; write int in eax to standard out");
			else { //data type is BOOLEAN
				emit("", "call", "WriteInt", "; write int in eax to standard out");

				if (definedStorage == false) {
					definedStorage = true;
					
				} // end if
			} // end else

			emit("", "call", "Crlf", "; write \\r\\n to standard out");
		}
		name = "";
	} // end for loop

	if (symbolTable.count(name) == 0)
		processError("reference to undefined symbol " + name);
	if (symbolTable.at(name).getInternalName() != contentsOfAReg) {
		emit("", "mov", "eax,[" + symbolTable.at(name).getInternalName() + "]", "; load " + name + " in eax");
		contentsOfAReg = symbolTable.at(name).getInternalName();
	}
	if (symbolTable.at(name).getDataType() == storeTypes::INTEGER)
		emit("", "call", "WriteInt", "; write int in eax to standard out");
	else { //data type is BOOLEAN
		emit("", "call", "WriteInt", "; write int in eax to standard out");

		if (definedStorage == false) {
			definedStorage = true;
		} // end if
	} // end else

	emit("", "call", "Crlf", "; write \\r\\n to standard out");
}

void Compiler::emitAssignCode(string operand1, string operand2) {         // op2 = op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);

	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("incompatible types for operator ':='");

	if (symbolTable.at(operand2).getMode() != modes::VARIABLE)
		processError("symbol on left-hand side of assignment must have a storage mode of VARIABLE");

	if (operand1 == operand2) return;

	if (symbolTable.at(operand1).getInternalName() != contentsOfAReg)
		emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);
	emit("", "mov", "[" + symbolTable.at(operand2).getInternalName() + "],eax", "; " + operand2 + " = AReg");

	contentsOfAReg = symbolTable.at(operand2).getInternalName();
	
	if (operand1[0] == 'T')
		freeTemp();
}

void Compiler::emitAdditionCode(string operand1, string operand2) {       // op2 + op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);

	if (symbolTable.at(operand1).getDataType() != storeTypes::INTEGER 
		|| symbolTable.at(operand2).getDataType() != storeTypes::INTEGER)
		processError("binary '+' requires integer operands");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		contentsOfAReg = "";
	}

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "add", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " + " + operand1);
	else
		emit("", "add", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " + " + operand2);

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::INTEGER);
	pushOperand(contentsOfAReg);
}

void Compiler::emitSubtractionCode(string operand1, string operand2) {    // op2 - op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);

	if (symbolTable.at(operand1).getDataType() != storeTypes::INTEGER
		|| symbolTable.at(operand2).getDataType() != storeTypes::INTEGER)
		processError("binary '-' requires integer operands");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{

		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		contentsOfAReg = "";
	}

	if (contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "sub", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " - " + operand1);

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::INTEGER);
	pushOperand(contentsOfAReg);
}
void Compiler::emitMultiplicationCode(string operand1, string operand2) { // op2 * op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);

	if (symbolTable.at(operand1).getDataType() != storeTypes::INTEGER ||
		symbolTable.at(operand2).getDataType() != storeTypes::INTEGER)
		processError("binary '*' requires integer operands");
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "imul", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " * " + operand1);
	else emit("", "imul", "dword [" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " * " + operand2);

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::INTEGER);
	pushOperand(contentsOfAReg);
}


void Compiler::emitDivisionCode(string operand1, string operand2) { // op2 / op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);
	
	if (symbolTable.at(operand1).getDataType() != storeTypes::INTEGER ||
		symbolTable.at(operand2).getDataType() != storeTypes::INTEGER)
		processError("binary 'div' requires integer operands");
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}
	emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");
	emit("", "idiv", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " div " + operand1);

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::INTEGER);
	pushOperand(contentsOfAReg);
}

void Compiler::emitModuloCode(string operand1, string operand2) {         // op2 % op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);
	
	if (symbolTable.at(operand1).getDataType() != storeTypes::INTEGER
		|| symbolTable.at(operand2).getDataType() != storeTypes::INTEGER)
		processError("binary 'mod' requires integer operands");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");
	emit("", "idiv", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " div " + operand1);
	emit("", "xchg", "eax,edx", "; exchange quotient and remainder");

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::INTEGER);
	pushOperand(contentsOfAReg);
}

void Compiler::emitNegationCode(string operand1, string) {           // -op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	
	if (symbolTable.at(operand1).getDataType() != storeTypes::INTEGER)
		processError("binary '-' requires integer operands");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);
		contentsOfAReg = symbolTable.at(operand1).getInternalName();
	}

	emit("", "neg", "eax", "; AReg = -AReg");
	if (operand1[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::INTEGER);
	pushOperand(contentsOfAReg);
}

void Compiler::emitNotCode(string operand1, string) {                // !op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	
	if (symbolTable.at(operand1).getDataType() != storeTypes::BOOLEAN)
		processError("binary 'not' requires boolean operands");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);
		contentsOfAReg = symbolTable.at(operand1).getInternalName();
	}

	emit("", "not", "eax", "; AReg = !AReg");
	if (operand1[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitAndCode(string operand1, string operand2) {            // op2 && op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);
	
	if (symbolTable.at(operand1).getDataType() != storeTypes::BOOLEAN ||
		symbolTable.at(operand2).getDataType() != storeTypes::BOOLEAN)
		processError("binary 'and' requires boolean operands");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "and", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " and " + operand1);
	else
		emit("", "and", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " and " + operand2);

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitOrCode(string operand1, string operand2) {             // op2 || op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);
	
	if (symbolTable.at(operand1).getDataType() != storeTypes::BOOLEAN ||
		symbolTable.at(operand2).getDataType() != storeTypes::BOOLEAN)
		processError("binary 'or' requires boolean operands");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "or", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " or " + operand1);
	else
		emit("", "or", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " or " + operand2);


	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitEqualityCode(string operand1, string operand2) {       // op2 == op1
	
	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}
	
	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	else
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);

	string firstLabel = getLabel(), secondLabel = getLabel();


	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "je", "." + firstLabel, "; if " + operand2 + " = " + operand1 + " then jump to set eax to TRUE");
	else 
		emit("", "je", "." + firstLabel, "; if " + operand1 + " = " + operand2 + " then jump to set eax to TRUE");

	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false", storeTypes::BOOLEAN, modes::CONSTANT, "0", allocation::YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true", storeTypes::BOOLEAN, modes::CONSTANT, "-1", allocation::YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitInequalityCode(string operand1, string operand2) {     // op2 <> op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);
	
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("incompatible types for operator '<>'");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	else
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);

	string firstLabel = getLabel(), secondLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "jne", "." + firstLabel, "; if " + operand2 + " <> " + operand1 + " then jump to set eax to TRUE");
	else
		emit("", "jne", "." + firstLabel, "; if " + operand1 + " <> " + operand2 + " then jump to set eax to TRUE");

	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false", storeTypes::BOOLEAN, modes::CONSTANT, "0", allocation::YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true", storeTypes::BOOLEAN, modes::CONSTANT, "-1", allocation::YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitLessThanCode(string operand1, string operand2) {       // op2 < op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);
	
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("incompatible types for operator '<'");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	else
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);

	string firstLabel = getLabel(), secondLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "jl", "." + firstLabel, "; if " + operand2 + " < " + operand1 + " then jump to set eax to TRUE");
	else 
		emit("", "jl", "." + firstLabel, "; if " + operand1 + " < " + operand2 + " then jump to set eax to TRUE");

	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false", storeTypes::BOOLEAN, modes::CONSTANT, "0", allocation::YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true", storeTypes::BOOLEAN, modes::CONSTANT, "-1", allocation::YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitLessThanOrEqualToCode(string operand1, string operand2) { // op2 <= op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);
	
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("incompatible types for operator '<='");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	else
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);

	string firstLabel = getLabel(), secondLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "jle", "." + firstLabel, "; if " + operand2 + " <= " + operand1 + " then jump to set eax to TRUE");
	else
		emit("", "jle", "." + firstLabel, "; if " + operand1 + " <= " + operand2 + " then jump to set eax to TRUE");

	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false", storeTypes::BOOLEAN, modes::CONSTANT, "0", allocation::YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true", storeTypes::BOOLEAN, modes::CONSTANT, "-1", allocation::YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitGreaterThanCode(string operand1, string operand2) {    // op2 > op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);
	
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("incompatible types for operator '>'");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	else
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);

	string firstLabel = getLabel(), secondLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "jg", "." + firstLabel, "; if " + operand2 + " > " + operand1 + " then jump to set eax to TRUE");
	else
		emit("", "jg", "." + firstLabel, "; if " + operand1 + " > " + operand2 + " then jump to set eax to TRUE");

	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false", storeTypes::BOOLEAN, modes::CONSTANT, "0", allocation::YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true", storeTypes::BOOLEAN, modes::CONSTANT, "-1", allocation::YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitGreaterThanOrEqualToCode(string operand1, string operand2) { // op2 >= op1

	if (symbolTable.count(operand1) == 0)
		processError("reference to undefined symbol " + operand1);
	else if (symbolTable.count(operand2) == 0)
		processError("reference to undefined symbol " + operand2);
	
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("incompatible types for operator '>='");

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName())
		contentsOfAReg = "";

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	else
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);

	string firstLabel = getLabel(), secondLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
		emit("", "jge", "." + firstLabel, "; if " + operand2 + " >= " + operand1 + " then jump to set eax to TRUE");
	else
		emit("", "jge", "." + firstLabel, "; if " + operand1 + " >= " + operand2 + " then jump to set eax to TRUE");

	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false", storeTypes::BOOLEAN, modes::CONSTANT, "0", allocation::YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true", storeTypes::BOOLEAN, modes::CONSTANT, "-1", allocation::YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (operand1[0] == 'T')
		freeTemp();
	if (operand2[0] == 'T')
		freeTemp();

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(storeTypes::BOOLEAN);
	pushOperand(contentsOfAReg);
}

// lexical scanner

string Compiler::nextToken() //returns the next token or end of file marker
{
	token = "";
	while (token == "")
	{
		if (ch == '{') //process comment
		{
			while (nextChar() != END_OF_FILE && ch != '}')
			{ //empty body 
			}
			if (ch == END_OF_FILE)
				processError("unexpected end of file");
			else
				nextChar();
		}

		else if (ch == '}') processError("'}' cannot begin token");
		else if (isspace(ch)) nextChar();
		else if (isSpecialSymbol(ch)) {
			//update as of stage 1

			token = ch;
			nextChar();
			if (token == ":" && ch == '=') {
				token += ch;
				nextChar();
			}
			else if ((token == "<" && ch == '>') || (token == "<" && ch == '=') || (token == ">" && ch == '=')) {
				token += ch;
				nextChar();
			}
		}
		else if (islower(ch)) {
			token = ch;
			while ((nextChar() == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
				|| (ch >= '0' && ch <= '9')) && ch != END_OF_FILE)
			{
				token += ch;
			}
			if (ch == END_OF_FILE)
				processError("unexpected end of file");
		}
		else if (isdigit(ch)) {
			token = ch;
			while (nextChar() != END_OF_FILE && ch >= '0' && ch <= '9')
			{
				token += ch;
			}
			if (ch == END_OF_FILE)
				processError("unexpected end of file");
		}

		else if (ch == END_OF_FILE) token = ch;
		else processError("illegal symbol");
	}
	return token;
}

char Compiler::nextChar() //returns the next character or end of file marker
{
	sourceFile.get(ch);

	static char prevChar = '\n';

	if (sourceFile.eof()) {
		ch = END_OF_FILE;
		return ch;
	}
	else {
		if (prevChar == '\n') {
			listingFile << setw(5) << ++lineNo << '|';
		}
		listingFile << ch;
	}

	prevChar = ch;
	return ch;
}

// Other routines

string Compiler::genInternalName(storeTypes stype) const {
	string internName;

	switch (stype) {
	case storeTypes::PROG_NAME:
		internName = "P0";
		break;
	case storeTypes::INTEGER:
	{
		int countNum = 0;
		for (auto itr : symbolTable) {
			if (itr.second.getDataType() == storeTypes::INTEGER && itr.first[0] != 'T') ++countNum;
		}

		internName = "I" + to_string(countNum);
		break;
	}
	case storeTypes::BOOLEAN:
	{
		int countBool = 0;
		for (auto itr : symbolTable) {
			if (itr.second.getDataType() == storeTypes::BOOLEAN) ++countBool;
		}

		internName = "B" + to_string(countBool);
		break;
	}
	case storeTypes::UNKNOWN: {}
	}

	return internName;
}

void Compiler::processError(string err)
{
	//Output err to listingFile
	//Call exit() to terminate program

	listingFile << endl << "Error: Line " << lineNo << ": " << err << endl;
	errorCount++;
	listingFile << "\nCOMPILATION TERMINATED      " << errorCount << " ERROR ENCOUNTERED" << endl;

	exit(-1);
}

void Compiler::freeTemp() {
	currentTempNo--;
	if (currentTempNo < -1)
		processError("compiler error, currentTempNo should be >= -1");
}

string Compiler::getTemp() {
	string temp;
	currentTempNo++;
	temp = "T" + to_string(currentTempNo);
	if (currentTempNo > maxTempNo) {
		insert(temp, storeTypes::UNKNOWN, modes::VARIABLE, "1", allocation::NO, 1);
		symbolTable.at(temp).setInternalName(temp);
		maxTempNo++;
	}

	return temp;
}

string Compiler::getLabel() {
	static int labelCount = -1;
	string label;
	labelCount++;
	label = "L" + to_string(labelCount);
	return label;
}

bool Compiler::isTemporary(string s) const { // determines if s represents a temporary
	if (s[0] == 'T') return true;
	else return false;
}