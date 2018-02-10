//
/// \file ExpressionType.h
/// \brief ExpressionType tests
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

#ifndef WEXPR_TESTS_EXPRESSIONTYPE_H
#define WEXPR_TESTS_EXPRESSIONTYPE_H

#include <libWexpr/ExpressionType.h>

#include "UnitTest.h"

WEXPR_UNITTEST_BEGIN (ExpressionTypeCanReturnString)

	const char* nullStr = wexpr_ExpressionType_toString(WexprExpressionTypeNull);
	const char* valueStr = wexpr_ExpressionType_toString(WexprExpressionTypeValue);
	const char* mapStr = wexpr_ExpressionType_toString(WexprExpressionTypeMap);
	const char* arrayStr = wexpr_ExpressionType_toString(WexprExpressionTypeArray);
	
	WEXPR_UNITTEST_ASSERT (strcmp (nullStr,  "Null")  == 0, "Null == Null");
	WEXPR_UNITTEST_ASSERT (strcmp (valueStr, "Value") == 0, "Value == Value");
	WEXPR_UNITTEST_ASSERT (strcmp (mapStr,   "Map")   == 0, "Map == Map");
	WEXPR_UNITTEST_ASSERT (strcmp (arrayStr, "Array") == 0, "Array == Array");
	
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_SUITE_BEGIN (ExpressionType)
	WEXPR_UNITTEST_SUITE_ADDTEST (ExpressionType, ExpressionTypeCanReturnString);
WEXPR_UNITTEST_SUITE_END ()

#endif // WEXPR_TESTS_EXPRESSIONTYPE_H
