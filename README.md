Juneki Hong and Paul O'Neil



GenomicsProject
===============

Meta Dynamic Programming. Dynamic Programming over all of our Dynamic Programming Problems

Usage
=====

The user interacts with the system via a command-line client, named ``mdpc``.

A genome can uploaded to the system by running:

    mdpc [--server <server>[:<port>]] genome upload <filename> <name>

The client reads the DNA sequence in the file given by ``filename`` and uploads it to the coordinator.
Later, the genome can be referenced by the the name given, and the file can be deleted.
Names must be unique.
The client reports whether the upload was successful.

The target server is optionally specified with ``--server``; the default server and port is ``localhost:5528``.

The list of uploaded genomes can be queried with

    mdpc [--server <server>[:<port>]] genome list


Genomes can be aligned with

    mdpc [--server <server>[:<port>]] local-align <genome_1> <genome_2> ...

The client outputs the end of the optimal alignment and the matrix.
Many pairs of genomes may be specified and they will be requested sequentially.


The coordinator (leader) is run with

    mdpl [--port <port>] [--storage <server>[:<port>]]

A worker is run with

    mdpw [--server <server>[:<port>]] [--storage <server>[:<port>]]

A storage node is run with:

    mdps [--port <port>]

Our benchmarks can be run with the included ``benchmark.py``.
The options there are described by running ``benchmark.py --help``.
The baseline program can be benchmarked with ``baseline.py``.
Building
========
To build from git:

    git submodule init
    git submodule update
    
    cd lib/msgpack
    ./bootstrap
    ./configure --enable-shared=no
    make
    
    cd ../..
    make


Internals
=========
You can generate documentation by running ``doxygen`` in the top-level directory.  You may need to create the ``build`` directory yourself in order to appease doxygen.
These may be slightly outdated (some bits of the coordinator design aren't right)...

[Protocol Description](md_doc_proto_design.html)

[Coordinator Description](md_doc_coord_design.html)

Libraries
=========
We depend on [Boost](http://www.boost.org) and assume that it is already installed.
It is distributed under the open source (Boost License)[http://www.boost.org/users/license.html].

We also use [Msgpack](http://github.com/msgpack/msgpack-c); it is a submodule in our repository.
It is distributed under the open source (Apache License)[https://github.com/msgpack/msgpack-c/blob/master/COPYING].
