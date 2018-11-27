
ROOTLIBS = $(shell root-config --evelibs)
CXXFLAGS = $(shell root-config --cflags)

# NOTE: .h(ヘッダファイル)をincludeした場合
INCPATH = -I${LCIO}/src/cpp/include -I${LCIO}/include
# NOTE: /lib/ などのライブラリをリンクした場合
LIBS = -L${LCIO}/lib -llcio

CXXFLAGS += -std=c++11

all	: Event.o Dict.o
ifeq (${LCIO},)
	$(error Error due to missing LCIO path)
endif
	g++ ${CXXFLAGS} sample.cc Event.o Dict.o ${ROOTLIBS} ${INCPATH} ${LIBS} -o run

# NOTE: Event.ccとEvent.hを材料にして、Event.oを生成。 (gcc -c Event.cc => Event.o生成)
Event.o	: Event.cc Event.h 
	g++ ${CXXFLAGS} ${ROOTLIBS} ${INCPATH} ${LIBS} -c Event.cc

# NOTE:  rootcint -f Dict.cxx で Dict.cxx
Dict.o : Event.cc Event.h
	rootcint -f Dict.cxx -c Event.h ${INCPATH} LinkDef.h 
	g++ ${ROOTLIBS} ${CXXFLAGS} ${INCPATH} -c Dict.cxx

clean :
	rm Dict.cxx Dict.h *~ *.o *.pcm run
