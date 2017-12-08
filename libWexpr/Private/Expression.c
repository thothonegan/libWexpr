//
/// \file libWexpr/Expression.c
/// \brief A wexpr expression
//
// #LICENSE_BEGIN:MIT#
// 
// Copyright (c) 2017, Kenneth Perry (thothonegan)
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

#include <libWexpr/Expression.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ThirdParty/sglib/sglib.h"
#include "ThirdParty/c_hashmap/hashmap.h"

// --- structures

typedef struct WexprExpressionPrivateArrayElement
{
	WexprExpression* expression; // we own
	struct WexprExpressionPrivateArrayElement* next; // next element
} WexprExpressionPrivateArrayElement;

#define WEXPREXPRESSIONPRIVATEARRAYELEMENT_COMPARATOR(e1, e2) ( (char*)(e1->expression) - (char*)(e2->expression))

SGLIB_DEFINE_LIST_PROTOTYPES (WexprExpressionPrivateArrayElement, WEXPREXPRESSIONPRIVATEARRAYELEMENT_COMPARATOR, next)
SGLIB_DEFINE_LIST_FUNCTIONS (WexprExpressionPrivateArrayElement, WEXPREXPRESSIONPRIVATEARRAYELEMENT_COMPARATOR, next)

typedef struct WexprExpressionPrivateMapElement
{
	char* key; // strdup, we own
	WexprExpression* value; // we own
} WexprExpressionPrivateMapElement;

// --- internals to WexprExpression based on the type it is

typedef struct WexprExpressionPrivateValue
{
	char* data; // UTF-8 zero terminated data, we own.
} WexprExpressionPrivateValue;

typedef struct WexprExpressionPrivateMap
{
	map_t hash;
	
} WexprExpressionPrivateMap;

typedef struct WexprExpressionPrivateArray
{
	WexprExpressionPrivateArrayElement* list;
	size_t listCount; // number of items in the list
	
} WexprExpressionPrivateArray;

// privates to WexprExpression
struct WexprExpression
{
	// our type
	WexprExpressionType m_type;
	
	// our data based on type
	union
	{
		WexprExpressionPrivateValue m_value;
		WexprExpressionPrivateMap m_map;
		WexprExpressionPrivateArray m_array;
	};
};

// ---------------------- PRIVATE ----------------------------------

typedef struct PrivateStringRef
{
	const char* ptr;
	size_t size; // in bytes left
} PrivateStringRef;

size_t s_InvalidIndex = SIZE_MAX;

static PrivateStringRef s_StringRef_create (const char* str)
{
	PrivateStringRef res = {
		str,
		strlen(str)
	};
	
	return res;
}

static PrivateStringRef s_StringRef_createInvalid ()
{
	PrivateStringRef res = { NULL, 0};
	return res;
}

static bool s_StringRef_isEqual (PrivateStringRef self, PrivateStringRef rhs)
{
	if (self.size != rhs.size)
		return false;
	
	for (size_t i=0; i < self.size; ++i)
	{
		if (self.ptr[i] != rhs.ptr[i])
			return false;
	}
	
	return true;
}

static PrivateStringRef s_StringRef_slice (PrivateStringRef self, size_t index)
{
	if (index >= self.size)
		return s_StringRef_createInvalid();
	
	PrivateStringRef res = self;
	res.ptr += index;
	res.size -= index;
	
	return res;
}

static PrivateStringRef s_StringRef_slice2 (PrivateStringRef self, size_t index, size_t length)
{
	if (index + length > self.size)
		return s_StringRef_createInvalid();
	
	PrivateStringRef res = self;
	res.ptr += index;
	res.size = length;
	
	return res;
}

static size_t s_StringRef_find (PrivateStringRef self, char character)
{
	for (size_t i=0; i < self.size ; ++i)
	{
		if (self.ptr[i] == character)
			return i;
	}
	
	return s_InvalidIndex;
}

static size_t s_StringRef_findString (PrivateStringRef self, PrivateStringRef rhs)
{
	if (rhs.size > self.size)
		return s_InvalidIndex;
	
	for (size_t i=0; i < self.size; ++i)
	{
		if (s_StringRef_isEqual(s_StringRef_slice(self, rhs.size), rhs))
		{
			return i;
		}
	}
	
	return s_InvalidIndex;
}

