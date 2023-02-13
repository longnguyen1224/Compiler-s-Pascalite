//Hoang Long Nguyen & Minh Ha Le
//CS 4301
//Compiler Stage 2

#include <ctime>                //This is to allow us to calculate the current time
#include <iomanip>              //This is to enable use of setw()
#include <stage2.h>
#include <iostream>

using namespace std;

Compiler::Compiler(char **argv) // constructor
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
	time_t now = time(0);
	char* time = ctime(&now);
	// outputs our names and the time of compilation
	listingFile << "STAGE2:  Hoang Long Nguyen & Minh Ha Le       " << time << endl;
	listingFile << "LINE NO.              " << "SOURCE STATEMENT" << endl << endl;
}


void Compiler::parser() 
{
	string error;
	nextChar();
  

	if (nextToken() != "program")
	{
		error = "keyword \"program\" expected";
		processError(error);
	}

	prog();
}



void Compiler::createListingTrailer()
{
	//outputs the listing trailer at the end of the .lst file 
	listingFile << endl << "COMPILATION TERMINATED" << right << setw(7) << errorCount << " ERRORS ENCOUNTERED" << endl;
}



void Compiler::processError(string err)
{
	// for processing errors outputs at the bottom with the specified error
	listingFile << endl << "Error: Line " << lineNo << ": " << err << endl;
	errorCount += 1;
	listingFile << "\nCOMPILATION TERMINATED      " << errorCount << " ERROR ENCOUNTERED" << endl;
	exit(-1);
}



void Compiler::prog()  //token should be "program"
{
	// error is made due to error with a \ within "" 
	string error;
	if (token != "program")
	{
		error = "keyword \"program\" expected"; 
		processError(error);
	}

	progStmt();

	if (token == "const") 
	{
		consts(); 
	}

	if (token == "var")
	{ 
		vars(); 
	}	

	if (token != "begin")
	{
		// we were not sure whether to add a case here for 017.lst
		error = "keyword \"begin\" expected"; 
		processError(error);
	}

	beginEndStmt();

	if (token[0] != END_OF_FILE)    
	{
		error = "no text may follow \"end\""; 
		processError(error);
	}
} 


void Compiler::progStmt()  //token should be "program"
{   
	string x;
	string error;
	if (token != "program")
	{
		error = "keyword \"program\" expected"; 
		processError(error);
	}

	x = nextToken(); 

	if (!isNonKeyId(x))	//throws an error if "program" is not followed by a non_key_id 
	{
		processError("program name expected");
	}
  
	string y = nextToken();
  
	if (y != ";") 
	{
		processError("semicolon expected");
	}
  
	nextToken();
	code("program", x);
	insert(x,PROG_NAME,CONSTANT,x,NO,0);
}



void Compiler::consts()  //token should be "const"
{  
	string error;
	if (token != "const")
	{
		error = "keyword \"const\" expected"; 
		processError(error);
	}


	if (!isNonKeyId(nextToken()))
	{
		error = "non-keyword identifier must follow \"const\""; 
		processError(error);
	}

	constStmts();
} 



void Compiler::vars()  //token should be "var"
{   
    string error;

    if (token != "var")
    { 
		error = "keyword \"var\" expected"; 
		processError(error);
    }

    // TOOK STRING X OUT HERE
    if (!isNonKeyId(nextToken()))
    {
		error = "non-keyword identifier must follow \"var\""; 
		processError(error);
    }

    varStmts();
}



//process begin-end statement
void Compiler::beginEndStmt()	//stage 1 production 1
{   
    string error;

    if (token != "begin")
    {
		error = "keyword \"begin\" expected";
		processError(error);
    }

    nextToken();
	execStmts();	//make call to execStmts
	

    if (token != "end")
    {
		error =  "keyword \"end\" expected";
		processError(error);
    }
	
	nextToken();
	
    if (token == ".") 
    {
		code("end", ".");
		token[0] = '$';
    }
	else if (token == ";")
	{
		//code("end", ";");
	}

	else
	{
		processError("'.' or ';' expected following \"end\"");
	}
	
}


 
//process execution statements
void Compiler::execStmts()	//stage 1 production 2
{  
    if (isNonKeyId(token) || token == "read" || token == "write" || token == "begin" || token == "if" || token == "while" || token == "repeat" || token == ";")
	{
		execStmt();
		execStmts();
	}

	
	else if (token == "end");
	
	else if (token == "until");

	else
	{
		processError("one of \";\", \"begin\", \"if\", \"read\", \"repeat\", \"while\", \"write\", \"end\", or \"until\" expected");		//error here
	}
}



//process execution statement
void Compiler::execStmt()	//stage 1 production 3
{	
    if (isNonKeyId(token))
    {
      assignStmt();
    }
	else if (token == "begin")
	{
		beginEndStmt();
	}
	else if (token == "end")
	{
		return;
	}
    else if (token == "read")
    {
      readStmt();
    }
    else if (token == "write")
    {
      writeStmt();
    }
	 if (token == "if")
    {
      ifStmt();
    }
    else if (token == "while")
    {
      whileStmt();
    }
    else if (token == "repeat")
    {
	  repeatStmt();
    }
	else if (token == ";")
    {
      nullStmt();
    }
    else
    {
      processError("non-keyword id, \"read\", or \"write\" expected ");
    }
}



//process assign statement
void Compiler::assignStmt()	//stage 1 production 4
{
    string first, second;

    if (!isNonKeyId(token))
    {
      processError("non - keyword identifier expected");
    }
  
    if (symbolTable.count(token) == 0)	//if the token is not defined in our symbol table
    {
	  processError("reference to undefined variable");
    }

    pushOperand(token);
	nextToken();

	if (token != ":=")
	{
		processError("':=' expected; found " + token);
	}

	pushOperator(":=");	//push the operator ":=" onto the stack
	nextToken();
  
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger(token) && !isNonKeyId(token) && token != ";")
	{
		processError("expected non_key_id, integer, \"not\", \"true\", \"false\", '(', '+', or '-'");
	}

	express();
  
	second = popOperand();
	first = popOperand();

	code(popOperator(), second, first);

}



//process read statement
void Compiler::readStmt()	//stage 1 production 5
{
	string x;
	if (token != "read")
	{	
		processError("read expected; found " + token);
	}

	nextToken();

	if (token != "(")
	{
		processError("'(' expected; found " + token);
	}

	nextToken();
	x = ids();
  
	if (token != ")")
	{
		processError("',' or ')' expected; found " + token);
	}

	code("read", x);
	nextToken();

	if (token != ";")
	{
		processError("';' expected; found " + token);
	}
}



//process write statement
void Compiler::writeStmt()	//stage 1 production 7
{
	string x;
	if (token != "write")
	{
		processError("write expected; found " + token);
	}

	nextToken();

	if (token != "(")
	{
		processError("'(' expected after \"write\"");
	}

	nextToken();
	x = ids();

	if (token != ")")
	{
		processError("',' or ')' expected; found " + token);
	}

	code("write", x);
	nextToken();

	if (token != ";")
	{
	processError("';' expected");
	}
}



