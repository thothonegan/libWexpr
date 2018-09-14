#!/usr/bin/env node
//
// WexprValidate.js
// Simple program designed to be compatible with libWexpr's test runner
//
// Usage: [node] ./WexprValidate.js [wexprfilename]
// Returns 0 on success, or 1 if invalid wexpr
//

// put libWexpr.js in the same dir as this file
require('libWexpr.js');

var fs = require('fs')

// For now, the only argument we take is the input file
// argument one is our program
inputFilePath = process.argv[2];

fs.readFile (inputFilePath, 'utf8', function(err, data) {
	if (err) {
		process.exit(1);
	}

	try {
		res = libWexpr.decode(data);
	} catch (err) {
		console.log ("exception");
		process.exit(1); // failure due to exception
	}
	
	// res[0] is data, res[1] is success
	if (res[1] != null) {
		console.log ("Error: " + res[1]);
		process.exit(1); // failure
	}

	// success
	process.exit();
})

