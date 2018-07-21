#!/bin/bash

SOURCEDIR=$1
BINDIR=$2
TESTNAME=$3

#TWDIR=${BINDIR}/micro_env ${BINDIR}/src/twflow/graywolf
TMPDIR=`mktemp -d`
rsync ${SOURCEDIR}/tests/${TESTNAME} ${TMPDIR}/ -a --copy-links -v

pushd ${TMPDIR}/${TESTNAME}
TWDIR=${BINDIR}/micro_env ${BINDIR}/src/twflow/graywolf -n ${TESTNAME}

RET=0

diff -Nau ${TESTNAME}.pl1 expected/${TESTNAME}.pl1
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.pl2 expected/${TESTNAME}.pl2
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi


if [ "$#" = "4" ] && [ "$4" == "1" ] ; then
  cp * ${SOURCEDIR}/tests/${TESTNAME}/expected/
  touch ${SOURCEDIR}/tests/${TESTNAME}/expected/updated
fi

popd
rm -rf ${TMPDIR}

exit $RET