void Compiler::ifStmt() // stage 2, production 3
{	
	if (token != "if")
	{
		processError("if expected; found " + token);
	}
	
	nextToken();
	express();

	if (token != "then")
	{
		processError("then expected; found " + token);
	}

	string temp = popOperand();
	code("then", temp);
	nextToken();

	if (isNonKeyId(token) || token == "read" || token == "write" || token == "if" || token == "while" || token == "repeat" || token == ";" || token == "begin")
	{
		execStmt();
	}
	
	elsePt();
}
 


void Compiler::elsePt() // stage 2, production 4
{
	if (token == "else")
	{	
		string temp = popOperand();
		code("else", temp);
		nextToken();

		execStmt();
		code("post_if", popOperand());
	}
	else if (isNonKeyId(token) || token == "end" || token == "write" || token == "read" || token == "repeat" || token == "if" || token == "while" || token == "begin" || token == "until" || token == ";")
	{
		code("post_if", popOperand());
	}
	else
	{
		processError("illegal character");
	}
}



void Compiler::whileStmt() // stage 2, production 5
{
	if (token != "while")
	{
		processError("received " + token + " expected while");
	}
	
	code("while");
	nextToken();
	express();
	
	if (token != "do")
	{
		processError("received " + token + " expected do");
	}
	
	code("do", popOperand());
	nextToken();
	
	execStmt();
	
	string second = popOperand();
	string first = popOperand();
	
	code("post_while", second, first);
}



void Compiler::repeatStmt() // stage 2, production 6
{
	if (token != "repeat")
	{
		processError("received " + token + " expected repeat");
	}

	code("repeat");
	nextToken();

	if (!isNonKeyId(token) && token != "read" && token != "write" && token != "end" && token != "write" && token != "read" && token != "repeat" && token != "if" && token != "while" && token != "begin" && token != "until" && token != ";")
	{
		processError("error1");
	}
	
	execStmts();

	if (token != "until")
	{
		processError("error2" + token);
	}
	
	nextToken();
	express();
	string second = popOperand();
	string first = popOperand();
	
	code("until", second, first);
	
	if (token != ";")
	{
		processError("received " + token + " expected ;");
	}
}


void Compiler::nullStmt() // stage 2, production 7
{
	if (token != ";")
	{
		processError("received " + token + " expected ;");
	}
	nextToken();
}


void Compiler::express()	//stage 1 production 9
{
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger(token) && !isNonKeyId(token))
	{
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", non - keyword identifier or integer expected" + token);
	}

	term();

	if (token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">")
	{
		expresses();
	}
}


void Compiler::expresses()	//stage 1 production 10
{
	string first, second;

	if (token != "=" && token != "<>" && token != "<=" && token != ">=" && token != "<" && token != ">")
	{
		processError("\"=\", \"<>\", \"<=\", \">=\", \"<\", or \">\" expected");  
	}

	pushOperator(token);
	nextToken();

	//error checking here
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger(token) && !isNonKeyId(token))
	{
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	}

	term();

	second = popOperand();
	first = popOperand();

	code(popOperator(), second, first);

	if (token == "=" || token == "<>" || token == "<=" || token == ">=" || token == "<" || token == ">")
	{
		expresses();
	}
}


void Compiler::term()	//stage 1 production 11
{
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger(token) && !isNonKeyId(token))
	{
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	}

	factor();

	if (token == "-" || token == "+" || token == "or")
	{
	
		terms();
	}
}


void Compiler::terms() //stage 1 production 12
{
	string first, second;

	if (token != "+" && token != "-" && token != "or")
	{
		processError("\"+\", \"-\", or \"or\" expected");
	}

	pushOperator(token);
	nextToken();

	//error checks here
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger(token) && !isNonKeyId(token))
	{
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	}

	factor();
	second = popOperand();
	first = popOperand();

	code(popOperator(), second, first);

	//error checks here
	if (token == "+" || token == "-" || token == "or")
	{
		terms();
	}
}


void Compiler::factor()	//stage 1 production 13
{
	// FACTOR {'not','true','false','(','+','-',INTEGER,NON_KEY_ID}
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger(token) && !isNonKeyId(token))
	{
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", INTEGER, or NON_KEY_ID expected");
	}
	
	// PART 
	part();

	// FACTORS {'*','div','mod','and'}
	if (token == "*" || token == "div" || token == "mod" || token == "and")
	{
		factors();
	}

	//{'<>','=','<=','>=','<','>',')',';','-','+','or'}
	else if (isNonKeyId(token) || token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">" || token == ")" || token == ";" || token == "-" || token == "+" || token == "or" || token == "begin" || token == "do" || token == "then")
	{

	}
	
	else 
	{
		processError("expected '(', integer, or non_key_id" + token);
	}
}


void Compiler::factors()	//stage 1 production 14
{
	string first, second;

	if (token != "*" && token != "div" && token != "mod" && token != "and")
	{
	  processError("\"*\", \"div\", \"mod\", or \"and\" expected");
	}

	pushOperator(token);
	nextToken();
	
	if (token != "not" && token != "(" && !isInteger(token) && !isNonKeyId(token) && token != "+" && token != "-" && token != "true" && token != "false")
	{
		processError("expected '(', integer, or non-keyword id " + token);
	}

	part();

	second = popOperand();		
	first = popOperand();
	
	code(popOperator(), second, first);

	if (token == "*" || token == "div" || token == "mod" || token == "and")
	{
		factors();
	}
}


void Compiler::part()	//stage 1 production 15
{
	string x = "";
	if (token == "not")
	{
		nextToken();
		
		if (token == "(") 
		{
			nextToken();
			
			if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger(token) && !isNonKeyId(token))
			{
				processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
			}
			
			express();
			
			if (token != ")")
			{
				processError(") expected; found " + token);
			}
			
			nextToken();
			code("not", popOperand());
		}

		else if (isBoolean(token)) 
		{
			if (token == "true") 
			{
				pushOperand("false");
				nextToken();
			}
			else 
			{
				pushOperand("true");
				nextToken();
			}
		}

		else if (isNonKeyId(token)) 
		{
			code("not", token);
			nextToken();
		}
	}

	else if (token == "+")
	{
		nextToken();
		if (token == "(") 
		{
			nextToken();
			
			if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger(token) && !isNonKeyId(token))
			{
				processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
			}
			
			express();
			
			if (token != ")")
			{
				processError("expected ')'; found " + token);
			}
			
			nextToken();
		}
		
		else if (isInteger(token) || isNonKeyId(token)) 
		{
			pushOperand(token);
			nextToken();
		}

		else 
		{
			processError("expected '(', integer, or non-keyword id; found " + token);
		}
	}

	else if (token == "-")
	{
		nextToken();
		
		if (token == "(") 
		{
			nextToken();
			
			if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger(token) && !isNonKeyId(token))
			{
				processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
			}
			
			express();
			
			if (token != ")")
			{
				processError("expected ')'; found " + token);
			}
			
			nextToken();
			code("neg", popOperand());
		}
		
		else if (isInteger(token)) 
		{
			pushOperand("-" + token);
			nextToken();
		}
		
		else if (isNonKeyId(token)) 
		{
			code("neg", token);
			nextToken();
		}
	}

	else if (token == "(") 
	{
		nextToken();
		
		if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger(token) && !isNonKeyId(token))
		{
			processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
		}
		
		express();
		
		if (token != ")") 
		{
			processError(") expected; found " + token);
		}
		
		nextToken();
	}

	else if (isInteger(token) || isBoolean(token) || isNonKeyId(token)) 
	{
		pushOperand(token);
		nextToken();
	}

	else 
	{
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, boolean, or non - keyword identifier expected");
	}

}



