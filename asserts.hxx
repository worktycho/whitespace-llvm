#include <valgrind/memcheck.h>

#define myassert(x) if(!(x)) {std::cerr << "fail assertion at line: " << __LINE__ << " in file " << __FILE__; abort();}

#define assertdefined(x) VALGRIND_CHECK_VALUE_IS_DEFINED(x);

