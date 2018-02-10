//
/// \file WexprTool/Application.cpp
/// \brief Tool to 
//
// #LICENSE_BEGIN:MIT#
// 
// Copyright (c) 2017-2018, Kenneth Perry (thothonegan)
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
// #LICENSE_END#
//

#include "CommandLineParser.hpp"

#include <libWexpr/libWexpr.h>

#include <cstdlib>
#include <fstream>
#include <iostream>

namespace
{
	std::string s_readAllInputFrom (const std::string& inputPath)
	{
		std::string data;
		
		if (inputPath == "-")
		{
			return std::string (
				std::istreambuf_iterator<char>(std::cin), {}
			);
		}
		else
		{
			std::fstream file (inputPath, std::ios::in);
			
			return std::string (
				std::istreambuf_iterator<char>(file), {}
			);
		}
		
	}

	void s_writeAllOutputTo (const std::string& outputPath, const std::string& str)
	{
		if (outputPath == "-")
		{
			std::cout << str << std::flush;
		}
		else
		{
			std::fstream file (outputPath, std::ios::out | std::ios::trunc);
			
			file << str << std::flush;
		}
	}
}

//
/// \brief App entry point
//
int main (int argc, char** argv)
{
	auto results = CommandLineParser::parse (argc, argv);
	
	if (results.version)
	{
		std::cout << "WexprTool " << wexpr_Version_major() << "." << wexpr_Version_minor() << "." << wexpr_Version_patch() << std::endl;
		
		return EXIT_SUCCESS;
	}
	
	if (results.help)
	{
		CommandLineParser::displayHelp(argc, argv);
		return EXIT_SUCCESS;
	}
	
	// normal flow
	if (results.command == CommandLineParser::Command::HumanReadable ||
		results.command == CommandLineParser::Command::Validate
	)
	{
		bool isValidate = (results.command == CommandLineParser::Command::Validate);
		
		auto inputStr = s_readAllInputFrom(results.inputPath);
		
		WexprError err = WEXPR_ERROR_INIT();
		
		auto expr = wexpr_Expression_createFromLengthString (
			inputStr.c_str(), inputStr.size(),
			WexprParseFlagNone,
			&err
		);
		
		if (err.code)
		{
			if (isValidate)
			{
				s_writeAllOutputTo(results.outputPath, "false\n");
				return EXIT_FAILURE;
			}
			else
			{
				std::cerr << "WexprTool: Error occurred with wexpr:" << std::endl;
				std::cerr << "WexprTool: " << err.message << std::endl;
				return EXIT_FAILURE;
			}
		}
		
		WEXPR_ERROR_FREE (err);
		
		if (isValidate)
		{
			s_writeAllOutputTo(results.outputPath, "true\n");
		}
		else
		{
			char* buffer = wexpr_Expression_createStringRepresentation (
				expr, 0, WexprWriteFlagHumanReadable
			);
			
				s_writeAllOutputTo(results.outputPath, std::string(buffer));
			free (buffer);
		}
	}
	
	else
	{
		std::cerr << "WexprTool: Unknown command" << std::endl;
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