void Compiler::constStmts() 
{ 

	string x, y, error;

	if (!isNonKeyId(token))
	{
		processError("non-keyword identifier expected");
	}
  
	
	if (token.back() == '_')
	{
		error = "illegal character to end a var";
		processError(error); 
	}
  

	for (unsigned int i = 0; i < token.size(); i++)
	{
		if (token[i] == '_' && token[i+1] == '_')
		{
			processError("'_' must be followed by a letter or number");
		}
	}
  
	x = token;
  
	if (nextToken() != "=")
	{
		error = "\"=\" expected";
		processError(error);
	}

	y = nextToken();
  
	if (!(isNonKeyId(y)) && y != "+" && y != "-" && y != "not" && !(isBoolean(y)) && !(isInteger(y)))
	{
		error = "token to right of \"=\" illegal";
		processError(error);
	}

	if (y == "+" || y == "-")
	{
		string temp = nextToken();
		if (!isInteger(temp))     
		{
			processError("integer expected after sign");
		}

		y = y + token;
	}

	if (y == "not")
	{
		if (!(isBoolean(nextToken())))  
		{
			error = "boolean expected after \"not\"";
			processError(error);
		}
      
		if (token == "true")
		{
			y = "false";
		}
		else
		{
			y = "true";
		}
	}

	if (nextToken() != ";")
	{
		processError("semicolon expected");
	}
	
	storeTypes temp = whichType(y);
	
	if (temp != INTEGER && temp != BOOLEAN)       
	{
		error = "data type of token on the right-hand side must be INTEGER or BOOLEAN";
		processError(error);
	}

	insert(x,whichType(y),CONSTANT,whichValue(y),YES,1);        
	x = nextToken();

	if (x != "begin" && x != "var" && !(isNonKeyId(x)))       
	{
		error = "non-keyword identifier, \"begin\", or \"var\" expected";               
		processError(error);
	}

	if (isNonKeyId(x))
	{
		constStmts();
	}
}



void Compiler::varStmts() //token should be NON_KEY_ID
{
	string x,y, error;
	if (!isNonKeyId(token))
	{
		processError("non-keyword identifier expected");
	}


	if (token.back() == '_')
	{
		error = "illegal character to end a var";
		processError(error); 
	}
   
	
	for (unsigned int i = 0; i < token.size(); i++)
	{
		if (token[i] == '_' && token[i+1] == '_')
		{
			processError("'_' must be followed by a letter or number");
		}
	}

	x = ids();

	if (token != ":")
	{
		error = "\":\" expected";
		processError(error);
	}

	if (nextToken() != "integer" && token != "boolean")	
	{
		error = "illegal type follows \":\"";
		processError(error);
	}

	y = token;

	if (nextToken() != ";")
	{
		processError("semicolon expected");
	}
	
	if (y == "integer")	
	{
		insert(x,INTEGER,VARIABLE,"1",YES,1);
	}
	else	
	{
		insert(x,BOOLEAN,VARIABLE,"1",YES,1);    
	}
	 
	if (nextToken() != "begin" && !(isNonKeyId(token)))   
	{
		error = "non-keyword identifier or \"begin\" expected";
		processError(error);
	}

	if (isNonKeyId(token))
	{
		varStmts();
	}
}


string Compiler::ids() //token should be NON_KEY_ID
{
	string temp,tempString;

	if (!(isNonKeyId(token)))    //use to catch when token is not a NON_KEY_ID
	{
		processError("non-keyword identifier expected");
	}

	tempString = token;
	temp = token;

	if (nextToken() == ",")
	{
		if (!(isNonKeyId(nextToken())))    //used to catch when nextToken() is not a NON_KEY_ID)
		{
			processError("non-keyword identifier expected");
		}
		
		tempString = temp + "," + ids();
	}
	
	return tempString;
}



bool Compiler::isKeyword(string s) const //determines if s is a keyword
{
	if (s == "program" || s == "const" || s == "var" || s == "integer" || s == "boolean" 
	|| s == "begin" || s == "end" || s == "true" || s == "false" || s == "not" || s == "mod" 
	|| s == "div" || s == "and" || s == "or" || s == "read" || s == "write" || s == "if"
	|| s == "then" || s == "else" || s == "while" || s == "do" || s == "repeat" || s == "until")
	{
		return true;
	}
	else 
	{
		return false;
	}
}



bool Compiler::isSpecialSymbol(char c) const //determines if c is a special symbol
{
	if (c == ':' || c == ',' || c == ';' || c == '=' || c == '+' || c == '-' || c == '.' || c == '*' || c == '<' || c == '>' || c == '(' || c == ')')
	{
		return true;
	}
	else 
	{
		return false;
	}
}



bool Compiler::isNonKeyId(string s) const //determines if s is a non_key_id
{
	if(!isKeyword(s) && !isInteger(s) && !isSpecialSymbol(s[0]))
	{
		return true;
	}
	
	return false;
}


bool Compiler::isInteger(string s) const //determines if s is an integer	
{
	try
	{
		stoi(s);
	}
	catch (invalid_argument&)
	{
		return false;
	}
	
	return true;
}

bool Compiler::isBoolean(string s) const // determines if s is a boolean
{
	if (s == "true" || s == "false")
	{
		return true;
	}
	else 
	{
		return false;
	}
}



bool Compiler::isLiteral(string s) const //determines if s is a literal
{
	//bool integer = isInteger(s);
	if (isInteger(s) || isBoolean(s) || s.front() == '+' || s.front() == '-')
	{    
		return true;
	}
	else
	{
		return false;
	}
}



string Compiler::genInternalName(storeTypes stype) const	//generate the internal name when called upon
{
	string internal;
	static int countI = 0, countB = 0, countU = 0;
	//use case statements "INTEGER", "BOOLEAN", "PROG"
	switch(stype)
	{
		case PROG_NAME:
		{
			internal = "P0";
			break;
		}
		
		case INTEGER:
		{
			internal = "I" + to_string(countI);
			++countI;
			break;
		}
		
		case BOOLEAN:
		{
			internal = "B" + to_string(countB);
			++countB;
			break;
		}
		case UNKNOWN:
		{
			internal = "U" + to_string(countU);
		}
	}
	
	return internal;
}

 
void Compiler::insert(string externalName,storeTypes inType, modes inMode, string inValue, allocation inAlloc, int inUnits)	//creates symbol table entries for each identifier in list of external names important to note that multiply inserted names are illegal
{
	string name;

	string::iterator itr = externalName.begin();		

	while (itr < externalName.end())
	{
		name = "";
		
		while (itr < externalName.end() && *itr != ',' )
		{
			name = name + *itr;
			++itr;
		}

		if (!name.empty())				
		{
			if (symbolTable.count(name) > 0)
			{
				processError("symbol " + name + " is multiply defined");
			}
			else if (isKeyword(name) && name != "true" && name != "false")
			{
				processError("illegal use of keyword");
			}
			else 
			{
				if (isupper(name[0]) || name == "true" || name == "false")
				{
					if (name == "true")
					{
						symbolTable.insert(pair<string, SymbolTableEntry>(name.substr(0, 15),
						SymbolTableEntry("TRUE", inType, inMode, inValue, inAlloc, inUnits)));
					}
					else if (name == "false")
					{
						symbolTable.insert(pair<string, SymbolTableEntry>(name.substr(0, 15),
						SymbolTableEntry("FALSE", inType, inMode, inValue, inAlloc, inUnits)));
					}
					else 
					{
						symbolTable.insert(pair<string, SymbolTableEntry>(name.substr(0, 15),
						SymbolTableEntry(name, inType, inMode, inValue, inAlloc, inUnits)));
					}
				}
				else
				{
					symbolTable.insert(pair<string, SymbolTableEntry>(name.substr(0, 15),
					SymbolTableEntry(genInternalName(inType), inType, inMode, inValue, inAlloc, inUnits)));
				}
			}
		}
		
		if (symbolTable.size() > 256)
		{
			processError("symbolTable overflow");
		}

		if (itr == externalName.end())
		{
			break;
		}
		else 
		{
			++itr;
		}
	}
}



