
  <div class="description user_content student-version enhanced"><h2><strong><span style="color: #ff0000;"><span style="background-color: #ffffff;"><span style="color: #993300;">Follow the instructions</span> <a style="background-color: #ffffff;" title="Assignment Policies and Procedures" href="/courses/36019/pages/assignment-policies-and-procedures" data-api-endpoint="https://canvas.ubc.ca/api/v1/courses/36019/pages/assignment-policies-and-procedures" data-api-returntype="Page">here</a> <span style="color: #993300;">to create your a3 repo. </span></span><br></span></strong></h2>
<p>Updates:</p>
<ul>
<li>
<strong>March 19: </strong>To put this all in one location, here the things you can and cannot change with respect to the structs:
<ul>
<li>You cannot change the types or format of the packets sent between the&nbsp; <strong>cmd</strong> program and a worker.</li>
<li>You can change anything in tmanager.h as long as you don't affect the coordinator's ability to handle 4 transactions with up to 6 workers each.&nbsp;</li>
<li>In tworker.h you cannot change&nbsp; transactionData nor can you change the first two fields of logFile. (We will write programs to read your worker's log but we will only care about the first two fields of the logFile struct since that represents the variables that are involved in the transaction.</li>
<li>You can add new values to the enum workerTxState, provided they are at the end.</li>
</ul>
</li>
<li>
<strong>March 19: </strong>Clarified description that the worker takes arguments for 2 port numbers.</li>
<li>
<strong>March 19: </strong>For command 9 - delay - the example has been corrected to indicate that the worker crashes before sending the response but after doing all the other work.&nbsp;</li>
<li>
<strong>March 19</strong>: The program cmd.c has a couple of&nbsp; bugs.&nbsp;
<ul>
<li>The parsing for the command begin should look at argv[1] not argv[0].</li>
<li>The delay command fails to include the amount to delay. For the delay command add the code</li>
</ul>
</li>
</ul>
<pre style="padding-left: 80px;">msg-&gt;delay = atoi(argv[4]);&nbsp;</pre>
<ul>
<li style="list-style-type: none;">
<ul>
<li style="list-style-type: none;">
<ul>
<li>just before the sendmessage</li>
</ul>
</li>
<li>These fixes have been pushed to the repo so you "might" get them if you do a pull and a merge. If not just add them by hand.&nbsp;</li>
</ul>
</li>
<li>
<strong>March 19: </strong>As the assignment says the protocol between the worker and coordinator is a design decision on your part. Should the protocol require a response, for example a BEGIN is sent to the coordinator and your design requires an OK or NO if the transaction ID is in progress, you may assume that the underlying UDP send is reliable. In addition the rule, that if a response isn't received within 10 seconds you can assume a crash,&nbsp; applies.</li>
<li>
<strong>March 19:</strong> Command 10, abort, means that the worker immediately tells the coordinator to abort the transaction. (i.e. it sends abort to the coordinator.) The coordinator will immediately undertake the actions to abort this transaction at all the workers.) If the worker is only to abort the transaction locally and not inform the coordinator use command 12, voteabort.)</li>
<li>
<strong>March 14</strong>: Removed requirement to submit Vector clock logs. If you see anything referring to vector clocks that hasn't been expunged from the assignment you can safely ignore it.</li>
<li>
<strong>March 14:</strong> Fixed the due date to March 28th.&nbsp;</li>
</ul>
<p>IMPORTANT: This assignment may be done in pairs and you are strongly encouraged to do that. Late assignments penalized at 33.333% per day pro-rated. Late assignments not accepted after 2 days. In addition your program must compile without warnings or errors on the departments Linux servers in the student domain. You are not permitted to change compiler options or add directives in the code to disable warnings.</p>
<p>In this assignment you will be implementation a distributed transaction management system using two phase commit.&nbsp;</p>
<p>You are required to write 2 programs as part of this assignment:</p>
<ol>
<li>The transaction manager or coordinator: tmanager</li>
<li>A worker: tworker</li>
</ol>
<p>A third program, cmd, that issues instructions to workers is provided for you. You are not to change this program without the permission of the course staff.</p>
<h3>Transaction Subsystem</h3>
<p>Recall that both the workers and the transaction manager keep log files to record their decisions. In addition, the worker records transaction information for the objects it modifies. Keeping the logfile can be challenging so we will be using some programming tricks and conventions to to simplify things. To simplify marking,&nbsp; everyone will implement a write-ahead logging system. (This is the one where both old and new values are recorded.) Each worker will be involved in only one transaction at a time.&nbsp;</p>
<p>Although a suggested log file format has been suggested, the precise information stored in the log file is a design decision.&nbsp; Keep in mind&nbsp; that you must also deal with the case when a worker or transaction manager crashes and then comes back to life.</p>
<h3>Transaction Manager</h3>
<p>The transaction manager is responsible for coordinating one or more simultaneous transactions. For this assignment you may assume that there are at most 4 transactions in progress at once and that the total number of worker per transaction is no more than 6 . The transaction manager takes a single argument, the port it is to listen on and send from.&nbsp; The transaction manger's log file is to be named TXMG_port.log where port is the transaction manger's port.</p>
<h3>Worker</h3>
<p>To simulate the modification of objects involved in a transaction, the worker has some state information it maintains in the form of two integer objects, A and B and a string identifier. Since the objects have to be durable, this state information is stored in a file and needs to be updated each time an object's value is changed and the transaction committed. The format of the state information can be found in the tworker.h file. The IDstring can be anything you want. The fields A and B will be updated by commands. The type and format of the messages exchanged between the workers and transaction manager are left as a design decision. However, if a message requires a response, you are to use a timeoutvalue of 10 seconds. After 10 seconds, the entity waiting for a response can assume the other side has crashed and perform whatever the appropriate action is at that point. If a worker is ever in the "uncertain" state and needs to get a decision from the coordinator it should wait 30 seconds for a decision after it has sent the vote that it is "prepared to commit" and once every 10 seconds after that until it gets a result.</p>
<p>The worker program takes a <span style="text-decoration: underline; background-color: #ffff00;">two</span> arguments, the UDP port number it will listen on for the commands described below <span style="text-decoration: underline;"><span style="background-color: #ffff00;">and the port number it will use when communicating with the coordinator</span>.</span>&nbsp; The worker will name its logfile TXWorker_port.log. Where port is the provided port number. On starting the worker will perform any required recovery if the file exists otherwise it will create the file for subsequent logging actions.&nbsp;</p>
<p>Note you will need to use a 2nd UDP port to communicate to the transaction manager. There is one important thing to remember, if the worker crashes when it comes back up in must use this same port to interact with the transaction manger. Consequently, when a worker is restarted after a crash it must be restarted with the same command port number specified on the command line that it was originally started with. Note that since the workers don't maintain the contact information for the other workers when they restart and are in the uncertain state they will have to poll the transaction manager until a response is received. This could be a long time if the manager is down.</p>
<h3>cmd</h3>
<p>A big challenge with testing the transaction management system is demonstrating that it works. The purpose of the <em>cmd</em> program is to help simplify this task. This program interprets the arguments supplied on the command line and sends a "command" to the identified worker. The worker then performs the action or actions specified by the command. These commands can be used to simulate various types of interactions between the transaction manager and the workers. Only your worker process needs to accept and respond to these commands. You can assume that the UDP packet is delivered and acted on by the worker. There is no response message from the worker to the cmd program. The commands are as follows:</p>
<ol>
<li>
<strong>begin</strong> WORKER_HOST WORKER_PORT TX_MANAGER_HOST TX_MANAGER PORT TID
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to send a begin transaction command to the <strong>TX_MANAGER TX_PORT</strong> instructing it to start a new transaction with the transaction ID TID. (Normally we would let the transaction manager select the TID, but to provide more control options we are providing the TID. If the transaction ID already exists then a failure indication is to be returned to the worker and a error message is to be printed and the worker is to exit.</li>
</ul>
</li>
<li>
<strong>join</strong> WORKER_HOST WORKER_PORT TX_MANAGER_HOST TX_MANAGER PORT TID
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to send a join transaction command to the <strong>TX_MANAGER TX_PORT</strong> instructing it to add this worker to the transaction. A success or failure indication needs to be returned from the transaction manager to the worker.&nbsp;</li>
</ul>
</li>
<li>
<strong>newa</strong> WORKER_HOST WORKER_PORT NEWVALUE
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to change the value of the A object to NEWVALUE. If a transaction is not currently underway the change is simply to be made and then synced to disk.</li>
</ul>
</li>
<li>
<strong>newb</strong> WORKER_HOST WORKER_PORT NEWVALUE
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to change the value of the B object to NEWVALUE. If a transaction is not currently underway the change is to be made and then synced to disk. If a transaction is not currently underway the change is simply to be made and then synced to disk.</li>
</ul>
</li>
<li>
<strong>newid</strong> WORKER_HOST WORKER_PORT newIDSTR
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to change the value of the ID string to newIDSTR.</li>
</ul>
</li>
<li>
<strong>crash</strong> WORKER_HOST WORKER_PORT
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to "crash". You are to simulate a crash by calling _exit() immediately. (Note that the _ is important.)</li>
</ul>
</li>
<li>
<strong>delay</strong> WORKER_HOST WORKER_PORT DELAY
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to delay all responses to the transaction manager by DELAY seconds. If the value is 0 it means respond immediately. The default behaviour is to respond immediately. If the value is negative, the worker is to wait the absolute value of the DELAY,&nbsp; perform the required local processing, and then crash immediately before responding. For example, if the value is -1000 then the worker is to make its decision and perform all the actions required by that decision but crash<span style="text-decoration: line-through;"> just after</span> before responding to the the coordinator.</li>
</ul>
</li>
<li>
<strong>commit</strong> WORKER_HOST WORKER_PORT
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to commit the transaction that is in progress. You may assume that a worker is only ever working with one transaction at a time. (However, the transaction manager may be dealing with more than one transaction.)</li>
</ul>
</li>
<li>
<strong>commitcrash</strong> WORKER_HOST WORKER_PORT
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to send a special commit message to coordinator such that the coordinator will crash after logging the commit decision but before responding to any workers. From the worker's perspective this is treated as a normal commit situation. You may assume that a worker is only ever working with one transaction at a time. (However, the transaction manager may be dealing with more than one transaction.)</li>
</ul>
</li>
<li>
<strong>abort</strong> WORKER_HOST WORKER_PORT
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to abort the transaction that is in progress. You may assume that a worker is only ever working with one transaction at a time. This will result in the worker sending an abort to the coordinator and the coordinator then aborting the transaction.</li>
</ul>
</li>
<li>
<strong>abortcrash</strong> WORKER_HOST WORKER_PORT
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to send a special abort message to coordinator such that the coordinator will crash after logging the abort decision but before responding to any workers. From the worker's perspective this is treated as a normal abort message to the coordinator with the added information for the transaction manager to abort. You may assume that a worker is only ever working with one transaction at a time. (However, the transaction manager may be dealing with more than one transaction.)</li>
</ul>
</li>
<li>
<strong>voteabort</strong> WORKER_HOST WORKER_PORT
<ul>
<li>Send a message to the worker process at <strong>WORKER_HOST WORKER_PORT</strong> instructing it to vote abort when the coordinator sends a prepare message. By default the worker will always vote commit to a prepare message. Note that a delay command will affect the timing of the response and whether or not the abort response is actually sent.</li>
</ul>
</li>
</ol>
<h3>General Implementation Notes</h3>
<p>Make sure to check out the .h files for the definitions A definition of an object store is provided in the file tworker.h. The example code in tworker.c illustrates how to create the backing store for this object and how to modify it. You are not allowed to change this structure or how the name of the file it is stored in is determined, or where in the file it is stored. Every time you change one of the 3 objects you must update the lastupdate time as part of the change along with updating the vector clock. The program dumpObject has been provided to print the contents of one of these object stores.</p>
<p>You will probably want to develop some scripts for testing your implementation. You should add these scripts to your repo and commit them.</p>
<p>&nbsp;</p>
<h3>Testing</h3>
<p>You are required to fully test your implementation with multiple workers. It is expected that your implementation will properly handle the arbitrary failure of the coordinator and zero or more workers simultaneously. From the worker's perspective you can assume that the transaction manager will always recover. Here are some ideas on what you should consider testing for: (This is not an exhaustive list. Note: TX stands for transaction)</p>
<ol>
<li>A properly completed transaction.</li>
<li>Coordinator crashes before a request to commit - TX should abort</li>
<li>Coordinator logs the decision to commit and then crashes</li>
<li>Worker asks to commit while coordinator is crashed - TX should abort</li>
<li>Worker crashes and recovers before TX commits, TX should abort.</li>
<li>Worker crashes after recording prepared, but before sending its response TX should abort.</li>
</ol>
<h3>What to hand in.</h3>
<p>All work is to be handed in via bitbucket. Do not, any any circumstances hand-in object code, executable files, or any other form of binary file and that includes word or PF documents. Make sure you hand-in:</p>
<ol>
<li>A working Makefile that will compile your program on the department's Linux machines and produce the required executable programs.</li>
<li>All the .c and .h files required to compile your program. Your code is to be well commented and appropriately formatted.</li>
<li>The coverpage.txt file. See the contents of the file for an explanation of how to hand it in.</li>
</ol>
<h3>Grading</h3>
<p>Here is a rough grading guideline. This is meant to provide you with some guidance, but I do not guarantee that that final grading rubric will match this mark distribution exactly, but it should be close. Implementation refers to both the correct functionality of the code and the design.</p>
<ol>
<li>Implementatiion of the coordinator 35%</li>
<li>Implementation of the worker 45%</li>
<li>Interview with TA 20%</li>
</ol>
<p>The following files will be in the repo when you get them.&nbsp;</p>
<table style="border-collapse: collapse; width: 23.4336%; height: 121px;" border="1">
<tbody>
<tr>
<td style="width: 100%;"><span class="instructure_file_holder link_holder"><a class="instructure_file_link" title="msg.h" href="/courses/36019/files/7305845/download?wrap=1" data-api-endpoint="https://canvas.ubc.ca/api/v1/courses/36019/files/7305845" data-api-returntype="File" data-id="7305845">msg.h</a><a class="file_preview_link" aria-hidden="true" href="/courses/36019/files/7305845/download?wrap=1" title="Preview the document" style="padding-left: 5px;" data-id="7305845"><img src="/images/preview.png" alt="Preview the document"></a></span></td>
</tr>
<tr>
<td style="width: 100%;"><span class="instructure_file_holder link_holder"><a class="instructure_file_link" title="tmanager.c" href="/courses/36019/files/7305846/download?wrap=1" data-api-endpoint="https://canvas.ubc.ca/api/v1/courses/36019/files/7305846" data-api-returntype="File" data-id="7305846">tmanager.c</a><a class="file_preview_link" aria-hidden="true" href="/courses/36019/files/7305846/download?wrap=1" title="Preview the document" style="padding-left: 5px;" data-id="7305846"><img src="/images/preview.png" alt="Preview the document"></a></span></td>
</tr>
<tr>
<td style="width: 100%;"><span class="instructure_file_holder link_holder"><a class="instructure_file_link" title="tmanager.h" href="/courses/36019/files/7305847/download?wrap=1" data-api-endpoint="https://canvas.ubc.ca/api/v1/courses/36019/files/7305847" data-api-returntype="File" data-id="7305847">tmanager.h</a><a class="file_preview_link" aria-hidden="true" href="/courses/36019/files/7305847/download?wrap=1" title="Preview the document" style="padding-left: 5px;" data-id="7305847"><img src="/images/preview.png" alt="Preview the document"></a></span></td>
</tr>
<tr>
<td style="width: 100%;"><span class="instructure_file_holder link_holder"><a class="instructure_file_link" title="tworker.c" href="/courses/36019/files/7305848/download?wrap=1" data-api-endpoint="https://canvas.ubc.ca/api/v1/courses/36019/files/7305848" data-api-returntype="File" data-id="7305848">tworker.c</a><a class="file_preview_link" aria-hidden="true" href="/courses/36019/files/7305848/download?wrap=1" title="Preview the document" style="padding-left: 5px;" data-id="7305848"><img src="/images/preview.png" alt="Preview the document"></a></span></td>
</tr>
<tr>
<td style="width: 100%; text-align: left;"><span class="instructure_file_holder link_holder"><a class="instructure_file_link" title="tworker.h" href="/courses/36019/files/7305849/download?wrap=1" data-api-endpoint="https://canvas.ubc.ca/api/v1/courses/36019/files/7305849" data-api-returntype="File" data-id="7305849">tworker.h</a><a class="file_preview_link" aria-hidden="true" href="/courses/36019/files/7305849/download?wrap=1" title="Preview the document" style="padding-left: 5px;" data-id="7305849"><img src="/images/preview.png" alt="Preview the document"></a></span></td>
</tr>
</tbody>
</table></div>


  <div style="display: none;">
    <span class="timestamp">1585637999</span>
    <span class="due_date_string">03/30/2020</span>
    <span class="due_time_string">11:59pm</span>
  </div>
</div>
