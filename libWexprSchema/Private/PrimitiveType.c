//
/// \file libWexprSchema/PrimitiveType.c
/// \brief Primitive types for schemas
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#include <libWexprSchema/PrimitiveType.h>

#include <string.h>

static const char* s_WexprSchemaPrimitiveTypeNull_String = "null";
static const char* s_WexprSchemaPrimitiveTypeArray_String = "array";
static const char* s_WexprSchemaPrimitiveTypeBinaryData_String = "binaryData";
static const char* s_WexprSchemaPrimitiveTypeMap_String = "map";
static const char* s_WexprSchemaPrimitiveTypeValue_String = "value";
static const char* s_pipeString = "|";

WexprSchemaPrimitiveType wexprSchema_PrimitiveType_fromString (const char* str)
{
	if (str == NULL) { return WexprSchemaPrimitiveTypeUnknown; }

	if (strcmp(str, "nullType") == 0) { return WexprSchemaPrimitiveTypeNull; }
	if (strcmp(str, "array") == 0) { return WexprSchemaPrimitiveTypeArray; }
	if (strcmp(str, "binaryData") == 0) { return WexprSchemaPrimitiveTypeBinaryData; }
	if (strcmp(str, "map") == 0) { return WexprSchemaPrimitiveTypeMap; }
	if (strcmp(str, "value") == 0) { return WexprSchemaPrimitiveTypeValue; }

	return WexprSchemaPrimitiveTypeUnknown;
}

bool wexprSchema_PrimitiveType_matchesExpressionType (WexprSchemaPrimitiveType self, WexprExpressionType type)
{
	switch (type)
	{
		case WexprExpressionTypeNull: return (self & WexprSchemaPrimitiveTypeNull);
		case WexprExpressionTypeArray: return (self & WexprSchemaPrimitiveTypeArray);
		case WexprExpressionTypeBinaryData: return (self & WexprSchemaPrimitiveTypeBinaryData);
		case WexprExpressionTypeMap: return (self & WexprSchemaPrimitiveTypeMap);
		case WexprExpressionTypeValue: return (self & WexprSchemaPrimitiveTypeValue);
		case WexprExpressionTypeInvalid:
		default:
			return false; // unknown
	}
}

WexprSchemaTwine wexprSchema_PrimitiveType_toTwine(WexprSchemaPrimitiveType self)
{
	static WexprSchemaTwine s_twines[5*2-1]; // number of possibilities * 2 - 1

	if (self == WexprSchemaPrimitiveTypeUnknown)
	{
		wexprSchema_Twine_init_CStr_Empty(s_twines+0, "Unknown");
		return s_twines[0];
	}

	// otherwise, go through the possibilities
	int curIndex = 0;
	#define HANDLE(value) \
	do { \
		if (self & value) { \
			if (curIndex == 0) { \
				wexprSchema_Twine_init_CStr_Empty(s_twines+curIndex, s_##value##_String); \
				curIndex += 1; \
			} else { \
				wexprSchema_Twine_init_Twine_CStr(s_twines+curIndex, s_twines+curIndex-1, s_pipeString);\
				wexprSchema_Twine_init_Twine_CStr(s_twines+curIndex+1, s_twines+curIndex, s_##value##_String); \
				curIndex += 2; \
			} \
		} \
	} while (0)

	HANDLE(WexprSchemaPrimitiveTypeNull);
	HANDLE(WexprSchemaPrimitiveTypeArray);
	HANDLE(WexprSchemaPrimitiveTypeBinaryData);
	HANDLE(WexprSchemaPrimitiveTypeMap);
	HANDLE(WexprSchemaPrimitiveTypeValue);

	return s_twines[curIndex-1];
}