.PHONY: all run clean new

BIN:=access
BINARGS:=

BUILD_DIR:=build
CFLAGS:=-g
CPPFLAGS:=-g
LFLAGS:=-g
LIBS:=

CROSS_COMPILE?=
GCC:=${CROSS_COMPILE}gcc
GPP:=${CROSS_COMPILE}g++

DIRS:=${BUILD_DIR}

SRCS_C:=$(wildcard *.c)
SRCS_CPP:=$(wildcard *.cpp)
SRCS:=${SRCS_C} ${SRCS_CPP}

OBJS_C:=$(addprefix ${BUILD_DIR}/,$(SRCS_C:.c=.o))
OBJS_CPP:=$(addprefix ${BUILD_DIR}/,$(SRCS_CPP:.cpp=.o))
OBJS:=${OBJS_C} ${OBJS_CPP}

DEPS:=$(OBJS:.o=.d)

#-------

all: ${BIN}

run: all
	./${BIN} ${BINARGS}

clean:
	rm -f ${BIN} ${BUILD_DIR}/*

new: clean
	@${MAKE} all

install: ${BIN}
	install access /bin/access -o root -g root -m4755

#-------

.%.dir:
	@if [ ! -d $* ]; then echo "mkdir -p $*"; mkdir -p $*; fi
	@touch $@

${BIN}: ${OBJS}
	${GPP} ${LFLAGS} ${OBJS} $(addprefix -l,${LIBS}) -o ${BIN}

${OBJS_C}: ${BUILD_DIR}/%.o: .${BUILD_DIR}.dir %.c
	${GCC} ${CFLAGS} -MMD -c $(filter %.c,$^) -o $@
${OBJS_CPP}: ${BUILD_DIR}/%.o: .${BUILD_DIR}.dir %.cpp
	${GPP} ${CPPFLAGS} -MMD -c $(filter %.cpp,$^) -o $@

-include $(DEPS)
