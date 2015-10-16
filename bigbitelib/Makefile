#------------------------------------------------------------------------------

# !Notice: Update SRC list from ./TreeSearch/Makefile to the following one
SRC  =\
    THaBigBite.cxx THaBigBiteE06010.cxx THaBigBiteTpMwdc.cxx \
    BBDecData.cxx \
    THaOptics.cxx THaOpticsAGen.cxx THaOpticsHRS.cxx \
        THaOpticsAnalytical.cxx THaOpticsNoFeild.cxx\
    THaTriggerPlane.cxx THaTriggerPlaneHit.cxx\
	THaScintPlane.cxx \
        THaAdcHit.cxx THaPartialHit.cxx THaScintBar.cxx \
        THaScintHit.cxx THaScintPMT.cxx THaTdcHit.cxx \
    THaMWDC.cxx \
        THaMWDCPlane.cxx THaMWDCGroup.cxx \
        THaMWDCHit.cxx THaMWDCLine.cxx \
        THaVDCTableTTDConv.cxx THaVDC2DTableTTDConv.cxx \
        THaVDCFuncTTDConv.cxx THaVDCLinearTTDConv.cxx \
    THaBBTotalShower.cxx THaTimingPlane.cxx THaScintPlaneDet.cxx\
        THaBBShower.cxx THaBBShowerCluster.cxx THaBBShowerBlock.cxx \
    MWDC.cxx WirePlane.cxx Hit.cxx TimeToDistConv.cxx  \
        Hitpattern.cxx Projection.cxx Pattern.cxx PatternTree.cxx  \
        PatternGenerator.cxx TreeWalk.cxx Road.cxx BigBite.cxx  \
	SpecialWirePlane.cxx \
    THaHe3Target.cxx THaADCHe3Spin.cxx THaBitHe3Spin.cxx THaHe3SpinModule.cxx \
THaOpticsE06010.cxx theoptics.cxx

EXTRAHDR = Helper.h Types.h

# Name of your package. 
# The shared library that will be built will get the name lib$(PACKAGE).so
PACKAGE = BigBite

LINKDEF = $(PACKAGE)_LinkDef.h

#------------------------------------------------------------------------------
# Compile debug version
# export DEBUG = 2
# export VERBOSE = 1
export TESTCODE = 1
#export I387MATH = 1
#export EXTRAWARN = 1

# Architecture to compile for
ARCH          = linux
#ARCH          = solarisCC5

#------------------------------------------------------------------------------
# Directory locations. All we need to know is INCDIRS.
# INCDIRS lists the location(s) of the C++ Analyzer header (.h) files

ifndef ANALYZER
  $(error $$ANALYZER environment variable not defined)
endif

INCDIRS  = $(wildcard $(addprefix $(ANALYZER)/, include src hana_decode hana_scaler))

#------------------------------------------------------------------------------
# Do not change anything  below here unless you know what you are doing

ifeq ($(strip $(INCDIRS)),)
  $(error No Analyzer header files found. Check $$ANALYZER)
endif

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLIBS     := $(shell root-config --libs)
ROOTGLIBS    := $(shell root-config --glibs)

INCLUDES      = $(ROOTCFLAGS) $(addprefix -I, $(INCDIRS) ) -I$(shell pwd)

USERLIB       = lib$(PACKAGE).so
USERDICT      = $(PACKAGE)Dict

LIBS          = 
GLIBS         = 

ifeq ($(ARCH),solarisCC5)
# Solaris CC 5.0
CXX           = CC
ifdef DEBUG
  CXXFLAGS    = -g
  LDFLAGS     = -g
  DEFINES     =
else
  CXXFLAGS    = -O3
  LDFLAGS     = -O3
  DEFINES     = -DNDEBUG
#  DEFINES     = -DNDEBUG
endif
DEFINES      += -DSUNVERS -DHAS_SSTREAM
CXXFLAGS     += -KPIC
LD            = CC
SOFLAGS       = -G
endif

ifeq ($(ARCH),linux)
# Linux with gcc (RedHat)
CXX           = g++
ifdef DEBUG
  CXXFLAGS    = -g -O0 
  LDFLAGS     = -g -O0
  DEFINES     =
else
  CXXFLAGS    = -O -march=core2 -mfpmath=sse
  LDFLAGS     = -O
  DEFINES     = 
#  DEFINES     = -DNDEBUG
endif
DEFINES      += -DLINUXVERS -DHAS_SSTREAM
CXXFLAGS     += -Wall -Woverloaded-virtual -fPIC -fno-strict-aliasing  
# added -fno-strict-aliasing to avoid warning during -O3 optimization

ifdef EXTRAWARN
CXXFLAGS     += -Wextra -Wno-missing-field-initializers 
endif
LD            = g++
SOFLAGS       = -shared
ifdef I387MATH
CXXFLAGS     += -mfpmath=387
else
CXXFLAGS     += 
endif
endif

ifeq ($(CXX),)
$(error $(ARCH) invalid architecture)
endif

ifdef VERBOSE
DEFINES      += -DVERBOSE
endif
ifdef TESTCODE
DEFINES      += -DTESTCODE
endif

CXXFLAGS     += $(DEFINES) $(INCLUDES)
LIBS         += $(ROOTLIBS) $(SYSLIBS)
GLIBS        += $(ROOTGLIBS) $(SYSLIBS)

MAKEDEPEND    = gcc

ifdef PROFILE
CXXFLAGS     += -pg
LDFLAGS      += -pg
endif

ifndef PKG
PKG           = lib$(PACKAGE)
LOGMSG        = "$(PKG) source files"
else
LOGMSG        = "$(PKG) Software Development Kit"
endif
DISTFILE      = $(PKG).tar.gz

#------------------------------------------------------------------------------
OBJ           = $(SRC:.cxx=.o)
HDR           = $(SRC:.cxx=.h) $(EXTRAHDR)
DEP           = $(SRC:.cxx=.d)
OBJS          = $(OBJ) $(USERDICT).o

all:		$(USERLIB)

$(USERLIB):	$(OBJS)
		$(LD) $(LDFLAGS) $(SOFLAGS) -o $@ $(OBJS)
		@echo "$@ done"

$(USERDICT).cxx: $(HDR) $(LINKDEF)
	@echo "Generating dictionary $(USERDICT)..."
	$(ROOTSYS)/bin/rootcint -f $@ -c $(INCLUDES) $(DEFINES) $^

install:	all
		$(error Please define install yourself)
# for example:
#		cp $(USERLIB) $(LIBDIR)

clean:
		rm -f *.o *~ $(USERLIB) $(USERDICT).*

realclean:	clean
		rm -f *.d

srcdist:
		rm -f $(DISTFILE)
		rm -rf $(PKG)
		mkdir $(PKG)
		cp -p $(SRC) $(HDR) $(LINKDEF) db*.dat Makefile $(PKG)
		gtar czvf $(DISTFILE) --ignore-failed-read \
		 -V $(LOGMSG)" `date -I`" $(PKG)
		rm -rf $(PKG)

.PHONY: all clean realclean srcdist

.SUFFIXES:
.SUFFIXES: .c .cc .cpp .cxx .C .o .d

%.o:	%.cxx
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# FIXME: this only works with gcc
%.d:	%.cxx
	@echo Creating dependencies for $<
	@$(SHELL) -ec '$(MAKEDEPEND) -MM $(INCLUDES) -c $< \
		| sed '\''s%^.*\.o%$*\.o%g'\'' \
		| sed '\''s%\($*\)\.o[ :]*%\1.o $@ : %g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

###

-include $(DEP)

