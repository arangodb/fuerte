var fuerte = require('bindings')('arango-node-driver')

// create server
var builder = new fuerte.ConnectionBuilder()
var con = builder.host("vst://127.0.0.1:8529").connect();

