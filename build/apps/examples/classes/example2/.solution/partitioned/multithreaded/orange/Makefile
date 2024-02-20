all:  		Example2

INC=-I../autogen -I.

Example2:  	Example2.cpp ../autogen/codec.c ../autogen/ClosureRemoteHalMaster.cpp
	 	$(CXX) $(COPTS) -o $@ $^ -std=c++11 $(INC)
# stackoverflow.com/questions/61473077/didnt-pass-ldflags-ldflags
# Suggestion to add: -Wl,--hash-style=gnu

clean:
	 	rm -f *.o Example2

