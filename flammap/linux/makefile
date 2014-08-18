#Adam Moreno, Beartooth Carbon Consulting Inc. 2011
#This make file creates the FlamMapLX.exe and all supporting objects
#This version of FlamMap is not compatible with Fsim
#This creates a stand alone version only.

ROOTDIR=${PWD}
#LIBDIR=${ROOTDIR}/FlamMap/Debug
LIBDIR=/usr/local/lib
#INCDIR=${ROOTDIR}/FlamMap
INCDIR=/usr/local/include
VERSION=1Lxbeta
USER=`whoami`
HOST=`hostname`
CC = g++
ProjectObjects= FlamMap.o  flmburn4.o flmcrwn.o rand3.o icf_input.o icf_chk.o stdafx.o flammap3.o flammap4.o FLM_TREP.o flmfms3.o Fsxwspot.o flmmech.o flmenvt2.o Fsxwutil.o icf_FSPRo.o
CFLAGS_GENERIC = -I${INCDIR} -DVERS="\\\"${VERSION}\\\"" -DUSER="\\\"${USER}\\\"" -DHOST="\\\"${HOST}\\\""
LDFLAGS_GENERIC = -lm  -L${LIBDIR} -lgdal
CFLAGS = -c -g -O0 -ansi ${CFLAGS_GENERIC}
#CFLAGS = -g -O0 -ansi -pedantic ${CFLAGS_GENERIC}
LDFLAGS = ${LDFLAGS_GENERIC}

MACROS=ROOTDIR=${ROOTDIR} LIBDIR=${LIBDIR} INCDIR=${INCDIR} \
	VERSION=${VERSION} CC=${CC} CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" 

.SUFFIXES: .cpp .cxx

.cxx.o:
	g++ $(CFLAGS) -c $<
.cpp.o:
	g++ $(CFLAGS) -c $<

all: TestFlamMap

FlamMap.a: $(ProjectObjects)
	ar r $@ $?
	#$(CC) $(CFLAGS_GENERIC) $(LDFLAGS) $(ProjectObjects) -o FlamMap.exe

FlamMap.o: ./FlamMap/FlamMap.cpp 
	$(CC) $(CFLAGS) ./FlamMap/FlamMap.cpp
	@echo $(ROOTDIR)
	@echo $(LIBDIR)
	@echo $(INCDIR)
	@echo $(VERSION)
	@echo $(USER)
	@echo $(HOST)
	@echo $(ProjectObjects)
	@echo $(CFLAGS)
	@echo $(LDFLAGS)
	@echo $(MACROS)
#fsxwatm.o: FlamMap/fsxwatm.h
#	$(CC) $(CFLAGS) ./FlamMap/Fsxwatm.cpp
flammap3.o: FlamMap/flammap3.cpp FlamMap/flammap3.h
	$(CC) $(CFLAGS) ./FlamMap/flammap3.cpp
flmburn4.o: flammap3.o
	$(CC) $(CFLAGS) ./FlamMap/flmburn4.cpp
flmcrwn.o: flammap3.o
	$(CC) $(CFLAGS) ./FlamMap/flmcrwn.cpp
rand3.o: ./Common/rand3.h
	$(CC) $(CFLAGS) ./Common/rand3.cpp
#icf_def.o: ./Common/icf_def.h icf_input.o icf_chk.o
#	$(CC) $(CFLAGS) ./Common/icf_def.cpp
icf_input.o:
	$(CC) $(CFLAGS) ./Common/icf_input.cpp
icf_chk.o:
	$(CC) $(CFLAGS) ./Common/icf_chk.CPP
stdafx.o: ./FlamMap/stdafx.h
	$(CC) $(CFLAGS) ./FlamMap/stdafx.cpp
flammap4.o:
	$(CC) $(CFLAGS) ./FlamMap/flammap4.cpp
FLM_TREP.o:
	$(CC) $(CFLAGS) ./FlamMap/FLM_TREP.CPP
flmfms3.o:
	$(CC) $(CFLAGS) ./FlamMap/flmfms3.cpp
Fsxwspot.o:
	$(CC) $(CFLAGS) ./FlamMap/Fsxwspot.cpp
flmmech.o:
	$(CC) $(CFLAGS) ./FlamMap/flmmech.cpp
flmenvt2.o:
	$(CC) $(CFLAGS) ./FlamMap/flmenvt2.cpp
Fsxwutil.o:
	$(CC) $(CFLAGS) ./FlamMap/Fsxwutil.cpp
icf_FSPRo.o:
	$(CC) $(CFLAGS) ./Common/icf_FSPRo.cpp
TestFlamMap.o: FlamMap.o
	$(CC) $(CFLAGS) ./TestFlamMap/TestFlamMap.cpp
TestFlamMap: TestFlamMap.o $(ProjectObjects)
	$(CC) $(CFLAGS_GENERIC) $(LDFLAGS) $(ProjectObjects)  TestFlamMap.o -o FlamMapLX.exe
