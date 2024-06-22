.PHONY: clean all

all: floor rand fibonacci sqrt eqn2  sqrt2 sqrt3

floor: callfloor.o floor.o
	clang++ -o floor callfloor.o floor.o
	@echo "1] FLOOR IS HERE\n\n"

callfloor.o: callfloor.cpp
	clang++ -c callfloor.cpp

floor.o: floor.k
	../kcomp floor.k 2> floor.ll
	./tobinary floor.ll
	
rand: callrand.o floor.o rand.o
	clang++ -o rand callrand.o floor.o rand.o
	@echo "2] RAND IS HERE\n\n"

callrand.o: callrand.cpp
	clang++ -c callrand.cpp

rand.o:	rand.k
	../kcomp rand.k 2> rand.ll
	./tobinary rand.ll

fibonacci: fibonacciIt.o callfibo.o
	clang++ -o fibonacci callfibo.o fibonacciIt.o
	@echo "3] FIBONACCI IS HERE\n\n"

callfibo.o:	callfibo.cpp
	clang++ -c callfibo.cpp
	
fibonacciIt.o:	fibonacciIt.k
	../kcomp fibonacciIt.k 2> fibonacciIt.ll
	./tobinary fibonacciIt.ll
	
sqrt: callsqrt.o sqrt.o
	clang++ -o sqrt callsqrt.o sqrt.o
	@echo "4] SQRT1 IS HERE\n\n"

callsqrt.o: callsqrt.cpp
	clang++ -c callsqrt.cpp

sqrt.o:	sqrt.k
	../kcomp sqrt.k 2> sqrt.ll
	./tobinary sqrt.ll
	
eqn2: calleqn2.o sqrt.o eqn2.o
	clang++ -o eqn2 calleqn2.o sqrt.o eqn2.o
	@echo "5] EQN2 IS HERE\n\n"

calleqn2.o: calleqn2.cpp
	clang++ -c calleqn2.cpp

eqn2.o:	eqn2.k
	../kcomp eqn2.k 2> eqn2.ll
	./tobinary eqn2.ll

sqrt2: callsqrt.o sqrt2.o
	clang++ -o sqrt2 callsqrt.o sqrt2.o
	@echo "6] SQRT2 IS HERE\n\n"

sqrt2.o:	sqrt2.k
	../kcomp sqrt2.k 2> sqrt2.ll
	./tobinary sqrt2.ll
	
sqrt3: callsqrt.o sqrt3.o
	clang++ -o sqrt3 callsqrt.o sqrt3.o
	@echo "7] SQRT3 IS HERE\n\n"

sqrt3.o:	sqrt3.k
	../kcomp sqrt3.k 2> sqrt3.ll
	./tobinary sqrt3.ll
	
clean:
	rm -f floor rand fibonacci sqrt eqn2 sqrt2 sqrt3 *~ *.o *.s *.bc *.ll