typedef struct PrivateParserState
{
	// position in the data we loaded
	uint32_t line;
	uint32_t column;
	
	// alias information list.
	// contains WexprExpressionPrivateMapElement that we own
	map_t aliasHash;
	
} PrivateParserState;

static const char* s_StartBlockComment = ";--(";
static const char* s_EndBlockComment = "--)";

static bool s_isNewline (char c)
{
	return (c == '\r' || c == '\n');
}

static bool s_isWhitespace (char c)
{
	return (c == ' ' || c == '\t' || s_isNewline(c));
}

// trims the given string by removing whitespace or comments from the beginning of the string

static PrivateStringRef s_trimFrontOfString (PrivateStringRef str, PrivateParserState* parserState)
{
	while (true)
	{
		if (str.size == 0) // trimmed everything
			return str;
		
		char first = str.ptr[0];
		
		// skip whitespace
		if (s_isWhitespace(first))
		{
			str = s_StringRef_slice (str, 1);
			
			if (s_isNewline (first))
			{
				parserState->line += 1;
				parserState->column = 0;
			}
			else
			{
				parserState->column += 1;
			}
		}
		
		// comment
		else if (first == ';')
		{
			bool isTillNewline = true;
			
			if (str.size >= 4)
			{
				if (s_StringRef_isEqual(
					s_StringRef_slice2 (str, 0, 4),
					s_StringRef_create(s_StartBlockComment)
				))
				{
					isTillNewline = false;
				}
			}
			
			size_t endIndex = 
				(isTillNewline
					? s_StringRef_find(str, '\n') // end of line
					: s_StringRef_findString(str, s_StringRef_create(s_EndBlockComment))
				);
				
			size_t lengthToSkip = isTillNewline ? 1 : strlen(s_EndBlockComment);
			
			if (endIndex == s_InvalidIndex
				|| endIndex > str.size - lengthToSkip)
			{
				str.size = 0; // dead
			}
			else // slice
			{
				str = s_StringRef_slice (str, endIndex+lengthToSkip); // skip the comment
			}
		}
		
		else
		{
			break;
		}
	}
	
	return str;
}

typedef struct PrivateWexprStringValue
{
	PrivateStringRef value; // the value parsed
	size_t endIndex; // index the end was found (past the value)
} PrivateWexprStringValue;

static PrivateWexprStringValue s_valueOfString (
	PrivateStringRef str,
	PrivateParserState* parserState,
	WexprError* error
)
{
	bool needsCorrespondingQuote = false;
	size_t valueStartPos = 0;
	size_t pos = 0;
	
	if (str.ptr[0] == '"')
	{
		needsCorrespondingQuote = true;
		++pos;
		++valueStartPos;
	}
	
	while (pos < str.size)
	{
		char c = str.ptr[pos];
		
		if (needsCorrespondingQuote)
		{
			// searching for quote
			if (c == '"')
			{
				// done!
				PrivateWexprStringValue res = { s_StringRef_slice2 (str, valueStartPos, pos-valueStartPos), pos+1};
				return res;
			}
		}
		
		else // searching for whitespace/end
		{
			if (s_isWhitespace(c) || c == ')')
			{
				// done! we hit a whitespace or an end construct
				PrivateWexprStringValue res = { s_StringRef_slice2 (str, valueStartPos, pos-valueStartPos), pos};
				return res;
			}
		}
		
		++pos; // next
	}
	
	// done, and didn't find. Return everything
	// unless it was quote, in which its a parse error.
	if (needsCorrespondingQuote)
	{
		if (error)
		{
			// TODO: append error information
			error->code = WexprErrorCodeStringMissingEndingQuote;
			error->message = strdup("String with quote is missing the end quote");
		}
		
		PrivateWexprStringValue res = { s_StringRef_createInvalid(), 0 };
		return res;
	}
	
	PrivateWexprStringValue res = { s_StringRef_slice2 (str, valueStartPos, pos-valueStartPos), pos};
	return res;
}

typedef struct PrivateWexprValueStringProperties
{
	bool isBarewordSafe;
	bool needsEscaping;
} PrivateWexprValueStringProperties;

