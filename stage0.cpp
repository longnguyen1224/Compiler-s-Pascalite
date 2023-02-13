// Hoang Long Nguyen & Minh Ha Le
// CS 4301
// Stage 0

#include <stage0.h>
#include <ctime>
#include <iomanip>

//member function

Compiler::Compiler(char** argv) // constructor
{
	sourceFile.open(argv[1]);
	listingFile.open(argv[2]);
	objectFile.open(argv[3]);
}

Compiler::~Compiler() // destructor
{
	sourceFile.close();
	listingFile.close();
	objectFile.close();
}

void Compiler::createListingHeader()
{
	time_t timeNow = time(NULL);
	listingFile << "STAGE0:  Hoang Long Nguyen & Minh Ha Le       " << ctime(&timeNow) << endl;
	listingFile << "LINE NO.              SOURCE STATEMENT" << endl << endl;
	//line numbers and source statements should be aligned under the headings
}

void Compiler::parser()
{
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

void Compiler::createListingTrailer()
{
	//print "COMPILATION TERMINATED", " ERRORS ENCOUNTERED"
	listingFile << "\nCOMPILATION TERMINATED      " << errorCount << " ERRORS ENCOUNTERED" << endl;

}

//production 1

void Compiler::prog() //token should be "program"
{
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

// production 2

void Compiler::progStmt() //token should be "program"
{
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

// production 3

void Compiler::consts() //token should be "const"
{
	if (token != "const")
		processError("keyword \"const\" expected");
	if (!isNonKeyId(nextToken()))
		processError("non - keyword identifier must follow \"const\"");
	constStmts();
}

// production 4

void Compiler::vars() //token should be "var"
{
	if (token != "var")
		processError("keyword \"var\" expected");
	if (!isNonKeyId(nextToken()))
		processError("non - keyword identifier must follow \"var\"");
	varStmts();
}

// production 5

void Compiler::beginEndStmt() //token should be "begin"
{
	if (token != "begin")
		processError("keyword \"begin\" expected");
	if (nextToken() != "end")
		processError("keyword \"end\" expected");
	if (nextToken() != ".")
		processError("period expected");
	nextToken();
	code("end", ".");
}

// production 6

void Compiler::constStmts() //token should be NON_KEY_ID
{
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

// production 7

void Compiler::varStmts() //token should be NON_KEY_ID
{
	string x, y;
	if (!isNonKeyId(token))
		processError("non - keyword identifier expected");
	x = ids();
	if (token != ":")
		processError("\":\" expected");
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

// production 8

string Compiler::ids() //token should be NON_KEY_ID
{
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

// Helper functions for the Pascallite lexicon
bool Compiler::isKeyword(string s) const// determines if s is a keyword
{
	if (s == "program"
		|| s == "const"
		|| s == "var"
		|| s == "integer"
		|| s == "boolean"
		|| s == "begin"
		|| s == "end"
		|| s == "true"
		|| s == "false"
		|| s == "not") {
		return true;
	}
	return false;
}

bool Compiler::isSpecialSymbol(char c) const // determines if c is a special symbol
{
	if (c == '='
		|| c == ':'
		|| c == ','
		|| c == ';'
		|| c == '.'
		|| c == '+'
		|| c == '-') {
		return true;
	}
	return false;
}
bool Compiler::isNonKeyId(string s) const // determines if s is a non_key_id
{
	return !(isKeyword(s) || isSpecialSymbol(s[0]) || isInteger(s));
}
bool Compiler::isInteger(string s) const // determines if s is an integer
{
	try {
		stoi(s);
	}
	catch (const invalid_argument& ia) {
		return false;
	}
	return true;
}
bool Compiler::isBoolean(string s) const // determines if s is a boolean
{
	if (s == "true" || s == "false") {
		return true;
	}
	return false;
}
bool Compiler::isLiteral(string s) const // determines if s is a literal
{
	if (s == "true" || s == "false" || s.front() == '+' || s.front() == '-' || isInteger(s))
		return true;

	return false;
}

// Action routines

void Compiler::insert(string externalName, storeTypes inType, modes inMode, string inValue, allocation inAlloc, int inUnits)
//create symbol table entry for each identifier in list of external names
//Multiply inserted names are illegal
{
	string name;

	auto itr = externalName.begin();

	while (itr < externalName.end()) {
		name = "";
		while (itr < externalName.end() && *itr != ',' ) {
			name += *itr;
			++itr;
		}

		if (name != "")
		{
			if (symbolTable.count(name) > 0)
				processError("symbol " + name + " is multiply defined");
			else if (isKeyword(name))
				processError("illegal use of keyword");
			else //create table entry
			{
				if (name.at(0) < 97)
					symbolTable.insert(pair<string, SymbolTableEntry>(name.substr(0, 15), 
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

storeTypes Compiler::whichType(string name) //tells which data type a name has
{
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

string Compiler::whichValue(string name) //tells which value a name has
{
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

void Compiler::code(string op, string operand1, string operand2)
{
	if (op == "program")
		emitPrologue(operand1);
	else if (op == "end")
		emitEpilogue();
	else
		processError("compiler error since function code should not be called with illegal arguments");
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
	objectFile << "; Hoang Long Nguyen & Minh Ha Le       " << ctime(&timeNow);
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
			token = ch;
			nextChar();
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
			if (itr.second.getDataType() == storeTypes::INTEGER) ++countNum;
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
