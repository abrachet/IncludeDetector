TARGET = ID_test
WARNING = -Wall -Wshadow --pedantic -Wvla -Werror
OPTIMIZATIONS = -O0
LIBRARIES = #-lc -lpthread
GCC = gcc -g -std=c11 $(OPTIMIZATIONS) $(WARNING) $(LIBRARIES) -I./
DEFINES = -DDEBUG

SRCS = Parsing/Scanner.c Parsing/keywords.c Parsing/TokenStream.c \
	   Export/export.c Export/sym_file.c Export/find_symbol.c Parsing/_parse_source.c \
	   Parsing/_parse_header.c Parsing/Parser.c arg_parser.c single_threaded_conductor.c \
	   ID.c main.c

OBJS = $(SRCS:%.c=%.o)

.c.o:
	$(GCC) $(DEFINES) -c $< -o $@

$(TARGET): $(OBJS)
	$(GCC) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $(TARGET)

VALGRIND = valgrind --tool=memcheck --verbose --log-file=memcheck.txt ./$(TARGET)

testmemory: $(TARGET)
	$(VALGRIND) ./$(TARGET)

debug: $(TARGET)
	lldb ./$(TARGET)

clean:
	-rm -f $(OBJS)
