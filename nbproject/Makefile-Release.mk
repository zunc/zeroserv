#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/cfg.o \
	${OBJECTDIR}/src/client.o \
	${OBJECTDIR}/src/common/ticks.o \
	${OBJECTDIR}/src/dbg.o \
	${OBJECTDIR}/src/ebtree/eb32tree.o \
	${OBJECTDIR}/src/ebtree/ebtree.o \
	${OBJECTDIR}/src/fd.o \
	${OBJECTDIR}/src/handler.o \
	${OBJECTDIR}/src/handler/echo.o \
	${OBJECTDIR}/src/handler/http.o \
	${OBJECTDIR}/src/handler/http/http_parser.o \
	${OBJECTDIR}/src/handler/http_pubsub.o \
	${OBJECTDIR}/src/handler/mc/assoc.o \
	${OBJECTDIR}/src/handler/mc/fat/slabs.o \
	${OBJECTDIR}/src/handler/mc/fat/sto_layer.o \
	${OBJECTDIR}/src/handler/mc/items.o \
	${OBJECTDIR}/src/handler/memcached.o \
	${OBJECTDIR}/src/handler/pubsub.o \
	${OBJECTDIR}/src/handler/pubsub/account.o \
	${OBJECTDIR}/src/handler/pubsub/idzen.o \
	${OBJECTDIR}/src/handler/pubsub/model.o \
	${OBJECTDIR}/src/handler/pubsub/topic.o \
	${OBJECTDIR}/src/handler_udp/echo_udp.o \
	${OBJECTDIR}/src/handler_udp/routing.o \
	${OBJECTDIR}/src/handler_udp/routing/local_port.o \
	${OBJECTDIR}/src/handler_udp/routing/mineip.o \
	${OBJECTDIR}/src/handler_udp/routing/proto_routing.o \
	${OBJECTDIR}/src/handler_udp/routing/route_map.o \
	${OBJECTDIR}/src/poller.o \
	${OBJECTDIR}/src/proto_tcp.o \
	${OBJECTDIR}/src/proto_udp.o \
	${OBJECTDIR}/src/protos.o \
	${OBJECTDIR}/src/signal.o \
	${OBJECTDIR}/src/task.o \
	${OBJECTDIR}/src/time.o \
	${OBJECTDIR}/src/util.o \
	${OBJECTDIR}/src/zeroserv.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lconfig

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk bin/zeroserv

bin/zeroserv: ${OBJECTFILES}
	${MKDIR} -p bin
	${LINK.c} -o bin/zeroserv ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/src/cfg.o: src/cfg.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/cfg.o src/cfg.c

${OBJECTDIR}/src/client.o: src/client.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/client.o src/client.c

${OBJECTDIR}/src/common/ticks.o: src/common/ticks.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/common/ticks.o src/common/ticks.c

${OBJECTDIR}/src/dbg.o: src/dbg.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/dbg.o src/dbg.c

${OBJECTDIR}/src/ebtree/eb32tree.o: src/ebtree/eb32tree.c 
	${MKDIR} -p ${OBJECTDIR}/src/ebtree
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ebtree/eb32tree.o src/ebtree/eb32tree.c

${OBJECTDIR}/src/ebtree/ebtree.o: src/ebtree/ebtree.c 
	${MKDIR} -p ${OBJECTDIR}/src/ebtree
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ebtree/ebtree.o src/ebtree/ebtree.c

${OBJECTDIR}/src/fd.o: src/fd.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/fd.o src/fd.c

${OBJECTDIR}/src/handler.o: src/handler.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler.o src/handler.c

${OBJECTDIR}/src/handler/echo.o: src/handler/echo.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/echo.o src/handler/echo.c

${OBJECTDIR}/src/handler/http.o: src/handler/http.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/http.o src/handler/http.c

${OBJECTDIR}/src/handler/http/http_parser.o: src/handler/http/http_parser.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/http
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/http/http_parser.o src/handler/http/http_parser.c

