#include "tokeniser.hxx"
#include <iostream>
#define debug_msg(x) std::cout << (x) << std::endl;

tokeniser::tokeniser(std::shared_ptr<std::ifstream> fh): filehandle(fh){
}

bool tokeniser::TryGetNextToken(tokens& token) throw (fileReadFailureException){
if (!filehandle->is_open()) {
	debug_msg("could not open file")
	fileReadFailureException ex;
	throw ex;
}
bool foundtoken = false;
while (filehandle->good() && !foundtoken){
	char ch;
	ch = filehandle->get();
	switch(ch){
		 case ' ':	
			token = tokens::space;
			foundtoken = true;
			break;
		case '\t':
			token = tokens::tab;
			foundtoken = true;
			break;
		case '\n':
			token = tokens::linefeed;
			foundtoken = true;
			break;
		default:
			//eat char
			break;
		}
}
if (filehandle->bad()) {
	debug_msg("filehandle failed")
	fileReadFailureException ex;
	throw ex;
}
//false if eof as failure has thrown by this point
return filehandle->good();
}
