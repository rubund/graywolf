#!/bin/bash

SOURCEDIR=$1
BINDIR=$2
TESTNAME=$3

#TWDIR=${BINDIR}/micro_env ${BINDIR}/src/twflow/graywolf
TMPDIR=`mktemp -d`
rsync ${SOURCEDIR}/tests/${TESTNAME} ${TMPDIR}/ -a --copy-links -v

pushd ${TMPDIR}/${TESTNAME}
TWDIR=${BINDIR}/micro_env ${BINDIR}/src/twflow/graywolf -n ${TESTNAME}


diff -Nau ${TESTNAME}.pl1 expected/${TESTNAME}.pl1
RET=$?

popd
rm -rf ${TMPDIR}

exit $RET
