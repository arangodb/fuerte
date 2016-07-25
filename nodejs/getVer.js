var node = require('bindings')('node-arangodb');

var server = new node.Server("http://127.0.0.1:8529");

var con1 = server.makeConnection();
var con2 = server.makeConnection();

console.log(con1);
console.log(con2);

con1.Address();
con2.Address();

server.version(con1);
server.version(con2);

con1.SetAsynchronous(false);
console.log(con1.Run());
con2.SetAsynchronous(false);
console.log(con2.Run());
