input=./src/main.cpp
output=test_case_generator
all:
	g++ $(input) -o $(output)
run:
	./$(output)