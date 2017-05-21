.PHONY: run-cli-test run-tests

run-cli-test: cli-test
	@./cli-test $(RUN_ARGS)

run-cli-test-cpp: cli-test-cpp
	@./cli-test-cpp $(RUN_ARGS)

run-tests: tests
	@./tests

cli-test: cli-test.c argparse.h argparse.c
	gcc -std=c99 -O0 -g cli-test.c argparse.c -o cli-test

cli-test-cpp: cli-test.c argparse.h argparse.c
	g++ -std=c++11 -O0 -g cli-test.c argparse.c -o cli-test-cpp

tests: tests.c argparse.h argparse.c
	gcc -std=c99 -O0 -g tests.c argparse.c unity/src/unity.c -o tests
