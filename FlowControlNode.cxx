/*#include "FlowControlNode.hxx"

#include <algorithm>

#include "llvm/Module.h"

void FlowControlNode::AddCallSite(std::shared_ptr<CallSite> site) {
	if(!returnnode) {
		this->callsites.push_back(site);
	} else {
		this->returnnode->AddCallSite(site);
	}
}

void LabelNode::setJumpPoint(llvm::BasicBlock* jumpBlock) {
	this->JumpPoint = jumpBlock;
	if (!this->JumpstoInsert.empty()) {
		llvm::IRBuilder<> Builder(llvm::getGlobalContext());
		for(auto iter = this->JumpstoInsert.begin(); iter != this->JumpstoInsert.end(); iter++) {
			llvm::BasicBlock* insertpoint = *iter;
			Builder.SetInsertPoint(insertpoint);
			Builder.CreateBr(jumpBlock);
		}
		JumpstoInsert.clear();
	}
	if (!this->CondJumpstoInsert.empty()) {
		llvm::IRBuilder<> Builder(llvm::getGlobalContext());
		for(auto iter = this->CondJumpstoInsert.begin(); 
				iter != this->CondJumpstoInsert.end(); 
				iter++) {
			conditionJumpData insertdata = *iter;
			Builder.SetInsertPoint(insertdata.InsertPoint);
			Builder.CreateCondBr(insertdata.condition,jumpBlock,insertdata.continueBlock);
		}
		JumpstoInsert.clear();
	}
}


void LabelNode::InsertJump(llvm::BasicBlock* insertpoint){
	if (!this->JumpPoint) {
		this->JumpstoInsert.push_back(insertpoint);
		return;
	}
	llvm::IRBuilder<> Builder(llvm::getGlobalContext());
	Builder.SetInsertPoint(insertpoint);
	Builder.CreateBr(this->JumpPoint);
}

void LabelNode::InsertCondJump(llvm::BasicBlock* insertpoint,
	llvm::BasicBlock* continueBlock,
	llvm::Value* condition){
	if (!this->JumpPoint) {
		this->CondJumpstoInsert.push_back(conditionJumpData({insertpoint, continueBlock, condition}));
		return;
	}
	llvm::IRBuilder<> Builder(llvm::getGlobalContext());
	Builder.SetInsertPoint(insertpoint);
	Builder.CreateCondBr(condition,this->JumpPoint,continueBlock);
}

long long int CallSite::callsiteidsource = 0;

ReturnSite::ReturnSite(std::shared_ptr<FlowControlNode> Previous, llvm::BasicBlock* ReturnBlock, llvm::Module* TheModule) : FlowControlNode(Previous) {
	llvm::IRBuilder<> Builder(llvm::getGlobalContext());
	this->ReturnBlock = ReturnBlock;
	Builder.SetInsertPoint(ReturnBlock);
	llvm::Function* PopInstr = TheModule->getFunction("PopCallStackInstr");
	this->calladdress = Builder.CreateCall(PopInstr,"popcall");
	std::vector<std::shared_ptr<CallSite>> callsites = Previous->getCallSites();
	for(std::shared_ptr<CallSite>& site : callsites) {
		this->AddCallSite(site);
	}
}

void ReturnSite::AddCallSite(std::shared_ptr<CallSite> callsite) {
	llvm::IRBuilder<> builder(llvm::getGlobalContext());
	llvm::BasicBlock* currentblock = this->ReturnBlock;
	llvm::Function *TheFunction = currentblock->getParent();
	llvm::BasicBlock *continueBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "inret", TheFunction);
	this->ReturnBlock = continueBB;
	builder.SetInsertPoint(currentblock);
	llvm::Value* label = callsite->getID();
	llvm::Value* condition = builder.CreateICmpEQ(label,this->calladdress,"cmp");
	builder.CreateCondBr(condition,callsite->getResumeBlock(),continueBB);

}

ReturnSite::~ReturnSite() {
	llvm::IRBuilder<> builder(llvm::getGlobalContext());
	builder.SetInsertPoint(this->ReturnBlock);
	builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()),1));
}
*/
