var fuerte = require('bindings')('arango-node-driver')

// create server
var builder = new fuerte.ConnectionBuilder()
var connection = builder.host("vst://127.0.0.1:8529").connect();

var request = new fuerte.Request();
request.setRestVerb("get");
request.setPath("_api/version/");

var onError = function(code, req, res){
  console.log("error")
}

var onSuccess = function(req, res){
  console.log("succes")
}

connection.sendRequest(request, onError, onSuccess);

//fuerte.poll();
fuerte.run();
