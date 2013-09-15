#include "parser.hxx"
#include <iostream>
#define debug_msg(x) //std::cout << (x) << std::endl;

parser::parser(std::shared_ptr<tokeniser> tok): m_tokeniser(tok){
}

bool parser::TryGetNextInstruction(std::shared_ptr<instruction>& instruction){
	tokens token;
	if(!m_tokeniser->TryGetNextToken(token)) return false;
	switch (token) {
		case tokens::space:
			if (!this->parseStackInstructions(instruction)) return false;
			break;
		case tokens::linefeed:
			if (!this->parseControlInstructions(instruction)) return false;
			break;
		case tokens::tab:
			if(!m_tokeniser->TryGetNextToken(token)) return false;
			switch(token) {
				case tokens::space:
					if (!this->parseArithmeticInstructions(instruction)) return false;
					break;
				case tokens::linefeed:
					if (!this->parseIOInstructions(instruction)) return false;
					break;
				case tokens::tab:
					if (!this->parseHeapInstructions(instruction)) return false;
					break;
			}
			break;
	}
	return true;
}

bool parser::parseStackInstructions(std::shared_ptr<instruction>& instruction){

	tokens token;
	if(!m_tokeniser->TryGetNextToken(token)) return false;

	switch(token) {
		case tokens::space:
			{ //restricts the scope of num to this case
				APInt num;
				if (!ParseNumber(num)) return false;
				instruction = std::make_shared<PushInstruction>(num);
			}
			break;
		case tokens::linefeed:
			if(!m_tokeniser->TryGetNextToken(token)) return false;
			switch(token) {
				case tokens::space:
					instruction = std::shared_ptr<DupInstruction>(new DupInstruction);
					break;
				case tokens::linefeed:
					instruction = std::shared_ptr<DiscardInstruction>(new DiscardInstruction);
					break;
				case tokens::tab:
					instruction = std::shared_ptr<SwapInstruction>(new SwapInstruction);
					break;
			}
			return true;			
			break;
		case tokens::tab:
			if(!m_tokeniser->TryGetNextToken(token)) return false;
			switch(token) {
				case tokens::space:
					{ //restricts the scope of num to this case
						APInt num;
						if (!ParseNumber(num)) return false;
						instruction = std::shared_ptr<CopyInstruction>(new CopyInstruction(num));
					}
					break;
				case tokens::linefeed:
					{
						APInt num;
						if (!ParseNumber(num)) return false;
						instruction = std::shared_ptr<SlideInstruction>(new SlideInstruction(num));
					}
					break;
				case tokens::tab:
					return false;
					break;
			}
			return true;	
			break;
	}
	return true;
}
bool parser::parseArithmeticInstructions(std::shared_ptr<instruction>& instruction){

	tokens token;
	if(!m_tokeniser->TryGetNextToken(token)) return false;

	switch(token) {
		case tokens::space:
			if(!m_tokeniser->TryGetNextToken(token)) return false;
			switch(token) {
				case tokens::space:
					instruction = std::shared_ptr<PlusInstruction>(new PlusInstruction);
					break;
				case tokens::linefeed:
					instruction = std::shared_ptr<TimesInstruction>(new TimesInstruction);
					break;
				case tokens::tab:
					instruction = std::shared_ptr<MinusInstruction>(new MinusInstruction);
					break;
			}
			return true;	
			break;
		case tokens::linefeed:
			return false;
			break;
		case tokens::tab:
			if(!m_tokeniser->TryGetNextToken(token)) return false;
			switch(token) {
				case tokens::space:
					instruction = std::shared_ptr<DivideInstruction>(new DivideInstruction);
					return true;
					break;
				case tokens::linefeed:
					return false;
					break;
				case tokens::tab:
					instruction = std::shared_ptr<ModuloInstruction>(new ModuloInstruction);
					return true;					
					break;
			}			
			break;
	}
}
bool parser::parseHeapInstructions(std::shared_ptr<instruction>& instruction){
	tokens token;
	if(!m_tokeniser->TryGetNextToken(token)) return false;
	switch(token) {
		case tokens::space:
			instruction = std::shared_ptr<StoreInstruction>(new StoreInstruction);
			break;
		case tokens::linefeed:
			return false;
		case tokens::tab:
			instruction = std::shared_ptr<RetriveInstruction>(new RetriveInstruction);
			break;
	}
	return true;
}
bool parser::parseControlInstructions(std::shared_ptr<instruction>& instruction){
	tokens token;
	if(!m_tokeniser->TryGetNextToken(token)) return false;
	switch(token) {
		case tokens::space:
			if(!m_tokeniser->TryGetNextToken(token)) return false;
			switch(token) {
				case tokens::space:
					{ //restricts the scope of num to this case
						APInt num;
						if (!ParseNumber(num)) return false;
						instruction = std::shared_ptr<LabelInstruction>(new LabelInstruction(num));
					}
					break;
				case tokens::linefeed:
					{ //restricts the scope of num to this case
						APInt num;
						if (!ParseNumber(num)) return false;
						instruction = std::shared_ptr<JumpInstruction>(new JumpInstruction(num));
					}
					break;
				case tokens::tab:
					{ //restricts the scope of num to this case
						APInt num;
						if (!ParseNumber(num)) return false;
						instruction = std::shared_ptr<CallInstruction>(new CallInstruction(num));
					}
					break;
			}			
			break;
		case tokens::linefeed:
			if(!m_tokeniser->TryGetNextToken(token)) return false;
			if (token == tokens::linefeed) {
				instruction = std::shared_ptr<EndInstruction>(new EndInstruction);
			} else {
				return false;
			}
			break;
		case tokens::tab:
			if(!m_tokeniser->TryGetNextToken(token)) return false;
			switch(token) {
				case tokens::space:
					{ //restricts the scope of num to this case
						APInt num;
						if (!ParseNumber(num)) return false;
						instruction = std::shared_ptr<IfZeroInstruction>(new IfZeroInstruction(num));
					}
					break;
				case tokens::linefeed:
					instruction = std::shared_ptr<ReturnInstruction>(new ReturnInstruction);
					break;
				case tokens::tab:
					{ //restricts the scope of num to this case
						APInt num;
						if (!ParseNumber(num)) return false;
						instruction = std::shared_ptr<IfNegInstruction>(new IfNegInstruction(num));
					}
					break;
			}			
			break;

	}
	return true;
}
bool parser::parseIOInstructions(std::shared_ptr<instruction>& instruction){
	tokens token;
	if(!m_tokeniser->TryGetNextToken(token)) return false;
	switch(token) {
		case tokens::space:
			if(!m_tokeniser->TryGetNextToken(token)) return false;
			switch(token) {
				case tokens::space:
					instruction = std::shared_ptr<OutputCharInstruction>(new OutputCharInstruction);
					break;
				case tokens::linefeed:
					return false;
					break;
				case tokens::tab:
					instruction = std::shared_ptr<OutputNumInstruction>(new OutputNumInstruction);
					break;
			}			
			break;
		case tokens::linefeed:
			return false;
			break;
		case tokens::tab:
			if(!m_tokeniser->TryGetNextToken(token)) return false;
			switch(token) {
				case tokens::space:
					instruction = std::shared_ptr<ReadCharInstruction>(new ReadCharInstruction);
					break;
				case tokens::linefeed:
					return false;
					break;
				case tokens::tab:
					instruction = std::shared_ptr<ReadNumInstruction>(new ReadNumInstruction);
					break;
			}			
			break;
	}
	return true;
}

bool parser::ParseNumber(APInt& num) {
	APIntBuilder builder;
	tokens token;
	if(!m_tokeniser->TryGetNextToken(token)) return false;
	if (token == tokens::space) {
	//do nothing
	} else if (token == tokens::tab) {
		builder.makeNegative();
	} else {
		return false;
	}
	if(!m_tokeniser->TryGetNextToken(token)) return false;
	while(token != tokens::linefeed) {
		builder.doubleval();
		if (token == tokens::tab) {
			builder.addOne();
		}
		if(!m_tokeniser->TryGetNextToken(token)) return false;
	}
	num = builder;
	return true;
}

