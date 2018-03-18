Simple webserver
================

This example of a simple webserver is inspired by IBM's really small 
webserver example, nweb.

Uses
----

This is obviously not meant to be used for real deployment, and is just out 
here in case someone wants to see a small c-based webserver in use. Basic 
socket use examples are in here from nweb. Also included are two small pages 
to serve up.

Building and Using
------------------

Known to build on OS X and OpenBSD.

Run the Makefile

`make`

You run the server like this:

`./build/server -p [port] -d [location to serve pages from] [& - to run in background]`

For example, to run it on port 8080, you'd do something like this:

`./build/server -p 8080 -d ./html &`