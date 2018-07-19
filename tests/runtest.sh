#!/bin/bash

SOURCEDIR=$1
BINDIR=$2
TESTNAME=$3

#TWDIR=${BINDIR}/micro_env ${BINDIR}/src/twflow/graywolf
TMPDIR=`mktemp -d`
cp -r ${SOURCEDIR}/tests/${TESTNAME} ${TMPDIR}/

pushd ${TMPDIR}/${TESTNAME}
TWDIR=${BINDIR}/micro_env ${BINDIR}/src/twflow/graywolf -n ${TESTNAME}


diff ${TESTNAME}.pl1 expected/${TESTNAME}.pl1
RET=$?

popd
rm -rf ${TMPDIR}

exit $RET
