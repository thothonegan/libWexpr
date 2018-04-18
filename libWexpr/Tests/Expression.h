//
/// \file Expression.h
/// \brief Expression tests
///
/// #LICENSE_BEGIN:MIT#
/// 
/// Copyright (c) 2017-2018, Kenneth Perry (thothonegan)
/// 
/// Permission is hereby granted, free of charge, to any person obtaining
/// a copy of this software and associated documentation files (the
/// "Software"), to deal in the Software without restriction, including
/// without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to
/// permit persons to whom the Software is furnished to do so, subject to
/// the following conditions:
/// 
/// The above copyright notice and this permission notice shall be
/// included in all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
/// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
/// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
/// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
/// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
/// 
/// #LICENSE_END#
//

#ifndef WEXPR_TESTS_EXPRESSION_H
#define WEXPR_TESTS_EXPRESSION_H

#include <libWexpr/Expression.h>

#include <stdbool.h>

#include "UnitTest.h"

WEXPR_UNITTEST_BEGIN (ExpressionCanCreateNull)

	WexprExpression* nullExpr = wexpr_Expression_createNull();

	WEXPR_UNITTEST_ASSERT (nullExpr, "Cannot create null expression (returned null)");
	
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(nullExpr) == WexprExpressionTypeNull, "Null expression was not null expression");
	
	wexpr_Expression_destroy(nullExpr);
	
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionCanCreateValue)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString("val", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (valueExpr, "Cannot create value expression");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(valueExpr) == WexprExpressionTypeValue, "Should be a value expression");
	WEXPR_UNITTEST_ASSERT (strcmp (wexpr_Expression_value(valueExpr), "val") == 0, "Expression should be 'val'");
	
	wexpr_Expression_destroy(valueExpr);
	WEXPR_ERROR_FREE (err);

WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionCanCreateQuotedValue)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString(" \"val\" ", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (valueExpr, "Cannot create value expression");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(valueExpr) == WexprExpressionTypeValue, "Should be a value expression");
	WEXPR_UNITTEST_ASSERT (strcmp (wexpr_Expression_value(valueExpr), "val") == 0, "Expression should be 'val'");
	
	wexpr_Expression_destroy(valueExpr);
	WEXPR_ERROR_FREE (err);

WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionCanCreateNumber)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString("2.45", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (valueExpr, "Cannot create value expression");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(valueExpr) == WexprExpressionTypeValue, "Should be a value expression");
	WEXPR_UNITTEST_ASSERT (strcmp (wexpr_Expression_value(valueExpr), "2.45") == 0, "Expression should be '2.45'");
	
	wexpr_Expression_destroy(valueExpr);
	WEXPR_ERROR_FREE (err);

WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionCanCreateArray)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* arrayExpr = wexpr_Expression_createFromString("#(1 2 3)", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (arrayExpr, "Cannot create array expression");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(arrayExpr) == WexprExpressionTypeArray, "Should be an array expression");
	
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_arrayCount(arrayExpr) == 3, "Should have 3 entries");
	
	WexprExpression* item0 = wexpr_Expression_arrayAt(arrayExpr, 0); // get not create, so we dont own.
	WexprExpression* item1 = wexpr_Expression_arrayAt(arrayExpr, 1);
	WexprExpression* item2 = wexpr_Expression_arrayAt(arrayExpr, 2);
	
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(item0) == WexprExpressionTypeValue, "0 should be a value");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(item1) == WexprExpressionTypeValue, "1 should be a value");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(item2) == WexprExpressionTypeValue, "2 should be a value");
	
	WEXPR_UNITTEST_ASSERT (strcmp (wexpr_Expression_value(item0), "1") == 0, "0 should be 1");
	WEXPR_UNITTEST_ASSERT (strcmp (wexpr_Expression_value(item1), "2") == 0, "1 should be 2");
	WEXPR_UNITTEST_ASSERT (strcmp (wexpr_Expression_value(item2), "3") == 0, "2 should be 3");
	
	wexpr_Expression_destroy(arrayExpr);
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionCanCreateMap)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* mapExpr = wexpr_Expression_createFromString("@(a b c d)", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (mapExpr, "Cannot create map expression");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(mapExpr) == WexprExpressionTypeMap, "Should be a map expression");
	
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_mapCount(mapExpr) == 2, "Should have two map entries");
	
	// we can iterate multiple ways
	
	// first, by index
	bool seenA = false;
	bool seenC = false;
	
	const char* mapKey0 = wexpr_Expression_mapKeyAt(mapExpr, 0);
	const char* mapKey1 = wexpr_Expression_mapKeyAt(mapExpr, 1);
	WexprExpression* mapValue0 = wexpr_Expression_mapValueAt(mapExpr, 0);
	WexprExpression* mapValue1 = wexpr_Expression_mapValueAt(mapExpr, 1);
	
	const char* mapKey0Value = mapKey0;
	const char* mapKey1Value = mapKey1;
	const char* mapValue0Value = wexpr_Expression_value(mapValue0);
	const char* mapValue1Value = wexpr_Expression_value(mapValue1);
	
	if (strcmp (mapKey0Value, "a") == 0)
	{
		WEXPR_UNITTEST_ASSERT (!seenA, "Shouldnt see A twice");
		WEXPR_UNITTEST_ASSERT (strcmp (mapValue0Value, "b") == 0, "a = b");
		
		seenA = true;
	}
	
	if (strcmp (mapKey1Value, "a") == 0)
	{
		WEXPR_UNITTEST_ASSERT (!seenA, "Shouldnt see A twice");
		WEXPR_UNITTEST_ASSERT (strcmp (mapValue1Value, "b") == 0, "a = b");
		
		seenA = true;
	}
	
	if (strcmp (mapKey0Value, "c") == 0)
	{
		WEXPR_UNITTEST_ASSERT (!seenC, "Shouldnt see C twice");
		WEXPR_UNITTEST_ASSERT (strcmp (mapValue0Value, "d") == 0, "c = d");
		
		seenC = true;
	}
	
	if (strcmp (mapKey1Value, "c") == 0)
	{
		WEXPR_UNITTEST_ASSERT (!seenC, "Shouldnt see C twice");
		WEXPR_UNITTEST_ASSERT (strcmp (mapValue1Value, "d") == 0, "c = d");
		
		seenC = true;
	}
	
	WEXPR_UNITTEST_ASSERT (seenA, "Should have seen A");
	WEXPR_UNITTEST_ASSERT (seenC, "Should have seen C");
	
	// second by key
	WexprExpression* val0 = wexpr_Expression_mapValueForKey (mapExpr, "a");
	WexprExpression* val1 = wexpr_Expression_mapValueForKey (mapExpr, "c");
	
	const char* val0Value = wexpr_Expression_value (val0);
	const char* val1Value = wexpr_Expression_value (val1);
	
	WEXPR_UNITTEST_ASSERT (strcmp(val0Value, "b") == 0, "a = b");
	WEXPR_UNITTEST_ASSERT (strcmp(val1Value, "d") == 0, "c = d");

	wexpr_Expression_destroy(mapExpr);
	WEXPR_ERROR_FREE (err);

