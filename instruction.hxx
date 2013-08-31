#ifndef instruction_hxx
#define instruction_hxx
#include <iostream>
#define debug_msg(x) //std::cout << (x) << std::endl;


#include <memory>
#include <string>

#include "llvm/DerivedTypes.h"

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
			llvm::Function* PushInstr = state->TheModule->getFunction("PushInstr");
			state->Builder.CreateCall(PushInstr, serializednum);
		}
		virtual std::string name () {return "Push";};
};

class DupInstruction : public stackInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			llvm::Value* value =  state->Builder.CreateCall(PopInstr,"dupval"); 
			llvm::Function* PushInstr = state->TheModule->getFunction("PushInstr");
			state->Builder.CreateCall(PushInstr, value);
			state->Builder.CreateCall(PushInstr, value);
		}
		virtual std::string name () {return "Dup";};
};

class DiscardInstruction : public stackInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			state->Builder.CreateCall(PopInstr); 
		}
		virtual std::string name () {return "Discard";};
};

class SwapInstruction : public stackInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){}
		virtual std::string name () {return "Swap";};
};

class CopyInstruction : public stackInstruction {
	public:
		CopyInstruction(APInt value){}
		virtual void Codegen(std::shared_ptr<codegenState> state){}
		virtual std::string name () {return "Copy";};
};

class SlideInstruction : public stackInstruction {
	public:
		SlideInstruction(APInt value){}
		virtual void Codegen(std::shared_ptr<codegenState> state){}
		virtual std::string name () {return "Slide";};
};

class PlusInstruction : public arithmeticInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			llvm::Value* value1 = state->Builder.CreateCall(PopInstr,"addval");
			llvm::Value* value2 = state->Builder.CreateCall(PopInstr,"addval");
			llvm::Function* AddInstr = state->TheModule->getFunction("AddInstr");
			llvm::Value* result = state->Builder.CreateCall2(AddInstr,value1,value2,"addresult");
			llvm::Function* PushInstr = state->TheModule->getFunction("PushInstr");
			state->Builder.CreateCall(PushInstr, result);
		}
		virtual std::string name () {return "Plus";};
};

class MinusInstruction : public arithmeticInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			llvm::Value* value1 = state->Builder.CreateCall(PopInstr,"addval");
			llvm::Value* value2 = state->Builder.CreateCall(PopInstr,"addval");
			llvm::Function* MinusInstr = state->TheModule->getFunction("MinusInstr");
			llvm::Value* result = state->Builder.CreateCall2(MinusInstr,value1,value2,"Minusresult");
			llvm::Function* PushInstr = state->TheModule->getFunction("PushInstr");
			state->Builder.CreateCall(PushInstr, result);
		}
		virtual std::string name () {return "Minus";};
};

class TimesInstruction : public arithmeticInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			llvm::Value* value1 = state->Builder.CreateCall(PopInstr,"timesval");
			llvm::Value* value2 = state->Builder.CreateCall(PopInstr,"timesval");
			llvm::Function* TimesInstr = state->TheModule->getFunction("TimesInstr");
			llvm::Value* result = state->Builder.CreateCall2(TimesInstr,value1,value2,"Timesresult");
			llvm::Function* PushInstr = state->TheModule->getFunction("PushInstr");
			state->Builder.CreateCall(PushInstr, result);
		}
		virtual std::string name () {return "Times";};
};

class DivideInstruction : public arithmeticInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){}
		virtual std::string name () {return "Divide";};
};

class ModuloInstruction : public arithmeticInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){}
		virtual std::string name () {return "Modulo";};
};

class StoreInstruction : public heapInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){}
		virtual std::string name () {return "Store";};
};
class RetriveInstruction : public heapInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){}
		virtual std::string name () {return "Retrive";};
};

