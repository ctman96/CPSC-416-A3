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
testname='Test 6 Start a worker in TX commit, Start a new TX do multiple updates then abort.'
echo "========= $testname  ========="
make cleanlogs

echo "**Starting manager"
./tmanager ${tmanagerPort}  > /dev/null 2>&1 & 

echo "**Starting worker"
./tworker ${host_1_cmd} ${host_1_tx}  > /dev/null 2>&1 & 
sleep 1
echo "**First set of updates in a TX"
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 666
sleep 0.5
./cmd newa "${workerhost}" ${host_1_cmd} 660
sleep 0.5
./cmd newb "${workerhost}" ${host_1_cmd} -660
sleep 0.5
./cmd newid "${workerhost}" ${host_1_cmd} "Test 6 Initial values 660, -660"
sleep 0.5
echo "**First commit TX"
./cmd commit "${workerhost}" ${host_1_cmd}
sleep 2
echo -e "\n(1)**Transaction should not be active"
./checktx TXw*.log
./dumplog TXw*.log  > l1.log
echo " "
echo "**Updating values 2nd TX"
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 51 
./cmd newa "${workerhost}" ${host_1_cmd} 1660 
./cmd newb "${workerhost}" ${host_1_cmd} -1660 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 6 - new data 1660 -1660"
echo "**More updates"
./cmd newa "${workerhost}" ${host_1_cmd} 16602 
./cmd newb "${workerhost}" ${host_1_cmd} -16602 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 6 - 2nd updates"
./dumplog TXw*.log > l2.log
echo -e "\n(2)**Pre-abort TX state, TX active"
./checktx TXw*.log
echo -e "\n**Abort"
./cmd abort "${workerhost}" ${host_1_cmd}
sleep 1
killall -q -w  tworker tmanager

echo "(3)**Log should have values 660 -660, the initial values reported for First Commit "
echo " "
echo -n "**I Contents->"
cat l1.log
echo -n "**M Contents->"
cat l2.log
echo -n "**E Contents->"
./dumplog TXw*.log

echo " "
echo "**Final TX state, should not be in progress"
./checktx TXw*.log

echo "=========Done  $testname  ========="
