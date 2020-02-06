//
/// \file Main.c
/// \brief Self tests
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

#include "Expression.h"
#include "ExpressionErrors.h"
#include "ExpressionType.h"
#include "ReferenceTable.h"
#include "UVLQ64.h"

int main (int argc, char** argv)
{
	WexprSuiteResult res = {0, 0};
	
	#define RUN_SUITE(name) \
		{ \
			WexprSuiteResult r = WEXPR_UNITTEST_SUITE_RUN(name); \
			res.failures += r.failures; \
			res.successes += r.successes; \
		}
	
	RUN_SUITE(Expression)
	RUN_SUITE(ExpressionErrors)
	RUN_SUITE(ExpressionType)
	RUN_SUITE(ReferenceTable)
	RUN_SUITE(UVLQ64)
	
	printf ("\nTEST RESULTS: Success: %d Failures: %d\n", res.successes, res.failures);
	
#undef RUN_SUITE
	
	return res.failures;
}
