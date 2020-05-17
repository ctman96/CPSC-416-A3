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
testname='Test 5 Start a worker in TX do multiple updates then commit.'
echo "========= $testname  ========="
make cleanlogs

echo "**Starting worker"
./tworker ${host_1_cmd} ${host_1_tx}  > /dev/null 2>&1 & 


echo "**Starting manager"
./tmanager ${tmanagerPort}  &

sleep 1
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 50

echo "**First set of updates in a TX"
sleep 0.5
./checktx TXw*.log
./cmd newa "${workerhost}" ${host_1_cmd} 550
sleep 0.5
./cmd newb "${workerhost}" ${host_1_cmd} -550
sleep 0.5
./cmd newid "${workerhost}" ${host_1_cmd} "Test 5 Initial values 550, -550"
sleep 0.5


echo "**Updating values 2nd part of TX"
./cmd newa "${workerhost}" ${host_1_cmd} 551 
./cmd newb "${workerhost}" ${host_1_cmd} -551 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 5 - 2nd udpate data 551 -551"
echo "**More updates"
./cmd newa "${workerhost}" ${host_1_cmd} 553 
./cmd newb "${workerhost}" ${host_1_cmd} -553 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 5 - 3rd  update 553 -553"

sleep 2
echo -e "\n(1)**TX 50 should be in progress"
./checktx TXw*.log
echo "Commit"
./cmd commit "${workerhost}" ${host_1_cmd}
sleep 3
killall -q -w   tworker tmanager
sleep 1
 echo -e "\n(2)**TX values should be 553 -553 and TX should no longer be active"
./dumplog TXw*.log
echo -n "    "
./checktx TXw*.log

echo "=========Done $testname  ========="