storeTypes Compiler::whichType(string name)	
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



string Compiler::whichValue(string name) 
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
	{
		emitPrologue(operand1);
	}

	else if (op == "end")
	{
		emitEpilogue();
	}

	else if (op == "read")
	{
		emitReadCode(operand1, "");
	}

	else if (op == "write")
	{
		emitWriteCode(operand1, "");
	}

	else if (op == "+") 
	{
		emitAdditionCode(operand1, operand2);
	}

	else if (op == "-") 
	{
		emitSubtractionCode(operand1, operand2);
	}

	else if (op == "neg") 
	{
		emitNegationCode(operand1, op);
	}

	else if (op == "not")
	{
		emitNotCode(operand1, op);
	}

	else if (op == "*")
	{
		emitMultiplicationCode(operand1, operand2);
	}

	else if (op == "div")
	{
		emitDivisionCode(operand1, operand2);
	}
	
	else if (op == "mod")
	{
		emitModuloCode(operand1, operand2);
	}

	else if (op == "and")
	{
		emitAndCode(operand1, operand2);
	}
	
	else if (op == "or")
	{
		emitOrCode(operand1, operand2);
	}

	else if (op == "<")
	{
		emitLessThanCode(operand1, operand2);
	}
	
	else if (op == "<=")
	{
		emitLessThanOrEqualToCode(operand1, operand2);
	}

	else if (op == ">")
	{
		emitGreaterThanCode(operand1, operand2);
	}

	else if (op == ">=")
	{
		emitGreaterThanOrEqualToCode(operand1, operand2);
	}

	else if (op == "<>")
	{
		emitInequalityCode(operand1, operand2);
	}
	
	else if (op == "=")
	{
		emitEqualityCode(operand1, operand2);
	}
	
	else if (op == ":=")
	{
		emitAssignCode(operand1, operand2);
	}

	else if (op == "then")
	{
		emitThenCode(operand1);
	}

	else if (op == "else")
	{
		emitElseCode(operand1);
	}
	
	else if (op == "while")
	{
		emitWhileCode();
	}
	
	else if (op == "do")
	{
		emitDoCode(operand1);
	}
	else if (op == "repeat")
	{
		emitRepeatCode();
	}
	
	else if (op == "until")
	{
		emitUntilCode(operand1, operand2);
	}
	else if (op == "post_if")
	{
		emitPostIfCode(operand1);
	}
	
	else if (op == "post_while")
	{
		emitPostWhileCode(operand1, operand2);
	}
	else
	{
		processError("compiler error; function code called with illegal arguments" + op);
	}
}



void Compiler::emit(string label, string instruction, string operands, string comment)	//For formatting our emit statements in the objectFile 
{
	//Turn on left justification in objectFile 
	objectFile.setf(ios_base::left);
	//Output label in a field of width 8 
	objectFile << left << setw(8) << label;             
	//Output instruction in a field of width 8 
	objectFile << left << setw(8) << instruction;
	//Output the operands in a field of width 24     
	objectFile << left << setw(24) << operands;
	//Output the comment 
	objectFile << comment << endl;
}


void Compiler::emitPrologue(string progName, string operand2)	//Output identifying comments at beginning of objectFile and Output the %INCLUDE directives
{
	time_t now = time(0);
	char* time = ctime(&now);
	objectFile << "; Hoang Long Nguyen & Minh Ha Le" << right << setw(6) << "" << time;
	objectFile << "%INCLUDE \"Along32.inc\"\n" << "%INCLUDE \"Macros_Along.inc\"\n" << endl;
	emit("SECTION", ".text");
	emit("global", "_start", "", "; program " + progName.substr(0, 15));
	objectFile << endl;
	emit("_start:");
}



void Compiler::emitEpilogue(string operand1, string operand2)	//emits our epilogue to the ASM file and calls emitStorage()
{
	emit("","Exit", "{0}");
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


void Compiler::emitReadCode(string operand, string)
{
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


void Compiler::emitAssignCode(string operand1, string operand2) // op2 = op1
{	
	
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	
	if (symbolTable.at(operand1).getDataType() !=symbolTable.at(operand2).getDataType())
	{
		processError("incompatible types for operator ':='");
	}

	if (symbolTable.at(operand2).getMode() != VARIABLE)
	{
		processError("symbol on left-hand side of assignment must have a storage mode of VARIABLE");
	}

	if (operand1 == operand2)
	{
		return;
	}

	//if operand1 is not in the A register then
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName())
	{
		//emit code to load operand1 into the A register
		emit("","mov","eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);
	}
	//emit code to store the contents of that register into the memory location pointed to by operand2
	emit("","mov","[" + symbolTable.at(operand2).getInternalName() + "],eax", "; " + operand2 + " = AReg");
	//set the contentsOfAReg = operand2
	contentsOfAReg = symbolTable.at(operand2).getInternalName();

	if (isTemporary(operand1))
	{
		freeTemp();
	}

}



void Compiler::emitAdditionCode(string operand1, string operand2) // op2 + op1
{
	
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	if (symbolTable.at(operand1).getDataType() != INTEGER || symbolTable.at(operand2).getDataType() != INTEGER)
	{
		processError("binary '+' requires integer operands");
	}

	//if the A Register holds a temp not operand1 nor operand2
	if (symbolTable.at(operand1).getInternalName() != contentsOfAReg && symbolTable.at(operand2).getInternalName() != contentsOfAReg && isTemporary(contentsOfAReg))
	{
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		
		symbolTable.at(contentsOfAReg).setAlloc(YES);

		//deassign it
		contentsOfAReg = "";
	}

	if (symbolTable.at(operand1).getInternalName() != contentsOfAReg && symbolTable.at(operand2).getInternalName() != contentsOfAReg && !isTemporary(contentsOfAReg))
	{
		//deassign it
		contentsOfAReg = "";
	}
   
	if (symbolTable.at(operand1).getInternalName() != contentsOfAReg && symbolTable.at(operand2).getInternalName() != contentsOfAReg)		
	{
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);		//CHECK THIS
		
		// set A reg == operand 2 
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}
	
	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("", "add", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " + " + operand1);
	}
	else
	{
		//emit code to perform register-memory addition with operand 2
		emit("", "add", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " + " + operand2);
	}
	
	//deassign all temporaries involved in the addition and free those names for reuse
	if (isTemporary(operand1))
	{
		freeTemp();		
	}
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);

	
	pushOperand(contentsOfAReg);
}


