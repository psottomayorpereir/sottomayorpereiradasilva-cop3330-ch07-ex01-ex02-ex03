#include "std_lib_facilities.h"

//declare global variables here
//number token
const char number = '8';
//quit token
const char quit   = 'q';
//print token
const char print  = ';';
//name token
const char name   = 'a';
//token to let the user change the value of a variable
const char let    = 'L';
//const token
const char con    = 'C';
//let keyword
const string declkey = "let";
//const keyword
const string constkey = "const";
const string prompt  = "> ";
//= sign declaration to indicate a result will be entered
const string result  = "= ";

class Token {
public:
    //type of token
    char kind;
    //numbers
    double value;
    //names
    string name;
    Token(char ch): kind(ch), value(0)   { }
    Token(char ch, double val): kind(ch), value(val) { }
    Token(char ch, string n): kind(ch), name(n)    { }
};

class Token_stream {
public: 
    //token stream to read user input
    Token_stream();
    //get a token
    Token get();
    //put a token back
    void putback(Token t);
    //discard tokens including c
    void ignore(char c);
private:
    //check if there is a token in the buffer
    bool full;
    Token buffer;
};

Token_stream::Token_stream()
:full(false), buffer(0) //the buffer has no token
{
}

void Token_stream::putback(Token t)
{
    //display error message
    if (full) error ("trying to put back into a full buffer");
    //copy token to buffer
    buffer = t;
    //update buffer to full
    full = true;
}

Token Token_stream::get()
{
    //read user input and build a token
    //check if there is a token ready
    if (full) {
        full=false;
        return buffer;
    }  
    //declare variables here
    char ch;
    //read user input without white spaces
    cin >> ch;
    //check all cases
    switch (ch) {
    case quit:
    case print:
    case '(':
    case ')':
    case '+':
    case '-':
    case '*':
    case '/': 
    case '%':
    case '=':
        return Token(ch);
    case '.':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
        //put number back into the input stream
        cin.putback(ch);
        double val;
        //read float number
        cin >> val;
        return Token(number,val);
    }
    default:
        if (isalpha(ch)) {
            string s;
            s += ch;
            while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch=='_')) s+=ch;
            cin.putback(ch);
            if (s == declkey) return Token(let);
            if (s == constkey) return Token(con);
            return Token(name,s);
        }
        error("Bad token");
    }
}

void Token_stream::ignore(char c)
{
    //check buffer
    if (full && c==buffer.kind) {
        full = false;
        return;
    }
    full = false;

    //get input:
    char ch = 0;
    while (cin>>ch)
        if (ch==c) return;
}

Token_stream ts;        // provides get() and putback() 

class Variable {
public:
    string name;
    double value;
	bool var;
    Variable (string n, double v, bool va = true) :name(n), value(v), var(va) { }
};

vector<Variable> var_table;

double get_value(string s)
{
    //get the value of the variable names
    for (int i = 0; i<var_table.size(); ++i)
        if (var_table[i].name == s) return var_table[i].value;
    error("trying to get undefined variable ", s);
}

void set_value(string s, double d)
{
    //set the value of the variable to d
    for (int i = 0; i<var_table.size(); ++i)
        if (var_table[i].name == s) {
			if (var_table[i].var==false) error(s," is a constant");
            var_table[i].value = d;
            return;
        }
    error("trying to set undefined variable ", s);
}

bool is_declared(string var)
    //check if the variable already in the table
{
    for (int i = 0; i<var_table.size(); ++i)
        if (var_table[i].name == var) return true;
    return false;
}

double define_name(string s, double val, bool var=true)
    //add the truple to the table
{
    if (is_declared(s)) error(s," was declared twice");
    var_table.push_back(Variable(s,val,var));
    return val;
}

double expression();

double primary()
{
    Token t = ts.get();
    switch (t.kind) {
    case '(':
        {
            double d = expression();
            t = ts.get();
            if (t.kind != ')') error("')' expected");
            return d;
        }
    case number:    
        return t.value;
    case name:
		{
			Token next = ts.get();
			if (next.kind == '=') {	
                //deal with name = expression
				double d = expression();
				set_value(t.name,d);
				return d;
			}
			else {
                //not ok, so return the value
				ts.putback(next);
                //return the value of the variable
				return get_value(t.name);
			}
		}
    case '-':
        return - primary();
    case '+':
        return primary();
    default:
        error("primary expected");
    }
}

double term()
{
    double left = primary();
    //get next token from stream
    Token t = ts.get();

    while(true) {
        switch (t.kind) {
        case '*':
            left *= primary();
            t = ts.get();
            break;
        case '/':
            {    
                double d = primary();
                if (d == 0) error("divide by zero");
                left /= d; 
                t = ts.get();
                break;
            }
        case '%':
            {    
                int i1 = narrow_cast<int>(left);
                int i2 = narrow_cast<int>(term());
                if (i2 == 0) error("%: divide by zero");
                left = i1%i2; 
                t = ts.get();
                break;
            }
        default: 
            //put back into stream
            ts.putback(t);
            return left;
        }
    }
}

double expression()
{
    //read input
    double left = term();
    //get next token from stream
    Token t = ts.get();

    while(true) {    
        switch(t.kind) {
        case '+':
            //perform operation
            left += term();
            t = ts.get();
            break;
        case '-':
            //perform operation
            left -= term();
            t = ts.get();
            break;
        default: 
            //put back into stream
            ts.putback(t);
            //return answer
            return left;
        }
    }
}

double declaration(Token k)
    //deal with = expression
    //set a variable 'name' to 'expression'
{
    Token t = ts.get();
    if (t.kind != name) error ("name expected in declaration");
    string var_name = t.name;

    Token t2 = ts.get();
    if (t2.kind != '=') error("= missing in declaration of ", var_name);

    double d = expression();
    define_name(var_name,d,k.kind==let);
    return d;
}

double statement()
{
    Token t = ts.get();
    switch (t.kind) {
    case let:
	case con:
        return declaration(t.kind);
    default:
        ts.putback(t);
        return expression();
    }
}

void clean_up_mess()
{ 
    ts.ignore(print);
}

void calculate()
{
    while (cin)
      try {
        cout << prompt;
        Token t = ts.get();
        while (t.kind == print) t=ts.get();
        if (t.kind == quit) return;
        ts.putback(t);
        cout << result << statement() << endl;
    }
    catch (exception& e) {
        //error message
        cerr << e.what() << endl;
        clean_up_mess();
    }
}

int main()
try {
    //set pi and e so the user cannot change its values:
    define_name("pi", 3.1415926535,false);
    define_name("e", 2.7182818284,false);
    calculate();
    keep_window_open();
    return 0;
}
catch (exception& e) {
    cerr << e.what() << endl;
    keep_window_open("~~");
    return 1;
}
catch (...) {
    cerr << "exception \n";
    keep_window_open("~~");
    return 2;
}