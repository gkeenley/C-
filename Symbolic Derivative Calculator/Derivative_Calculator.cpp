#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <string>
#include <vector>
#include <cctype>
#include <stdlib.h>
#include <ctime>

using namespace std;

// SYMBOLIC DERIVATIVE CALCULATOR - Overview

// This code solves problems of the form of problems 1-3 in the Final Project, by accepting user input.
// The code prompts the user to enter the function x'(t), as well as several parameters such as the 
// calculation interval and the exact solution x(t) (for error calculations).

// Step 1) Code parses input string and computes derivative of corresponding function using standard
// laws of calculus. Outputs these derivatives to the standard output.
// Step 2) Code takes the resulting strings representing the n derivatives of the input function and
// again parses them so that they can be used as functions of x and t.
// Step 3) Code uses computed derivatives to solve Problem 1 of Final Project via the Taylor Method.


//////////


// Define structures used when parsing strings.


// STRUCTURE Terms Sum Difference

// Structure that holds information about terms in a sum or difference. It can contain an arbitrary 
// number of terms because when differentiating a sum/difference, we differentiate each and then
// add/subtract them.
struct terms_sum_difference{
	vector<string> terms; // vector holding the added/subtracted terms
	vector<int> indices; // vector holding the indices of the + or - terms comprising the sum/difference
	vector<char> symbols; // vector holding the character (+ or -) at each of the indices
};

// STRUCTURE Terms Product Quotient

// Structure that holds information about terms in a product or quotient. It can contain up to two terms
// only, because when differentiating a product/quotient, we consider only two terms at a time, and if
// either is itself a product/quotient, we differentiate that again in a further step. For example, for
// f(t)=x(t)y(t)z(t), we consider the two terms x(t) and y(t)z(t), and then further consider y(t) and
// z(t) in a subsequent step.
struct terms_product_quotient{
	vector<string> terms; // vector holding the multiplied/divided terms
	int index; // int holding the index of the * or / term
	char symbol; // character (* or /) at the index
};


//////////


// FUNCTION PROTOTYPES

// ORGANIZATION FUNCTIONS - Functions used for parsing strings into terms of a sum, difference, product, or quotient.
terms_sum_difference break_into_plus_minus(string str);
terms_product_quotient break_into_mult_divide(string str);
void print_terms(terms_sum_difference);
void print_terms(terms_product_quotient);

// DERIVATIVE FUNCTIONS - Functions used for computing derivatives of functions dfined by input strings.
string differentiate(string);
string output_derivative(string str, vector<string> & vector, int num_terms);
string product_rule(string, string, vector<string> &, int);
string quotient_rule(string, string, vector<string> &, int);

// CLEAN UP FUNCTIONS - Functions used to format functions defined by output (differentiated) strings to make them more easily readable.
void clear_duplicate_symbols(string &);
void clear_head_or_tail_symbols(string &);
void clear_outer_brackets(string &);
bool outer_brackets(string);
void clear_ones(string &);
void clear_zeros(string &);
void clear_unnecessary_brackets(string &);
void clean_up(string &);

// TAYLOR METHOD FUNCTIONS - Functions used for implementing Taylor Method. Similar to those used in Problem 1 of Final Project.
void reverse_array(double, int);
void taylor(double x1(double, double, string, vector<string>), double, double, double, int, int, char*, vector<string>, int);
int solve_problem(vector<string>, int);
double evaluate(string, vector<string>, double, double);
double dx(double, double, string, vector<string>);

///////////


// START ORGANIZATION FUNCTIONS


// FUNCTION - Print terms
// When a sum or difference is broken into terms, this function prints the terms, as well as the indices of + or - characters, and the characters themselves.
void print_terms(terms_sum_difference term){
	cout<<"Terms1: ";
	for (vector<string>::iterator itr=term.terms.begin(); itr!=term.terms.end(); ++itr)
		cout<<*itr<<" ";
	cout<<endl;
	for (vector<int>::iterator itr=term.indices.begin(); itr!=term.indices.end(); ++itr)
		cout<<*itr<<" ";
	cout<<endl;
	for (vector<char>::iterator itr=term.symbols.begin(); itr!=term.symbols.end(); ++itr)
		cout<<*itr<<" ";
	cout<<endl;
}


// FUNCTION - Print terms
// When a product or quotient is broken into terms, this function prints the terms, as well as the index of the * or / character, and the character itself.
void print_terms(terms_product_quotient term){
	if (term.terms.size()==1)
		cout<<"Terms2: "<<term.terms[0]<<" "<<term.index<<" "<<term.symbol<<endl;
	else
		cout<<"Terms2: "<<term.terms[0]<<" "<<term.terms[1]<<" "<<term.index<<" "<<term.symbol<<endl;
}


