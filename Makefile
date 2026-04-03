
UNAME=$(shell uname)

SRCS=runset.cpp

ifeq ($(findstring NT-, $(UNAME)),)
EXEXT=
TARGET_DIR=build/gcc/linux
BUILD_SYS=gcc
else
MSBUILD=/c/Program\ Files/Microsoft\ Visual\ Studio/18/Community/MSBuild/Current/Bin/amd64/MSBuild.exe	
MAGICK=magick
EXEXT=.exe
TARGET_DIR=build/msvc/win
BUILD_SYS=msvc
endif

ifeq ($(MAKECMDGOALS),gcc)
undefine MSBUILD
MAGICK=magick
EXEXT=.exe
TARGET_DIR=build/gcc/win
BUILD_SYS=gcc
ifeq ($(MSYSTEM),CLANG64)
CPPFLAGS += -I /clang64/include/c++
CXX=clang++
endif
LDFLAGS += -static
#LDFLAGS += -static-libgcc -static-libstdc++
CPPFLAGS += -DUNICODE -D_UNICODE 
endif

TARGET=${TARGET_DIR}/runset${EXEXT}

all : ${TARGET}

gcc : ${TARGET}

runset.cpp : runset.ico

#	@fold -w 253 runset.svg | sed -e 's/"/\\"/g;s/\(.*\)/"\1" \\/' >>runset_icon.h
runset_icon.h : runset.svg
	@echo -n "const char *svg_data=" >$@
	@sed -e 's/"/\\"/g;s/\(.*\)/"\1" \\/' $< >>$@
	@echo ";" >>$@

runset.ico : runset.svg
	${MAGICK} -density 256x256 -background none $< -define icon:auto-resize=128,96,64,48,32,16 -colors 256 $@
#	${MAGICK} $< -density 300 -define icon:auto-resize=128,96,64,48,32,16 -background none $@


ifeq ($(BUILD_SYS),msvc)
OBJS=$(SRCS:.cpp=.obj)
OBJS:=$(addprefix  ${TARGET_DIR}/,${OBJS})
MSVC_SLN=runset.slnx
${TARGET} : ${SRCS}
	@${MSBUILD} ${MSVC_SLN} -p:Configuration=Release
	@echo "${TARGET} OK"
else
OBJS=$(SRCS:.cpp=.o)
OBJS:=$(addprefix  ${TARGET_DIR}/,${OBJS})
CXXFLAGS += -std=c++23
CXXFLAGS += -Wall -pedantic -Wextra # Utiliser ces 2 dernières options de temps en temps peut-être utile ...
# Optim
#CXXFLAGS += -Oz
#LDFLAGS += -fno-rtti
LINK     = $(CXX)
${TARGET} : ${OBJS}
	$(LINK.cc) ${OBJS} $(LOADLIBES) $(LDLIBS) -o $@
endif

ALL_SRCS=$(wildcard *.cpp) $(wildcard *.h)
format :
	@echo "Formatting with clang, the following files: ${ALL_SRCS}"
	@clang-format -style="{ BasedOnStyle: Microsoft, ColumnLimit: 256, IndentWidth: 2, TabWidth: 2, UseTab: Never }" --sort-includes -i ${ALL_SRCS}

cfg : 
	@echo "Building TARGET [${TARGET}] for system [${UNAME}] with built tool [${BUILD_SYS}]"
	@echo "CPPFLAGS: ${CPPFLAGS}"
	@echo ${OBJS}
	@echo "LINK.cc: ${LINK.cc}"
	@echo "OBJS: ${OBJS}"
	@echo "LOADLIBES: ${LOADLIBES}"
	@echo "LDLIBS: ${LDLIBS}"

clean :
	 rm -f $(OBJS) $(SRCS:.cpp=.d) *~
	 rm -rf build/msvc build/gcc

ifneq ($(MAKECMDGOALS),clean)
# Implicit rule for building dep file from .c
%.d: %.cpp
	@echo "Building "$@" from "$<
	@${CXX} ${CPPFLAGS} -MM $< -o $@
#	@$(COMPILE.c) -isystem /usr/include -MM $< > $@

${TARGET_DIR}/%.o: %.cpp
	@mkdir -p ${TARGET_DIR}
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

# Inclusion of the dependency files '.d'
ifdef SRCS
-include $(SRCS:.cpp=.d)
endif
endif