void Compiler::emitSubtractionCode(string operand1, string operand2) // op2 - op1
{
	 // check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	// check that neither operand is empty
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	//if type of either operand is not integer
	if (symbolTable.at(operand1).getDataType() != INTEGER || symbolTable.at(operand2).getDataType() != INTEGER)
	{
		processError("illegal type. binary '-' requires integer operands");
	}

	//if the A Register holds a temp not operand2
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		//change the allocate entry for the temp in the symbol table to yes
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}

	//if the A register holds a non-temp not operand1 nor operand2
	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		contentsOfAReg = "";
	}

	if (contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		// A Reg == operand2
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	//emit code to perform register-memory subtraction with operand1
	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("", "sub", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " - " + operand1);
	}

	//deassign all temporaries involved in the addition and free those names for reuse
	if (isTemporary(operand1))
	{
		freeTemp();		
	}
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}



void Compiler::emitMultiplicationCode(string operand1, string operand2) // op2 * op1
{
	 // check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	 if (symbolTable.at(operand1).getDataType() != INTEGER || symbolTable.at(operand2).getDataType() != INTEGER)
	 {
		 processError("Illegal type");
	 }

	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		emit("","mov","[" + contentsOfAReg + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	// if the A register holds a non-temp not operand2 then deassign it
	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		contentsOfAReg = "";
	}
	
	
	if (symbolTable.at(operand1).getInternalName() != contentsOfAReg && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}
	

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("", "imul", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " * " + operand1);
	}
	
	else 
	{
		emit("", "imul", "dword [" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " * " + operand2);	
	}

	if (isTemporary(operand1))
	{
		freeTemp();
	}
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}



void Compiler::emitDivisionCode(string operand1, string operand2) // op2 / op1
{
	 // check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	//if type of either operand is not integer
	if (symbolTable.at(operand1).getDataType() != INTEGER || symbolTable.at(operand2).getDataType() != INTEGER)
	{
		processError("binary 'div' requires integer operands");
	}

	//if the A Register holds a temp not operand2
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to store that temp into memory
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}

	//if the A register holds a non-temp not operand2
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//deassign it
		contentsOfAReg = "";
	}

	if (symbolTable.at(operand2).getInternalName() != contentsOfAReg)
	{
		
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}
	
	emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");

	emit("", "idiv", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " div " + operand1);	


	if (isTemporary(operand1))
	{
		freeTemp();
	}
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}

void Compiler::emitModuloCode(string operand1, string operand2) // op2 % op1
{


	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	if (symbolTable.at(operand1).getDataType() != INTEGER || symbolTable.at(operand2).getDataType() != INTEGER)
	{
		processError("binary 'mod' requires integer operands");
	}

	//if the A Register holds a temp not operand2
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to store that temp into memory
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}

	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//deassign it
		contentsOfAReg = "";
	}

	//if operand2 is not in the A register
	if (symbolTable.at(operand2).getInternalName() != contentsOfAReg)
	{

		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}
		
	//emit code to extend sign of dividend from the A register to edx:eax
	emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");
	emit("", "idiv", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " div "  + operand1);	
	emit("", "xchg", "eax,edx", "; exchange quotient and remainder");

	if (isTemporary(operand1))
	{
		freeTemp();
	}
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);


	pushOperand(contentsOfAReg);
}


void Compiler::emitNegationCode(string operand1, string) // -op1
{
	 // check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}

	//if type of either operand is not boolean
	if (symbolTable.at(operand1).getDataType() != INTEGER)
	{
		//processError(illegal type)
		processError("illegal type");
	}
	//if the A Register holds a temp not operand1
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName())
	{
		//emit code to store that temp into memory
		emit("","mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		//change the allocate entry for the temp in the symbol table to yes
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}
	//if the A register holds a non-temp not operand1 then deassign it
	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName())
	{
		//deassign it
		contentsOfAReg = "";
	} 

	//if neither operand is in the A register then
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName())
	{
		//emit code to load operand1 into the A register
		emit("","mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);
		contentsOfAReg = symbolTable.at(operand1).getInternalName();
	}

	//emit code to perform register-memory NOT
	emit("","neg", "eax", "; AReg = -AReg");
	
	//deassign all temporaries involved in the and operation and free those names for reuse
	if (isTemporary(operand1))
	{
		freeTemp();
	}

	//A Register = next available temporary name and change type of its symbol table entry to INTEGER
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);

	//push the name of the result onto operandStk
	pushOperand(contentsOfAReg);
}



void Compiler::emitNotCode(string operand1, string) // !op1
{
	// check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}

	//if type of either operand is not boolean
	if (symbolTable.at(operand1).getDataType() != BOOLEAN)
	{
		//processError(illegal type)
		processError("illegal type");
	}
	//if the A Register holds a temp not operand1
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName())
	{
		//emit code to store that temp into memory
		emit("","mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		//change the allocate entry for the temp in the symbol table to yes
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}
	//if the A register holds a non-temp not operand1 then deassign it
	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName())
	{
		//deassign it
		contentsOfAReg = "";
	} 

	//if neither operand is in the A register then
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName())
	{
		//emit code to load operand1 into the A register
		emit("","mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);
		contentsOfAReg = symbolTable.at(operand1).getInternalName();
	}

	//emit code to perform register-memory NOT
	emit("","not", "eax", "; AReg = !AReg");
	
	//deassign all temporaries involved in the and operation and free those names for reuse
	if (isTemporary(operand1))
	{
		freeTemp();
	}

	//A Register = next available temporary name and change type of its symbol table entry to boolean
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);

	//push the name of the result onto operandStk
	pushOperand(contentsOfAReg);
}

void Compiler::emitAndCode(string operand1, string operand2) // op2 && op1
 {
	 // check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	//if type of either operand is not boolean
	if (symbolTable.at(operand1).getDataType() != BOOLEAN || symbolTable.at(operand2).getDataType() != BOOLEAN)
	{
		//processError(illegal type)
		processError("binary 'and' requires boolean operands");
	}
	//if the A Register holds a temp not operand1 nor operand2
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		emit("","mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}
	//if the A register holds a non-temp not operand1 nor operand2 then deassign it
	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//deassign it
		contentsOfAReg = "";
	} 

	//if neither operand is in the A register then
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to load operand2 into the A register
		emit("","mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	//emit code to perform register-memory and
	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","and", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " and " + operand1);
	}
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("","and", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " and " + operand2);
	}

	//deassign all temporaries involved in the and operation and free those names for reuse
	if (isTemporary(operand1))
	{
		freeTemp();
	}
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);

	//push the name of the result onto operandStk
	pushOperand(contentsOfAReg);
}


