#ifndef instruction_hxx
#define instruction_hxx
#include <iostream>
#define debug_msg(x) //std::cout << (x) << std::endl;


#include <memory>
#include <string>

#include "llvm/DerivedTypes.h"

#include "asserts.hxx"
#include "state.hxx"
#include "APInt.hxx"

class instruction {
public:
	virtual ~instruction() = default;
	virtual void Codegen(std::shared_ptr<codegenState> state) = 0;
	virtual std::string name() = 0;	
};

class stackInstruction : public instruction {
};

class arithmeticInstruction : public instruction {
};

class heapInstruction : public instruction {
};

class controlInstruction : public instruction {
};

class ioInstruction : public instruction {
};

class PushInstruction : public stackInstruction {
	private:
		APInt Num;
	public:
		PushInstruction(APInt value): Num(value){debug_msg("constructed Pushinstruction");} 
		~PushInstruction() {debug_msg("deconstructed Pushinstruction");}
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Value* serializednum = Num.Serialize(state);
			DataSourceNode::createPushNode(state,serializednum);
		}
		virtual std::string name () {return "Push";};
};

class DupInstruction : public stackInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Value* value = DataConsumerNode::createPopNode(state);
			DataSourceNode::createPushNode(state,value);
			DataSourceNode::createPushNode(state,value);
			/*llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			llvm::Value* value =  state->Builder.CreateCall(PopInstr,"dupval"); 
			llvm::Function* PushInstr = state->TheModule->getFunction("PushInstr");
			state->Builder.CreateCall(PushInstr, value);
			state->Builder.CreateCall(PushInstr, value);*/
		}
		virtual std::string name () {return "Dup";};
};

class DiscardInstruction : public stackInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			DataConsumerNode::createPopNode(state); 
		}
		virtual std::string name () {return "Discard";};
};

class SwapInstruction : public stackInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Value* value1 = DataConsumerNode::createPopNode(state);
			llvm::Value* value2 = DataConsumerNode::createPopNode(state);
			DataSourceNode::createPushNode(state,value1);
			DataSourceNode::createPushNode(state,value2);
		}
		virtual std::string name () {return "Swap";};
};

class CopyInstruction : public stackInstruction {
	private:
		APInt value;
	public:
		CopyInstruction(APInt val) : value(val){}
		virtual void Codegen(std::shared_ptr<codegenState> state){
			std::vector<llvm::Value*> nodes;
			for(APIntBuilder iter; iter < this->value; iter.addOne()) {
				nodes.push_back(DataConsumerNode::createPopNode(state));
			}
			llvm::Value* tocopy = DataConsumerNode::createPopNode(state);
			DataSourceNode::createPushNode(state,tocopy);
			for(llvm::Value* node: nodes) {
				DataSourceNode::createPushNode(state,node);
			}
			DataSourceNode::createPushNode(state,tocopy);
		}
		virtual std::string name () {return "Copy";};
};

class SlideInstruction : public stackInstruction {
	private:
		APInt value;
	public:
		SlideInstruction(APInt val) : value(val){}
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Value* top = DataConsumerNode::createPopNode(state);
			for(APIntBuilder iter; iter < this->value; iter.addOne()) {
				DataConsumerNode::createPopNode(state);
			}
			DataSourceNode::createPushNode(state,top);
		}
		virtual std::string name () {return "Slide";};
};

class PlusInstruction : public arithmeticInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			/*llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			llvm::Value* value1 = state->Builder.CreateCall(PopInstr,"addval");
			llvm::Value* value2 = state->Builder.CreateCall(PopInstr,"addval");*/
			llvm::Value* value1 = DataConsumerNode::createPopNode(state);
			llvm::Value* value2 = DataConsumerNode::createPopNode(state);
			llvm::Function* AddInstr = state->TheModule->getFunction("AddInstr");
			llvm::Value* result = state->Builder.CreateCall2(AddInstr,value1,value2,"addresult");
			DataSourceNode::createPushNode(state,result);
			/*llvm::Function* PushInstr = state->TheModule->getFunction("PushInstr");
			state->Builder.CreateCall(PushInstr, result);*/
		}
		virtual std::string name () {return "Plus";};
};

class MinusInstruction : public arithmeticInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			//top value is rhs
			llvm::Value* value2 = DataConsumerNode::createPopNode(state);
			llvm::Value* value1 = DataConsumerNode::createPopNode(state);
			llvm::Function* MinusInstr = state->TheModule->getFunction("MinusInstr");
			llvm::Value* result = state->Builder.CreateCall2(MinusInstr,value1,value2,"Minusresult");
			DataSourceNode::createPushNode(state,result);
		}
		virtual std::string name () {return "Minus";};
};

class TimesInstruction : public arithmeticInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Value* value1 = DataConsumerNode::createPopNode(state);
			llvm::Value* value2 = DataConsumerNode::createPopNode(state);
			llvm::Function* TimesInstr = state->TheModule->getFunction("TimesInstr");
			llvm::Value* result = state->Builder.CreateCall2(TimesInstr,value1,value2,"Timesresult");
			DataSourceNode::createPushNode(state,result);
		}
		virtual std::string name () {return "Times";};
};

