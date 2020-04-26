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
testname='Test 9 Multiple workers in a TX, one worker dies, then commit, worker is restarted'
echo "========= $testname  ========="
make cleanlogs

echo "**Starting manager"
./tmanager ${tmanagerPort}  > /dev/null 2>&1 &


echo "**Starting worker 1"
./tworker ${host_1_cmd} ${host_1_tx}  > /dev/null 2>&1 & 

echo "**Starting worker 2"
./tworker ${host_2_cmd} ${host_2_tx}  > /dev/null 2>&1 & 

echo "**Starting worker 3"
./tworker ${host_3_cmd} ${host_3_tx}  > /dev/null 2>&1 & 
worker3=$!
sleep 1

echo "**Everyone joins the TX"
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 90
./cmd join "${workerhost}" ${host_2_cmd} "${managerhost}" ${tmanagerPort} 90
./cmd join "${workerhost}" ${host_3_cmd} "${managerhost}" ${tmanagerPort} 90

echo "Setting worker 1 values"
./cmd newa "${workerhost}" ${host_1_cmd} 880
./cmd newb "${workerhost}" ${host_1_cmd} -990
./cmd newid "${workerhost}" ${host_1_cmd} "Test 9 Initial values 990, -990, ${host_1_cmd}"

echo "Setting worker 2 values"
./cmd newa "${workerhost}" ${host_2_cmd} 991
./cmd newb "${workerhost}" ${host_2_cmd} -991
./cmd newid "${workerhost}" ${host_2_cmd} "Test 9 Initial values 991, -991, ${host_2_cmd}"

echo "Setting worker 3 values"
./cmd newa "${workerhost}" ${host_3_cmd} 992
./cmd newb "${workerhost}" ${host_3_cmd} -992
./cmd newid "${workerhost}" ${host_3_cmd} "Test 9 Initial values 992, -992, ${host_3_cmd}"

sleep 3
echo -e "\n(1)**Pre Commit  states - all in progress"
./checktx TXw*.log

sleep 2

echo -e "\nWorker 3 crashes"
./cmd crash "${workerhost}" ${host_3_cmd} 
kill -9 ${worker3}
sleep 2
echo "**Commit ordered by worker 1, waiting 32 seconds"
./cmd commit "${workerhost}" ${host_1_cmd} 
sleep 32
killall -q -w  tworker
echo " "
echo "(2)**TX states  2 should be aborted/inactive worker 3  should be in progress"
./checktx TXw*.log

#echo "**Logs should have 0s and null strings, except for ${host_3_cmd}"
./dumplog TXw*.log >l1.log
echo "**Restart worker 3"

./tworker ${host_3_cmd} ${host_3_tx}  > /dev/null 2>&1 & 

sleep 4
killall -q -w  tworker tmanager
wait
echo " " 
echo "(3)**All TX should be abort"
./checktx TXw*.log
echo 
echo "(4)**First 2 0s null,  3rd TX of crashed worker, Last 3 all aborted"
cat l1.log
echo " " 
./dumplog TXw*.log


echo "=========Done: $testname  ========="
