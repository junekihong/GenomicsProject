Protocol Design
==============

There are 4 different kinds of actors in our system.
First is the command-line client.
Next is the coordinator, repsonsible for allocating work and reporting results.
There are worker nodes that compute the bulk of the answer to the problem.
Finally, there is a storage and caching agent.

Client - Coordinator Interaction
--------------------------------

The client starts by opening a TCP connection to the coordinator.
If the TCP connection closes unexpectedly, the entire operation fails.

The client sends the message for that command, sends any extra data, waits for a response, then closes the connection.

### Client messages

Genome upload message:

    Genome Upload Request ID
    Genome name
    Length of genome
Once this message is sent, the genome is sent over the TCP connection.
**TODO** figure out flow control.  Maybe sendfile()?

Genome list request:

    Genome List Request ID

Genome list Response:

    Genome List Response ID
    List of Tuples:
        Name 1, Genome 1
        Name 2, Genome 2
        ...

Local Alignment request:

    Local Alignment Request ID
    Genome Name 1
    Genome Name 2

It is not entirely clear to Paul what the response should be just yet.
Maybe something like the output of the Rosalind [Edit Distance Aligment problem](http://rosalind.info/problems/edta/).

Local Alignment Response:

    Local Alignment Reponse ID
    ** TODO **
    

Coordinator - Worker Interaction
--------------------------------
The coordinator is responsible for dividing up the dynamic programming problems, providing them to the workers, and storing the results.
Each worker connects to the coordinator and maintains the connection for the lifetime of the worker.
Workers ask the coordinator for the list of unsolved problems, then requests one or more of them, solve them, and report answers back to the coordinator.
The coordinator decides whether or not to grant the request and responds to the worker.
The coordinator may not grant part of the request; the request is atomic.
Once a problem has been allocate to a worker, it may not be allocated to another worker.
The worker may not attempt to claim more problems until it has completed the problems already allocated to it.
We assume that the worker will complete every problem it is allocated without failure.

Once a worker node has either solved its subproblem and stored it in the database, it informs the Coordinator by returning the problem ID and the solution ID.

Problem list request:

    Problem List Request ID

Problem list response:

    Problem List Response ID
    List of problem descriptions:
        Problem 1
        Problem 2
        ...

Claim request:

    Claim Request Message ID
    List of claimed problems:
        Problem ID 1
        Problem ID 2
        ...

Problem claim response:

    Claim Response ID
    Boolean - Yes / No

Solution (only from worker -> coordinator):

    Solution Message ID
    Solution ID
    Problem ID
    
The solution ID is the problem ID of the first instance of this subproblem.


Storage
-------
The storage system saves genomes and previously solved problems.
The problems may be queried by problem ID or by initial conditions.
The storage system may reply either with the solution for that problem, or with nothing (a NACK).
For queries by initial conditions, the storage system may respond with the solution to a similar problem instead of the one queried.
If this is the case, it will be the problem most similar to the one requested.
The requestor may also query whether or not a problem is present; the storage system responds with the problem description of the most similar problem, but  not with the solution.

These do not have to go over the wire; the storage system may be in the same process as another actor, in which case these may be function or method calls.


Insert a new solution:

    Solution insertion ID
    Problem ID
    Problem Description
    Maximum value and location in matrix
    Contents of the matrix (in row-major order)

Insert genome data:

    Genome insertion ID
    Genome name
    Start index of chunk
    Length of chunk
    Data

In the future, this should have an ACK.

Query for genome information:

    Genome Info Request ID
    Genome name

Genome Info Query response:

    Genome Info Response ID
    Genome name
    Genome length

Genome Content Query:

    Genome Content Request ID
    Genome name
    Start position
    Length of genome section (negative values mean the end?)

Genome Content Response:

    Genome Content Response ID
    Genome name
    Start position
    Length of genome section
    Genome Data!

Query by problem ID:

    ID Query Request ID
    Problem ID
    Boolean - Whether to return the entire solution, or just the maximum value

Query by initial conditions:

    Conditions Query Request ID
    Problem Description
    Boolean - Whether to return the solution to partial matches

Query Response:

    Query Response ID
    Boolean -> Was a potential match found?
    Boolean -> Is this an exact match
    Problem Description
    Maximum value and location in the matrix
    Solution - the matrix for this problem in row-major order.  This may be omitted if it was not requested.
