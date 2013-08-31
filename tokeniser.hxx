#ifndef tokeniser_hxx
#define tokeniser_hxx

#include <memory>
#include <fstream>
#include <exception>

enum tokens {
	space,
	tab,
	linefeed
};

class fileReadFailureException: public std::exception{};

class tokeniser {
	public:
		tokeniser(std::shared_ptr<std::ifstream> filehandle);
		~tokeniser() = default;

		bool TryGetNextToken(tokens& token) throw (fileReadFailureException);
	private:
		std::shared_ptr<std::ifstream> filehandle;
};
#endif
