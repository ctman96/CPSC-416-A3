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
 
 

cleanup() {
    make scrub 
    make 
    wait
}


workerhost="127.0.0.1"
managerhost="127.0.0.1"

# ============================================== TEST ============================================== #
testname='Test 3 Start a worker in a TX with old data and do multiple updates, then abort'
echo "========= $testname  ========="


echo "**Starting worker"
./tworker ${host_1_cmd} ${host_1_tx}  > /dev/null 2>&1 & 
sleep 4
echo "**Starting values"
./dumplog TXw*.log > d0.log



echo "**Starting manager"
./tmanager ${tmanagerPort}  > /dev/null 2>&1 & 

sleep 0.5
echo "**Updating values"
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 1 
./cmd newa "${workerhost}" ${host_1_cmd} 30 
./cmd newb "${workerhost}" ${host_1_cmd} -30 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 3 - Worker new data ${host_1_cmd}"
sleep 3
./dumplog TXw*.log > d1.log

./cmd newa "${workerhost}" ${host_1_cmd} 31 
./cmd newb "${workerhost}" ${host_1_cmd} -32 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 3 - Worker new data 2 ${host_1_cmd}"
sleep 0.5
./dumplog TXw*.log > d2.log

./cmd newa "${workerhost}" ${host_1_cmd} 33 
./cmd newb "${workerhost}" ${host_1_cmd} -34 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 3 - Worker new data 3 ${host_1_cmd}"

sleep 2;
./dumplog TXw*.log > d3.log
echo -e "(1)**Transaction should be in progress.\n"
./checktx TXw*.log
echo "**Abort"
./cmd abort "${workerhost}" ${host_1_cmd}
echo "**Sleeping 6"
sleep 6
killall -q -w  tmanager tworker

echo -e "\n(2)**Transaction values - last line should equal first\n"
echo -n "Initial values ->"
cat d0.log
echo -n "First update   ->"
cat d1.log
echo -n "Second update  ->"
cat d2.log
echo -n "Third update   ->"
cat d3.log
echo -n "After abort    ->"
./dumplog TXw*.log 
echo -e "\n(3)**Transaction should not be in progress.\n"

./checktx TXw*.log

echo "========= Done $testname  ========="
