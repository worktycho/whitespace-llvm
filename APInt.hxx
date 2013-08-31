#ifndef APInt_hxx
#define APInt_hxx

#include <memory>

#include "llvm/Constant.h"

class APInt;

#include "state.hxx"

struct JITAPInt {
	long long int value;
	JITAPInt* next;
};

void FreeJITAPInt(JITAPInt* value);
bool JITAPIntlessthan(const JITAPInt const * lhs, const JITAPInt const* rhs);

class APInt {
	private:
		JITAPInt value;
		llvm::Constant* Serializehelper(std::shared_ptr<codegenState> state, JITAPInt* pos);
	public:
		APInt() : value({0,nullptr}) {}
		APInt(long long int value) : value({value,nullptr}) {}
		APInt(JITAPInt* value);
		~APInt();
		llvm::Constant* Serialize(std::shared_ptr<codegenState> state);
		bool operator< (const APInt rhs) const;
		operator std::string () const;
		JITAPInt GetJITAPInt() {return value;}
};

class APIntBuilder {
	private:
		JITAPInt value;
		void addOne(JITAPInt* value);
	public:
		APIntBuilder() : value({0,nullptr}) {}
		~APIntBuilder();
		void makeNegative();
		void doubleval();
		void addOne();
		operator APInt();
};


#endif
