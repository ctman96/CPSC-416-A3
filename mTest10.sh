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
testname='Test 10 - Mutltiple workers 2 TX, 1 TX commits, 2nd tx does multiple updates and then commits'
echo "========= $testname  ========="
make cleanlogs


echo "**Starting manager"
./tmanager ${tmanagerPort} > /dev/null 2>&1 & 

sleep 1
echo "**Starting workers"
./tworker ${host_1_cmd} ${host_1_tx} & #  > /dev/null 2>&1 &
./tworker ${host_2_cmd} ${host_2_tx} & # > /dev/null 2>&1 &
./tworker ${host_3_cmd} ${host_3_tx} & # > /dev/null 2>&1 &
./tworker ${host_4_cmd} ${host_4_tx} & # > /dev/null 2>&1 & 
sleep 1

sleep 0.5
./cmd begin "${workerhost}" ${host_1_cmd} "${managerhost}" ${tmanagerPort} 13 
./cmd newa "${workerhost}" ${host_1_cmd} 13 
./cmd newb "${workerhost}" ${host_1_cmd} -13 
./cmd newid "${workerhost}" ${host_1_cmd} "Test 13 - Worker ${host_1_cmd}"
sleep .5

./cmd join "${workerhost}" ${host_2_cmd} "${managerhost}" ${tmanagerPort} 13 
./cmd newa "${workerhost}" ${host_2_cmd} -13 
./cmd newb "${workerhost}" ${host_2_cmd} 13 
./cmd newid "${workerhost}" ${host_2_cmd} "Test 13 - Worker ${host_2_cmd}"
sleep .5

./cmd begin "${workerhost}" ${host_3_cmd} "${managerhost}" ${tmanagerPort} 131313 
./cmd newa "${workerhost}" ${host_3_cmd} 26 
./cmd newb "${workerhost}" ${host_3_cmd} -26 
./cmd newid "${workerhost}" ${host_3_cmd} "Test 13 - Worker ${host_3_cmd}"
sleep .5


./cmd join "${workerhost}" ${host_4_cmd} "${managerhost}" ${tmanagerPort} 131313 
./cmd newa "${workerhost}" ${host_4_cmd} -26 
./cmd newb "${workerhost}" ${host_4_cmd} 26  
./cmd newid "${workerhost}" ${host_4_cmd} "Test 13 - Worker ${host_4_cmd}"
sleep .5

echo "(1)**Pre-commit TX state"
./checktx TXw*.log
./dumplog TXw*.log > l1.log


echo -e "\nOrder Tx 10 committed"
./cmd commit "${workerhost}" ${host_1_cmd}
sleep 3
echo " "
echo "(2)**Two workers should be finished and 2 still active"
./checktx TXw*.log
./dumplog TXw*.log > l2.log

echo " "
echo "More updates to TX 26"
./cmd newa "${workerhost}" ${host_3_cmd} 260 
./cmd newb "${workerhost}" ${host_3_cmd} -260 
./cmd newid "${workerhost}" ${host_3_cmd} "Test 13 - Worker ${host_3_cmd} Update 2"
./cmd newa "${workerhost}" ${host_4_cmd} -260 
./cmd newb "${workerhost}" ${host_4_cmd} 260  
./cmd newid "${workerhost}" ${host_4_cmd} "Test 13 - Worker ${host_4_cmd} Update 2"

echo "Order Tx 26 committed - sleep 32"
./cmd  commit "${workerhost}" ${host_3_cmd}
sleep 5
killall -q -w  tmanager tworker


echo " "
echo "(3)**All workers should be finished"
./checktx TXw*.log
echo " "
cat l1.log
echo " "
cat l2.log
echo " "
./dumplog TXwo*.log

echo "=========Done $testname  ========="