void Compiler::emitOrCode(string operand1, string operand2) // op2 || op1
{
	 // check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	if (symbolTable.at(operand1).getDataType() != BOOLEAN || symbolTable.at(operand2).getDataType() != BOOLEAN)
	{
		processError("illegal type");
	}
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to store that temp into memory
		emit("","mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}
	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//deassign it
		contentsOfAReg = "";
	} 

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		emit("","mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","or", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " or " + operand1);
	}
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("","or", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " or " + operand2);
	}

	if (isTemporary(operand1))
	{
		freeTemp();
	}
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}


void Compiler::emitEqualityCode(string operand1, string operand2) // op2 == op1
{
	// check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	//if types of operands are not the same
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
	{
		processError("incompatible types");
	}

	//if the A Register holds a temp not operand1 nor operand2 then 
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		emit("","mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}

	//if the A register holds a non-temp not operand2 nor operand1 then deassign it
	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//deassign it
		contentsOfAReg = "";
	}

	// if neither operand is in the A register then
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to load operand2 into the A register
		emit("","mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}
	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	}
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("","cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
	}

	//emit code to jump if equal to the next available Ln (call getLabel)
	string newLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","je", "." + newLabel, "; if " + operand2 + " = " + operand1 + " then jump to set eax to TRUE");
	}
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("","je", "." + newLabel, "; if " + operand2 + " = " + operand1 + " then jump to set eax to TRUE");
	}

	//emit code to load FALSE into the A register
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	//insert FALSE in symbol table with value 0 and external name false
	if (symbolTable.count("false") == 0)
	{
		insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}

	string secondLabel = getLabel();
	//emit code to perform an unconditional jump to the next label (call getLabel should be L(n+1))
	emit("","jmp","." + secondLabel, "; unconditionally jump");
	
	emit("." + newLabel + ":");
	//emit code to load TRUE into A register
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	//insert TRUE in symbol table with value -1 and external name true
	if (symbolTable.count("true") == 0)
	{
		insert("true", BOOLEAN, CONSTANT, "-1", YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}

	//emit code to label the next instruction with the second acquired label L(n+1)
	emit("." + secondLabel + ":");

	//deassign all temporaries involved and free those names for reuse
	if (isTemporary(operand1))
	{
		freeTemp();
	}
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	//A Register = next available temporary name and change type of its symbol table entry to boolean
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);

	//push the name of the result onto operandStk
	pushOperand(contentsOfAReg);
}


void Compiler::emitInequalityCode(string operand1, string operand2) // op2 != op1
{
	// check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
	    processError("reference to undefined symbol " + operand1);
	}
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}
	
	//if types of operands are not the same
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
	{
		processError("incompatible types for operator '<>'");
	}

	//if the A Register holds a temp not operand1 nor operand2 then 
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName()) 
	{
		//emit code to store that temp into memory
		emit("","mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		//change the allocate entry for it in the symbol table to yes
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}

	//if the A register holds a non-temp not operand2 nor operand1 then deassign it
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//deassign it 
		contentsOfAReg = "";
	}

	// if neither operand is in the A register then
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName()) 
	{
		////emit code to load operand2 into the A register
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	//emit code to perform a register-memory compare
	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	}
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
	}

	//emit code to jump if equal to the next available Ln (call getLabel)
	string label_1 = getLabel(), label_2 = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("", "jne", "." + label_1, "; if " + operand2 + " <> " + operand1 + " then jump to set eax to TRUE");
	}
	else
	{
		emit("", "jne", "." + label_1, "; if " + operand2 + " <> " + operand1 + " then jump to set eax to TRUE");
	}

	//emit code to load FALSE into the A register
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	//insert FALSE in symbol table with value 0 and external name false
	if (symbolTable.count("false") == 0) 
	{
		insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	
	//emit code to perform an unconditional jump to the next label (call getLabel should be L(n+1))
	emit("", "jmp", "." + label_2, "; unconditionally jump");

	emit("." + label_1 + ":");
	//emit code to load TRUE into A register
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	//insert TRUE in symbol table with value -1 and external name true
	if (symbolTable.count("true") == 0) 
	{
		insert("true", BOOLEAN, CONSTANT, "-1", YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}

	//emit code to label the next instruction with the second acquired label L(n+1)
	emit("." + label_2 + ":");

	//deassign all temporaries involved and free those names for reuse
	if (isTemporary(operand1))
	{
		freeTemp();
	}
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	//A Register = next available temporary name and change type of its symbol table entry to boolean
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);

	//push the name of the result onto operandStk
	pushOperand(contentsOfAReg);
 }

void Compiler::emitLessThanCode(string operand1, string operand2) // op2 < op1
{
	 // check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	//if types of operands are not the same
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
	{
		processError("incompatible types");
	}

	//if the A Register holds a temp not operand1 nor operand2 then 
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to store that temp into memory
		emit("","mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		//change the allocate entry for it in the symbol table to yes
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}

	//if the A register holds a non-temp not operand2 nor operand1 then deassign it
	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//deassign it
		contentsOfAReg = "";
	}

	// if neither operand is in the A register then
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to load operand2 into the A register
		emit("","mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	//emit code to perform a register-memory compare
	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	}
	
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		emit("","cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	}

	//emit code to jump if NOT equal to the next available Ln (call getLabel)
	string newLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","jl", "." + newLabel, "; if " + operand2 + " < " + operand1 + " then jump to set eax to TRUE");
	}
	
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("","jl", "." + newLabel, "; if " + operand2 + " < " + operand1 + " then jump to set eax to TRUE");
	}

	//emit code to load FALSE into the A register
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	//insert FALSE in symbol table with value 0 and external name false
	if (symbolTable.count("false") == 0)
	{
		insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
	}

	string secondLabel = getLabel();
	//emit code to perform an unconditional jump to the next label (call getLabel should be L(n+1))
	emit("","jmp","." + secondLabel, "; unconditionally jump");
	
	emit("." + newLabel + ":");
	
	//emit code to load TRUE into A register
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	//insert TRUE in symbol table with value -1 and external name true
	if (symbolTable.count("true") == 0)
	{
		insert("true", BOOLEAN, CONSTANT, "-1", YES, 1);
	}

	//emit code to label the next instruction with the second acquired label L(n+1)
	emit("." + secondLabel + ":");

	//deassign all temporaries involved and free those names for reuse
	if (isTemporary(operand1))
	{
		freeTemp();
	}
	
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}


void Compiler::emitLessThanOrEqualToCode(string operand1, string operand2) // op2 <= op1
{
	 // check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	//if types of operands are not the same
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
	{
		processError("incompatible types");
	}

	//if the A Register holds a temp not operand1 nor operand2 then 
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to store that temp into memory
		emit("","mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}

	//if the A register holds a non-temp not operand2 nor operand1 then deassign it
	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//deassign it
		contentsOfAReg = "";
	}

	// if neither operand is in the A register then
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to load operand2 into the A register
		emit("","mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}
	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	}
	
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("","cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
	}

	string newLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","jle", "." + newLabel, "; if " + operand2 + " <= " + operand1 + " then jump to set eax to TRUE");
	}
	
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("","jle", "." + newLabel, "; if " + operand2 + " <= " + operand1 + " then jump to set eax to TRUE");
	}

	//emit code to load FALSE into the A register
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0)
	{
		insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
	}

	string secondLabel = getLabel();
	//emit code to perform an unconditional jump to the next label (call getLabel should be L(n+1))
	emit("","jmp","." + secondLabel, "; unconditionally jump");
	
	emit("." + newLabel + ":");
	//emit code to load TRUE into A register
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	//insert TRUE in symbol table with value -1 and external name true
	if (symbolTable.count("true") == 0)
	{
		insert("true", BOOLEAN, CONSTANT, "-1", YES, 1);
	}

	//emit code to label the next instruction with the second acquired label L(n+1)
	emit("." + secondLabel + ":");

	//deassign all temporaries involved and free those names for reuse
	if (isTemporary(operand1))
	{
		freeTemp();
	}
	
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);

	//push the name of the result onto operandStk
	pushOperand(contentsOfAReg);
 }


