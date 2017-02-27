var fuerte = require('.')
var vpack = require('node-velocypack')

// create server
var builder = new fuerte.ConnectionBuilder()

console.log("creating connections")
var connection_vst = builder.host("vst://127.0.0.1:8530").connect();
var connection_http = builder.host("http://127.0.0.1:8529").connect();


console.log("creating request")
var request = new fuerte.Request();
request.setRestVerb("get");
request.setPath("/_api/version");

console.log("creating callbacks")
var onError = function(code, req, res){
  console.log("\n#### error ####\n")
  console.log("error code: " + code)
  if(res.notNull()){
    console.log("response code: " + res.getResponseCode())
    console.log("content type: " + res.getConentType())
    console.log("raw payload: " + res.payload())
  }
}

var onSuccess = function(req, res){
  console.log("\n#### succes ####\n")
  // we maybe return a pure js objects (req/res)
  if(res.notNull()){
    console.log("response code: " + res.getResponseCode())
    var contentType = res.getContentType();
    var payload = res.payload();
    console.log("content type: " + contentType)
    if(contentType.indexOf("application/json") !== -1){
      console.log(payload.toString());
    } else {
      console.log("raw payload: " + payload)
    }
  }
}

function run_example(connection, proto){
  console.log("queue 1 " + proto)
  connection.sendRequest(request, onError, onSuccess);
  fuerte.run();
  console.log("1 done " + proto)
  console.log("------------------------------------------")

  console.log("queue 2 " + proto)
  connection.sendRequest(request, onError, onSuccess);
  console.log("queue 3 " + proto)
  connection.sendRequest(request, onError, onSuccess);
  fuerte.run();
  console.log("2,3 done " + proto)
  console.log("------------------------------------------")

  console.log("queue 4 " + proto)
  var requestCursor = new fuerte.Request();
  var slice = vpack.encode({"query": "FOR x IN 1..5 RETURN x"});
  requestCursor.setRestVerb("post");
  requestCursor.setPath("/_api/cursor");
  requestCursor.addVPack(slice);
  connection.sendRequest(requestCursor, onError, onSuccess);
  fuerte.run();
  console.log("4 done " + proto)
  console.log("------------------------------------------")

  console.log("queue 5 " + proto)
  var requestCursor = new fuerte.Request();
  var slice = vpack.encode({});
  requestCursor.setRestVerb("post");
  requestCursor.setPath("/_api/document/_users");
  requestCursor.addVPack(slice);
  connection.sendRequest(requestCursor, onError, onSuccess);
  fuerte.run();
  console.log("5 done " + proto)
  console.log("------------------------------------------")

  console.log("queue 6 " + proto)
  var requestCursor = new fuerte.Request();
  var slice = vpack.encode({});
  requestCursor.setRestVerb("post");
  requestCursor.setPath("/_api/document/_users");
  requestCursor.addVPack(slice);
  requestCursor.setAcceptType("application/json")
  connection.sendRequest(requestCursor, onError, onSuccess);
  fuerte.run();
  console.log("6 done " + proto)
  console.log("------------------------------------------")
}

console.log("run examples with velocystream")
run_example(connection_vst, "vst");
console.log("run examples with http")
run_example(connection_http, "http");
