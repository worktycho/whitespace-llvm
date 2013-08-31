#include "parser.hxx"
#include <iostream>
#define debug_msg(x) //std::cout << (x) << std::endl;

parser::parser(std::shared_ptr<tokeniser> tok): m_tokeniser(tok){
}

bool parser::TryGetNextInstruction(std::shared_ptr<instruction>& instruction) throw (syntaxErrorException){
	tokens token;
	if(!m_tokeniser->TryGetNextToken(token)) return false;
		switch (token) {
			case tokens::space:
				instruction = this->parseStackInstructions();
				break;
			case tokens::linefeed:
				instruction = this->parseControlInstructions();
				break;
			case tokens::tab:
				switch(this->GetNextToken()) {
					case tokens::space:
						instruction = this->parseArithmeticInstructions();
						break;
					case tokens::linefeed:
						instruction = this->parseIOInstructions();
						break;
					case tokens::tab:
						instruction = this->parseHeapInstructions();
						break;
				}
				break;
		}
	return true;
}
tokens parser::GetNextToken() {
tokens token;
if(!m_tokeniser->TryGetNextToken(token)) {
	debug_msg("could not fetch next token")
	syntaxErrorException ex;
	throw ex;
}
return token;
}

std::shared_ptr<stackInstruction> parser::parseStackInstructions() throw (syntaxErrorException){
	switch(this->GetNextToken()) {
		case tokens::space:
			return std::make_shared<PushInstruction>(ParseNumber());
			break;
		case tokens::linefeed:
			switch(this->GetNextToken()) {
				case tokens::space:
					return std::shared_ptr<DupInstruction>(new DupInstruction);
					break;
				case tokens::linefeed:
					return std::shared_ptr<DiscardInstruction>(new DiscardInstruction);
					break;
				case tokens::tab:
					return std::shared_ptr<SwapInstruction>(new SwapInstruction);
					break;
			}			
			break;
		case tokens::tab:
			switch(this->GetNextToken()) {
				case tokens::space:
					return std::shared_ptr<CopyInstruction>(new CopyInstruction(ParseNumber()));
					break;
				case tokens::linefeed:
					return std::shared_ptr<SlideInstruction>(new SlideInstruction(ParseNumber()));
					break;
				case tokens::tab:
					{
					debug_msg("tab tab in stack");
					syntaxErrorException ex;
					throw ex;
					}
					break;
			}			
			break;
	}
}
std::shared_ptr<arithmeticInstruction> parser::parseArithmeticInstructions() throw (syntaxErrorException){
	switch(this->GetNextToken()) {
		case tokens::space:
			switch(this->GetNextToken()) {
				case tokens::space:
					return std::shared_ptr<PlusInstruction>(new PlusInstruction);
					break;
				case tokens::linefeed:
					return std::shared_ptr<TimesInstruction>(new TimesInstruction);
					break;
				case tokens::tab:
					return std::shared_ptr<MinusInstruction>(new MinusInstruction);
					break;
			}			
			break;
		case tokens::linefeed:
			{
			debug_msg("lf in arthmetic");
			syntaxErrorException ex;
			throw ex;
			}	
			break;
		case tokens::tab:
			switch(this->GetNextToken()) {
				case tokens::space:
					return std::shared_ptr<DivideInstruction>(new DivideInstruction);
					break;
				case tokens::linefeed:
					{
					debug_msg("tab lf in arthmetic");
					syntaxErrorException ex;
					throw ex;
					}
					break;
				case tokens::tab:
					return std::shared_ptr<ModuloInstruction>(new ModuloInstruction);
					break;
			}			
			break;
	}
}
std::shared_ptr<heapInstruction> parser::parseHeapInstructions() throw (syntaxErrorException){
	switch(this->GetNextToken()) {
		case tokens::space:
			return std::shared_ptr<StoreInstruction>(new StoreInstruction);
			break;
		case tokens::linefeed:
			{
			debug_msg("lf in heap");
			syntaxErrorException ex;
			throw ex;
			}
			break;
		case tokens::tab:
			return std::shared_ptr<RetriveInstruction>(new RetriveInstruction);
			break;
	}
}
std::shared_ptr<controlInstruction> parser::parseControlInstructions() throw (syntaxErrorException){
	switch(this->GetNextToken()) {
		case tokens::space:
			switch(this->GetNextToken()) {
				case tokens::space:
					return std::shared_ptr<LabelInstruction>(new LabelInstruction(ParseNumber()));
					break;
				case tokens::linefeed:
					return std::shared_ptr<JumpInstruction>(new JumpInstruction(ParseNumber()));
					break;
				case tokens::tab:
					return std::shared_ptr<CallInstruction>(new CallInstruction(ParseNumber()));
					break;
			}			
			break;
		case tokens::linefeed:
			if (this->GetNextToken() == tokens::linefeed) {
				return std::shared_ptr<EndInstruction>(new EndInstruction);
			} else {
				debug_msg("lf not followed by lf in control");
				syntaxErrorException ex;
				throw ex;
			}
			break;
		case tokens::tab:
			switch(this->GetNextToken()) {
				case tokens::space:
					return std::shared_ptr<IfZeroInstruction>(new IfZeroInstruction(ParseNumber()));
					break;
				case tokens::linefeed:
					return std::shared_ptr<ReturnInstruction>(new ReturnInstruction);
					break;
				case tokens::tab:
					return std::shared_ptr<IfNegInstruction>(new IfNegInstruction(ParseNumber()));
					break;
			}			
			break;

	}
}
std::shared_ptr<ioInstruction> parser::parseIOInstructions() throw (syntaxErrorException){
	switch(this->GetNextToken()) {
		case tokens::space:
			switch(this->GetNextToken()) {
				case tokens::space:
					return std::shared_ptr<OutputCharInstruction>(new OutputCharInstruction);
					break;
				case tokens::linefeed:
					{
					debug_msg("space lf in IO");
					syntaxErrorException ex;
					throw ex;
					}
					break;
				case tokens::tab:
					return std::shared_ptr<OutputNumInstruction>(new OutputNumInstruction);
					break;
			}			
			break;
		case tokens::linefeed:
			{
			debug_msg("lf in IO");
			syntaxErrorException ex;
			throw ex;
			}
			break;
		case tokens::tab:
			switch(this->GetNextToken()) {
				case tokens::space:
					return std::shared_ptr<ReadCharInstruction>(new ReadCharInstruction);
					break;
				case tokens::linefeed:
					{
					debug_msg("tab lf in IO");
					syntaxErrorException ex;
					throw ex;
					}
					break;
				case tokens::tab:
					return std::shared_ptr<ReadNumInstruction>(new ReadNumInstruction);
					break;
			}			
			break;
	}
}

APInt parser::ParseNumber() {
	APIntBuilder builder;
	tokens token;
	token = this->GetNextToken();
	if (token == tokens::space) {
	//do nothing
	} else if (token == tokens::tab) {
		builder.makeNegative();
	} else {
		debug_msg("empty num");
		syntaxErrorException ex;
		throw ex;
	}
	while((token = this->GetNextToken()) != tokens::linefeed) {
		builder.doubleval();
		if (token == tokens::tab) {
			builder.addOne();
		}
	}
	return builder;
}