static PrivateWexprValueStringProperties s_wexprValueStringProperties (PrivateStringRef ref)
{
	PrivateWexprValueStringProperties props;
	
	props.isBarewordSafe = true; // default to being safe
	props.needsEscaping = false; // but we dont need escaping
	
	size_t len = ref.size;
	
	for (size_t i=0; i < len; ++i)
	{
		// For now, we cant escape so that stays false.
		// Bareword safe we'll just check for a few symbols
		char c = ref.ptr[i];
		
		// see any symbols that makes it not bareword safe?
		if (c == '#' || c == '@' || c == '(' || c == ')' || c == ';'
			|| c == '[' || c == ']'
			|| c == '\r' || c == '\n' || c == ' ' || c == '\t' // whitespace must preserve
		)
		{
			props.isBarewordSafe = false;
			break;
		}
	}
	
	return props;
}

static int s_copyToHash (any_t hashToWriteTo, any_t data)
{
	map_t hash = hashToWriteTo;
	WexprExpressionPrivateMapElement* elem = data;
	
	WexprExpressionPrivateMapElement* newElem = malloc(sizeof(WexprExpressionPrivateMapElement));
	newElem->key = strdup (elem->key);
	newElem->value = wexpr_Expression_createCopy(elem->value);
	
	hashmap_put (hash, newElem->key, newElem);
	
	return MAP_OK; // continue
}

// Copy an expression into self. self should be null cause we dont cleanup ourself atm.
static void s_Expression_copyInto (WexprExpression* self, WexprExpression* rhs)
{
	
	// copy recursively
	switch (wexpr_Expression_type(rhs))
	{
		case WexprExpressionTypeValue:
		{
			self->m_type = WexprExpressionTypeValue;
			self->m_value.data = strdup (rhs->m_value.data);
			break;
		}
		
		case WexprExpressionTypeArray:
		{
			self->m_type = WexprExpressionTypeArray;
			self->m_array.listCount = self->m_array.listCount;
			
			for (size_t i=0; i < self->m_array.listCount; ++i)
			{
				WexprExpression* child = wexpr_Expression_arrayAt(rhs, i);
				WexprExpression* childCopy = wexpr_Expression_createCopy(child);
				
				// add to our array
				WexprExpressionPrivateArrayElement* lelem = malloc(sizeof(WexprExpressionPrivateArrayElement));
				lelem->expression = childCopy;
				lelem->next = NULL;
				
				WexprExpressionPrivateArrayElement* endOfList = self->m_array.list;
				if (endOfList) {
					while (endOfList->next) { endOfList = endOfList->next; }
					endOfList->next = lelem;
				}
				else
				{
					self->m_array.list = lelem;
				}
			}
			
			break;
		}
		
		case WexprExpressionTypeMap:
		{
			self->m_type = WexprExpressionTypeMap;
			self->m_map.hash = hashmap_new();
			
			hashmap_iterate(rhs->m_map.hash, &s_copyToHash, self->m_map.hash);
			break;
		}
		
		default:
		{} // ignore
	}
}