class LabelInstruction : public controlInstruction {
	private:
		APInt label;
	public:
		LabelInstruction(APInt value) : label(value) {}
		virtual void Codegen(std::shared_ptr<codegenState> state){
			std::map<APInt,std::shared_ptr<LabelNode>>::iterator it = state->LabelCollection.find(this->label);
			std::shared_ptr<LabelNode> node;
			if(it != state->LabelCollection.end())
			{
   				node = it->second;
			} else {
				node = std::shared_ptr<LabelNode>(new LabelNode);
				state->LabelCollection.insert(std::pair<APInt,std::shared_ptr<LabelNode>>(this->label,node));
			}
			llvm::BasicBlock* currentblock = state->Builder.GetInsertBlock();
			llvm::Function *TheFunction = currentblock->getParent();
			//put apInt as label once output sorted
			llvm::BasicBlock *continueBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "label", TheFunction);
			state->Builder.CreateBr(continueBB);
			state->Builder.SetInsertPoint(continueBB);
			node->setJumpPoint(continueBB);
			state->CurrentNode = node;
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
			llvm::BasicBlock* currentblock = state->Builder.GetInsertBlock();
			llvm::Function *TheFunction = currentblock->getParent();
			llvm::BasicBlock *continueBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "afterjump", TheFunction);
			std::map<APInt,std::shared_ptr<LabelNode>>::iterator it = state->LabelCollection.find(this->label);
			std::shared_ptr<LabelNode> node;
			if(it != state->LabelCollection.end())
			{
   				node = it->second;
			} else {
				node = std::shared_ptr<LabelNode>(new LabelNode());
				state->LabelCollection.insert(std::pair<APInt,std::shared_ptr<LabelNode>>(this->label,node));
			}
			node->InsertJump(currentblock);
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
			llvm::BasicBlock* currentblock = state->Builder.GetInsertBlock();
			llvm::Function *TheFunction = currentblock->getParent();
			//put apInt as label once output sorted
			llvm::BasicBlock *continueBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "resumecall", TheFunction);
			std::map<APInt,std::shared_ptr<LabelNode>>::iterator it = state->LabelCollection.find(this->label);
			std::shared_ptr<LabelNode> labelnode;
			if(it != state->LabelCollection.end())
			{
   				labelnode = it->second;
			} else {
				labelnode = std::shared_ptr<LabelNode>(new LabelNode());
				state->LabelCollection.insert(std::pair<APInt,std::shared_ptr<LabelNode>>(this->label,labelnode));
			}
			std::shared_ptr<CallSite> node(new CallSite(continueBB,labelnode));
			node->setMyPointer(node);
			llvm::Function* PushInstr = state->TheModule->getFunction("PushCallStackInstr");
			state->Builder.CreateCall(PushInstr,node->getID());
			state->CurrentNode = node;
			labelnode->InsertJump(currentblock);
			state->Builder.SetInsertPoint(continueBB);
		}
		virtual std::string name () {return ((std::string)"Call").append(label);};
};


class IfZeroInstruction : public controlInstruction {
	private:
		APInt label;
	public:
		IfZeroInstruction(APInt value) : label(value){}
		virtual void Codegen(std::shared_ptr<codegenState> state){
			//TODO:convert to bool
			llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			llvm::Value* value = state->Builder.CreateCall(PopInstr,"pop");
			llvm::StructType* APIntType = state->TheModule->getTypeByName("JITAPInt");
			llvm::Constant* child = llvm::ConstantPointerNull::get(APIntType->getPointerTo());
			llvm::Constant* zerostructvalue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()),0,true);
			llvm::Constant* zerostruct = llvm::ConstantStruct::get(APIntType,zerostructvalue,child, nullptr);
			llvm::Function* CmpZeroInstr = state->TheModule->getFunction("CmpEqualInstr");
			llvm::Value* cond = state->Builder.CreateCall2(CmpZeroInstr,value,zerostruct,"cmp");
			std::map<APInt,std::shared_ptr<LabelNode>>::iterator it = state->LabelCollection.find(this->label);
			std::shared_ptr<LabelNode> node;
			if(it != state->LabelCollection.end())
			{
   				node = it->second;
			} else {
				node = std::shared_ptr<LabelNode>(new LabelNode());
				state->LabelCollection.insert(std::pair<APInt,std::shared_ptr<LabelNode>>(this->label,node));
			}
			llvm::BasicBlock* currentblock = state->Builder.GetInsertBlock();
			llvm::Function *TheFunction = currentblock->getParent();
			llvm::BasicBlock *continueBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "afterjump", TheFunction);
			node->InsertCondJump(currentblock,continueBB,cond);
			state->Builder.SetInsertPoint(continueBB);
		}
		virtual std::string name () {return "Jump If Zero";};
};
class IfNegInstruction : public controlInstruction {
	public:
		IfNegInstruction(APInt value){}
		virtual void Codegen(std::shared_ptr<codegenState> state){}
		virtual std::string name () {return "Jump If Negative";};
};

class ReturnInstruction : public controlInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			//break in block
			//callsite helper functions
			// push and pop callstack
			llvm::BasicBlock* currentblock = state->Builder.GetInsertBlock();
			llvm::Function *TheFunction = currentblock->getParent();
			llvm::BasicBlock *continueBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "afterret", TheFunction);
			ReturnSite* node = new ReturnSite(state->CurrentNode, currentblock,state->TheModule);
			state->CurrentNode = std::shared_ptr<FlowControlNode>(node);
			//leave returnSite to terminate previous block
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
			state->Builder.SetInsertPoint(continueBB);

		}
		virtual std::string name () {return "End";};
};

class OutputCharInstruction : public ioInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			llvm::Value* ch = state->Builder.CreateCall(PopInstr,"outputchar");
			llvm::Function* OutputCharInstr = state->TheModule->getFunction("OutputCharInstr");
			state->Builder.CreateCall(OutputCharInstr, ch);
		}
		virtual std::string name () {return "Output Char";};
};

class OutputNumInstruction : public ioInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){
			llvm::Function* PopInstr = state->TheModule->getFunction("PopInstr");
			llvm::Value* num = state->Builder.CreateCall(PopInstr,"outputnum");
			llvm::Function* OutputNumInstr = state->TheModule->getFunction("OutputNumInstr");
			state->Builder.CreateCall(OutputNumInstr, num);
		}
		virtual std::string name () {return "Output Num";};
};

class ReadCharInstruction : public ioInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){}
		virtual std::string name () {return "Read Char";};
};

class ReadNumInstruction : public ioInstruction {
	public:
		virtual void Codegen(std::shared_ptr<codegenState> state){}
		virtual std::string name () {return "Read Num";};
};
#endif