// FUNCTION - Break Into Plus Minus

// If function is a sum or difference, break into summed/subtracted terms, returning a structure of type terms_sum_difference.

terms_sum_difference break_into_plus_minus(string str){ // (a+b)*c+d*h*l-e/(f-g)
	terms_sum_difference plus_minus_terms; // Create 'terms' object which will hold the added or subtracted terms in the string.
	int brackets=0; // If 'brackets' is 0, all brackets are closed.
	for (int i=0; i<str.length(); i++){ // For each element of string
		if (str[i]=='(')
			brackets++; // Increment 'brackets' to tell us there will be a ) bracket later in the string.
		if (str[i]==')')
			brackets--; // Decrement 'brackets'. If this sets 'brackets' to zero, string so far is a +- term.
		if (((brackets==0)&&(str[i]=='+'))||((brackets==0)&&(str[i]=='-')&&(i>0))){ // If there is a + not enclosed within brackets...
			if (plus_minus_terms.indices.empty()) // If this is the first such term...
				plus_minus_terms.terms.push_back(str.substr(0,i));
			else // If this is not the first such term...
				plus_minus_terms.terms.push_back(str.substr((plus_minus_terms.indices.back()+1),(i-plus_minus_terms.indices.back()-1)));
			plus_minus_terms.indices.push_back(i);
			plus_minus_terms.symbols.push_back(str[i]);
		}
		if (i==(str.length()-1)){ // If we've reached the end of the string...
			if (plus_minus_terms.indices.empty()) // If there were no + or -...
				plus_minus_terms.terms.push_back(str.substr(0,str.length())); // 012345
			else // If there has been at least one + or - symbol...
				plus_minus_terms.terms.push_back(str.substr((plus_minus_terms.indices.back()+1),str.length()-plus_minus_terms.indices.back()-1));
		}
	}
	//print_terms(plus_minus_terms);
	return plus_minus_terms;
}


// FUNCTION - Break Into Mult Divide

// If function is a product or quotient, break into multiplied/divided terms, returning a structure of type terms_mult/divide.

terms_product_quotient break_into_mult_divide(string str){
	terms_product_quotient mult_divide_terms; // Create 'terms' object which will hold the added or subtracted terms in the string.
	int brackets=0; // If 'brackets' is 0, all brackets are closed.
	for (int i=0; i<str.length(); i++){ // For each element of string
		if (str[i]=='(')
			brackets++; // Increment 'brackets' to tell us there will be a ) bracket later in the string.
		if (str[i]==')')
			brackets--; // Decrement 'brackets'. If this sets 'brackets' to zero, string so far is a +- term.
		if ((brackets==0)&&((str[i]=='*')||(str[i]=='/'))){ // If there is a + not enclosed within brackets...
			mult_divide_terms.terms.push_back(str.substr(0,i));
			mult_divide_terms.terms.push_back(str.substr(i+1,str.length()-i-1));
			mult_divide_terms.index=i;
			mult_divide_terms.symbol=str[i];
			//print_terms(mult_divide_terms);
			return mult_divide_terms;
		}
	}
	mult_divide_terms.terms.push_back(str.substr(0,str.length()));
	mult_divide_terms.index=0;
	mult_divide_terms.symbol='\0';
	//print_terms(mult_divide_terms);
	return mult_divide_terms;
}

// END ORGANIZATION FUNCTIONS



// START DERIVATIVE FUNCTIONS

