Coordinator Design
==================

The coordinator is responsible for handling request from clients, allocating work to workers, and reporting results.


Data Structures
---------------

* Set of in progress uploads.
* Set of in progress alignments.
* Set of prepared problems.
* Set of claimed problems.

* Set of sockets to clients
* Set of sockets to workers
* Set of sockets to storage?


### Upload

* Name of genome
* Current progress

### Alignment

* Pointer to the initially available problem(s)
* Set of all problems.  This might be created cleverly and lazily.

### Problem

* Problem ID
* Pointer to the parent alignment job
* Reference to the section of the genome along the top.
    * This could be a reference to a section of a genome, i.e. (name, start index, length).
* Reference to the section of the genome along the left
* Incoming values along the top
* Incoming values along the left
* Maximum value and its location in the matrix

If the problem is missing incoming values on either axis, it is not considered prepared.
When both are set, it becomes prepared, and inserted into the set of prepared problems.

When a problem is claimed, it is removed from the set of prepared problems and inserted into the set of claimed problems.

When a solution is received from a worker, the coordinator looks up the ID in the set of claimed problems, identifies the problem, updates the appropriate aligment job, and creates newly prepared problems.


Control Flow
------------
All actions in the coordinator are driven by network events.
The main loop wakes up on network events.
It looks up which other node it is communicating with, deserializes the message, and calls into an Actions handler, either WorkerActions or ClientActions.
The Actions object does all of the decision making.
It has a Protocol object to communicate with the network again.
The Protocol object serializes messages and sends them to the other nodes.
There needs to be some kind of factory to produce Action objects.
They also probably need a better name than "Action" since that's pretty generic.