#############################################################################

ifeq ($(strip $(BOOST_ROOT)),)
	BOOST_ROOT = /usr/local/include
endif

CC:=g++
SL:=g++ -shared

ROOTINC:=`root-config --libs` -lMinuit -lCore 
ROOTLIB:=`root-config --cflags`

LIBROOTFFTWRAPPERINC:=-I$(LIBROOTFFTWWRAPPER_DIR)
LIBROOTFFTWRAPPERLIB:=-L$(LIBROOTFFTWWRAPPER_DIR) -lRootFftwWrapper

ARAROOTINC:=-DARA_UTIL_EXISTS -I$(ARA_UTIL_INSTALL_DIR)/include
ARAROOTLIB:=-L$(ARA_UTIL_INSTALL_DIR)/lib -lAraEvent -lRootFftwWrapper

ARASIMINC:=-I$(ARASIM)
ARASIMLIB:=-L$(ARASIM) -lSim 

ASINC:=-I/home/aschultz/Ferrum
ASLIB:=-L/home/aschultz/Ferrum -lAnalyticSphereMethod 

SQLITEINC:=-I/net/ara/install_dir/utilities/include
SQLITEINC:=-L/net/ara/install_dir/utilities/lib -lsqlite3
#SQLITEINC:=-I$(SQLITE_ROOT)/include
#SQLITELIB:=-L$(SQLITE_ROOT)/lib -lsqlite3

INCLUDES:=$(ROOTINC) $(SQLITEINC) $(ARAROOTINC) $(ARASIMINC) $(ASINC)
LIBS:=$(ROOTLIB) $(SQLITELIB) $(ARAROOTLIB) $(ARASIMLIB) $(ASLIB) $(ARARECOLIB)
CFLAGS:= -Wl,--no-as-needed -fPIC

DLLSUF = ${DllSuf}
OBJSUF = ${ObjSuf}
SRCSUF = ${SrcSuf}

PROGRAM = exactNumAraSimDPFinder

all : $(PROGRAM)

$(PROGRAM) : $(PROGRAM).C
	@echo "<**Compiling**> "$<
	$(CC) $(LIBS) $(INCLUDES)  $(CFLAGS) $< -o $@
	@echo "Done."

clean:
	@rm -f $(PROGRAM)

#############################################################################