// FUNCTION - Differentiate
// Return derivative of string.
string differentiate(string str, vector<string> & vector, int num_taylor_terms){

	// STEP 1 - If original function is entirely enclosed by brackets, remove these brackets.
	if (outer_brackets(str)) // If there are brackets enclosing a +- term, such as in (a+b)*c
		return '('+differentiate(str.substr(1,str.length()-2), vector, num_taylor_terms)+')'; // Differentiate contents of brackets and return, enclosed in brackets

	// STEP 2 - Test if function is a sum or difference, and if so differentiate and add/subtract these derivatives.
	terms_sum_difference plus_minus=break_into_plus_minus(str); // Break into summed/subtracted terms.

	if (plus_minus.indices.empty()==false){ // If string is a sum or difference...
		string d_plus_minus=differentiate(plus_minus.terms[0], vector, num_taylor_terms); // Differentiate first term.
		for (int i=0; i<plus_minus.indices.size(); i++){ // For each subsequent term...
			d_plus_minus=d_plus_minus+plus_minus.symbols[i]+differentiate(plus_minus.terms[i+1], vector, num_taylor_terms); // Differentiate and add to output string of derivatives.
		}
		return d_plus_minus; // Return completely differentiated string.
	}

	// If we have reached this point, str was not a sum/difference, so we treat it as a product/quotient.

	// STEP 3 - Test if function is a product or quotient, and if so use product/quotient rule to differentiate.
	terms_product_quotient mult_divide=break_into_mult_divide(str); // 012345

	if (mult_divide.index!=0){ // If string is a product or quotient...
		if (mult_divide.symbol=='*') // If it is a product...
			return product_rule(str.substr(0,mult_divide.index), str.substr(mult_divide.index+1, str.length()-mult_divide.index-1), vector, num_taylor_terms); // Differentiate using product rule.
		else // If it is a quotient...
			return quotient_rule(str.substr(0,mult_divide.index), str.substr(mult_divide.index+1, str.length()-mult_divide.index-1), vector, num_taylor_terms); // Differentiate using quotient rule.
	}

	// If we have reached this point, str was neither a sum/difference nor a product/quotient, so we treat it as an individual elementary function.

	// STEP 4 - Differentiate individual elementary function.

	if (str[0]=='-') // If function is negative...
		return "-"+differentiate(str.substr(1,str.length()-1), vector, num_taylor_terms);

	// CASE 1: str is a constant
	if (::atof(str.c_str())!=0.0){
		return "0"; // Its derivative is zero (we will remove any terms by which it is multiplied later)
	}

	// CASE 2: str has an exponent
	// CASE 2a: it is exp(____)
	if (str.substr(0,3)=="exp"){
		string y=str.substr(3,str.length()-3); // Take argument (as in exp(argument))
		return differentiate(y, vector, num_taylor_terms)+"*exp"+y; // Derivative is argument' * exp(argument)
	}
	// CASE 2b: it is any other kind of exponential

	if (str.substr(0,3)=="pow"){ // pow(we,erty)  pow(t,2)
		int index_of_comma; // Index of string at which comma appears.
		for (int i=4; i<(str.length()-2); i++){ // For each element of argument to 'pow'...
			if (str[i]==',') // If character is a comma...
				index_of_comma=i; // Save this index.
		}
		string argument=str.substr(4,(index_of_comma-4)); // argument of exponent.
		string exponent=str.substr(index_of_comma+1,(str.length()-index_of_comma-2)); // exponent
		if (::atof(exponent.c_str())!=0.0){ // If exponent is a number...
			if (stod(exponent)==2) // ...and if it is 2...
				return "2*"+differentiate(argument, vector, num_taylor_terms)+"*"+argument;
			return exponent+"*"+differentiate(argument, vector, num_taylor_terms)+"*pow("+argument+","+to_string(stod(exponent)-1)+")";
			//return x+'^'+'('+y+"-1)*("+y+'*'+differentiate(x)+'+'+x+"*log("+x+"))*"+differentiate(y);
		}
	}

	// CASE 3: function is x or a derivative of x.
	for (int i=0; i<(num_taylor_terms); i++){
		if (str==vector[i]) // If function is i-th derivative of x...
			return vector[i+1]; // Return next higher derivative.
	}

	// CASE 4: function is t
	if (str=="t")
		return "1";

	// CASE 5: function is trigonometric.
	if (str.substr(0,3)=="sin"){
		string temp=str;
		temp.replace(0,3,"cos");
		return differentiate(temp.substr(3,temp.length()-3), vector, num_taylor_terms)+"*"+temp;
	}

	if (str.substr(0,3)=="cos"){
		string temp=str;
		temp.replace(0,3,"sin");
		return "(-"+differentiate(temp.substr(3,temp.length()-3), vector, num_taylor_terms)+"*"+temp+")";
	}

	if (str.substr(0,3)=="tan"){
		string temp=str;
		temp.replace(0,3,"cos");
		return differentiate(temp.substr(3,temp.length()-3), vector, num_taylor_terms)+"*(1/tan("+temp.substr(3,temp.length()-3)+"))^2";
	}

	// If there are +- terms within the */ term, re-iterate derivative process on them, such as (a+b)*c
	for (int i=0; i<str.length(); i++){
		if ((str[i]=='+')||(str[i]=='-')){
			differentiate(str, vector, num_taylor_terms);
		}
	}

	return "error";
}


// FUNCTION - Product Rule