WEXPR_UNITTEST_END ()

// exprCanLoadUTF8
// exprCanIgnoreComments
// exprCanIgnoreInlineComment
// exprCanConvertBackToString

WEXPR_UNITTEST_BEGIN (ExpressionrCanUnderstandReference)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* expr = wexpr_Expression_createFromString("@(first [val]\"name\")", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeNone, "Should have no error");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(expr) == WexprExpressionTypeMap, "Should be a map");
	
	WexprExpression* val = wexpr_Expression_mapValueForKey(expr, "first");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(val) == WexprExpressionTypeValue, "Should be a value");
	WEXPR_UNITTEST_ASSERT (strcmp (wexpr_Expression_value(val), "name") == 0, "Should ignore the reference");
	
	wexpr_Expression_destroy(expr);
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END()

WEXPR_UNITTEST_BEGIN (ExpressionCanDerefReference)

	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* expr = wexpr_Expression_createFromString("@(first [val]\"name\" second *[val])", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeNone, "Should have no error");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(expr) == WexprExpressionTypeMap, "Should be a map");
	
	WexprExpression* val = wexpr_Expression_mapValueForKey(expr, "second");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(val) == WexprExpressionTypeValue, "Should be a value");
	WEXPR_UNITTEST_ASSERT (strcmp (wexpr_Expression_value(val), "name") == 0, "Should have followed the reference");
	
	wexpr_Expression_destroy(expr);
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END()

WEXPR_UNITTEST_BEGIN (ExpressionCanDerefArrayReference)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* expr = wexpr_Expression_createFromString("@(first [val]#(1 2) second *[val])", WexprParseFlagNone, &err);

	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeNone, "Should have no error");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(expr) == WexprExpressionTypeMap, "Should be a map");

	WexprExpression* val = wexpr_Expression_mapValueForKey(expr, "second");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(val) == WexprExpressionTypeArray, "Should be an array");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_arrayCount(val) == 2, "Should have 2 items");
	
	wexpr_Expression_destroy(expr);
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END()

WEXPR_UNITTEST_BEGIN (ExpressionCanDerefMapProperly)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* expr = wexpr_Expression_createFromString("@(first [val] @(a b) second *[val])", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeNone, "Should have no error");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(expr) == WexprExpressionTypeMap, "Should be a map");
	
	WexprExpression* val = wexpr_Expression_mapValueForKey(expr, "second");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(val) == WexprExpressionTypeMap, "Should be a map");
	
	WexprExpression* val2 = wexpr_Expression_mapValueForKey(val, "a");
	
	WEXPR_UNITTEST_ASSERT (strcmp (wexpr_Expression_value(val2), "b") == 0, "Copied the map properly");
	
	wexpr_Expression_destroy(expr);
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END()

WEXPR_UNITTEST_BEGIN (ExpressionCanCreateString)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* expr = wexpr_Expression_createFromString(
		"@(first #(a b) second \"20% cooler\")",
		WexprParseFlagNone, &err
	);
	
