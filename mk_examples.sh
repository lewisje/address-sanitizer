#!/bin/bash

export ASAN_OPTIONS=symbolize=1:strip_path_prefix=$(pwd)/
for source in example_*.cc; do
  name=$(echo $source | sed 's/example_//g; s/\.cc$//g')
  run=$(grep RUN: $source | sed "s/.*RUN: //g; s/%t/$source/g")
  echo run: "$run"
  echo $name
  wiki="Example$name.wiki"
  rm -f $wiki
  printf "#summary Example: $name\n" >> $wiki
  printf "{{{\n"                     >> $wiki
  cat $source                        >> $wiki
  printf "}}}\n"                     >> $wiki

  echo "$run" > tmp

  printf "{{{\n"                     >> $wiki
  . tmp                              >> $wiki 2>&1
  printf "}}}\n"                     >> $wiki
  printf "Read CallStack about symolizing callstack\n" >> $wiki
  rm -f ./a.out tmp
done
