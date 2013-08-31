#ifndef FlowControlNode_hxx
#define FlowControlNode_hxx

#include "llvm/IRBuilder.h"

struct conditionJumpData {
	llvm::BasicBlock* InsertPoint;
	llvm::BasicBlock* continueBlock;
	llvm::Value* condition;
};

class CallSite;
class ReturnSite;

class FlowControlNode {
private:
	std::shared_ptr<ReturnSite> returnnode;
	std::vector<std::shared_ptr<CallSite>> callsites;
	std::shared_ptr<FlowControlNode> prevnode;
public:
	FlowControlNode(std::shared_ptr<FlowControlNode> node) : prevnode(node){}
	virtual ~FlowControlNode() = default;
	virtual void AddCallSite(std::shared_ptr<CallSite> site);
	virtual inline std::vector<std::shared_ptr<CallSite>> getCallSites() {return this->callsites;};
	virtual inline void setReturnNode(std::shared_ptr<ReturnSite> node) {
		this->returnnode = node;
		prevnode->setReturnNode(node);
	}
};

class LabelNode : public FlowControlNode {
	private:
		llvm::BasicBlock* JumpPoint;
		std::vector<llvm::BasicBlock*> JumpstoInsert;
		std::vector<conditionJumpData> CondJumpstoInsert;
	public:
		LabelNode(std::shared_ptr<FlowControlNode> prevNode) : FlowControlNode(prevNode) {}
		void setJumpPoint(llvm::BasicBlock* jumpBlock);
		void InsertJump(llvm::BasicBlock* startBlock);
		void InsertCondJump(llvm::BasicBlock* startBlock, 
			llvm::BasicBlock* continueBlock, 
			llvm::Value* condition);
};

class ConditionalBranch : FlowControlNode {
};

class CallSite : public FlowControlNode {
	private:
		static long long int callsiteidsource;
		llvm::Value* id;
		llvm::BasicBlock* ResumeBlock;
		std::weak_ptr<CallSite> mypointer;
		std::shared_ptr<LabelNode> JumpToNode;
	public:
		CallSite(llvm::BasicBlock* block, std::shared_ptr<LabelNode> JumpTo,std::shared_ptr<FlowControlNode> prevNode) : FlowControlNode(prevNode), ResumeBlock(block), id(llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()),callsiteidsource)),JumpToNode(JumpTo) {
			callsiteidsource++;
		}
		llvm::Value* getID() {return this->id;}
		llvm::BasicBlock* getResumeBlock() {return this->ResumeBlock;}
		virtual inline std::vector<std::shared_ptr<CallSite>> getCallSites() {return std::vector<std::shared_ptr<CallSite>>(1,mypointer.lock()); }
		void setMyPointer(std::weak_ptr<CallSite> mypointer) {
			this->mypointer = mypointer;
			this->JumpToNode->AddCallSite(mypointer.lock());
		}
};

class ReturnSite : public FlowControlNode {
	private:
		llvm::BasicBlock* ReturnBlock;
		llvm::Value* calladdress;
	public:
		ReturnSite(std::shared_ptr<FlowControlNode> Previous, llvm::BasicBlock* ReturnBlock, llvm::Module* TheModule);
		void AddCallSite(std::shared_ptr<CallSite> callsite);
		virtual inline std::vector<std::shared_ptr<CallSite>> getCallSites() {return std::vector<std::shared_ptr<CallSite>>(); };
		~ReturnSite();

};

class EndNode : FlowControlNode {
};

class StartNode : public FlowControlNode {
public:
	StartNode() : FlowControlNode(nullptr) {}
	virtual inline std::vector<std::shared_ptr<CallSite>> getCallSites() {return std::vector<std::shared_ptr<CallSite>>(); };
};
#endif
