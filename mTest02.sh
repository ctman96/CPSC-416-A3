#!/usr/bin/env bash
killall -q -w   tmanager tworker checkpolling

host_1_cmd="49501"
host_1_tx="59601" 
  
host_2_cmd="49502"
host_2_tx="59602"
 
host_3_cmd="49503"
host_3_tx="59603"
 
host_4_cmd="49504"
host_4_tx="59604"
 
host_5_cmd="49505"
host_5_tx="59605"
 
host_6_cmd="49506"
host_6_tx="59606"
 
tmanagerPort="60213"

workerhost="127.0.0.1"
managerhost="127.0.0.1"

# ============================================== TEST ============================================== #
testname='Test 2 Start a worker with old data from Test 1 and abort'
echo " "
echo "========= $testname  ========="

./dumplog TXw*.log > start.log

echo "Starting worker"
./tworker ${host_1_cmd} ${host_1_tx}  > /dev/null 2>&1 & 

echo "Starting manager"
./tmanager ${tmanagerPort}  > /dev/null 2>&1 & 

echo " "; echo "(0)**Reading an old file the TX should not be in progress"
./checktx TXw*.log
sleep 2
echo " "
echo "Updating values"
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 111111 
./cmd newa "${workerhost}" ${host_1_cmd} 10 
./cmd newb "${workerhost}" ${host_1_cmd} -10 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 2 - Worker new data ${host_1_cmd}"

sleep 2
echo -e  "\n(1)** TX should be in progress and there should be updated values of 10 -10\n"
./checktx TXw*.log

./dumplog TXw*.log >middle.log
hexdump -C  TXw*.log > a1.log

echo "Aborting"
./cmd abort "${workerhost}" ${host_1_cmd}
sleep 3

killall -q -w  tworker tmanager

echo -e "\n(2)**Line E should have Line I contents\n"

hexdump -C  TXw*.log > a2.log
echo " "
echo -n "**I Contents->"
cat start.log
echo -n "**M Contents->"
cat middle.log
echo -n "**E Contents->"
./dumplog TXw*.log 
#echo "**Differences offset 58"
#diff a1.log a2.log
echo "**Final TX state should not be active"
./checktx TXw*.log
echo " "
echo "=========Done $testname  ========="

