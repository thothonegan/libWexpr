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

#include <stdio.h>
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

static char* s_dupLengthString (const char* s, size_t n)
{
	size_t len = n;
	char* result = (char*)malloc (len + 1);
	if (!result)
		return NULL;

	memcpy (result, s, len);
	result[len] = '\0';
	return result;
}

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

static PrivateStringRef s_stringRef_createFromPointerSize (const char* str, size_t size)
{
	PrivateStringRef res = {
		str,
		size
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
		PrivateStringRef sliceStrRef = s_StringRef_slice2(self, i, rhs.size);
		if (s_StringRef_isEqual(sliceStrRef, rhs))
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

static const char* s_StartBlockComment = ";(--";
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
	
	if (len == 0)
		props.isBarewordSafe = false; // empty string is not safe, since that will be nothing
	
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
		elem->key = s_dupLengthString (refName.ptr, refName.size);
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
		char* refStr = s_dupLengthString (refName.ptr, refName.size);
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
		self->m_value.data = s_dupLengthString (val.value.ptr, val.value.size);
		
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

static size_t s_byteSizeForIndent (size_t indent)
{
	return indent; // one \t just costs one byte
}

void s_fillIndent (char* buffer, size_t indent)
{
	for (size_t i=0; i < indent; ++i)
		buffer[i] = '\t';
}

// --------------------- PRIVATE ----------------------------------

// should have no room for the null pointer. append at end
// NOTE THESE BUFFERS ARE ACTUALLY MUTABLE AND WE'RE PASSING AROUND MALLOC OWNED ADDRESSES.
//
// Human Readablle notes:
// even though you pass an indent, we assume you're already indented for the start of the object
// we assume this so that an object for example as a key-value will be writen in the correct spot.
// if it writes multiple lines, we will use the given indent to predict.
// it will end after writing all data, no newline generally at the end.
//
// Note that ownership moves around the PrivateStringRefs automatically.
static PrivateStringRef p_wexpr_Expression_appendStringRepresentationToAllocatedBuffer (WexprExpression* self, WexprWriteFlags flags, size_t indent, PrivateStringRef strBuffer)
{
	
	bool writeHumanReadable = ((flags & WexprWriteFlagHumanReadable) == WexprWriteFlagHumanReadable);
	WexprExpressionType type = wexpr_Expression_type(self);
	
	char* buffer = (char*) strBuffer.ptr;
	size_t curBufferSize = strBuffer.size;
	
	if (type == WexprExpressionTypeNull)
	{
		fprintf (stderr, "p_wexpr_Expression_appendStringRepresentationToAllocatedBuffer() - Cannot represent a null expression.\n");
		abort();
	}
	
	else if (type == WexprExpressionTypeValue)
	{
		// value - always write directly
		
		const char* value = wexpr_Expression_value(self);
		size_t len = strlen(value);
		
		PrivateWexprValueStringProperties props = s_wexprValueStringProperties(
			s_StringRef_create(value)
		);
		
		char* newBuffer = buffer;
		size_t newSize = curBufferSize + len + (props.isBarewordSafe ? 0 : 2); // add quotes if needed
		newBuffer = realloc (newBuffer, newSize);
		
		// copy the value, taking into account quotes or not
		strncpy (newBuffer+curBufferSize + (props.isBarewordSafe ? 0 : 1), value, len);
		
		if (!props.isBarewordSafe)
		{
			// add quotes
			newBuffer[curBufferSize] = '\"';
			newBuffer[newSize-1] = '\"';
		}
		
		return s_stringRef_createFromPointerSize(newBuffer, newSize);
	}
	
	else if (type == WexprExpressionTypeArray)
	{
		size_t arraySize = wexpr_Expression_arrayCount(self);
		
		if (arraySize == 0)
		{
			// straightforward, always empty structure
			size_t newSize = curBufferSize + 3;
			char* newBuffer = realloc(buffer, newSize);
			strncpy (newBuffer+curBufferSize, "#()", 3);
			return s_stringRef_createFromPointerSize(newBuffer, newSize);
		}
		
		// otherwise, we have items
		
		// array : human readable we'll write each one on its own line.
		size_t newSize = curBufferSize + 2 + (writeHumanReadable ? 1 : 0); // room for #( and newline if needed
		char* newBuffer = realloc(buffer, newSize);
		
		if (writeHumanReadable)
			strncpy (newBuffer+curBufferSize, "#(\n", 3);
		else
			strncpy (newBuffer+curBufferSize, "#(", 2);
		
		for (size_t i=0; i < arraySize; ++i)
		{
			WexprExpression* obj = wexpr_Expression_arrayAt(self, i);
			
			// if human readable, we need to indent the line, output the object, then add a newline
			if (writeHumanReadable)
			{
				size_t indentBytes = s_byteSizeForIndent(indent+1);
				newSize += indentBytes;
				newBuffer = realloc(newBuffer, newSize);
				s_fillIndent(newBuffer+newSize-indentBytes, indent+1);
				
				// now add our normal
				PrivateStringRef newBuf = p_wexpr_Expression_appendStringRepresentationToAllocatedBuffer(
					obj, flags, indent+1, 
					s_stringRef_createFromPointerSize(newBuffer, newSize)
				);
				newBuffer = (char*) newBuf.ptr; newSize = newBuf.size;
				
				// add the newline
				newSize += 1;
				newBuffer = realloc(newBuffer, newSize);
				newBuffer[newSize-1] = '\n';
			}
			
			// if not human readable, we just need to either output the object, or put a space then the object
			else
			{
				if (i > 0)
				{
					// we need a space
					newSize += 1;
					newBuffer = realloc(newBuffer, newSize);
					newBuffer[newSize-1] = ' ';
				}
				
				// now add our normal
				PrivateStringRef newBuf = p_wexpr_Expression_appendStringRepresentationToAllocatedBuffer(
					obj, flags, indent,
					s_stringRef_createFromPointerSize (newBuffer, newSize)
				);
				newBuffer = (char*) newBuf.ptr; newSize = newBuf.size;
				
			}
		}
		
		// done with the core of the array
		// if human readable, indent and add the end array
		// otherwise, just add the end array
		if (writeHumanReadable)
		{
			size_t indentBytes = s_byteSizeForIndent(indent);
			newSize += indentBytes;
			newBuffer = realloc(newBuffer, newSize);
			s_fillIndent(newBuffer+newSize-indentBytes, indent);
		}
		
		newSize += 1;
		newBuffer = realloc(newBuffer, newSize);
		newBuffer[newSize-1] = ')';
		
		// and done
		return s_stringRef_createFromPointerSize(newBuffer, newSize);
	}
	
	else if (type == WexprExpressionTypeMap)
	{
		size_t mapSize = wexpr_Expression_mapCount(self);
		
		if (mapSize == 0)
		{
			// straightforward, always empty structure
			size_t newSize = curBufferSize + 3;
			char* newBuffer = realloc(buffer, newSize);
			strncpy (newBuffer + curBufferSize, "@()", 3);
			return s_stringRef_createFromPointerSize(newBuffer, newSize);
		}
		
		// otherwise, we have items
		
		// map : human readable we'll write each one on its own line
		size_t newSize = curBufferSize + 2 + (writeHumanReadable ? 1 : 0); // room for @( and newline if needed
		char* newBuffer = realloc (buffer, newSize);
		
		if (writeHumanReadable)
			strncpy (newBuffer+curBufferSize, "@(\n", 3);
		else
			strncpy (newBuffer+curBufferSize, "@(", 2);
		
		for (size_t i=0; i < mapSize; ++i)
		{
			const char* key = wexpr_Expression_mapKeyAt(self, i);
			size_t keyLength = strlen(key);
			WexprExpression* value = wexpr_Expression_mapValueAt(self, i);
			
			// if human readable, indent the line, output the key, space, object, newline
			if (writeHumanReadable)
			{
				size_t indentBytes = s_byteSizeForIndent(indent+1);
				size_t prevSize = newSize;
				newSize += indentBytes + keyLength + 1; // get us to the object
				newBuffer = realloc(newBuffer, newSize);
				s_fillIndent(newBuffer+prevSize, indent+1);
				strncpy (newBuffer+prevSize+indentBytes, key, keyLength);
				newBuffer[newSize-1] = ' ';
				
				// add the value
				PrivateStringRef newBuf = p_wexpr_Expression_appendStringRepresentationToAllocatedBuffer(
					value, flags, indent+1,
					s_stringRef_createFromPointerSize(newBuffer, newSize)
				);
				newBuffer = (char*) newBuf.ptr; newSize = newBuf.size;
				
				// add the newline
				newSize += 1;
				newBuffer = realloc(newBuffer, newSize);
				newBuffer[newSize-1] = '\n';
			}
			
			// if not human readable, just output with spaces as needed
			else
			{
				if (i > 0)
				{
					// we need a space
					newSize += 1;
					newBuffer = realloc(newBuffer, newSize);
					newBuffer[newSize-1] = ' ';
				}
				
				// now key, space, value
				size_t prevSize = newSize;
				newSize += keyLength+1;
				newBuffer = realloc(newBuffer, newSize);
				strncpy (newBuffer+prevSize, key, keyLength);
				newBuffer[newSize-1] = ' ';
				
				// add our value
				PrivateStringRef newBuf = p_wexpr_Expression_appendStringRepresentationToAllocatedBuffer(
					value, flags, indent+1,
					s_stringRef_createFromPointerSize(newBuffer, newSize)
				);
				newBuffer = (char*) newBuf.ptr; newSize = newBuf.size;
			}
		}
		
		// done with the core of the map
		// if human readable, indent and add the end map
		// otherwise, just add the end map
		if (writeHumanReadable)
		{
			size_t indentBytes = s_byteSizeForIndent(indent);
			newSize += indentBytes;
			newBuffer = realloc(newBuffer, newSize);
			s_fillIndent(newBuffer+newSize-indentBytes, indent);
		}
		
		newSize += 1;
		newBuffer = realloc(newBuffer, newSize);
		newBuffer[newSize-1] = ')';
		
		// and done
		return s_stringRef_createFromPointerSize(newBuffer, newSize);
	}
	
	else
	{
		fprintf (stderr, "p_wexpr_Expression_appendStringRepresentationToAllocatedBuffer() - Unknown type to generate string for\n");
		abort();
	}
}

// ---------------------- PUBLIC -----------------------------------

// --- Construction/Destruction

WexprExpression* wexpr_Expression_createFromString (
	const char* str, WexprParseFlags flags,
	WexprError* error
)
{
	return wexpr_Expression_createFromLengthString (
		str, strlen(str), flags, error
	);
}

WexprExpression* wexpr_Expression_createFromLengthString (
	const char* str, size_t length, WexprParseFlags flags,
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
		PrivateStringRef rest = s_Expression_parseFromString (expr, 
			s_stringRef_createFromPointerSize(str, length),
			flags, &parserState, &err
		);
		
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


WexprExpression* wexpr_Expression_createNull (void)
{
	WexprExpression* expr = malloc (sizeof(WexprExpression));
	expr->m_type = WexprExpressionTypeNull;
	
	return expr;
}

WexprExpression* wexpr_Expression_createValue (const char* val)
{
	WexprExpression* expr = wexpr_Expression_createNull();
	if (expr)
	{
		wexpr_Expression_changeType(expr, WexprExpressionTypeValue);
		wexpr_Expression_valueSet(expr, val);
	}
	
	return expr;
}

WexprExpression* wexpr_Expression_createValueFromLengthString (const char* val, size_t length)
{
	WexprExpression* expr = wexpr_Expression_createNull();
	if (expr)
	{
		wexpr_Expression_changeType(expr, WexprExpressionTypeValue);
		wexpr_Expression_valueSetLengthString(expr, val, length);
	}
	
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
	// null doesnt store anything, so can use this to destroy it
	if (self)
	{
		wexpr_Expression_changeType(self, WexprExpressionTypeNull);
	}
	
	free (self);
}

// --- Information

WexprExpressionType wexpr_Expression_type (WexprExpression* self)
{
	return self->m_type;
}

void wexpr_Expression_changeType (WexprExpression* self, WexprExpressionType type)
{
	// first destroy
	if (self->m_type == WexprExpressionTypeValue)
	{
		free (self->m_value.data);
	}
	
	else if (self->m_type == WexprExpressionTypeArray)
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
	
	else if (self->m_type == WexprExpressionTypeMap)
	{
		hashmap_iterate(self->m_map.hash, &s_freeHashData, NULL);
		
		hashmap_free (self->m_map.hash);
	}
	
	// then set
	self->m_type = type;
	
	// then init
	if (self->m_type == WexprExpressionTypeValue)
	{
		self->m_value.data = NULL;
	}
	
	else if (self->m_type == WexprExpressionTypeArray)
	{
		self->m_array.list = NULL;
		self->m_array.listCount = 0;
	}
	
	else if (self->m_type == WexprExpressionTypeMap)
	{
		self->m_map.hash = hashmap_new();
	}
}

char* wexpr_Expression_createStringRepresentation (WexprExpression* self, size_t indent, WexprWriteFlags flags)
{
	PrivateStringRef ref = p_wexpr_Expression_appendStringRepresentationToAllocatedBuffer (self, flags,
		/*indent*/ indent,
		/*buffer*/ s_StringRef_createInvalid()
	);
	
	// reallocate the for the null
	char* buf = realloc( (void*)ref.ptr, ref.size+1);
	buf[ref.size] = 0;
	
	return buf;
}

// --- Value

const char* wexpr_Expression_value (WexprExpression* self)
{
	if (self->m_type != WexprExpressionTypeValue)
		return NULL;
	
	return self->m_value.data;
}

void wexpr_Expression_valueSet (WexprExpression* self, const char* str)
{
	if (self->m_type != WexprExpressionTypeValue)
		return;
	
	free (self->m_value.data);
	self->m_value.data = strdup(str);
}

void wexpr_Expression_valueSetLengthString (WexprExpression* self, const char* str, size_t length)
{
	if (self->m_type != WexprExpressionTypeValue)
		return;
	
	free (self->m_value.data);
	self->m_value.data = malloc(length+1);
	memcpy (self->m_value.data, str, length);
	self->m_value.data[length] = 0;
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

void wexpr_Expression_arrayAddElementToEnd (WexprExpression* self, WexprExpression* element)
{
	if (self->m_type != WexprExpressionTypeArray)
		return;
	
	WexprExpressionPrivateArrayElement* elem = malloc(sizeof(WexprExpressionPrivateArrayElement));
	elem->expression = element;
	elem->next = NULL;
	
	if (self->m_array.listCount == 0)
	{
		self->m_array.list = elem;
	}
	else
	{
		WexprExpressionPrivateArrayElement* head = self->m_array.list;
		
		while (head->next != NULL) { head = head->next; }
		
		head->next = elem;
	}
	
	++(self->m_array.listCount);
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

WexprExpression* wexpr_Expression_mapValueForLengthKey (WexprExpression* self, const char* key, size_t length)
{
	// key has to be 0 terminated for our hash
	// TODO: use stack functions for this if its short? then we dont have to hit memory
	char* newKey = malloc(length+1);
	memcpy (newKey, key, length);
	newKey[length] = 0; // end terminator
	
	WexprExpression* res = wexpr_Expression_mapValueForKey (self, newKey);
	free (newKey);
	
	return res;
}

void wexpr_Expression_mapSetValueForKey (WexprExpression* self, const char* key, WexprExpression* value)
{
	if (self->m_type != WexprExpressionTypeMap)
		return;
	
	WexprExpressionPrivateMapElement* elem = malloc (sizeof(WexprExpressionPrivateMapElement));
	elem->key = strdup(key);
	elem->value = value;
	
	hashmap_put(self->m_map.hash, elem->key, elem);
}

void wexpr_Expression_mapSetValueForKeyLengthString (WexprExpression* self, const char* key, size_t length, WexprExpression* value)
{
	if (self->m_type != WexprExpressionTypeMap)
		return;
	
	WexprExpressionPrivateMapElement* elem = malloc (sizeof(WexprExpressionPrivateMapElement));
	elem->value = value;
	elem->key = malloc(length+1);
	memcpy (elem->key, key, length);
	elem->key[length] = 0;
	
	hashmap_put(self->m_map.hash, elem->key, elem);
}