string product_rule(string term_one, string term_two, vector<string> & vector, int num_taylor_terms){
	string d1=differentiate(term_one, vector, num_taylor_terms);
	string d2=differentiate(term_two, vector, num_taylor_terms);
	if (((d1=="0")&&(d2=="0"))||((d1=="(0)")&&(d2=="0"))||((d1=="0")&&(d2=="(0)"))||((d1=="(0)")&&(d2=="(0)")))
		return "0";
	if ((d1=="0")||(d1=="(0)"))
		return term_one+'*'+d2;
	if ((d2=="0")||(d2=="(0)"))
		return d1+'*'+term_two;
	return '('+d1+'*'+term_two+'+'+term_one+'*'+d2+')';
}


// FUNCTION - Quotient rule

string quotient_rule(string term_one, string term_two, vector<string> & vector, int num_taylor_terms){
	string d1=differentiate(term_one, vector, num_taylor_terms);
	string d2=differentiate(term_two, vector, num_taylor_terms);
	if (((d1=="0")&&(d2=="0"))||((d1=="(0)")&&(d2=="0"))||((d1=="0")&&(d2=="(0)"))||((d1=="(0)")&&(d2=="(0)")))
		return "0";
	if ((d1=="0")||(d1=="(0)"))
		return '-'+term_one+'*'+d2+'/'+term_two+"^2";
	if ((d2=="0")||(d2=="(0)"))
		return d1+'*'+term_two+"/"+term_two+"^2";
	return '('+d1+'*'+term_two+'-'+term_one+'*'+d2+")/("+term_two+"^2)";
}


// FUNCTION - Output Derivative

// Performs Derivative and Clean Up operations on an input string.

string output_derivative(string str, vector<string> & vector, int num_terms){
	string derivative=differentiate(str, vector, num_terms);
	clean_up(derivative);
	return derivative;
}

// END DERIVATIVE FUNCTIONS



// START CLEANUP FUNCTIONS


// FUNCTION - Clear Duplicate Symbols

// Scans through output (differentiated) string and removes any obsolete +-*/ symbols. For example,
// if we differentiate x+4-t we get x'-1, but this will initially appear as x'+-1 because d(4)/dt=0,
// and this character will be removed. Thus, we remove the + character.
void clear_duplicate_symbols(string & str){
	for (int i=str.length()-1; i>=0; i--){ // For each character in string (starting from the end so that removing a character does not affect the indices of characters subsequently under consideration)...
		if (((str[i]=='+')||(str[i]=='-')||(str[i]=='*')||(str[i]=='/'))&&((str[i-1]=='+')||(str[i-1]=='-')||(str[i-1]=='*')||(str[i-1]=='/'))) // If there are two consecutive +-*/ characters...
			str.erase(i-1,1); // Erase the first, because it is either adding/subtracting 0, or multiplying or dividing by 1.
	}
}


// FUNCTION - Clear Head Or Tail Symbols

// If there is a +-*/ character at the beginning or end of an output (differentiated) string, except
// in the case of a - character at the beginning, remove it.
void clear_head_or_tail_symbols(string & str){
	if ((str[0]=='+')||(str[0]=='*')||(str[0]=='/')) // If first character is +, -, or *...
		str.erase(0,1); // Remove it.
	if ((str[str.length()-1]=='+')||(str[str.length()-1]=='*')||(str[str.length()-1]=='/')) // If last character is +, -, *, or /...
		str.erase(str.length()-1,1); // Remove it.
}


// FUNCTION - Clear Outer Brackets

// If output (differentiated) string is entirely enclosed in brackets, remove these brackets.
void clear_outer_brackets(string & str){
	if (outer_brackets(str)){ // If string is entirely enclosed by brackets...
		str.erase(0,1); // Erase first bracket
		str.erase(str.length()-1,1); // And erase last bracket
	}
}


// FUNCTION - Outer Brackets

// Returns true if a string is entirely enclosed by brackets.
bool outer_brackets(string str){
	if (str[0]=='('){ // If first character is '('
		int brackets=1; // If 'brackets' is zero, brackets are closed at this point in the string.
	    for (int i=1; i<str.length(); i++){ // For each character in string after initial bracket...
			if (str[i]=='(')
				brackets++;
			if (str[i]==')')
				brackets--;
			if ((brackets==0)&&(i<(str.length()-1))) // If brackets are closed and we have not yet reached end of string (ie. no outer brackets)...
				return false; // Return false because there are no outer brackets.
	    }
	return true; // If we have reached this point, there are outer brackets because the initial bracket was not closed in the middle of the string.
	}
	return false;
}


// FUNCTION - Clear Ones

