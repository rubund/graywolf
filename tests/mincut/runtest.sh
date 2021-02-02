#!/usr/bin/env bash

SOURCEDIR=$1
BINDIR=$2
TESTNAME=$3

#TWDIR=${BINDIR}/micro_env ${BINDIR}/src/twflow/graywolf
TMPDIR=`mktemp -d`
rsync ${SOURCEDIR}/tests/mincut/${TESTNAME} ${TMPDIR}/ -a --copy-links -v


pushd ${TMPDIR}/${TESTNAME}
TWDIR=${BINDIR}/micro_env ${BINDIR}/micro_env/bin/Mincut ${TESTNAME}

RET=0
diff ${TESTNAME}.scel expected/${TESTNAME}.scel
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi
diff ${TESTNAME}.mcel expected/${TESTNAME}.mcel
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi
diff ${TESTNAME}.stat expected/${TESTNAME}.stat
RETPART=$?
if [ "$RETPART" != "0" ] ; then
  RET=-1
fi

if [ "$#" = "4" ] && [ "$4" == "1" ] ; then
  cp * ${SOURCEDIR}/tests/mincut/${TESTNAME}/expected/
  touch ${SOURCEDIR}/tests/mincut/${TESTNAME}/expected/updated
fi

popd
rm -rf ${TMPDIR}
#echo ${TMPDIR}

exit $RET