${OBJECTDIR}/src/handler/http_pubsub.o: src/handler/http_pubsub.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/http_pubsub.o src/handler/http_pubsub.c

${OBJECTDIR}/src/handler/mc/assoc.o: src/handler/mc/assoc.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/mc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/mc/assoc.o src/handler/mc/assoc.c

${OBJECTDIR}/src/handler/mc/fat/slabs.o: src/handler/mc/fat/slabs.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/mc/fat
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/mc/fat/slabs.o src/handler/mc/fat/slabs.c

${OBJECTDIR}/src/handler/mc/fat/sto_layer.o: src/handler/mc/fat/sto_layer.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/mc/fat
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/mc/fat/sto_layer.o src/handler/mc/fat/sto_layer.c

${OBJECTDIR}/src/handler/mc/items.o: src/handler/mc/items.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/mc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/mc/items.o src/handler/mc/items.c

${OBJECTDIR}/src/handler/memcached.o: src/handler/memcached.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/memcached.o src/handler/memcached.c

${OBJECTDIR}/src/handler/pubsub.o: src/handler/pubsub.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/pubsub.o src/handler/pubsub.c

${OBJECTDIR}/src/handler/pubsub/account.o: src/handler/pubsub/account.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/pubsub
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/pubsub/account.o src/handler/pubsub/account.c

${OBJECTDIR}/src/handler/pubsub/idzen.o: src/handler/pubsub/idzen.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/pubsub
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/pubsub/idzen.o src/handler/pubsub/idzen.c

${OBJECTDIR}/src/handler/pubsub/model.o: src/handler/pubsub/model.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/pubsub
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/pubsub/model.o src/handler/pubsub/model.c

${OBJECTDIR}/src/handler/pubsub/topic.o: src/handler/pubsub/topic.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/pubsub
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler/pubsub/topic.o src/handler/pubsub/topic.c

${OBJECTDIR}/src/handler_udp/echo_udp.o: src/handler_udp/echo_udp.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler_udp/echo_udp.o src/handler_udp/echo_udp.c

${OBJECTDIR}/src/handler_udp/routing.o: src/handler_udp/routing.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler_udp/routing.o src/handler_udp/routing.c

${OBJECTDIR}/src/handler_udp/routing/local_port.o: src/handler_udp/routing/local_port.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp/routing
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler_udp/routing/local_port.o src/handler_udp/routing/local_port.c

${OBJECTDIR}/src/handler_udp/routing/mineip.o: src/handler_udp/routing/mineip.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp/routing
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler_udp/routing/mineip.o src/handler_udp/routing/mineip.c

${OBJECTDIR}/src/handler_udp/routing/proto_routing.o: src/handler_udp/routing/proto_routing.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp/routing
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler_udp/routing/proto_routing.o src/handler_udp/routing/proto_routing.c

${OBJECTDIR}/src/handler_udp/routing/route_map.o: src/handler_udp/routing/route_map.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp/routing
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/handler_udp/routing/route_map.o src/handler_udp/routing/route_map.c

${OBJECTDIR}/src/poller.o: src/poller.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/poller.o src/poller.c

${OBJECTDIR}/src/proto_tcp.o: src/proto_tcp.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/proto_tcp.o src/proto_tcp.c

${OBJECTDIR}/src/proto_udp.o: src/proto_udp.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/proto_udp.o src/proto_udp.c

${OBJECTDIR}/src/protos.o: src/protos.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/protos.o src/protos.c

${OBJECTDIR}/src/signal.o: src/signal.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/signal.o src/signal.c

${OBJECTDIR}/src/task.o: src/task.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/task.o src/task.c

${OBJECTDIR}/src/time.o: src/time.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/time.o src/time.c

${OBJECTDIR}/src/util.o: src/util.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/util.o src/util.c

${OBJECTDIR}/src/zeroserv.o: src/zeroserv.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/zeroserv.o src/zeroserv.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} bin/zeroserv

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