// If there are any instances of a function multipled or divided by 1, remove these.
void clear_ones(string & str){
	if (str.length()>4){
		if (str.substr(0,2)=="1*")
			str.erase(0,2);
		if (str.substr(0,4)=="(1)*")
			str.erase(0,4);
		if (str.substr(str.length()-2,2)=="*1")
			str.erase(str.length()-2,2);
		if (str.substr(str.length()-2,4)=="*(1)")
			str.erase(str.length()-2,4);
		for (int i=0; i<str.length(); i++){
			if (str.substr(i,3)=="+1*")
				str.erase(i+1,2);
			if (str.substr(i,5)=="+(1)*")
				str.erase(i+1,4);
			if (str.substr(i,3)=="-1*")
				str.erase(i+1,2);
			if (str.substr(i,5)=="-(1)*")
				str.erase(i+1,4);
			if (str.substr(i,3)=="(1*")
				str.erase(i+1,2);
			if (str.substr(i,3)=="*1+")
				str.erase(i,2);
			if (str.substr(i,5)=="*(1)+")
				str.erase(i,4);
			if (str.substr(i,3)=="*1-")
				str.erase(i,2);
			if (str.substr(i,5)=="*(1)-")
				str.erase(i,4);
			if (str.substr(i,3)=="*1)")
				str.erase(i,2);
			if (str.substr(i,3)=="*1*")
				str.erase(i,2);
			if (str.substr(i,5)=="*(1)*")
				str.erase(i,4);
			if (str.substr(i,3)=="/1*")
				str.erase(i,2);
			if (str.substr(i,5)=="/(1)*")
				str.erase(i,4);
			if (str.substr(i,3)=="/1-")
				str.erase(i,2);
			if (str.substr(i,5)=="/(1)-")
				str.erase(i,4);
			if (str.substr(i,3)=="/1)")
				str.erase(i,2);
			if (str.substr(i,3)=="/(1))")
				str.erase(i,4);
			if (str.substr(i,5)=="*(1))")
				str.erase(i,4);
			if (str.substr(i,5)=="((1)*")
				str.erase(i+1,4);
		}
	}
}


// FUNCTION - Clear Zeros

// If there are any instances of 0 added or removed from a function, remove them.

void clear_zeros(string & str){
	if (str.substr(0,2)=="0+")
		str.erase(0,2);
	if (str.substr(0,4)=="(0)+")
		str.erase(0,4);
	if (str.substr(0,2)=="0-")
		str.erase(0,2);
	if (str.substr(0,2)=="0-")
		str.erase(0,4);
	for (int i=0; i<str.length(); i++){
		if (str.substr(i,3)=="+0+")
			str.erase(i,2);
		if (str.substr(i,5)=="+(0)+")
			str.erase(i,4);
		if (str.substr(i,3)=="+0-")
			str.erase(i,2);
		if (str.substr(i,5)=="+(0)-")
			str.erase(i,4);
		if (str.substr(i,3)=="-0+")
			str.erase(i,2);
		if (str.substr(i,5)=="-(0)+")
			str.erase(i,4);
		if (str.substr(i,3)=="-0-")
			str.erase(i,2);
		if (str.substr(i,5)=="-(0)-")
			str.erase(i,4);
		if (str.substr(i,3)=="(0+")
			str.erase(i+1,2);
		if (str.substr(i,3)=="(0-")
			str.erase(i+1,2);
		if (str.substr(i,3)=="+0)")
			str.erase(i,2);
		if (str.substr(i,3)=="-0)")
			str.erase(i,2);
	}
}


// FUNCTION - Clear Unnecessary Brackets

// Clears any brackets enclosing terms that are part of a sum or difference. For example, (a+b)-(sin(x)*t)+(c/d) would become a+b-sin(x)*t+c/d.
void clear_unnecessary_brackets(string & str){
	terms_sum_difference sum_difference_terms=break_into_plus_minus(str); // If str is a sum or difference, break into terms.
	for (int i=0; i<sum_difference_terms.indices.size(); i++){ // For each + or - symbol in sum/difference...
		if (sum_difference_terms.terms[i][0]=='('){ // If term to the left of symbol is enclosed in brackets...
			str.erase(sum_difference_terms.indices[i]-sum_difference_terms.terms[i].length(),1); // Remove first bracket of this term.
			str.erase(sum_difference_terms.indices[i]-1,1); // Remove second bracket of this term.
			if (i==(sum_difference_terms.indices.size()-1)){ // If this is the last symbol in sequence of sum/difference symbols...
				str.erase(sum_difference_terms.indices[i]+1,1); // Erase first bracket in term to the right of this symbol.
				str.erase(sum_difference_terms.indices[i]+sum_difference_terms.terms[i].length(),1); // Erase second bracket in this term.
			}
		}
	}
}


// FUNCTION - Clean Up

// Takes an output (differentiated) string and puts it into a more legible form.
void clean_up(string & str){
	clear_duplicate_symbols(str);
	clear_head_or_tail_symbols(str);
	clear_outer_brackets(str);
	clear_ones(str);
	clear_zeros(str);
}

