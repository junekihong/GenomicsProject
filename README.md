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

    mdpc [--server <server>[:<port>]] local-align <genome_1> <genome_2>

The client outputs the optimal alignment.

The coordinator (leader) is run with

    mdpl [--port <port>]

A worker is run with

    mdpw [--server <server>:[<port>]] [--port <port>]

**TODO** Is storage a separate process, or is it part of the coordinator (and each worker)?

Internals
=========
You can generate documentation by running ``doxygen`` in the top-level directory.  You may need to create the ``build`` directory yourself in order to appease doxygen.

[Protocol Description](md_doc_proto_design.html)

[Coordinator Description](md_doc_coord_design.html)
