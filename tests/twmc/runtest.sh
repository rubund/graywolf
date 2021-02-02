#!/usr/bin/env bash

SOURCEDIR=$1
BINDIR=$2
TESTNAME=$3

#TWDIR=${BINDIR}/micro_env ${BINDIR}/src/twflow/graywolf
TMPDIR=`mktemp -d`
rsync ${SOURCEDIR}/tests/twmc/${TESTNAME} ${TMPDIR}/ -a --copy-links -v


pushd ${TMPDIR}/${TESTNAME}
TWDIR=${BINDIR}/micro_env ${BINDIR}/micro_env/bin/TimberWolfMC -n ${TESTNAME}

RET=0

diff -Nau ${TESTNAME}.blk expected/${TESTNAME}.blk
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.gen expected/${TESTNAME}.gen
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.gsav expected/${TESTNAME}.gsav
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.mdat expected/${TESTNAME}.mdat
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.mgeo expected/${TESTNAME}.mgeo
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.mpin expected/${TESTNAME}.mpin
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.mpth expected/${TESTNAME}.mpth
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.msav expected/${TESTNAME}.msav
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.mver expected/${TESTNAME}.mver
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.mvio expected/${TESTNAME}.mvio
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

diff -Nau ${TESTNAME}.scel expected/${TESTNAME}.scel
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

if [ "$#" = "4" ] && [ "$4" == "1" ] ; then
  cp * ${SOURCEDIR}/tests/twmc/${TESTNAME}/expected/
  touch ${SOURCEDIR}/tests/twmc/${TESTNAME}/expected/updated
fi

popd
rm -rf ${TMPDIR}
#echo ${TMPDIR}

exit $RET
