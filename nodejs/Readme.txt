Node.js example to get the ArangoDB version details as json, or an error message

Files that should be in the install directory

Readme.txt
GetVersion.cpp
GetVersion.h
Buffer.cpp
Buffer.h
getVer.js
binding.gyp
package.json

Requirements

Only configured for Linux at present, developed using Fedora 24

The following need to be installed

npm :               The node.js package manager
node :              The node.js Javascript console
nope-gyp :       Node.js build tool 
curl :                The curl library
curlpp :            The curlpp library
gcc/sttdc++:    The C++ development tools and standard library

How build and run:

Open a bash console in the directory containing the code then run the commands as follows

npm install
node-gyp rebuild
node getVer.js

If the ArangoDB server is not running you should get the following output

[jbufton@localhost GetVersion]$ node getVer.js
Complete runs
Couldn't connect to server
Couldn't connect to server
Asynchronous runs
Couldn't connect to server
Couldn't connect to server
Iterations : 2

If the ArangoDB server is running locally using port 8529 you should get output similar to the following

[jbufton@localhost GetVersion]$ node getVer.js
Complete runs
{"server":"arango","version":"3.0.x-devel"}
{"server":"arango","version":"3.0.x-devel"}
Asynchronous runs
{"server":"arango","version":"3.0.x-devel"}
{"server":"arango","version":"3.0.x-devel"}
Iterations : 4