// returns the part of the string remaining
// will load into self, setting up everything. Assumes we're empty/null to start.
static PrivateStringRef s_Expression_parseFromString (WexprExpression* self, PrivateStringRef str, WexprParseFlags parseFlags,
	PrivateParserState* parserState, WexprError* error)
{
	if (str.size == 0)
	{
		if (error)
		{
			error->code = WexprErrorCodeEmptyString;
			error->message = strdup("Was told to parse an empty string");
		}
		
		return s_StringRef_createInvalid();
	}
	
	// now we parse
	str = s_trimFrontOfString (str, parserState);
	
	if (str.size == 0)
	{
		return s_StringRef_createInvalid(); // nothing left to parse
	}
	
	// start parsing types:
	// if first two characters are #(, we're an array.
	// if @( we're a map.
	// if [] we're a ref.
	// otherwise, we're a value.
	
	if (str.size >= 2 && s_StringRef_isEqual(s_StringRef_slice2 (str, 0, 2), s_StringRef_create("#(")))
	{
		// We're an array
		self->m_type = WexprExpressionTypeArray;
		self->m_array.listCount = 0;
		self->m_array.list = NULL;
		
		// move our string forward
		str = s_StringRef_slice(str, 2);
		
		// continue building children as needed
		while (true)
		{
			str = s_trimFrontOfString (str, parserState);
			
			if (s_StringRef_isEqual(
				s_StringRef_slice2(str, 0, 1),
				s_StringRef_create(")") // end array
			))
			{
				break; // done
			}
			else
			{
				// parse as a new expression
				WexprExpression* newExpression = wexpr_Expression_createNull();
				str = s_Expression_parseFromString(newExpression, str, parseFlags, parserState, error);
				
				if (error && error->code)
				{
					wexpr_Expression_destroy(newExpression); // not added
					return s_StringRef_createInvalid(); // fail, exit
				}
				
				// otherwise, add it to our array
				WexprExpressionPrivateArrayElement* lelem = malloc(sizeof(WexprExpressionPrivateArrayElement));
				lelem->expression = newExpression;
				lelem->next = NULL;
				
				WexprExpressionPrivateArrayElement* endOfList = self->m_array.list;
				if (endOfList) {
					while (endOfList->next) { endOfList = endOfList->next; }
					endOfList->next = lelem;
				}
				else
				{
					self->m_array.list = lelem;
				}
				
				(self->m_array.listCount)++;
				
			}
		}
		
		str = s_StringRef_slice(str, 1); // remove the end array
		
		// done with array
		return str;
	}
	
	else if (str.size >= 2 && s_StringRef_isEqual(s_StringRef_slice2 (str, 0, 2), s_StringRef_create("@(")))
	{
		// We're a map
		self->m_type = WexprExpressionTypeMap;
		self->m_map.hash = hashmap_new();
		
		// move our string accordingly
		str = s_StringRef_slice(str, 2);
		
		// build our children as needed
		while (true)
		{
			str = s_trimFrontOfString(str, parserState);
			
			if (str.size == 0)
			{
				error->code = WexprErrorCodeMapMissingEndParen;
				error->message = strdup("A Map was missing its ending paren");
				return s_StringRef_createInvalid();
			}
			
			if (str.size >= 1 && s_StringRef_isEqual(
				s_StringRef_slice2(str, 0, 1),
				s_StringRef_create(")") // end map
			))
			{
				break; // done
			}
			
			else
			{
				// parse as a new expression - we'll alternate keys and values
				WexprExpression* keyExpression = wexpr_Expression_createNull();
				str = s_Expression_parseFromString(keyExpression, str, parseFlags, parserState, error);
				
				if (wexpr_Expression_type(keyExpression) != WexprExpressionTypeValue)
				{
					error->code = WexprErrorCodeMapKeyMustBeAValue;
					error->message = strdup("Map keys must be a value");
					
					wexpr_Expression_destroy(keyExpression);
					
					return s_StringRef_createInvalid();
				}
				
				WexprExpression* valueExpression = wexpr_Expression_createNull();
				str = s_Expression_parseFromString(valueExpression, str, parseFlags, parserState, error);
				
				// ok we now have the key and the value
				// both malloc so can free later
				WexprExpressionPrivateMapElement* elem = malloc (sizeof(WexprExpressionPrivateMapElement));
				elem->key = strdup(wexpr_Expression_value(keyExpression));
				elem->value = valueExpression;
				
				hashmap_put(self->m_map.hash, elem->key, elem);
				
				// destroy our key since thats not stored anywhere
				wexpr_Expression_destroy(keyExpression);
			}
		}
		
		// remove the end map
		str = s_StringRef_slice(str, 1);
		
		// done with map
		return str;
	}
	
	else if (str.size >= 1 && s_StringRef_isEqual(s_StringRef_slice2 (str, 0, 1), s_StringRef_create("[")))
	{
		// the current expression being processed is the one the attribute will be linked to.
		
		// process till the closing ]
		size_t endingBracketIndex = s_StringRef_find(str, ']');
		if (endingBracketIndex == s_InvalidIndex)
		{
			error->code = WexprErrorCodeReferenceMissingEndBracket;
			error->message = strdup ("A reference [] is missing its ending bracket");
			
			return s_StringRef_createInvalid();
		}
		
		PrivateStringRef refName = s_StringRef_slice2(str, 1, endingBracketIndex-1);
		str = s_StringRef_slice(str, endingBracketIndex+1);
		
		// continue parsing at the same level : stored the reference name
		PrivateStringRef resultString = s_Expression_parseFromString(self, str, parseFlags, parserState, error);
		if (error->code != WexprErrorCodeNone)
		{
			return s_StringRef_createInvalid(); // failed when parsing
		}
		
		// now bind the ref - creating a copy of what was made. This will be used for the template.
		WexprExpressionPrivateMapElement* elem = malloc(sizeof(WexprExpressionPrivateMapElement));
		elem->key = strndup (refName.ptr, refName.size);
		elem->value = wexpr_Expression_createCopy (self);
		
		hashmap_put(parserState->aliasHash, elem->key, elem);
		
		// and continue
		return resultString;
	}
	
	else if (str.size >= 2 && s_StringRef_isEqual(s_StringRef_slice2 (str, 0, 2), s_StringRef_create("*[")))
	{
		// parse the reference name
		size_t endingBracketIndex = s_StringRef_find(str, ']');
		if (endingBracketIndex == s_InvalidIndex)
		{
			error->code = WexprErrorCodeReferenceInsertMissingEndBracket;
			error->message = strdup ("A reference insert *[] is missing its ending bracket");
			
			return s_StringRef_createInvalid();
		}
		
		PrivateStringRef refName = s_StringRef_slice2(str, 2, endingBracketIndex-2);
		str = s_StringRef_slice(str, endingBracketIndex+1);
	
		WexprExpressionPrivateMapElement* elem = NULL;
		char* refStr = strndup (refName.ptr, refName.size);
		int found = hashmap_get(parserState->aliasHash, refStr, (void**) &elem);
		
		free (refStr);
		
		if (found != MAP_OK || !elem)
		{
			// not found
			error->code = WexprErrorCodeReferenceUnknownReference;
			error->message = strdup ("Tried to insert a reference, but couldn't find it.");
			
			return s_StringRef_createInvalid();
		}
		
		// copy this into ourself
		s_Expression_copyInto (self, elem->value);
		
		return str;
	}
	
	else // its a value
	{
		PrivateWexprStringValue val = s_valueOfString (str, parserState, error);
		
		if (error && error->code != WexprErrorCodeNone)
			return s_StringRef_createInvalid();
		
		self->m_type = WexprExpressionTypeValue;
		self->m_value.data = strndup (val.value.ptr, val.value.size);
		
		return s_StringRef_slice (str, val.endIndex);
	}
}

