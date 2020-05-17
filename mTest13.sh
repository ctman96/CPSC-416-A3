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
testname="Test 13 -  3  workers, 1 orders commit crash - workers shoud all be told to prepare then go into poll mode"
echo "========= $testname  ========="
make cleanlogs

echo "**Starting manager"
./tmanager ${tmanagerPort}  > /dev/null 2>&1 & 
sleep 2
echo "**Starting workers"
./tworker ${host_1_cmd} ${host_1_tx}  > /dev/null 2>&1 &
 ./tworker ${host_3_cmd} ${host_3_tx}  > /dev/null 2>&1 &
 ./tworker ${host_2_cmd} ${host_2_tx}  > /dev/null 2>&1 &
 
sleep 1
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 1313
./cmd newa "${workerhost}" ${host_1_cmd} 13 
./cmd newb "${workerhost}" ${host_1_cmd} -13 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 13 - Worker ${host_1_cmd}" 
 
sleep 1
./cmd join "${workerhost}" ${host_2_cmd} "${managerhost}" ${tmanagerPort} 1313
./cmd newa "${workerhost}" ${host_2_cmd} 113 
./cmd newb "${workerhost}" ${host_2_cmd} -113 
./cmd newid "${workerhost}" ${host_2_cmd} "Test 13 - Worker ${host_1_cmd}"

 sleep 1
./cmd join "${workerhost}" ${host_3_cmd} "${managerhost}" ${tmanagerPort} 1313
./cmd newa "${workerhost}" ${host_3_cmd} 1113 
./cmd newb "${workerhost}" ${host_3_cmd} -1113 
./cmd newid "${workerhost}" ${host_3_cmd} "Test 13 - Worker ${host_1_cmd}"

sleep 3

./dumplog TXw*.log > l1.log

echo " "; echo "(1)**All workers should be in transaction"
./checktx TXw*.log
echo "Order Tx commitcrash, wait 7s"
./cmd commitcrash "${workerhost}" ${host_1_cmd}
sleep 7
killall -q -w  -q tmanager   #just in case
echo " "; echo "(2)**All workers should be in prepared and should be polling"
./checktx TXw*.log
./dumplog TXw*.log > l2.log
echo " "
./checkpolling ${tmanagerPort} &#  > poll.log 2>&1&
sleep 40
killall -q -w  checkpolling
echo "**Restarting manager"
./tmanager ${tmanagerPort}  > /dev/null 2>&1 &
sleep 15
echo " ";  echo "(3)**All TX should now be in finished state"
 ./checktx TXw*.log

echo " ";  echo "(4) All Sets should agree and have '13' theme to values"
echo "     Initial values themed on 12's"
cat l1.log
echo "     Values after 20s"
cat l2.log
echo "     Must agree with first group"
./dumplog TXw*.log
 killall -q -w  tworker tmanager
