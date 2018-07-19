#!/bin/sh

rm -rf $2/micro_env
mkdir -p $2/micro_env
mkdir -p $2/micro_env/bin
cp $2/src/mincut/Mincut $2/micro_env/bin/
cp $2/src/twmc/TimberWolfMC $2/micro_env/bin/
cp $2/src/twsc/TimberWolfSC $2/micro_env/bin/
cp $2/src/genrows/genrows $2/micro_env/bin/
cp $2/src/mc_compact/mc_compact $2/micro_env/bin/
cp $2/src/syntax/syntax $2/micro_env/bin/
cp $2/script/show_flows $2/micro_env/bin/
cp $1/script/splt_file.a $2/micro_env/bin/
cp -r $1/flow $2/micro_env/bin/
ln -sf flow.noroute $2/micro_env/bin/flow/flow
