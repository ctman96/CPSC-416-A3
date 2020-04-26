#!/usr/bin/env bash

pwd
echo " "

./mTest01.sh > out01.txt 2>&1
./mTest02.sh > out02.txt 2>&1
./mTest03.sh > out03.txt 2>&1
./mTest04.sh > out04.txt 2>&1
./mTest05.sh > out05.txt 2>&1
./mTest06.sh > out06.txt 2>&1
./mTest07.sh > out07.txt 2>&1
./mTest08.sh > out08.txt 2>&1
./mTest09.sh > out09.txt 2>&1
./mTest10.sh > out10.txt 2>&1
./mTest11.sh > out11.txt 2>&1
./mTest12.sh > out12.txt 2>&1
./mTest13.sh > out13.txt 2>&1
./mTest14.sh > out14.txt 2>&1

cat coverpage.txt compile.txt out0[0123456789].txt  > allTests.txt
cat out1[0123456789].txt >> allTests.txt
echo "**************************************"
