//
/// \file WexprTool/CommandLineParser.hpp
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

#ifndef WEXPRTOOL_COMMANDLINEPARSER_HPP
#define WEXPRTOOL_COMMANDLINEPARSER_HPP

#include <map>
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

			//
			// Can be 3 different types of values:
			// - if blank, ignore and dont try to validate as a schema
			// - if "(internal)", grab the root object's $schema, and use that.
			// - if anything else, load it as either a filepath or a url as the root schema.
			std::string schemaID = "";

			//
			// List of schema mappings
			// Will map the given schema IDs to a different path to load. 
			// If the schema is loaded, it'll use the mapping's path instead of the ID.
			//
			std::map<std::string, std::string> schemaMappings;
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
		//
		/// \brief Add a schema mapping from the wexpr string array.
		/// \return true if success, false if not
		//
		static bool p_addMappingFromWexprString (Results& res, const std::string& mappingStr);
};

#endif // WEXPRTOOL_COMMANDLINEPARSER_HPP
