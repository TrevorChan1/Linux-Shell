override CFLAGS := -Wall -Werror -std=gnu99 -O0 -g $(CFLAGS) -I.

# Build the parser .o file
myshell_parser.o: myshell_parser.c myshell_parser.h

# Add rules for any additional test programs here
myshell : myshell.c myshell_parser.o

# Add any additional tests here
test_files=test_simple_input

.PHONY: clean check checkprogs

# Build all of the test programs
checkprogs: $(test_files)

# Run the test programs
check: checkprogs
	run_tests.sh $(test_files)

clean:
	rm -f *.o $(test_files) $(test_o_files)
