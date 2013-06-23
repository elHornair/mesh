Mesh in C
=======

About
---------------------------
This is a school project. You can find further information on it [here](https://github.com/elHornair/mesh-doc).

Compilation
---------------------------
You just need to run:

    make

Cleanup
---------------------------
Again, with make, run:

    make clean

Usage
---------------------------
A mesh node can be started with the following shell command:

    mesh portnum [-q|-z]

`portnum` ist the number of the port you want the mesh node to run on. `-q` creates a source node, `-z` creates a sink
node. [Here](https://github.com/tpo/mesh-network-py) you can find a handy script for sending packages around.
