CPU_SRCS = ../cpu
MEMORY_SRCS = ../memory

./test: $(CPU_SRCS)/MOS6502.o $(MEMORY_SRCS)/Memory.o ./test.cpp
	g++ -c -o ./test.o ./test.cpp
	g++ -o ./test ./test.o $(CPU_SRCS)/MOS6502.o $(MEMORY_SRCS)/Memory.o

$(CPU_SRCS)/MOS6502.o: $(CPU_SRCS)/MOS6502.h $(CPU_SRCS)/MOS6502.cpp
	g++ -c -D _NO_DELAY_ -o $(CPU_SRCS)/MOS6502.o $(CPU_SRCS)/MOS6502.cpp

$(MEMORY_SRCS)/Memory.o: $(MEMORY_SRCS)/Memory.h $(MEMORY_SRCS)/Memory.cpp
	g++ -c -o $(MEMORY_SRCS)/Memory.o $(MEMORY_SRCS)/Memory.cpp

clean:
	rm -f $(CPU_SRCS)/*.o
	rm -f $(MEMORY_SRCS)/*.o
	rm -f ./test.o
	rm -f ./test
