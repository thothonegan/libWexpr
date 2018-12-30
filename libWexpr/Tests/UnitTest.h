//
/// \file UnitTest.h
/// \brief Quick and dirty unit testing system for C.
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

#ifndef WEXPR_TESTS_UNITTEST_H
#define WEXPR_TESTS_UNITTEST_H

#include <stdio.h> // fprintf
#include <stdlib.h> // malloc/free
#include <string.h> // strcpy

#ifdef __cplusplus
extern "C" {
#endif

typedef enum WexprUnitTestResultCode
{
	WexprUnitTestResultCodeFailure,
	WexprUnitTestResultCodeSuccess
} WexprUnitTestResultCode;

typedef struct WexprUnitTestResult
{
	WexprUnitTestResultCode code;
	const char* file; // failure file. not owned (will be global constant).
	int line; // failure line
	
	char* reason; // if not null, must be malloc'd
} WexprUnitTestResult;

typedef struct WexprSuiteResult
{
	int successes;
	int failures;
} WexprSuiteResult;

#ifdef __cplusplus
	#define WEXPR_UNITTEST_STATICCAST(type, val) static_cast<type>(val)
#else
	#define WEXPR_UNITTEST_STATICCAST(type, val) ((type) val)
#endif

#define WEXPR_UNITTEST_BEGIN(name) \
	WexprUnitTestResult test_##name (void); \
	WexprUnitTestResult test_##name (void) \
	{

#define WEXPR_UNITTEST_END() \
		{ \
			WexprUnitTestResult canis_SuccessResult; \
			canis_SuccessResult.code = WexprUnitTestResultCodeSuccess; \
			canis_SuccessResult.file = 0; \
			canis_SuccessResult.line = 0; \
			canis_SuccessResult.reason = NULL; \
			return canis_SuccessResult; \
		} \
	} /* from begin */

#define WEXPR_UNITTEST_FAIL(reasonString) \
	do { \
		size_t reasonLen = strlen(reasonString); \
		WexprUnitTestResult canis_FailResult; \
		canis_FailResult.code = WexprUnitTestResultCodeFailure; \
		canis_FailResult.reason = WEXPR_UNITTEST_STATICCAST(char*, malloc(reasonLen+1)); \
		strncpy (canis_FailResult.reason, reasonString, reasonLen); \
		canis_FailResult.reason[reasonLen] = '\0'; \
		canis_FailResult.file = __FILE__; \
		canis_FailResult.line = __LINE__; \
		return canis_FailResult; \
	} while (0)

#define WEXPR_UNITTEST_ASSERT(check, reasonString) \
	do { \
		if (!(check)) \
		{ WEXPR_UNITTEST_FAIL(reasonString); } \
	} while (0)

// --- SUITE methods

#define WEXPR_UNITTEST_SUITE_BEGIN(name) \
	WexprSuiteResult suite_##name (void); \
	WexprSuiteResult suite_##name (void) \
	{ \
		WexprSuiteResult suiteRes = {0, 0};

#define WEXPR_UNITTEST_SUITE_ADDTEST(suite, test) \
	do { \
		WexprUnitTestResult res = test_##test(); \
		if (res.code == WexprUnitTestResultCodeFailure) \
		{ \
			suiteRes.failures++; \
			char* r = res.reason; \
			fprintf (stderr, "%s:%d: test error: [" #suite "::" #test "] Failed: %s\n", res.file, res.line, r ? r : "[none]"); \
			free (r); res.reason = NULL; \
		} \
		else \
		{ \
			suiteRes.successes++; \
		} \
	} while (0)
	
#define WEXPR_UNITTEST_SUITE_END() \
	return suiteRes; \
}

#define WEXPR_UNITTEST_SUITE_RUN(name) \
	suite_##name()

#ifdef __cplusplus
}
#endif

#endif // WEXPR_TESTS_UNITTEST_H
