var node = require('bindings')('node-arangodb');

var server = new node.Server("http://127.0.0.1:8529");

var con1 = server.makeConnection();
var con2 = server.makeConnection();

var val = con1.EnumValues();

console.log(val);
val.JSon = 3;
console.log(val);

console.log(con1);
console.log(con2);

con1.Address();
con2.Address();

server.version(con1);
server.version(con2);

var loops = 0;
con1.SetAsynchronous(false);
do {
  ++loops;
  con1.Run();
} while (con1.IsRunning());
console.log("Sync connection result : " + con1.Result());
console.log("Sync connection loops : " + loops);
console.log("Sync response code : " + con1.ResponseCode());

loops = 0;
con2.SetAsynchronous(true);
do {
  ++loops;
  con2.Run();
} while (con2.IsRunning());
console.log(con2.Result());
console.log("Async connection result : " + con1.Result());
console.log("Async connection loops : " + loops);
console.log("ASync response code : " + con2.ResponseCode());

server.version(con1);
server.version(con2);

var loops = 0;
con1.SetAsynchronous(true);
do {
  ++loops;
  con1.Run();
} while (con1.IsRunning());
console.log("Async connection result : " + con1.Result());
console.log("Async connection loops : " + loops);
console.log("Async response code : " + con1.ResponseCode());

loops = 0;
con2.SetAsynchronous(false);
do {
  ++loops;
  con2.Run();
} while (con2.IsRunning());
console.log(con2.Result());
console.log("Sync connection result : " + con2.Result());
console.log("Sync connection loops : " + loops);
console.log("Sync response code : " + con2.ResponseCode());
