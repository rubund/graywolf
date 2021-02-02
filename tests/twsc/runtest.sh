#!/usr/bin/env bash

SOURCEDIR=$1
BINDIR=$2
TESTNAME=$3

#TWDIR=${BINDIR}/micro_env ${BINDIR}/src/twflow/graywolf
TMPDIR=`mktemp -d`
rsync ${SOURCEDIR}/tests/twsc/${TESTNAME} ${TMPDIR}/ -a --copy-links -v


pushd ${TMPDIR}/${TESTNAME}
TWDIR=${BINDIR}/micro_env ${BINDIR}/micro_env/bin/TimberWolfSC -n ${TESTNAME}


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

diff -Nau ${TESTNAME}.pth expected/${TESTNAME}.pth
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.pin expected/${TESTNAME}.pin
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.sav expected/${TESTNAME}.sav
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

echo "Just showing diff for. out:"
diff -Nau ${TESTNAME}.out expected/${TESTNAME}.out


if [ "$#" = "4" ] && [ "$4" == "1" ] ; then
  cp * ${SOURCEDIR}/tests/twsc/${TESTNAME}/expected/
  touch ${SOURCEDIR}/tests/twsc/${TESTNAME}/expected/updated
fi

popd
rm -rf ${TMPDIR}
#echo ${TMPDIR}

exit $RET
