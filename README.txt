Notes:

- After reading piazza question @481, we implemented the worker so that it syncs both the 
TXdata and the log at the same time. As per the same post, we are also not differentiating
between a trunc and a commit, and assuming that a commit state means that all data has been 
flushed to disk. The same would be true of an abort state, as we would assume that the data
has been reverted BEFORE setting the state to WTX_ABORTED.
- According to piazza @399, we do not have to process any requests while waiting for a
response. We therefore ignore all command requests while waiting for a success or fail.
