##############################
# Makefile
##############################

CC=gcc
CPP=g++
CFLAGS=-Wall -O2 -DLINUX
INC = -I/usr/local/include/
LIB_PGR = -L/usr/local/lib -lpgrlibdcstereo -ltriclops -lpnmutils -lraw1394 -ldc1394
LIB_CV = -lcv -lhighgui -lcvaux -lml -lm
# LIB_PLAYER = `pkg-config --libs playerc++`
# CFLAGS_PLAYER = `pkg-config --cflags playerc++`
LIB_SIFT = -lfeat

BIN =  me132_tutorial_2 \
 	   me132_tutorial_3

all:	$(BIN)

clean:
	@echo "Cleaning up directory."
	rm -rf *~ *.o $(BIN)


me132_tutorial_2: me132_tutorial_2.cc
	$(CPP) $(CFLAGS)   $^ -o $@ $(LIB_CV) $(LIB_SIFT)

me132_tutorial_3: me132_tutorial_3.o bb2.o
	$(CPP) $(CFLAGS) $^ -o $@ $(LIB_CV) $(LIB_PGR) $(LIB_SIFT)

# object files
bb2.o: bb2.cc
	$(CPP) -c $^ -o $@

me132_tutorial_3.o: me132_tutorial_3.cc
	$(CPP) -c $^ -o $@