// END OF CLEANUP FUNCTIONS



// START OF TAYLOR METHOD FUNCTIONS


// FUNCTION Evaluate

// Evaluates an expression given by str, as a function of x and t, as well as other derivatives of x, held in 'derivatives'.
double evaluate(string str, vector<string> derivatives, double x, double t){

	// STEP 1 - If original function is entirely enclosed by brackets, remove these brackets.
	if (outer_brackets(str)) // If there are brackets enclosing a +- term, such as in (a+b)*c
		return evaluate(str.substr(1,str.length()-2), derivatives, x, t);

	// STEP 2 - Test if function is a sum or difference, and if so differentiate and add/subtract these derivatives.
	terms_sum_difference plus_minus=break_into_plus_minus(str); // Break into summed/subtracted terms.

	if (plus_minus.indices.empty()==false){ // If string is a sum or difference...
		double eval_plus_minus=evaluate(plus_minus.terms[0], derivatives, x, t);
		for (int i=0; i<plus_minus.indices.size(); i++){
			if (plus_minus.symbols[i]=='+')
				eval_plus_minus=eval_plus_minus+evaluate(plus_minus.terms[i+1], derivatives, x, t);
			else
				eval_plus_minus=eval_plus_minus-evaluate(plus_minus.terms[i+1], derivatives, x, t);
		}
		return eval_plus_minus;
	}

	// If we have reached this point, str was not a sum/difference, so we treat it as a product/quotient.

	// STEP 3 - Test if function is a product or quotient, and if so use product/quotient rule to differentiate.
	terms_product_quotient mult_divide=break_into_mult_divide(str); // Break into multiplied/divided terms.

	if (mult_divide.index!=0){ // If string is a product or quotient...
		if (mult_divide.symbol=='*') // If it is a product...
			return evaluate(mult_divide.terms[0], derivatives, x, t)*evaluate(mult_divide.terms[1], derivatives, x, t);
		else // If it is a quotient...
			return evaluate(mult_divide.terms[0], derivatives, x, t)/evaluate(mult_divide.terms[1], derivatives, x, t);
	}

	// If we have reached this point, str was neither a sum/difference nor a product/quotient, so we treat it as an individual elementary function.

	// STEP 4 - Evaluate individual elementary function.

	if (str[0]=='-') // If function is negative...
		return (-1)*evaluate(str.substr(1,str.length()-1), derivatives, x, t);
	
	// CASE 1: str is a constant
	if (::atof(str.c_str())!=0.0){
		return stod(str); // Its derivative is zero (we will remove any terms by which it is multiplied later)
	}

	// CASE 2: str has an exponent
	// CASE 2a: it is exp(____)
	if (str.substr(0,3)=="exp"){
		string y=str.substr(3,str.length()-3); // Take argument (as in exp(argument))
		return exp(evaluate(y, derivatives, x, t)); // Derivative is argument' * exp(argument)
	}
	// CASE 2b: it is any other kind of exponential

	if (str.substr(0,3)=="pow"){ // pow(we,erty)  pow(t,2)
		int index_of_comma; // Index of string at which comma appears.
		for (int i=4; i<(str.length()-2); i++){ // For each element of argument to 'pow'...
			if (str[i]==',') // If character is a comma...
				index_of_comma=i; // Save this index.
		}
		
		return pow(evaluate(str.substr(4,(index_of_comma-4)), derivatives, x, t), evaluate(str.substr(index_of_comma+1,(str.length()-index_of_comma-2)), derivatives, x, t));
	}

	// CASE 3: function is x or a derivative of x.
	if (str=="x")
		return x;

	if (str=="x\'")
		return evaluate(derivatives[0], derivatives, x, t);

	if (str=="x\'\'")
		return evaluate(derivatives[1], derivatives, x, t);

	if (str=="x\'\'\'")
		return evaluate(derivatives[2], derivatives, x, t);

	if (str=="x\'\'\'\'")
		return evaluate(derivatives[3], derivatives, x, t);

	// CASE 4: function is t
	if (str=="t")
		return t;

	// CASE 5: function is trigonometric.
	if (str.substr(0,3)=="sin"){ // sin(234)
		return sin(evaluate(str.substr(3, str.length()-3), derivatives, x, t));
	}

	if (str.substr(0,3)=="cos"){
		return cos(evaluate(str.substr(3, str.length()-3), derivatives, x, t));
	}

	if (str.substr(0,3)=="tan"){
		return tan(evaluate(str.substr(3, str.length()-3), derivatives, x, t));
	}

	// If there are +- terms within the */ term, re-iterate derivative process on them, such as (a+b)*c
	for (int i=0; i<str.length(); i++){
		if ((str[i]=='+')||(str[i]=='-')){
			return evaluate(str, derivatives, x, t);
		}
	}
	return 0;
}


