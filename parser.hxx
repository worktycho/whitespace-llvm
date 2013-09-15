#ifndef parser_hxx
#define parser_hxx

#include <memory>
#include <exception>

#include "tokeniser.hxx"
#include "instruction.hxx"
#include "APInt.hxx"

class parser {
	public:
		parser(std::shared_ptr<tokeniser> tokeniser);
		~parser() = default;
		
		bool TryGetNextInstruction(std::shared_ptr<instruction>& instruction);
	private:
		std::shared_ptr<tokeniser> m_tokeniser;
		bool parseStackInstructions(std::shared_ptr<instruction>& instruction);
		bool parseArithmeticInstructions(std::shared_ptr<instruction>& instruction );
		bool parseHeapInstructions(std::shared_ptr<instruction>& instruction);
		bool parseControlInstructions(std::shared_ptr<instruction>& instruction);
		bool parseIOInstructions(std::shared_ptr<instruction>& instruction);
		bool ParseNumber(APInt& num);
};
#endif