#define EL "\n"
	
	const char* notHumanReadableString1 = "@(second \"20% cooler\" first #(a b))";
	const char* notHumanReadableString2 = "@(first #(a b) second \"20% cooler\")";
	const char* humanReadableString1 =
		"@(" EL
		"	second \"20% cooler\"" EL
		"	first #(" EL
		"		a" EL
		"		b" EL
		"	)" EL
		")"
	;
	const char* humanReadableString2 =
		"@(" EL
		"	first #(" EL
		"		a" EL
		"		b" EL
		"	)" EL
		"	second \"20% cooler\"" EL
		")"
		;
#undef EL
	
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeNone, "Should have no error");
	
	char* buffer = wexpr_Expression_createStringRepresentation(expr, 0, WexprWriteFlagNone);
	char* buffer2 = wexpr_Expression_createStringRepresentation(expr, 0,  WexprWriteFlagHumanReadable);
	
	WEXPR_UNITTEST_ASSERT (
		(strcmp (buffer, notHumanReadableString1) == 0) ||
		(strcmp (buffer, notHumanReadableString2) == 0), "Should match non-human readable"
	);

	WEXPR_UNITTEST_ASSERT (
		(strcmp (buffer2, humanReadableString1) == 0) ||
		(strcmp (buffer2, humanReadableString2) == 0), "Should match human readable");
	
	free (buffer);
	free (buffer2);
	
	wexpr_Expression_destroy(expr);
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END()

WEXPR_UNITTEST_BEGIN(ExpressionCanChangeType)
	WexprExpression* expr = wexpr_Expression_createNull();
	wexpr_Expression_changeType(expr, WexprExpressionTypeValue);
	
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(expr) == WexprExpressionTypeValue, "Changed type properly");
	
	wexpr_Expression_destroy(expr);
WEXPR_UNITTEST_END()

WEXPR_UNITTEST_BEGIN(ExpressionCanSetValue)
	WexprExpression* expr = wexpr_Expression_createNull();
	wexpr_Expression_changeType(expr, WexprExpressionTypeValue);
	
	wexpr_Expression_valueSet (expr, "asdf");
	
	WEXPR_UNITTEST_ASSERT (strcmp (wexpr_Expression_value(expr), "asdf") == 0, "String not set properly");
	
	wexpr_Expression_destroy(expr);
WEXPR_UNITTEST_END()

WEXPR_UNITTEST_BEGIN(ExpressionCanAddToArray)
	WexprExpression* expr = wexpr_Expression_createNull();
	wexpr_Expression_changeType(expr, WexprExpressionTypeArray);
	
	WexprExpression* elem = wexpr_Expression_createValue("a");
	wexpr_Expression_arrayAddElementToEnd (expr, elem); // transfers ownership
	
	elem = wexpr_Expression_createValue("b");
	wexpr_Expression_arrayAddElementToEnd (expr, elem);
	
	elem = wexpr_Expression_createValue("c");
	wexpr_Expression_arrayAddElementToEnd (expr, elem);
	
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_arrayCount(expr) == 3, "Should have 3 elements");
	
	const char* expected [3] = {
		"a", "b", "c"
	};
	
	for (size_t i=0; i < 3; ++i)
	{
		WexprExpression* val = wexpr_Expression_arrayAt(expr, i);
		WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(val) == WexprExpressionTypeValue, "Should be v alue");
		WEXPR_UNITTEST_ASSERT (strcmp(wexpr_Expression_value(val), expected[i]) == 0, "Value expected");
	}
	
	wexpr_Expression_destroy(expr);
WEXPR_UNITTEST_END()

WEXPR_UNITTEST_BEGIN(ExpressionCanSetInMap)
	WexprExpression* expr = wexpr_Expression_createNull();
	wexpr_Expression_changeType(expr, WexprExpressionTypeMap);
	
	wexpr_Expression_mapSetValueForKey (expr, "key", wexpr_Expression_createValue("value"));
	
	WexprExpression* val = wexpr_Expression_mapValueForKey(expr, "key");
	
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(val) == WexprExpressionTypeValue, "Should be v alue");
	WEXPR_UNITTEST_ASSERT (wexpr_Expression_type(val) == WexprExpressionTypeValue, "Should be v alue");
	
	wexpr_Expression_destroy(expr);
	
WEXPR_UNITTEST_END()

WEXPR_UNITTEST_SUITE_BEGIN (Expression)
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanCreateNull);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanCreateValue);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanCreateQuotedValue);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanCreateNumber);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanCreateArray);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanCreateMap);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionrCanUnderstandReference);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanDerefReference);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanDerefArrayReference);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanDerefMapProperly);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanCreateString);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanChangeType);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanSetValue);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanAddToArray);
	WEXPR_UNITTEST_SUITE_ADDTEST (Expression, ExpressionCanSetInMap);
WEXPR_UNITTEST_SUITE_END ()

#endif // WEXPR_TESTS_EXPRESSION_H
