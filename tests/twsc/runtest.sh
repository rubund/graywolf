#!/bin/bash

SOURCEDIR=$1
BINDIR=$2
TESTNAME=$3

#TWDIR=${BINDIR}/micro_env ${BINDIR}/src/twflow/graywolf
TMPDIR=`mktemp -d`
rsync ${SOURCEDIR}/tests/twsc/${TESTNAME} ${TMPDIR}/ -a --copy-links -v


pushd ${TMPDIR}/${TESTNAME}
TWDIR=${BINDIR}/micro_env ${BINDIR}/micro_env/bin/TimberWolfSC -n ${TESTNAME}


diff ${TESTNAME}.pl1 expected/${TESTNAME}.pl1
RET=$?

popd
#rm -rf ${TMPDIR}
echo ${TMPDIR}

exit $RET