static int s_freeHashData (any_t userData, any_t data)
{
	WexprExpressionPrivateMapElement* elem = data;
	free (elem->key);
	wexpr_Expression_destroy(elem->value);
	free (elem);
	
	return MAP_OK; // keep iterating
}

// ---------------------- PUBLIC -----------------------------------

// --- Construction/Destruction

WexprExpression* wexpr_Expression_createFromString (
	const char* str, WexprParseFlags flags,
	WexprError* error
)
{
	WexprExpression* expr = malloc (sizeof(WexprExpression));
	expr->m_type = WexprExpressionTypeNull;
	
	PrivateParserState parserState;
	parserState.aliasHash = hashmap_new();
	
	WexprError err = WEXPR_ERROR_INIT();
	
	// we dont check that str is valid UTF8. Possibly TODO [WolfWexpr does].
	if (true)
	{
		// now start parsing
		PrivateStringRef rest = s_Expression_parseFromString (expr, s_StringRef_create(str), flags, &parserState, &err);
		
		PrivateStringRef postRest = s_trimFrontOfString (rest, &parserState);
		
		if (postRest.size != 0)
		{
			err.code = WexprErrorCodeExtraDataAfterParsingRoot;
			err.message = strdup ("Extra data after parsing the root expression");
		}
	}
	else
	{
		err.code = WexprErrorCodeInvalidUTF8;
		err.message = strdup ("Invalid UTF8");
	}
	
	// cleanup our parser state
	hashmap_iterate(parserState.aliasHash, &s_freeHashData, NULL);
	hashmap_free(parserState.aliasHash);
	
	if (err.code != WexprErrorCodeNone)
	{
		wexpr_Expression_destroy(expr);
		
		if (error)
		{
			// move our error so we dont double free
			error->code = err.code; err.code = WexprErrorCodeNone;
			error->message = err.message; err.message = NULL;
		}
		
		WEXPR_ERROR_FREE (err);
		return NULL;
	}
	
	return expr;
}


WexprExpression* wexpr_Expression_createNull ()
{
	WexprExpression* expr = malloc (sizeof(WexprExpression));
	expr->m_type = WexprExpressionTypeNull;
	
	return expr;
}

WexprExpression* wexpr_Expression_createCopy (WexprExpression* rhs)
{
	WexprExpression* expr = wexpr_Expression_createNull();
	
	s_Expression_copyInto(expr, rhs);
	
	return expr; // you own
}