void Compiler::emitGreaterThanCode(string operand1, string operand2) // op2 > op1
{
	// check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	//if types of operands are not the same
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
	{
		processError("incompatible types");
	}

	//if the A Register holds a temp not operand1 nor operand2 then 
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to store that temp into memory
		emit("","mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		//change the allocate entry for it in the symbol table to yes
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}

	//if the A register holds a non-temp not operand2 nor operand1 then deassign it
	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//deassign it
		contentsOfAReg = "";
	}

	// if neither operand is in the A register then
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to load operand2 into the A register
		emit("","mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	//emit code to perform a register-memory compare
	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	}
	
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("","cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
	}

	//emit code to jump if NOT equal to the next available Ln (call getLabel)
	string newLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","jg", "." + newLabel, "; if " + operand2 + " > " + operand1 + " then jump to set eax to TRUE");
	}
	
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("","jg", "." + newLabel, "; if " + operand1 + " > " + operand2 + " then jump to set eax to TRUE");
	}

	//emit code to load FALSE into the A register
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	//insert FALSE in symbol table with value 0 and external name false
	if (symbolTable.count("false") == 0)
	{
		insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
	}

	string secondLabel = getLabel();
	//emit code to perform an unconditional jump to the next label (call getLabel should be L(n+1))
	emit("","jmp","." + secondLabel, "; unconditionally jump");
	
	emit("." + newLabel + ":");
	//emit code to load TRUE into A register
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	//insert TRUE in symbol table with value -1 and external name true
	if (symbolTable.count("true") == 0)
	{
		insert("true", BOOLEAN, CONSTANT, "-1", YES, 1);
	}

	//emit code to label the next instruction with the second acquired label L(n+1)
	emit("." + secondLabel + ":");

	//deassign all temporaries involved and free those names for reuse
	if (isTemporary(operand1))
	{
		freeTemp();
	}
	
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	//A Register = next available temporary name and change type of its symbol table entry to boolean
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);

	//push the name of the result onto operandStk
	pushOperand(contentsOfAReg);
}


void Compiler::emitGreaterThanOrEqualToCode(string operand1, string operand2) // op2 >= op1
{
	 // check that neither operand is empty
	if (symbolTable.count(operand1) == 0)
	{
		processError("reference to undefined symbol " + operand1);
	}
	
	else if (symbolTable.count(operand2) == 0)
	{
		processError("reference to undefined symbol " + operand2);
	}

	 //if types of operands are not the same
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
	{
		processError("incompatible types");
	}

	//if the A Register holds a temp not operand1 nor operand2 then 
	if (isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to store that temp into memory
		emit("","mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		//change the allocate entry for it in the symbol table to yes
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		//deassign it
		contentsOfAReg = "";
	}

	//if the A register holds a non-temp not operand2 nor operand1 then deassign it
	if (!isTemporary(contentsOfAReg) && contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//deassign it
		contentsOfAReg = "";
	}

	// if neither operand is in the A register then
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		//emit code to load operand2 into the A register
		emit("","mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	//emit code to perform a register-memory compare
	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
	}
	
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("","cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
	}

	//emit code to jump if NOT equal to the next available Ln (call getLabel)
	string newLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName())
	{
		emit("","jge", "." + newLabel, "; if " + operand2 + " >= " + operand1 + " then jump to set eax to TRUE");
	}
	
	else if (contentsOfAReg == symbolTable.at(operand1).getInternalName())
	{
		emit("","jge", "." + newLabel, "; if " + operand2 + " >= " + operand1 + " then jump to set eax to TRUE");
	}

	//emit code to load FALSE into the A register
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	//insert FALSE in symbol table with value 0 and external name false
	if (symbolTable.count("false") == 0)
	{
		insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}

	string secondLabel = getLabel();
	//emit code to perform an unconditional jump to the next label (call getLabel should be L(n+1))
	emit("","jmp","." + secondLabel, "; unconditionally jump");
	
	emit("." + newLabel + ":");
	//emit code to load TRUE into A register
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	//insert TRUE in symbol table with value -1 and external name true
	if (symbolTable.count("true") == 0)
	{
		insert("true", BOOLEAN, CONSTANT, "-1", YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}

	//emit code to label the next instruction with the second acquired label L(n+1)
	emit("." + secondLabel + ":");

	//deassign all temporaries involved and free those names for reuse
	if (isTemporary(operand1))
	{
		freeTemp();
	}
	
	if (isTemporary(operand2))
	{
		freeTemp();
	}

	//A Register = next available temporary name and change type of its symbol table entry to boolean
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);

	//push the name of the result onto operandStk
	pushOperand(contentsOfAReg);
}


// Emit functions for Stage 2
// emitThenCode
void Compiler::emitThenCode(string operand1, string)
{

	string tempLabel;

	if (symbolTable.at(operand1).getDataType() != BOOLEAN)
	{
		processError("the predicate of \"if\" must be of type BOOLEAN");
	}

	//assign next label to tempLabel
	tempLabel = getLabel();

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName())
	{
		emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);	// instruction to move operand1 to the A register
	}
	
	emit("", "cmp", "eax,0", "; compare eax to 0");	// instruction to compare the A register to zero (false)
	emit("", "je", "." + tempLabel, "; if " + operand1 + " is false then jump to end of if");	// code to branch to tempLabel if the compare indicates equality

	// push tempLabel onto operandStk
	pushOperand(tempLabel);

	// if operand1 is a temp
	if (isTemporary(operand1))
	{
		// free operand's name for reuse (is this right?)
		freeTemp();
	}

	// deassign operands from all registers (is this right?)
	contentsOfAReg = "";
}



// emitElseCode
void Compiler::emitElseCode(string operand1, string)
{
	string tempLabel;

	// assign next label to tempLabel
	tempLabel = getLabel();

	// emit instruction to branch unconditionally to tempLabel
	emit("", "jmp", "." + tempLabel, "; jump to end if");

	// emit instruction to label this point of object code with the argument operand1
	emit ("." + operand1 + ":", "", "", "; else");	

	// push tempLabel onto operandStk
	pushOperand(tempLabel);

	// deassign operands from all registers
	contentsOfAReg = "";
}

// emitPostIfCode
void Compiler::emitPostIfCode(string operand1, string)
{
	//emit instruction to label this point of object code with the argument operand1
	emit ("." + operand1 + ":", "", "", "; end if");	

 	//deassign operands from all registers
	contentsOfAReg = "";
}


