var node = require('bindings')('arango-node-driver');

var server = new node.Server("http://127.0.0.1:8529");

var con1 = server.makeConnection();
var con2 = server.makeConnection();

var val = con1.EnumValues(); // what is this?

console.log(val);
val.JSon = 3;        // what is this?
console.log(val);

console.log("con1: " +  con1);
console.log("con2: " + con2);

var sink = null

//con1.Address(); //why was address called?
//con2.Address();

server.version(con1);
server.version(con2);

console.log("\n\ntest1:")
var loops = 0;
sink = con1.SetAsynchronous(false);
do {
  ++loops;
  con1.Run();
} while (con1.IsRunning());
console.log("Sync connection result : " + con1.Result());
console.log("Sync connection loops : " + loops);
console.log("Sync response code : " + con1.ResponseCode());

console.log("\n\ntest2:")
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

console.log("\n\ntest3:")
loops = 0;
con1.SetAsynchronous(true);
do {
  ++loops;
  con1.Run();
} while (con1.IsRunning());
console.log("Async connection result : " + con1.Result());
console.log("Async connection loops : " + loops);
console.log("Async response code : " + con1.ResponseCode());

console.log("\n\ntest4:")
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
