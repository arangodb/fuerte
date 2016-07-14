var node = require('bindings')('AsyncDbVer');

var obj = [new node.GetVersion(),new node.GetVersion()];

// ArangoDB  version or connection error
console.log("Complete runs");
console.log(obj[0].version());
console.log(obj[1].version());

console.log("Asynchronous runs");
var done = [ obj[0].config(), obj[1].config() ];
if (done[0] ==  true && done[1] == true) {
		var loops = 0;
		var idx = 0;
		done = [ false,false ];
		do {
				++loops
				if (done[idx] == false) {
						done[idx] = obj[idx].process();
						if (done[idx] == true) {
								console.log(obj[idx].result());
								if (done[1 -idx] == true) {
										break;
								}
						}
				}
				idx = 1 - idx;
		} while (true);
		console.log("Iterations : " + loops);
}
