
debug: *.cxx
	clang++ *.cxx `/usr/lib/llvm-3.2/bin/llvm-config --libs core jit native` `/usr/lib/llvm-3.2/bin/llvm-config --cxxflags --cppflags --ldflags` -rdynamic -fexceptions -g -std=c++11 -O0 -o wspace-jit
