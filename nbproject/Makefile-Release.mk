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
OBJECTFILES=


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

${OBJECTDIR}/src/cfg.c.gch: src/cfg.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/cfg.c

${OBJECTDIR}/src/client.c.gch: src/client.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/client.c

${OBJECTDIR}/src/common/ticks.c.gch: src/common/ticks.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/common/ticks.c

${OBJECTDIR}/src/dbg.c.gch: src/dbg.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/dbg.c

${OBJECTDIR}/src/ebtree/eb32tree.c.gch: src/ebtree/eb32tree.c 
	${MKDIR} -p ${OBJECTDIR}/src/ebtree
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/ebtree/eb32tree.c

${OBJECTDIR}/src/ebtree/ebtree.c.gch: src/ebtree/ebtree.c 
	${MKDIR} -p ${OBJECTDIR}/src/ebtree
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/ebtree/ebtree.c

${OBJECTDIR}/src/fd.c.gch: src/fd.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/fd.c

${OBJECTDIR}/src/handler.c.gch: src/handler.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler.c

${OBJECTDIR}/src/handler/echo.c.gch: src/handler/echo.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/echo.c

${OBJECTDIR}/src/handler/http.c.gch: src/handler/http.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/http.c

${OBJECTDIR}/src/handler/mc/assoc.c.gch: src/handler/mc/assoc.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/mc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/mc/assoc.c

${OBJECTDIR}/src/handler/mc/fat/slabs.c.gch: src/handler/mc/fat/slabs.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/mc/fat
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/mc/fat/slabs.c

${OBJECTDIR}/src/handler/mc/fat/sto_layer.c.gch: src/handler/mc/fat/sto_layer.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/mc/fat
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/mc/fat/sto_layer.c

${OBJECTDIR}/src/handler/mc/items.c.gch: src/handler/mc/items.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/mc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/mc/items.c

${OBJECTDIR}/src/handler/memcached.c.gch: src/handler/memcached.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/memcached.c

${OBJECTDIR}/src/handler/pubsub.c.gch: src/handler/pubsub.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/pubsub.c

${OBJECTDIR}/src/handler/pubsub/account.c.gch: src/handler/pubsub/account.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/pubsub
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/pubsub/account.c

${OBJECTDIR}/src/handler/pubsub/idzen.c.gch: src/handler/pubsub/idzen.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/pubsub
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/pubsub/idzen.c

${OBJECTDIR}/src/handler/pubsub/model.c.gch: src/handler/pubsub/model.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/pubsub
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/pubsub/model.c

${OBJECTDIR}/src/handler/pubsub/topic.c.gch: src/handler/pubsub/topic.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler/pubsub
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler/pubsub/topic.c

${OBJECTDIR}/src/handler_udp/echo_udp.c.gch: src/handler_udp/echo_udp.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler_udp/echo_udp.c

${OBJECTDIR}/src/handler_udp/routing.c.gch: src/handler_udp/routing.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler_udp/routing.c

${OBJECTDIR}/src/handler_udp/routing/local_port.c.gch: src/handler_udp/routing/local_port.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp/routing
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler_udp/routing/local_port.c

${OBJECTDIR}/src/handler_udp/routing/mineip.c.gch: src/handler_udp/routing/mineip.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp/routing
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler_udp/routing/mineip.c

${OBJECTDIR}/src/handler_udp/routing/proto_routing.c.gch: src/handler_udp/routing/proto_routing.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp/routing
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler_udp/routing/proto_routing.c

${OBJECTDIR}/src/handler_udp/routing/route_map.c.gch: src/handler_udp/routing/route_map.c 
	${MKDIR} -p ${OBJECTDIR}/src/handler_udp/routing
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/handler_udp/routing/route_map.c

${OBJECTDIR}/src/poller.c.gch: src/poller.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/poller.c

${OBJECTDIR}/src/proto_tcp.c.gch: src/proto_tcp.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/proto_tcp.c

${OBJECTDIR}/src/proto_udp.c.gch: src/proto_udp.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/proto_udp.c

${OBJECTDIR}/src/protos.c.gch: src/protos.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/protos.c

${OBJECTDIR}/src/signal.c.gch: src/signal.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/signal.c

${OBJECTDIR}/src/task.c.gch: src/task.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/task.c

${OBJECTDIR}/src/time.c.gch: src/time.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/time.c

${OBJECTDIR}/src/util.c.gch: src/util.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/util.c

${OBJECTDIR}/src/zeroserv.c.gch: src/zeroserv.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o "$@" src/zeroserv.c

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
