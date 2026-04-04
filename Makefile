
UNAME=$(shell uname)

SRCS=runset.cpp options.cpp utils.cpp

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

.PHONY: FORCE

ECHO=echo -e
VERSION=$(shell git describe --abbrev=0 --tags 2>/dev/null || echo 'Unknown_version')
COPYRIGHT=(C) D. LALANNE - MIT License.
DECORATION=
COMMIT=$(shell git rev-parse --short HEAD 2>/dev/null || echo 'Unknown_commit')
ISO8601 := $(shell date +%Y-%m-%dT%H:%M:%SZ)
TMSTAMP := $(shell date +%Y%m%d%H%M%S)

TARGET=${TARGET_DIR}/runset${EXEXT}

all : ${TARGET}

gcc : ${TARGET}

options.cpp : version.h

# Génération du version.h intégré dans l'appli
version.h : version_check.txt
	@${ECHO} "Building C++ header $@"
	${ECHO} "#ifndef VERSION_H\n#define VERSION_H\nstruct\n{\n  std::string name, version, copyright, decoration, commit, created_at;\n} app_info = {\"${PREFIX}\", \"${VERSION}\", \"${COPYRIGHT}\", \"${DECORATION}\", \"${COMMIT}\", \"${ISO8601}\"};\n#endif" >$@

# Pour regénérer silencieusement version.h dès qu'un des champs version ou copyright ou decoration ou commit, est modifié.
version_check.txt : FORCE
	@${ECHO} "Version:${VERSION}, copyright:${COPYRIGHT}, decoration:${DECORATION}, commit:${COMMIT}" >new_$@
	@-( if [ ! -f $@ ]; then cp new_$@ $@; sleep 0.4; fi )
	@-( if diff new_$@ $@ >/dev/null 2>&1; then rm -f new_$@; \
		  else mv -f new_$@ $@; rm -f ${PREFIX}.iss ${PREFIX}-standalone.iss; fi )

runset.cpp : runset.ico

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
LDFLAGS += -mwindows
#LDFLAGS += -luser32 -lgdi32 -lgdiplus -lcomctl32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32 -lws2_32 -lwldap32 -lcrypt32 -lNormaliz -lCrypt32 -lshlwapi -lmpr -lPathcch -lDwmapi 
LDLIBS += -lurlmon
LDLIBS += -lwsock32 -lole32 -luuid -lcomctl32 -loleaut32

LINK     = $(CXX)
${TARGET} : ${OBJS}
	$(LINK.cc) ${OBJS} $(LOADLIBES) $(LDLIBS) -o $@
endif

ALL_SRCS=$(wildcard *.cpp) $(wildcard *.h)
format :
	@echo "Formatting with clang, the following files: ${ALL_SRCS}"
	@clang-format -style="{ BasedOnStyle: Microsoft, ColumnLimit: 256, IndentWidth: 2, TabWidth: 2, UseTab: Never, AllowShortIfStatementsOnASingleLine: AllIfsAndElse }" --sort-includes -i ${ALL_SRCS}

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

