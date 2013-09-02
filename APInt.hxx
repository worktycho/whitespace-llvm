#ifndef APInt_hxx
#define APInt_hxx

#include <memory>

#include "llvm/Constant.h"

class APInt;

#include "state.hxx"
#define UNSIGN_LONG_LONG_HIGH_BIT 0x8000000000000000LL
#define LONG_LONG_HIGH_BIT        0x4000000000000000LL
#define STD_LONG_LONG_MAX         0x7FFFFFFFFFFFFFFFLL
#define LONG_LONG_LOWER_HALF      0x00000000FFFFFFFFLL
#define LONG_LONG_UPPER_HALF      0x7FFFFFFF00000000LL
#define LONG_LONG_LOWER_31        0x000000007FFFFFFFLL
#define UNSIGN_LONG_LONG_UPPER_33 0xFFFFFFFF80000000LL
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
		JITAPInt GetJITAPInt() const {return value;}
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
		bool operator< (const APInt rhs) const;
};


#endif
