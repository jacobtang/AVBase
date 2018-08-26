# Makefile

include ../../config.mak

# DEBUG can be set to YES to include debugging info, or NO otherwise
# DEBUG          := YES

# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO

PRJECTNAME 		:=libMediaMuxer
GROUPNAME		:=AVBase
#****************************************************************************

OUTPUT_OBJDIR	:=${OUTPUT_OBJDIR}${GROUPNAME}/${PRJECTNAME}/

ifeq (YES, ${DEBUG})
   CFLAGS       := ${DEBUG_CFLAGS} -I../../include/${GROUPNAME}
   CXXFLAGS     := ${DEBUG_CXXFLAGS} -I../../include/${GROUPNAME}
   LDFLAGS      := ${DEBUG_LDFLAGS} -L../../lib/android/${GROUPNAME}
   OUTPUT_BINDIR := ${OUTPUT_BINDIR}${GROUPNAME}/debug/
   OUTPUT	:= ${OUTPUT_BINDIR}${PRJECTNAME}.so
   OUTPUT_OBJDIR := ${OUTPUT_OBJDIR}debug
else
   CFLAGS       := ${RELEASE_CFLAGS} -I../../include/${GROUPNAME}
   CXXFLAGS     := ${RELEASE_CXXFLAGS} -I../../include/${GROUPNAME}
   LDFLAGS      := ${RELEASE_LDFLAGS} -L../../lib/android/${GROUPNAME}
   OUTPUT_BINDIR := ${OUTPUT_BINDIR}${GROUPNAME}/release/
   OUTPUT	:= ${OUTPUT_BINDIR}${PRJECTNAME}.so
   OUTPUT_OBJDIR := ${OUTPUT_OBJDIR}release
endif

ifeq (YES, ${PROFILE})
   CFLAGS   := ${CFLAGS} -pg -O3
   CXXFLAGS := ${CXXFLAGS} -pg -O3
   LDFLAGS  := ${LDFLAGS} -pg
endif

LIBS := -lavcodec -lavutil  -lstdc++ -ldl -lpthread -lrt -lm

#****************************************************************************
# Include paths
#****************************************************************************

INCS := -I../ffmpeg/output/android/include

#****************************************************************************
# Static library paths
#****************************************************************************

LIBDIR := -L../ffmpeg/output/android/lib

#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************

all: ${OUTPUT}
	if [ ! -d "${OUTPUT_OBJDIR}" ] ; then mkdir "${OUTPUT_OBJDIR}" -p -v; fi
	mv ${OBJS} ${OUTPUT_OBJDIR} -v 			
	mv ${OBJSD} ${OUTPUT_OBJDIR} -v 	
	
#****************************************************************************
# Source files
#****************************************************************************

SRCS := FFDecDecoder.cpp FFLock.cpp

# Add on the sources for libraries
SRCS := ${SRCS}

OBJS := $(addsuffix .o,$(basename ${SRCS}))
OBJSD := $(addsuffix .d,$(basename ${SRCS}))

#****************************************************************************
# Output
#****************************************************************************

${OUTPUT}: ${OBJS}
	if [ ! -d "${OUTPUT_BINDIR}" ] ; then mkdir "${OUTPUT_BINDIR}" -p -v; fi
	${LD} ${LDFLAGS} ${OBJS} ${LIBDIR} ${LIBS} ${EXTRA_LIBS} -o $@ 

#****************************************************************************
# common rules
#****************************************************************************

# Rules for compiling source files to object files
%.o : %.cpp
	${CXX} ${CXXFLAGS} ${INCS} -c $< -o $@

%.o : %.c
	${CC} ${CFLAGS} ${INCS} -c $< -o $@

dist:
	bash makedistlinux

clean:
	rm -f core ${OBJS} ${OUTPUT}
	rm -f core ${OUTPUT_OBJDIR}/*.o

depend:
	#makedepend ${INCS} ${SRCS}