class DivideInstruction : public arithmeticInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			abort();
		}
		virtual std::string name () {return "Divide";};
};

class ModuloInstruction : public arithmeticInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			abort();
		}
		virtual std::string name () {return "Modulo";};
};

class StoreInstruction : public heapInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Value* value = DataConsumerNode::createPopNode(state);
			llvm::Value* address = DataConsumerNode::createPopNode(state);
			assertdefined(value)
			assertdefined(address)
			DataSourceNode::createMemStore(state,address,value);
		}
		virtual std::string name () {return "Store";};
};
class RetriveInstruction : public heapInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Value* address = DataConsumerNode::createPopNode(state);
			llvm::Value* value = DataConsumerNode::createMemRetrive(state,address);
			DataSourceNode::createPushNode(state,value);
}
		virtual std::string name () {return "Retrive";};
};


//uses reverse tail call optemiztion for jumps
//tail call optimization is when you convert a call followed by a return in to a jump
//in reverse tail call a jump is represented by a call followed by a return
//this allows the labels to use a common namespace for jumps and calls and use
//predecareation for backwards jumps

inline llvm::Function* MakeFunction(APInt label,std::shared_ptr<codegenState> state){
	llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()), false);
	llvm::Function *Function = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, (std::string)label, state->TheModule);
	//Fastcall so tailcail optermization can be applied
	Function->setCallingConv(llvm::CallingConv::ID::Fast);
	// If F conflicted, there was already something named 'Name'.  If it has a
	// body, don't allow redefinition or reextern.
	if (Function->getName() != (std::string)label) {
		// Delete the one we just made and get the existing one.
		Function->eraseFromParent();
		Function = state->TheModule->getFunction((std::string)label);

		// If F already has a body, reject this.
		if (!Function->empty()) {
			std::cerr << "warning: redefinition of label " << (std::string)label << std::endl;
	  		return nullptr;
		}
	}
	return Function;
}


class LabelInstruction : public controlInstruction {
	private:
		APInt label;
	public:
		LabelInstruction(APInt value) : label(value) {}
		virtual void Codegen(std::shared_ptr<codegenState> state){
			/*llvm::BasicBlock* currentblock = state->Builder.GetInsertBlock();
			llvm::Function *TheFunction = currentblock->getParent();*/
			DataConsumerNode::SaveStack(state);
			llvm::Function* Function = MakeFunction(label,state);
			//ignore redefs
			if (Function) {
				llvm::BasicBlock *functionstartBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", Function);
				llvm::Value* success = state->Builder.CreateCall(Function,"callsuccess");
				state->Builder.CreateRet(success);
				state->Builder.SetInsertPoint(functionstartBB);
			}
		}
		virtual std::string name () {return ((std::string)"Label").append(label);};
};

class JumpInstruction : public controlInstruction {
	private:
		APInt label;
	public:
		JumpInstruction(APInt value) : label(value) {}
		std::string getLabelAsString() {return label;}
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Function* placetojump = state->TheModule->getFunction((std::string)label);
			if (!placetojump)
				placetojump = MakeFunction(label,state);
			DataConsumerNode::SaveStack(state);
			llvm::Value* success = state->Builder.CreateCall(placetojump,"callsuccess");
			state->Builder.CreateRet(success);
			llvm::BasicBlock* currentblock = state->Builder.GetInsertBlock();
			llvm::Function *TheFunction = currentblock->getParent();
			llvm::BasicBlock *continueBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "afterjump", TheFunction);
			state->Builder.SetInsertPoint(continueBB);
		}
		virtual std::string name () {return ((std::string)"Jump").append(this->getLabelAsString());};
};

//TODO:test return before call
class CallInstruction : public controlInstruction {
	private:
		APInt label;
	public:
		CallInstruction(APInt value) : label (value){}
		virtual void Codegen(std::shared_ptr<codegenState> state){
			DataConsumerNode::SaveStack(state);
			llvm::Function* placetojump = state->TheModule->getFunction((std::string)label);
			if (!placetojump)
				placetojump = MakeFunction(label,state);
			llvm::Value* successcode = state->Builder.CreateCall(placetojump,"callsuccess");
			llvm::Value* success = state->Builder.CreateICmpEQ(successcode,llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()),0),"callsuccessbool");

			//return if failed
			llvm::BasicBlock* currentblock = state->Builder.GetInsertBlock();
			llvm::Function *TheFunction = currentblock->getParent();
			llvm::BasicBlock *retBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "callblock", TheFunction);
			llvm::BasicBlock *continueBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "aftercalljump");
			state->Builder.CreateCondBr(success,continueBlock,retBlock);
			state->Builder.SetInsertPoint(retBlock);
			state->Builder.CreateRet(successcode);
			TheFunction->getBasicBlockList().push_back(continueBlock);
			state->Builder.SetInsertPoint(continueBlock);
		}
		virtual std::string name () {return ((std::string)"Call").append(label);};
};


