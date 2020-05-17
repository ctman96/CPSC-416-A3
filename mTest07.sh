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
testname='Test 7 Multiple workers in a TX'
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
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 777
./cmd join "${workerhost}" ${host_2_cmd} "${managerhost}" ${tmanagerPort} 777
./cmd join "${workerhost}" ${host_3_cmd} "${managerhost}" ${tmanagerPort} 777

echo "Setting worker 1 values"
./cmd newa "${workerhost}" ${host_1_cmd} 770
./cmd newb "${workerhost}" ${host_1_cmd} -770
./cmd newid "${workerhost}" ${host_1_cmd} "Test 7 Initial values 770, -770, ${host_1_cmd}"

echo "Setting worker 2 values"
./cmd newa "${workerhost}" ${host_2_cmd} 771
./cmd newb "${workerhost}" ${host_2_cmd} -771
./cmd newid "${workerhost}" ${host_2_cmd} "Test 7 Initial values 771, -771, ${host_2_cmd}"

echo "Setting worker 3 values"
./cmd newa "${workerhost}" ${host_3_cmd} 772
./cmd newb "${workerhost}" ${host_3_cmd} -772
./cmd newid "${workerhost}" ${host_3_cmd} "Test 7 Initial values 772, -772, ${host_3_cmd}"
echo -e "\n(1)**Precommit TX states"
./checktx TXw*.log

sleep 2

echo "**Commit"
./cmd commit "${workerhost}" ${host_2_cmd} 
sleep 2
killall -q -w  tworker tmanager
wait
echo -e "\n(2)** All tx should not be in progress"
./checktx TXw*.log
echo -e "\n(3)**Logs should have 77[012] and -77[012]"
./dumplog TXw*.log
echo " "
echo "========= $testname  ========="
