#ifndef parser_hxx
#define parser_hxx

#include <memory>
#include <exception>

#include "tokeniser.hxx"
#include "instruction.hxx"
#include "APInt.hxx"

class syntaxErrorException: public std::exception{};

class parser {
	public:
		parser(std::shared_ptr<tokeniser> tokeniser);
		~parser() = default;
		
		bool TryGetNextInstruction(std::shared_ptr<instruction>& instruction) throw (syntaxErrorException);
	private:
		std::shared_ptr<tokeniser> m_tokeniser;
		tokens GetNextToken();
		std::shared_ptr<stackInstruction> parseStackInstructions() throw (syntaxErrorException);
		std::shared_ptr<arithmeticInstruction> parseArithmeticInstructions() throw (syntaxErrorException);
		std::shared_ptr<heapInstruction> parseHeapInstructions() throw (syntaxErrorException);
		std::shared_ptr<controlInstruction> parseControlInstructions() throw (syntaxErrorException);
		std::shared_ptr<ioInstruction> parseIOInstructions() throw (syntaxErrorException);
		APInt ParseNumber();
};
#endif
