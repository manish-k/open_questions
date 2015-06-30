#!/bin/bash

if [ $# -lt 1 ] ; then 
  echo -e "$0 soln_repo_dir [inputfile=$HOME/open_questions/algoques/prod_data_20131213v.txt] [our_repo_dir=$HOME/open_questions]\nSpecify complete file/dir-paths.\n"; 
  exit 0;
fi

soln_repo=$1; shift;
inputfile="$HOME/open_questions/algoques/prod_data_20131213v.txt";
our_repo="$HOME/open_questions";
if [ $# -ge 1 ] ; then inputfile=$1; shift; fi
if [ $# -ge 1 ] ; then our_repo=$1; shift; fi

cd $our_repo/algoques;
#git checkout .;
git checkout sample;
g++ -O3 process_data.cpp -o $HOME/our_soln;

diffcount=`diff -w $our_repo/algoques/process_data.cpp $soln_repo/algoques/process_data.cpp | wc -l`;
if [ $diffcount -gt 0 ] ; then 
  echo "$our_repo/algoques/process_data.cpp $soln_repo/algoques/process_data.cpp differs.";
  diff -w $our_repo/algoques/process_data.cpp $soln_repo/algoques/process_data.cpp;
fi

cd $soln_repo/algoques;
g++ -O3 process_data.cpp -o $HOME/soln;


cmd="$HOME/soln $our_repo/algoques/sample_random_forest.txt $inputfile $HOME/output 30 2>/dev/null";
echo $cmd;
$cmd 2>/dev/null;

cmd="$HOME/our_soln $our_repo/algoques/sample_random_forest.txt $inputfile $HOME/our_output 30 2>/dev/null";
echo $cmd;
$cmd 2>/dev/null;

#Removing outputs untill all indicators are updated atleast once
sed -i 1,26d $HOME/our_output;
sed -i 1,26d $HOME/output;

$our_repo/algoques/compute_diff.R $HOME/our_output $HOME/output;