// emitWhileCode
void Compiler::emitWhileCode(string, string)
{
	string tempLabel;

	// assign next label to tempLabel
	tempLabel = getLabel();

	// emit instruction to label this point of object code as tempLabel
	emit ("." + tempLabel + ":", "", "", "; while");	
	
	// push tempLabel onto operandStk
	pushOperand(tempLabel);

	// deassign operands from all registers
	contentsOfAReg = "";
}


// emitDoCode
void Compiler::emitDoCode(string operand1, string)
{
	string tempLabel;

	// if the type of operand1 is not boolean
	if (symbolTable.at(operand1).getDataType() != BOOLEAN)
	{
		processError("while predicate must be of type boolean");
	}

	// assign next label to tempLabel
	tempLabel = getLabel();

	// if operand1 is not in the A register then
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName())
	{
		emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);	// instruction to move operand1 to the A register
	}
	
	emit("", "cmp", "eax,0", "; compare eax to 0");	// instruction to compare the A register to zero (false)
	emit("", "je", "." + tempLabel, "; if " + operand1 + " is false then jump to end while");	// code to branch to tempLabel if the compare indicates equality

	// push tempLabel onto operandStk
	pushOperand(tempLabel);

	// if operand1 is a temp
	if (isTemporary(operand1))
	{
		// free operand's name for reuse (is this right?)
		freeTemp();
	}

	// deassign operands from all registers (is this right?)
	contentsOfAReg = "";
}

// emitPostWhileCode
void Compiler::emitPostWhileCode(string operand1, string operand2)
{
	// emit instruction which branches unconditionally to the beginning of the loop, i.e., to the value of operand2
	emit("", "jmp", "." + operand2, "; end while");

	// emit instruction which labels this point of the object code with the argument operand1
	emit ("." + operand1 + ":", "", "", "");	

	// deassign operands from all registers (is this right?)
	contentsOfAReg = "";
}

// emitRepeatCode
void Compiler::emitRepeatCode(string, string)
{

	string tempLabel;

	// assign next label to tempLabel
	tempLabel = getLabel();

	// emit instruction to label this point in the object code with the value of tempLabel
	emit("." + tempLabel + ":", "", "", "; repeat");

	// push tempLabel onto operandStk
	pushOperand(tempLabel);

	// deassign operands from all registers (is this right?)
	contentsOfAReg = "";
}

// emitUntilCode
void Compiler::emitUntilCode(string operand1, string operand2)
{

	if (symbolTable.at(operand1).getDataType() != BOOLEAN)
	{
		processError("the predicate of \"if\" must be of type BOOLEAN");
	}

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName())
	{
		// instruction to move operand1 to the A register
		emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + symbolTable.at(operand1).getInternalName());
		contentsOfAReg = symbolTable.at(operand1).getInternalName(); // reassign
	}
	
	// instruction to compare the A register to zero (false)
	emit("", "cmp", "eax,0", "; compare eax to 0");

	// code to branch to tempLabel if the compare indicates equality
	emit("", "je", "." + operand2, "; until " + operand1 + " is true");

	// if operand1 is a temp
	if (isTemporary(operand1))
	{
		// free operand's name for reuse (is this right?)
		freeTemp();
	}

	// deassign operands from all registers (is this right?)
	contentsOfAReg = "";
}


string Compiler::nextToken()        
{
	token = "";	
	while(token == "")
	{
		if (ch == '{')
		{
			while (nextChar() != END_OF_FILE && ch != '}')
			{
				
			}
			
			if (ch == END_OF_FILE)
			{
				processError("unexpected end of file: '}' expected");
			}
			
			else 
			{
				nextChar();
			}
		}
		
		else if (ch == '}')
		{
			processError("'}' cannot begin token");
		}
		
		else if (isspace(ch))
		{
			nextChar();
		}
		
		else if (isSpecialSymbol(ch))
		{
			token = ch;
			nextChar();
    
			if (token == ":" && ch == '=') 
			{
				token += ch;
				nextChar();
			}
			
			if ((token == "<" && ch == '=') || (token == "<" && ch == '>') || (token == ">" && ch == '=')) 
			{
				token += ch;
				nextChar();
			}
		}
		
		else if (islower(ch))
		{
			token = ch;

			while((nextChar() == '_' || islower(ch) || isupper(ch) || isdigit(ch)) && ch != END_OF_FILE)
			{
				token = token + ch;
			}

			if (ch == END_OF_FILE)
			{
				processError("unexpected end of file");
			}
		}
		
		else if (isdigit(ch))
		{
			token = ch;
			
			while (nextChar() != END_OF_FILE && isdigit(ch))
			{
				token += ch;
			}

			if (ch == END_OF_FILE)
			{
				processError("unexpected end of file");
			}
		}

		else if (ch == END_OF_FILE)
		{
			token = ch;
		}
		
		else
		{
			processError("illegal symbol");
		}
	}
	
	token = token.substr(0,15);

	return token;
}



char Compiler::nextChar()  
{ 
	// read in next character   
	sourceFile.get(ch);
  
	static char prev = '\n';

	if (sourceFile.eof())
	{
		ch = END_OF_FILE;
	}
	
	else 
	{
	
		if (prev == '\n')
		{
		lineNo += 1;
		listingFile << right << setw(5) << lineNo << '|';    
		}
		
		listingFile << ch;  
	}
	
	prev = ch;
  
	return ch;
}



void Compiler::pushOperator(string name) //push name onto operatorStk
{
 	operatorStk.push(name);
}


void Compiler::pushOperand(string name) //push name onto operandStk
 
{
 	if (symbolTable.count(name) == 0)																					
	{
		if (isInteger(name) || name == "true" || name == "false")
		{
 		  insert(name, whichType(name), CONSTANT, whichValue(name), YES, 1);		//insert symbol table entry, call whichType to determine the data type of the literal
		 																															
		}
	}
	
	operandStk.push(name);	
}


string Compiler::popOperator() //pop name from operatorStk
{
	string top;
 
	if (!operatorStk.empty())
	{
		top = operatorStk.top();
		operatorStk.pop();
	}
 
	else
	{
		processError("compiler error; operator stack underflow");
	}
 
	return top;
}



string Compiler::popOperand() //pop name from operandStk
{
	string top;
 
	if (!operandStk.empty())
	{
		top = operandStk.top();
		operandStk.pop();
	}
 
	else
	{
		processError("compiler error; operand stack underflow");
	}
 
	return top;
}


void Compiler::freeTemp()
{
	currentTempNo--;
	if (currentTempNo < -1)
	{
		processError("compiler error, currentTempNo should be >= –1");
	}
}


string Compiler::getTemp()
{
	string temp;
	currentTempNo++;

	temp = "T" + to_string(currentTempNo);

	if (currentTempNo > maxTempNo)
	{
		insert(temp, UNKNOWN, VARIABLE, "1", NO, 1);
		symbolTable.at(temp).setInternalName(temp);
		maxTempNo++;
	}
	
	return temp;
}


string Compiler::getLabel()
{
	string label;
	static int count = 0;
	
	label = "L" + to_string(count);
	
	count++;

	return label;
}

bool Compiler::isTemporary(string s) const 
{
	if (s[0] == 'T') 
	{
		return true;
	}
	return false;
}