
ROOTLIBS = $(shell root-config --evelibs)
CXXFLAGS = $(shell root-config --cflags)

INCPATH = -I${LCIO}/src/cpp/include -I${LCIO}/include
LIBS = -L${LCIO}/lib -llcio
CXXFLAGS += -std=c++11

all	: Dict.o
ifeq (${LCIO},)
	$(error Error due to missing LCIO path)
endif
	g++ ${CXXFLAGS} sample.cc Dict.o ${ROOTLIBS} ${INCPATH} ${LIBS} -o run

# NOTE:  rootcint -f Dict.cxx で Dict.cxx
Dict.o :
	rootcint -f Dict.cxx -c ${INCPATH} LinkDef.h 
	g++ ${ROOTLIBS} ${CXXFLAGS} ${INCPATH} -c Dict.cxx

clean :
	rm Dict.cxx Dict.h *~ *.o *.pcm run
