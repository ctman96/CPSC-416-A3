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
testname='Test 8 Multiple workers in a TX, one worker votes abort'
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
sleep 2

echo "**Everyone joins the TX"
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 80
./cmd join "${workerhost}" ${host_2_cmd} "${managerhost}" ${tmanagerPort} 80
./cmd join "${workerhost}" ${host_3_cmd} "${managerhost}" ${tmanagerPort} 80

sleep 2
echo "Setting worker 1 values"
./cmd newa "${workerhost}" ${host_1_cmd} 880
./cmd newb "${workerhost}" ${host_1_cmd} -880
./cmd newid "${workerhost}" ${host_1_cmd} "Test 8 Initial values 880, -880, ${host_1_cmd}"

echo "Setting worker 2 values"
./cmd newa "${workerhost}" ${host_2_cmd} 881
./cmd newb "${workerhost}" ${host_2_cmd} -881
./cmd newid "${workerhost}" ${host_2_cmd} "Test 8 Initial values 881, -881, ${host_2_cmd}"

echo "Setting worker 3 values"
./cmd newa "${workerhost}" ${host_3_cmd} 882
./cmd newb "${workerhost}" ${host_3_cmd} -882
./cmd newid "${workerhost}" ${host_3_cmd} "Test 8 Initial values 882, -882, ${host_3_cmd}"

sleep 2
echo " "
echo "(1)**Pre abort - TX state in progress"
./checktx TXw*.log
./dumplog TXw*.log > l1.log
echo " "
echo "Worker 3 ordered to vote abort"
./cmd voteabort "${workerhost}" ${host_3_cmd} 
sleep 2
echo -e "\n(2)**Trasactions should be in progress except maybe 3\n"
./checktx TXw*.log
echo "**Commit ordered by worker 1"
./cmd commit "${workerhost}" ${host_1_cmd} 
sleep 2
killall -q -w  tworker tmanager
sleep 2
echo -e "\n(3)**Worker 3 ordered to abort"
./checktx TXw*.log
echo " "
echo "(4)**Logs should have [+-]88[012] and then end with 0s and null strings"
cat l1.log
./dumplog TXw*.log
echo "=========Done  $testname  ========="
