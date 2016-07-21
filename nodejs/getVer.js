var node = require('bindings')('node-arangodb');

var server = new node.Server();

var con1 = server.makeConnection();
var con2 = server.makeConnection();

con1.Address();
con2.Address();

server.version(con1);
server.version(con2);

console.log(con1.Run());
console.log(con2.Run());