// FUNCTION - dx

// Function handle passed to Taylor function that calls Evaluate on input string
double dx(double t, double x, string str, vector<string> symbolic_derivatives){
	return evaluate(str, symbolic_derivatives, x, t);
}


// FUNCTION - Reverse Array

// Function that reverses elements of array. Used to output results in order when doing backward Taylor method.
  void reverse_array(double ar[], int size)
  {
    int i;
    double temp[size];
    for (i=0; i<size; i++)
    {
      temp[size-i-1]=ar[i];
    }
    for (i=0; i<size; i++)
    {
      ar[i]=temp[i];    
    }
  }


// FUNCTION - Taylor

// Implements Taylor Method
void taylor(double x1(double, double, string, vector<string>), double t, double x, double h, int n, int a_or_b, char* fout, vector<string> symbolic_derivatives, int number_of_terms)
  {
    // Set up input/output
    ofstream file(fout); // Create output stream for output file in which we will save results.
    cout.setf(ios::left); // Left justify output
    cout.setf(ios::showpoint); // Show decimal point

    // Declare variables for holding data
    double x_1, x_2, x_3, x_4, x_5; // Variables to hold computed derivative values at each iteration.
    double t_out[n+1], exact_out[n+1], x_out[n+1], error_out[n+1]; // Arrays for holding computed data.

    // Row headers
    cout<<"\n";
    cout.width(7); cout<<"t ";
    cout.width(19); cout<<"Exact ";
    cout.width(19); cout<<"Taylor ";
    cout.width(19); cout<<"Error";
    cout<<"\n";

    // Initial values
    t_out[0]=t;
    exact_out[0]=dx(t, x, symbolic_derivatives[number_of_terms], symbolic_derivatives);
    x_out[0]=x;
    error_out[0]=fabs(dx(t, x, symbolic_derivatives[number_of_terms], symbolic_derivatives) - x);
    file << t << " " << dx(t, x, symbolic_derivatives[number_of_terms], symbolic_derivatives) << " " << x << " " << fabs(dx(t, x, symbolic_derivatives[number_of_terms], symbolic_derivatives) - x) << " ";
    file << "\n";

    // Perform iterations of Taylor method
    for (int i = 1; i <= n; i++)
    {
    	vector<double> derivatives;
      // Compute derivatives
    	for (int j=0; j<number_of_terms; j++)
    		derivatives.push_back(dx(t,x, symbolic_derivatives[j], symbolic_derivatives));

      int k=number_of_terms;
      double p=derivatives[number_of_terms-1]*h/number_of_terms;
      while(k>=2){
      	p=(p+derivatives[k-2])*h/(k-1);
        k=k-1;
      }	
      if (a_or_b==1) // If performing forward Taylor method...
      {
        // increment
        x += p;
        t+=h;
      } else // If performing backward Taylor method...
      {
        // decrement
        x -= p;
        t-=h;
      }
      // Compute and save next set of values
      t_out[i]=t;
      exact_out[i]=dx(t, x, symbolic_derivatives[number_of_terms], symbolic_derivatives);
      x_out[i]=x;
      error_out[i]=fabs(dx(t, x, symbolic_derivatives[number_of_terms], symbolic_derivatives) - x);
      file << t << " " << dx(t, x, symbolic_derivatives[number_of_terms], symbolic_derivatives) << " " << x << " " << fabs(dx(t, x, symbolic_derivatives[number_of_terms], symbolic_derivatives) - x) << " ";
      file << "\n";
    }

    if (a_or_b==2) // If performing backward Taylor method...
    {
      // Reverse all arrays
      reverse_array(t_out,sizeof(t_out)/sizeof(*t_out));
      reverse_array(exact_out,sizeof(exact_out)/sizeof(*exact_out));
      reverse_array(x_out,sizeof(x_out)/sizeof(*x_out));
      reverse_array(error_out,sizeof(error_out)/sizeof(*error_out));
    }

    // Display computed values.
    for (int i = 0; i <= n; i++)
    {
      if ((i%50)==0) // If t=1.00, 1.10, etc
      {
        cout << fixed;
        cout << setprecision(2);
        cout.width(6); cout << ((abs(t_out[i]) < 0.0005)? 0.000: t_out[i])<<" ";
        cout << fixed;
        cout << setprecision(13);
        cout.width(18); cout<<exact_out[i]<<" ";
        cout.width(18); cout<<x_out[i]<<" ";
        cout.width(18); cout<<error_out[i];
        cout<<"\n";
      }
    }
    return;
  }


