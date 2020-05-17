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
# Start worker and coordinator. Make some changes and then commit. 
testname='Test 1 Simple commit'
echo " "
echo "========= $testname  ======================="
make cleanlogs

echo "Starting worker"
./tworker ${host_1_cmd} ${host_1_tx}  > /dev/null 2>&1 & 

echo "Starting manager"
./tmanager ${tmanagerPort}  > /dev/null 2>&1 & 

sleep 0.5
./cmd begin "${workerhost}" ${host_1_cmd} ${managerhost} ${tmanagerPort} 33153
./cmd newa "${workerhost}" ${host_1_cmd} 1 
./cmd newb "${workerhost}" ${host_1_cmd} -1 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 1 - values 1 -1 Worker ${host_1_cmd}"

sleep 2

#./dumplog TXw*.log
hexdump -C TXw*.log >l1.log

echo -e "\n(1)**Transaction 33153 is active and values are 1 and -1\n"
./dumplog TXw*.log
echo "Order Tx committed"
./checktx TXw*.log
./cmd commit "${workerhost}" ${host_1_cmd}
sleep 3

echo " "
killall -q -w  tworker tmanager

hexdump -C TXw*.log >l2.log
wait
echo -e "\n(2)**Check should indicate that the TX is not in progress and values are 1 and -1\n"
#diff l1.log l2.log
./dumplog TXw*.log
./checktx TXw*.log

echo " "
echo "=========Done:  $testname  ========="
