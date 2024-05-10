ifdef RELEASE
	OTHER_FLAGS = -DNDEBUG -O3 -I../third_party/3d_collision_helpers/src
else
	OTHER_FLAGS = -Og -I../third_party/3d_collision_helpers/src
endif

SOURCES = \
		../src/ems.cc \
		../src/main.cc \
		../src/screen.cc \
		../src/shared_data.cc \
		../src/screen_debug.cc \
		../src/screen_battle.cc \
		../src/resource_handler.cc \
		../third_party/3d_collision_helpers/src/sc_sacd.cpp

HEADERS = \
		../src/screen.h \
		../src/shared_data.h \
		../src/screen_debug.h \
		../src/screen_battle.h \
		../src/resource_handler.h \
		../third_party/3d_collision_helpers/src/sc_sacd.h

OBJDIR = objdir
OUTDIR = out

OBJECTS = $(addprefix ${OBJDIR}/,$(subst ..,PREVDIR,$(subst .cpp,.cpp.o,$(subst .cc,.cc.o,${SOURCES}))))

EMSDK_ENV_SH ?= ${HOME}/git/emsdk/emsdk_env.sh

CXX = source ${EMSDK_ENV_SH} &>/dev/null && em++

all: ${OUTDIR}/GanderBattle.html

${OUTDIR}/GanderBattle.html: ./raylib/libraylib.a ./raylib/raylib.h ./raylib/raymath.h ${OBJECTS} ${HEADERS}
	@mkdir -p ${OUTDIR}
	${CXX} -std=c++20 -o ${OUTDIR}/GanderBattle.html \
		-s USE_GLFW=3 -Iraylib -Lraylib -lraylib \
		-sEXPORTED_FUNCTIONS=_main \
		-sEXPORTED_RUNTIME_METHODS=ccall \
		--preload-file ../res \
		${OTHER_FLAGS} \
		${OBJECTS}
	cd ${OUTDIR} && ln -sf GanderBattle.html index.html

.SECONDEXPANSION:

${OBJDIR}/%.cc.o: $$(subst PREVDIR,..,%.cc) ${HEADERS}
	@mkdir -p "$(dir $@)"
	${CXX} -std=c++20 -I ./raylib -c ${OTHER_FLAGS} -o $@ $<
${OBJDIR}/%.cpp.o: $$(subst PREVDIR,..,%.cpp) ${HEADERS}
	@mkdir -p "$(dir $@)"
	${CXX} -std=c++20 -I ./raylib -c ${OTHER_FLAGS} -o $@ $<

.PHONY: clean

clean:
	rm -rf ${OBJDIR}
	rm -rf ${OUTDIR}