class IfZeroInstruction : public controlInstruction {
	private:
		APInt label;
	public:
		IfZeroInstruction(APInt value) : label(value){}
		virtual void Codegen(std::shared_ptr<codegenState> state){
			//get value
			llvm::Value* value = DataConsumerNode::createPopNode(state);
			//preform comparason
			llvm::StructType* APIntType = state->TheModule->getTypeByName("JITAPInt");
			llvm::Constant* child = llvm::ConstantPointerNull::get(APIntType->getPointerTo());
			llvm::Constant* zerostructvalue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()),0,true);
			llvm::Constant* zerostruct = llvm::ConstantStruct::get(APIntType,zerostructvalue,child, nullptr);
			llvm::Function* CmpZeroInstr = state->TheModule->getFunction("CmpEqualInstr");
			llvm::Value* cond = state->Builder.CreateCall2(CmpZeroInstr,value,zerostruct,"cmp");

			//do jump
			llvm::BasicBlock* currentblock = state->Builder.GetInsertBlock();
			llvm::Function *TheFunction = currentblock->getParent();
			llvm::BasicBlock *callBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "callblock", TheFunction);
			llvm::BasicBlock *elseBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "aftercondjump");
			state->Builder.CreateCondBr(cond,callBlock,elseBlock);
			state->Builder.SetInsertPoint(callBlock);
			DataConsumerNode::SaveStack(state,false);
			llvm::Function* placetojump = state->TheModule->getFunction((std::string)label);
			if (!placetojump)
				placetojump = MakeFunction(label,state);
			llvm::Value* success = state->Builder.CreateCall(placetojump,"callsuccess");
			state->Builder.CreateRet(success);
			TheFunction->getBasicBlockList().push_back(elseBlock);
			state->Builder.SetInsertPoint(elseBlock);
		}
		virtual std::string name () {return "Jump If Zero";};
};
class IfNegInstruction : public controlInstruction {
	public:
		IfNegInstruction(APInt value){
			abort();
		}
		virtual void Codegen(std::shared_ptr<codegenState> state){}
		virtual std::string name () {return "Jump If Negative";};
};

class ReturnInstruction : public controlInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			DataConsumerNode::SaveStack(state);
			state->Builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()),0));
llvm::BasicBlock* currentblock = state->Builder.GetInsertBlock();
			llvm::Function *TheFunction = currentblock->getParent();
			llvm::BasicBlock *continueBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "afterreturn", TheFunction);
			state->Builder.SetInsertPoint(continueBB);
			
		}
		virtual std::string name () {return "Return";};
};

class EndInstruction : public controlInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::BasicBlock* currentblock = state->Builder.GetInsertBlock();
			llvm::Function *TheFunction = currentblock->getParent();
			//put apInt as label once output sorted
			llvm::BasicBlock *continueBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "afterend", TheFunction);
			state->Builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()),0));
			if (!state->stack.empty()) {
				DataConsumerNode::SaveStack(state);
				std::cerr << "program cannot end normally" << std::endl;
				abort();
			}
			state->Builder.SetInsertPoint(continueBB);

		}
		virtual std::string name () {return "End";};
};

class OutputCharInstruction : public ioInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			/*llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			llvm::Value* ch = state->Builder.CreateCall(PopInstr,"outputchar");*/
			llvm::Value* ch = DataConsumerNode::createPopNode(state);
			llvm::Function* OutputCharInstr = state->TheModule->getFunction("OutputCharInstr");
			state->Builder.CreateCall(OutputCharInstr, ch);
		}
		virtual std::string name () {return "Output Char";};
};

class OutputNumInstruction : public ioInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			/*llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			llvm::Value* num = state->Builder.CreateCall(PopInstr,"outputnum");*/
			llvm::Value* num = DataConsumerNode::createPopNode(state);
			llvm::Function* OutputNumInstr = state->TheModule->getFunction("OutputNumInstr");
			state->Builder.CreateCall(OutputNumInstr, num);
		}
		virtual std::string name () {return "Output Num";};
};

class ReadCharInstruction : public ioInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Value* address = DataConsumerNode::createPopNode(state);
			llvm::Function* ReadNumInstr = state->TheModule->getFunction("ReadCharInstr");
			llvm::Value* ch = state->Builder.CreateCall(ReadNumInstr,"charread");
			DataSourceNode::createMemStore(state,address,ch);
			/*llvm::Function* PushInstr = state->TheModule->getFunction("PushInstr");
			state->Builder.CreateCall(PushInstr, ch);*/
		}
		virtual std::string name () {return "Read Char";};
};

class ReadNumInstruction : public ioInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){

			llvm::Value* address = DataConsumerNode::createPopNode(state);
			llvm::Function* ReadNumInstr = state->TheModule->getFunction("ReadNumInstr");
			llvm::Value* num = state->Builder.CreateCall(ReadNumInstr,"numread");
			DataSourceNode::createMemStore(state,address,num);
			/*llvm::Function* PushInstr = state->TheModule->getFunction("PushInstr");
			state->Builder.CreateCall(PushInstr, num);*/
		}
		virtual std::string name () {return "Read Num";};
};
#endif