// FUNCTION - Solve Problem

// Solves Problem from Final Project, now using symbolic derivatives rather than user-defined derivatives.
int solve_problem(vector<string> derivatives, int number_of_terms, double h, double a, double b, double xa, int forward_backward)
{
	int start_s=clock();

    // Define constants
    double t;
    if (forward_backward==1)
    	t=a;
    else
    	t=b;
    int n = (b - a) / h;

    // Execute Taylor Method
    taylor(dx, t, xa, h, n, forward_backward, "solve_problem.dat", derivatives, number_of_terms);
    int stop_s=clock();
    cout<<endl<<"runtime: "<<(stop_s-start_s)/double(CLOCKS_PER_SEC)*1000<<" ms"<<endl;
    return 0;
}

// END OF TAYLOR METHOD FUNCTIONS


//////////


int main()
{
	// Define variables
	string function, exact, xa; // Function that we will receive as input from user, specifying x'(x,t).
	int number_of_terms, forward_backward; // Number of terms desired in Taylor series expansion, also from input from user.
	double h, a, b;
	vector<string> vector_of_derivatives; // Vector of x, x', x'', etc terms
	vector<string> symbolic_derivatives; // Vector of symbolic expressions for evaluated derivatives

	// Gather user input.
	cout<<endl<<"For all input, please use syntax that c++ can read, such as pow(x,2) rather than x^2."<<endl<<endl;
	cout<<endl<<"Please enter function to differentiate:"<<endl<<"(For Problem 1, enter x+pow(x,2))"<<endl<<"(For Problem 2, enter exp(t)*x)"<<endl<<endl;
	cin>>function;
	cout<<endl<<"Please specify the number of terms in the taylor series expansion:"<<endl<<endl;
	cin>>number_of_terms;
	//cout<<endl<<"Please enter the exact solution x(t):"<<endl<<"(For Problem 1, enter exp(t)/(16-exp(t))"<<endl<<"(For Problem 2, enter exp(exp(t)-exp(2)))"<<endl<<endl;
	//cin>>exact;
	//cout<<endl<<"Please enter the width h of subnintervals:"<<endl<<endl;
	//cin>>h;
	//cout<<endl<<"Please enter the lefthand boundary a of the interval:"<<endl<<"(For Problem 1, enter 1.00)"<<endl<<"(For Problem 2, enter 0)"<<endl<<endl;
	//cin>>a;
	//cout<<endl<<"Please enter the righthand boundary b of the interval:"<<endl<<"(For Problem 1, enter 2.77)"<<endl<<"(For Problem 2, enter 2)"<<endl<<endl;
	//cin>>b;
	//cout<<endl<<"Please enter the initial condition x(a):"<<endl<<"(For Problem 1, enter exp(1)/(16-exp(1)))"<<endl<<"(For Problem 2, enter 1)"<<endl<<endl;
	//cin>>xa;
	//cout<<endl<<"Please enter 1 if the initial condition is given for the lefthand boundary, and 2 if it is the righthand:"<<endl<<"(For Problem 1, enter 1)"<<endl<<"(For Problem 2, enter 2)"<<endl<<endl;
	//cin>>forward_backward;

	// Save terms x, x', x'', etc in vector that we will pass by reference when differentiating input function.
	vector_of_derivatives.push_back("x");
	for (int i=1; i<=number_of_terms; i++){ // For each derivative...
		vector_of_derivatives.push_back("x"); // Derivative terms begins with x.
		for (int j=1; j<=i; j++) // And for each order of derivative...
			vector_of_derivatives[i]=vector_of_derivatives[i]+"\'"; // Add an apostrophe.
	}

	// Output computed derivatives.
	symbolic_derivatives.push_back(function); // Save original x' function in derivatives vector.
	cout<<endl<<"Derivatives are:"<<endl<<endl<<"x' = "<<function<<endl<<endl;
	for (int i=1; i<number_of_terms; i++){ // For each computed derivative...
		function=output_derivative(function, vector_of_derivatives, number_of_terms); // Compute derivative...
		symbolic_derivatives.push_back(function); // ...and add to derivatives vector.
		cout<<vector_of_derivatives[i+1]<<" = "<<function<<endl<<endl;
	}

	symbolic_derivatives.push_back(exact); // Append to end of derivatives vector.

	// Now that we have gathered and computed derivatives, execute problem 1.
	//solve_problem(symbolic_derivatives, number_of_terms, h, a, b, evaluate(xa, vector_of_derivatives, 0, 0), forward_backward);

	cout<<endl;
}