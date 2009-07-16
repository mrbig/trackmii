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
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=

# Macros
PLATFORM=GNU-Linux-x86

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/Plugin/${PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/pose.o \
	${OBJECTDIR}/linux-track/wiimote_driver.o \
	${OBJECTDIR}/linux-track/tir4_driver.o \
	${OBJECTDIR}/gui.o \
	${OBJECTDIR}/_ext/home/mrbig/xtras/xplane/linux-track/src/cal.o \
	${OBJECTDIR}/trackmii_plugin.o

# C Compiler Flags
CFLAGS=-DLIN=1

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lcwiid -lm -lusb

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Plugin.mk dist/Plugin/${PLATFORM}/trackmii_plugin.xpl

dist/Plugin/${PLATFORM}/trackmii_plugin.xpl: ${OBJECTFILES}
	${MKDIR} -p dist/Plugin/${PLATFORM}
	${LINK.c} -shared -o dist/Plugin/${PLATFORM}/trackmii_plugin.xpl -s ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/pose.o: pose.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -s -I../SDK/CHeaders/XPLM -I../SDK/CHeaders/Widgets -MMD -MP -MF $@.d -o ${OBJECTDIR}/pose.o pose.c

${OBJECTDIR}/linux-track/wiimote_driver.o: linux-track/wiimote_driver.c 
	${MKDIR} -p ${OBJECTDIR}/linux-track
	${RM} $@.d
	$(COMPILE.c) -g -Wall -s -I../SDK/CHeaders/XPLM -I../SDK/CHeaders/Widgets -MMD -MP -MF $@.d -o ${OBJECTDIR}/linux-track/wiimote_driver.o linux-track/wiimote_driver.c

${OBJECTDIR}/linux-track/tir4_driver.o: linux-track/tir4_driver.c 
	${MKDIR} -p ${OBJECTDIR}/linux-track
	${RM} $@.d
	$(COMPILE.c) -g -Wall -s -I../SDK/CHeaders/XPLM -I../SDK/CHeaders/Widgets -MMD -MP -MF $@.d -o ${OBJECTDIR}/linux-track/tir4_driver.o linux-track/tir4_driver.c

${OBJECTDIR}/gui.o: gui.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -s -I../SDK/CHeaders/XPLM -I../SDK/CHeaders/Widgets -MMD -MP -MF $@.d -o ${OBJECTDIR}/gui.o gui.c

${OBJECTDIR}/_ext/home/mrbig/xtras/xplane/linux-track/src/cal.o: ../linux-track/src/cal.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/home/mrbig/xtras/xplane/linux-track/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -s -I../SDK/CHeaders/XPLM -I../SDK/CHeaders/Widgets -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/home/mrbig/xtras/xplane/linux-track/src/cal.o ../linux-track/src/cal.c

${OBJECTDIR}/trackmii_plugin.o: trackmii_plugin.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -s -I../SDK/CHeaders/XPLM -I../SDK/CHeaders/Widgets -MMD -MP -MF $@.d -o ${OBJECTDIR}/trackmii_plugin.o trackmii_plugin.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Plugin
	${RM} dist/Plugin/${PLATFORM}/trackmii_plugin.xpl

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
