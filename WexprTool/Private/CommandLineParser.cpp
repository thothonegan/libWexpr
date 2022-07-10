//
/// \file WexprTool/CommandLineParser.cpp
/// \brief Parses commandline arguments
//
// #LICENSE_BEGIN:MIT#
// 
// Copyright (c) 2017-2020, Kenneth Perry (thothonegan)
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// SPDX-License-Identifier: MIT
// #LICENSE_END#
//
 
#include "CommandLineParser.hpp"

#include <iostream>

#include <libWexpr/libWexpr.h>

namespace
{
	CommandLineParser::Command s_commandFromString (const std::string& str)
	{
		if (str == "humanReadable")
		{ return CommandLineParser::Command::HumanReadable; }
		else if (str == "validate")
		{ return CommandLineParser::Command::Validate; }
		else if (str == "mini")
		{ return CommandLineParser::Command::Mini; }
		else if (str == "binary")
		{ return CommandLineParser::Command::Binary; }
		
		return CommandLineParser::Command::Unknown;
	}
}

CommandLineParser::Results CommandLineParser::parse(int argc, char ** argv)
{
	CommandLineParser::Results r;
	
	for (int argIndex=0; argIndex < argc; ++argIndex)
	{
		std::string arg (argv[argIndex]);
		
		if (arg == "-h" || arg == "--help")
		{ r.help = true; }
		
		else if (arg == "-v" || arg == "--version")
		{ r.version = true; }
		
		else if (arg == "-c" || arg == "--command")
		{
			if ( (argIndex+1) < argc)
			{
				r.command = s_commandFromString(argv[argIndex+1]);
				++argIndex;
			}
		}
		
		else if (arg == "-i" || arg == "--input")
		{
			if ( (argIndex+1) < argc)
			{
				r.inputPath = argv[argIndex+1];
				++argIndex;
			}
		}
		
		else if (arg == "-o" || arg == "--output")
		{
			if ( (argIndex+1) < argc)
			{
				r.outputPath = argv[argIndex+1];
				++argIndex;
			}
		}

		else if (arg == "-s" || arg == "--schema")
		{
			if ((argIndex+1) < argc)
			{
				r.schemaID = argv[argIndex+1];
				++argIndex;
			}
		}

		else if (arg == "-m" || arg == "--schemaMap")
		{
			if ((argIndex+1) < argc)
			{
				std::string mappingStr = argv[argIndex+1];
				if (!p_addMappingFromWexprString(r, mappingStr))
				{
					std::cerr << "Failed to add mapping: " << mappingStr << std::endl;
					abort();
				}

				++argIndex;
			}
		}
	}
	
	return r;
}

void CommandLineParser::displayHelp(int argc, char** argv)
{
	using std::cout;
	
	std::string arg = (argc > 0) ? argv[0] : "WexprTool";
	
	cout << "Usage: " << arg << " [OPTIONS]" << std::endl;
	cout << "Performs operations on wexpr data" << std::endl;
	cout << std::endl;
	cout << "-c, --cmd     Perform the requested command" << std::endl;
	cout << "              humanReadable      - [default] Makes the wexpr input human readable and outputs." << std::endl;
	cout << "              validate           - Checks the wexpr for correct syntax. If valid outputs 'true' and returns 0, otherwise 'false' and 1." << std::endl;
	cout << "              mini               - Minifies the wexpr output" << std::endl;
	cout << "              binary             - Write the wexpr out as binary" << std::endl;
	cout << std::endl;
	cout << "-i, --input   The input file to read from (default is -, stdin)." << std::endl;
	cout << "-o, --output  The place to write the output (default is -, stdout)." << std::endl;
	cout << "-s, --schema  (validate) If provided, will also validate it against the given schema (or if the magic value '(internal)' it'll use the $schema on the root object)." << std::endl;
	cout << "-m, --schemaMap '#(originalId newPath)' If provided as a 2 wexpr array, will map the given ID to the given location to load from. Can be multiple times for differnet IDs." << std::endl;
	cout << "-h, --help    Display this help and exit" << std::endl;
	cout << "-v, --version Output the version and exit" << std::endl;
}

// --- private

bool CommandLineParser::p_addMappingFromWexprString (Results& res, const std::string& mappingStr)
{
	// parse as wexpr.
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* expr = wexpr_Expression_createFromLengthString(
		mappingStr.c_str(), mappingStr.size(),
		WexprParseFlagNone,
		&err
	);

	if (err.code)
	{
		std::cerr << "WexprTool: Error occurred with schemaMap wexpr:" << std::endl;
		std::cerr << "WexprTool: [schemaMap]:" << err.line << ":" << err.column << ": " << err.message << std::endl;
		return false;
	}

	if (!expr)
	{
		std::cerr << "WexprTool: Got an empty expression back " << std::endl;
		return false;
	}

	WEXPR_ERROR_FREE(err);

	if (wexpr_Expression_type(expr) != WexprExpressionTypeArray
		|| wexpr_Expression_arrayCount(expr) != 2
		|| wexpr_Expression_type(wexpr_Expression_arrayAt(expr, 0)) != WexprExpressionTypeValue
		|| wexpr_Expression_type(wexpr_Expression_arrayAt(expr, 1)) != WexprExpressionTypeValue
	)
	{
		std::cerr << "WexprTool: SchemaMap entry must be an array of 2 values" << std::endl;
		return false;
	}

	std::string schemaID = wexpr_Expression_value(wexpr_Expression_arrayAt(expr, 0));
	std::string schemaPath = wexpr_Expression_value(wexpr_Expression_arrayAt(expr, 1));

	res.schemaMappings[schemaID] = schemaPath;
	return true;
}
