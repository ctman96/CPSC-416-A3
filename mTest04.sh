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
testname='Start a worker in TX, kill coordinator, commit, wait 35 seconds. TX should have aborted and rolled back'
echo "========= $testname  ========="
make cleanlogs

echo "**Starting worker"
./tworker ${host_1_cmd} ${host_1_tx}  > /dev/null 2>&1 & 

echo "**Starting manager"
./tmanager ${tmanagerPort}  > /dev/null 2>&1 & 

echo "**First set of updates in a TX"
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 222
sleep 0.5
./cmd newa "${workerhost}" ${host_1_cmd} 444
sleep 0.5
./cmd newb "${workerhost}" ${host_1_cmd} -445
sleep 0.5
./cmd newid "${workerhost}" ${host_1_cmd} "Test 4 Initial values 440, -440"
sleep 0.5
echo "**First commit TX"
./cmd commit "${workerhost}" ${host_1_cmd}
sleep 4
#echo "**Log file should have initial committed values 440, -440"
./dumplog TXw*.log > l1.log
echo -e "\n(1)**Transaction should not be in progress\n"
./checktx TXw*.log
echo "**Updating values 2nd TX"
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 333 
sleep 1
echo "**Kill Coordinator"
killall -q -w  tmanager
./cmd newa "${workerhost}" ${host_1_cmd} 40 
./cmd newb "${workerhost}" ${host_1_cmd} -40 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 4 - new data 40 -40"

sleep 1

echo "(2)**Transaction 333 should be in progress"
./checktx TXw*.log
echo "**Commit transaction 333"
./cmd commit "${workerhost}" ${host_1_cmd}
sleep 4
echo "(3)**Post-commit state - should be in uncertain state or abort"
./checktx TXw*.log
#echo "**Kill worker"
#killall -q -w  tworker
#echo "**Log should have new values"
./dumplog TXw*.log > l2.log
#echo "**Restart coordinator"
#./tmanager ${tmanagerPort}  > /dev/null 2>&1 & 

#sleep 3
#echo "**Restart worker"
#./tworker ${host_1_cmd} ${host_1_tx}  > /dev/null 2>&1 & 
echo "Sleeping 41 seconds"
sleep 41
echo "TX should have automatically aborted and rolled back"
echo "**Stop worker "
killall -q -w  tworker 
sleep 3
echo "(4)**TX should be aborted or not in progress"
./checktx TXw*.log
./dumplog TXw*.log > log3.log
echo -e "\n(5)**Transaction values - last line should equal first, 444, -445\n"
echo -n "Initial values ->"
cat l1.log
echo -n "First update   ->"
cat l2.log
echo -n "After abort    ->"
./dumplog TXw*.log

echo "=========Done $testname  ========="
