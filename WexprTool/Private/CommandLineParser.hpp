//
/// \file WexprTool/CommandLineParser.hpp
/// \brief Parses commandline arguments
//
// #LICENSE_BEGIN:MIT#
// 
// Copyright (c) 2017-2019, Kenneth Perry (thothonegan)
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

#ifndef WEXPRTOOL_COMMANDLINEPARSER_HPP
#define WEXPRTOOL_COMMANDLINEPARSER_HPP

#include <string>

//
/// \brief Parse commandline arguments
//
class CommandLineParser
{
	public:
		enum class Command
		{
			Unknown,
			
			/// Make the wexpr human readable
			HumanReadable,
			
			/// Validate the wexpr, output 'true' or 'false'
			Validate,
			
			/// Minify the input
			Mini,
			
			/// Convert the wexpr to binary
			Binary
		};
		
		struct Results
		{
			bool help = false;
			bool version = false;
			bool validate = false;
			
			Command command = Command::HumanReadable;
			std::string inputPath = "-";
			std::string outputPath = "-";
		};
		
		//
		/// \brief Parse arguments into results
		//
		static Results parse (int argc, char** argv);
		
		//
		/// \brief Display help
		//
		static void displayHelp(int argc, char** argv);
		
	private:
};

#endif // WEXPRTOOL_COMMANDLINEPARSER_HPP
