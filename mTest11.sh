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
testname='Test 11 Multiple workers in a TX, abortcrash'
echo "========= $testname  ========="
make cleanlogs

echo "**Starting manager"
./tmanager ${tmanagerPort}  > /dev/null 2>&1 &

echo "**Starting worker 1"
./tworker ${host_1_cmd} ${host_1_tx}  > /dev/null 2>&1 & 
sleep 1

echo "**Starting worker 2"
./tworker ${host_2_cmd} ${host_2_tx}  > /dev/null 2>&1 & 

sleep 1

echo "**Starting worker 3"
./tworker ${host_3_cmd} ${host_3_tx}  > /dev/null 2>&1 & 

sleep 1

echo "**Everyone joins the TX"
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 70
./cmd join "${workerhost}" ${host_2_cmd} "${managerhost}" ${tmanagerPort} 70
./cmd join "${workerhost}" ${host_3_cmd} "${managerhost}" ${tmanagerPort} 70

echo "Setting worker 1 values"
./cmd newa "${workerhost}" ${host_1_cmd} 11770
./cmd newb "${workerhost}" ${host_1_cmd} -11770
./cmd newid "${workerhost}" ${host_1_cmd} "Test 11 Initial values 11770, -11770, ${host_1_cmd}"

echo "Setting worker 2 values"
./cmd newa "${workerhost}" ${host_2_cmd} 11771
./cmd newb "${workerhost}" ${host_2_cmd} -11771
./cmd newid "${workerhost}" ${host_2_cmd} "Test 11 Initial values 11771, -11771, ${host_2_cmd}"

echo "Setting worker 3 values"
./cmd newa "${workerhost}" ${host_3_cmd} 11772
./cmd newb "${workerhost}" ${host_3_cmd} -11772
./cmd newid "${workerhost}" ${host_3_cmd} "Test 11 Initial values 11772, -11772, ${host_3_cmd}"

sleep 2
echo " "; echo "(1)**Pre abortCrash  state, all workers active"
./checktx TXw*.log

echo " "; echo "**Abort Crash"
./cmd abortcrash "${workerhost}" ${host_2_cmd} 
sleep 2
echo " "; echo "(2)**Post abort TX state workers could be in all manner of states"
./checktx TXw*.log

echo " "; echo "**TX manager is dead workers may be waiting for a decision"
./dumplog TXw*.log > log1.log
echo "**Waiting 31 seconds, some workers could have aborted"
sleep 32
./checktx TXw*.log
echo " "; echo "**Restarting coordinator"
./tmanager ${tmanagerPort}  > /dev/null 2>&1 &

echo "Waiting one timeout round"
sleep 31
echo "Workers may have polled for decision and got abort"
echo "But in case they haven't have them commit, and should get abort"
./cmd commit "${workerhost}" ${host_1_cmd}
./cmd commit "${workerhost}" ${host_3_cmd}
sleep 3
killall -q -w  tworker tmanager
wait
echo " "; echo "(3) All workers should have aborted"
./checktx TXw*.log

echo " "; echo "(4) First group should show initial or partial logs"
cat log1.log
echo " " ; echo "    **Logs should have 0s and null strings"
./dumplog TXw*.log

echo "========= $testname  ========="