void wexpr_Expression_destroy (WexprExpression* self)
{
	if (self->m_type == WexprExpressionTypeValue)
	{
		free (self->m_value.data);
	}
	
	if (self->m_type == WexprExpressionTypeArray)
	{
		struct sglib_WexprExpressionPrivateArrayElement_iterator it;
		for (WexprExpressionPrivateArrayElement* list = sglib_WexprExpressionPrivateArrayElement_it_init(&it, self->m_array.list);
			 list != NULL; list = sglib_WexprExpressionPrivateArrayElement_it_next(&it))
		{
			wexpr_Expression_destroy (list->expression);
			free (list);
		}
		self->m_array.listCount = 0;
	}
	
	if (self->m_type == WexprExpressionTypeMap)
	{
		hashmap_iterate(self->m_map.hash, &s_freeHashData, NULL);
		
		hashmap_free (self->m_map.hash);
	}
	
	free (self);
}

// --- Information

WexprExpressionType wexpr_Expression_type (WexprExpression* self)
{
	return self->m_type;
}

// --- Value

const char* wexpr_Expression_value (WexprExpression* self)
{
	if (self->m_type != WexprExpressionTypeValue)
		return NULL;
	
	return self->m_value.data;
}

// --- Array

size_t wexpr_Expression_arrayCount (WexprExpression* self)
{
	if (self->m_type != WexprExpressionTypeArray)
		return 0;
	
	return self->m_array.listCount;
}

WexprExpression* wexpr_Expression_arrayAt (WexprExpression* self, size_t index)
{
	if (self->m_type != WexprExpressionTypeArray)
		return NULL;
	
	for (WexprExpressionPrivateArrayElement* list = self->m_array.list;
		 list != NULL; list = list->next)
	{
		if (index == 0)
			return list->expression;
		
		--index;
	}
	
	// Couldnt find, out of range.
	return NULL;
}

// --- Map

size_t wexpr_Expression_mapCount (WexprExpression* self)
{
	if (self->m_type != WexprExpressionTypeMap)
		return 0;
	
	return hashmap_length(self->m_map.hash);
}

typedef struct PrivateGetKeyValueAtIndex
{
	size_t index; // the index we're requesting
	void* result; // will be null if not found, or a value if found.
} PrivateGetKeyValueAtIndex;

static int s_getKeyAtIndex (any_t userData, any_t data)
{
	PrivateGetKeyValueAtIndex* ud = userData;
	WexprExpressionPrivateMapElement* elem = data;
	
	if (ud->index == 0)
	{
		ud->result = elem->key;
		return !MAP_OK; // exit
	}
	else
	{
		ud->index -= 1;
	}
	
	return MAP_OK;
}

static int s_getValueAtIndex (any_t userData, any_t data)
{
	PrivateGetKeyValueAtIndex* ud = userData;
	WexprExpressionPrivateMapElement* elem = data;
	
	if (ud->index == 0)
	{
		ud->result = elem->value;
		return !MAP_OK; // exit
	}
	else
	{
		ud->index -= 1;
	}
	
	return MAP_OK;
}

const char* wexpr_Expression_mapKeyAt (WexprExpression* self, size_t index)
{
	if (self->m_type != WexprExpressionTypeMap)
		return NULL; // not a map
	
	PrivateGetKeyValueAtIndex val;
	val.index = index;
	val.result = NULL;
	
	hashmap_iterate (self->m_map.hash, &s_getKeyAtIndex, &val);
	
	return val.result;
}

WexprExpression* wexpr_Expression_mapValueAt (WexprExpression* self, size_t index)
{
	if (self->m_type != WexprExpressionTypeMap)
		return NULL; // not a map
		
	PrivateGetKeyValueAtIndex val;
	val.index = index;
	val.result = NULL;
	
	hashmap_iterate (self->m_map.hash, &s_getValueAtIndex, &val);
	
	return val.result;
}

WexprExpression* wexpr_Expression_mapValueForKey (WexprExpression* self, const char* key)
{
	if (self->m_type != WexprExpressionTypeMap)
		return NULL; // not a map
	
	WexprExpressionPrivateMapElement* elem = NULL;
	int res = hashmap_get (self->m_map.hash, (char*) key, (void**) &elem);
	
	if (res == MAP_OK && elem)
	{
		return elem->value;
	}
	
	return NULL;
}
