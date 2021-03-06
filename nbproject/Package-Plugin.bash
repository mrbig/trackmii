#!/bin/bash -x

#
# Generated - do not edit!
#

# Macros
TOP=`pwd`
PLATFORM=GNU-Linux-x86
TMPDIR=build/Plugin/${PLATFORM}/tmp-packaging
TMPDIRNAME=tmp-packaging
OUTPUT_PATH=dist/Plugin/${PLATFORM}/trackmii_plugin.xpl
OUTPUT_BASENAME=trackmii_plugin.xpl
PACKAGE_TOP_DIR=trackmii/

# Functions
function checkReturnCode
{
    rc=$?
    if [ $rc != 0 ]
    then
        exit $rc
    fi
}
function makeDirectory
# $1 directory path
# $2 permission (optional)
{
    mkdir -p "$1"
    checkReturnCode
    if [ "$2" != "" ]
    then
      chmod $2 "$1"
      checkReturnCode
    fi
}
function copyFileToTmpDir
# $1 from-file path
# $2 to-file path
# $3 permission
{
    cp "$1" "$2"
    checkReturnCode
    if [ "$3" != "" ]
    then
        chmod $3 "$2"
        checkReturnCode
    fi
}

# Setup
cd "${TOP}"
mkdir -p dist/Plugin/${PLATFORM}/package
rm -rf ${TMPDIR}
mkdir -p ${TMPDIR}

# Copy files and create directories and links
cd "${TOP}"
makeDirectory ${TMPDIR}/trackmii
copyFileToTmpDir "${OUTPUT_PATH}" "${TMPDIR}/${PACKAGE_TOP_DIR}${OUTPUT_BASENAME}" 0755

cd "${TOP}"
makeDirectory ${TMPDIR}/trackmii
copyFileToTmpDir "README" "${TMPDIR}/${PACKAGE_TOP_DIR}README" 0644

cd "${TOP}"
makeDirectory ${TMPDIR}/trackmii
copyFileToTmpDir "AUTHORS" "${TMPDIR}/${PACKAGE_TOP_DIR}AUTHORS" 0644

cd "${TOP}"
makeDirectory ${TMPDIR}/trackmii
copyFileToTmpDir "COPYING" "${TMPDIR}/${PACKAGE_TOP_DIR}COPYING" 0644


# Generate tar file
cd "${TOP}"
rm -f dist/Plugin/${PLATFORM}/package/trackmii.tar.gz
cd ${TMPDIR}
tar -vzcf ../../../../dist/Plugin/${PLATFORM}/package/trackmii.tar.gz *
checkReturnCode

# Cleanup
cd "${TOP}"
rm -rf ${TMPDIR}
