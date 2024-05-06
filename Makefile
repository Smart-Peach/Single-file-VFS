.PHONY: clean, interpreter

CXX = clang++
STD = -std=c++20

ASAN := -fsanitize=address
UBSAN := -fsanitize=undefined
SAN := -fsanitize=undefined,address
TEST_FLAGS := -lgtest -lgmock -pthread

ifeq (test,$(firstword $(MAKECMDGOALS)))
  TEST_COMMANDS :=  $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  $(eval $(TEST_COMMANDS):;@:)
endif

all: interpreter clean 


interpreter:
	$(CXX) -c interpreter/Parser.cpp  interpreter/Interpreter.cpp src/vfs_mtt_main.cpp $(STD)
	$(CXX) vfs_mtt_main.o Parser.o Interpreter.o -o main.out $(STD)
	rm *.o
	./main.out

interpreter-san:
	$(CXX) -c interpreter/Parser.cpp  interpreter/Interpreter.cpp src/vfs_mtt_main.cpp $(SAN) $(STD)
	$(CXX) vfs_mtt_main.o Parser.o Interpreter.o -o main.out $(TEST_FLAGS) $(SAN) $(STD)
	rm *.o
	./main.out

test-interpreter: 
	$(CXX) -c interpreter/Parser.cpp tests/test_interpreter.cpp interpreter/Interpreter.cpp $(STD)
	$(CXX) test_interpreter.o Parser.o Interpreter.o -o tests/test.out $(STD)
	./tests/test.out
	rm *.o 



test-binary:
	$(CXX) tests/test_main_binary.cpp -o test_bin.out
	./test_bin.out
	rm *.out

test-superblock:
	$(CXX) tests/test_superblock_func.cpp src/structures/Superblock.cpp src/structures/Inode.cpp src/structures/InodeMap.cpp src/structures/AwesomeFileSystem.cpp -o test_bin.out
	./test_bin.out
	rm *.out

clean: 
	rm *.out 