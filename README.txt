Notes:

- After reading piazza question @481, we implemented the worker so that it syncs both the 
TXdata and the log at the same time. As per the same post, we are also not differentiating
between a trunc and a commit, and assuming that a commit state means that all data has been 
flushed to disk. The same would be true of an abort state, as we would assume that the data
has been reverted BEFORE setting the state to WTX_ABORTED.
- According to piazza @399, we do not have to process any requests while waiting for a
response. We therefore ignore all command requests while waiting for a success or fail.
- From piazza @483, we enter the uncertain state after sending a "preparedToCommit" message
to the txManager, and then crashing. When we come back, we will wait for 
- As per piazza @412, we enter an 'uncertain state' after sending a prepared message but not 
yet recieving a commit message from the txmanager. In this state, we wait 30 seconds, and then
send another prepared message every 10 seconds.
- From @392, Prof. Acton states that "If there is no transaction under way and you get a 
voteabort it can simply be ignored." We follow this design descision, in that we only abort
- As per @407, we do not process ANY messages while delayed.