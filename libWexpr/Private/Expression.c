//
/// \file libWexpr/Expression.c
/// \brief A wexpr expression
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

#include <libWexpr/Expression.h>

#include <libWexpr/Endian.h>
#include <libWexpr/UVLQ64.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "Base64.h"

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

typedef struct WexprExpressionPrivateBinaryData
{
	void* data;
	size_t size; // in bytes
} WexprExpressionPrivateBinaryData;

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
		WexprExpressionPrivateBinaryData m_binaryData;
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
	WexprLineNumber line;
	WexprColumnNumber column;
	
	// alias information list.
	// contains WexprExpressionPrivateMapElement that we own
	map_t aliasHash;
	
} PrivateParserState;

void s_privateParserState_init (PrivateParserState* state)
{
	state->aliasHash = hashmap_new();
	
	// first position in the file
	state->line = 1;
	state->column = 1;
}

void s_privateParserState_free (PrivateParserState* state)
{
	hashmap_free(state->aliasHash);
}

void s_privateParserState_moveForwardBasedOnString (PrivateParserState* parserState, PrivateStringRef str)
{
	for (size_t i=0; i < str.size; ++i)
	{
		if (str.ptr[i] == '\n') // newline
		{
			parserState->line += 1;
			parserState->column = 1;
		}
		else // normal
		{
			parserState->column += 1;
		}
	}
}

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

static bool s_isNotBarewordSafe (char c)
{
	return (c == '*'
		|| c == '#'
		|| c == '@'
		|| c == '(' || c == ')'
		|| c == '[' || c == ']'
		|| c == '^'
		|| c == '<' || c == '>'
		|| c == '"'
		|| c == ';'
		|| s_isWhitespace(c)
	);
}

static bool s_isEscapeValid (char c)
{
	return (c == '"' || c == 'r' || c == 'n' || c == 't' || c == '\\');
}

static char s_valueForEscape (char c)
{
	if (c == '"') return '"';
	if (c == 'r') return '\r';
	if (c == 'n') return '\n';
	if (c == 't') return '\t';
	if (c == '\\') return '\\';
	
	return 0; // invalid escape
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
				parserState->column = 1;
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
			
			// Move forward columns/rows as needed
			s_privateParserState_moveForwardBasedOnString(
				parserState,
				s_stringRef_createFromPointerSize(
					str.ptr, (endIndex == s_InvalidIndex)
						? str.size : (endIndex+lengthToSkip)
				)
			);
			
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
	char* value; // the value parsed. You own (malloc)
	size_t endIndex; // index the end was found (past the value)
} PrivateWexprStringValue;

// Will copy out the value of the string to a new buffer.
// The buffer is mallocd and must be freed by the caller.
// Returns NULL on failure.
static PrivateWexprStringValue s_createValueOfString (
	PrivateStringRef str,
	PrivateParserState* parserState,
	WexprError* error
)
{
	// two pass:
	// first pass, get the length of the size
	// second pass, store the buffer
	
	size_t bufferLength = 0;
	bool isQuotedString = false;
	bool isEscaped = false;
	size_t pos = 0; // position we're parsing at
	
	if (str.ptr[0] == '"')
	{
		isQuotedString = true;
		++pos;
	}
	
	while (pos < str.size)
	{
		char c = str.ptr[pos];
		
		if (isQuotedString)
		{
			if (isEscaped)
			{
				// we're in an escape. is it valid?
				if (s_isEscapeValid(c))
				{
					++bufferLength; // counts
					isEscaped = false; // escape ended
				}
				else
				{
					if (error)
					{
						error->code = WexprErrorCodeInvalidStringEscape;
						error->message = "Invalid escape found in the string";
						error->column = parserState->column;
						error->line = parserState->line;
					}
					
					PrivateWexprStringValue ret;
					ret.value = NULL;
					ret.endIndex = pos;
					return ret;
				}
			}
			else
			{
				if (c == '"')
				{
					// end quote - part of us
					++pos;
					break;
				}
				else if (c == '\\')
				{
					// we're escaping
					isEscaped = true;
				}
				else
				{
					// otherwise it's a character
					++bufferLength;
				}
			}
		}
		else
		{
			// have we ended the word?
			if (s_isNotBarewordSafe(c))
			{
				// ended - not part of us
				break;
			}
			
			// otherwise, its a character
			++bufferLength;
		}
		
		++pos;
	}
	
	if (bufferLength == 0 && !isQuotedString) // cannot have an empty barewords string
	{
		if (error)
		{
			error->code = WexprErrorCodeEmptyString;
			error->message = strdup("Was told to parse an empty string");
			error->line = parserState->line;
			error->column = parserState->column;
		}
		
		PrivateWexprStringValue ret;
		ret.value = NULL;
		ret.endIndex = 0;
		
		return ret;
	}
	
	size_t end = pos;
	
	// we now know our buffer size and the string has been checked
	char* buffer = malloc(bufferLength+1);
	if (!buffer) {
		PrivateWexprStringValue ret;
		ret.value = NULL;
		ret.endIndex = end;
		return ret;
	}
	
	memset(buffer, 0, bufferLength+1);
	
	size_t writePos = 0;
	pos = 0;
	if (isQuotedString) pos = 1;
	
	while (writePos < bufferLength)
	{
		char c = str.ptr[pos];
		
		if (isQuotedString)
		{
			if (isEscaped)
			{
				char escapedValue = s_valueForEscape(c);
				buffer[writePos] = escapedValue;
				++writePos;
				
				isEscaped = false;
			}
			else
			{
				if (c == '\\')
				{
					// we're escaping
					isEscaped = true;
				}
				else
				{
					// otherwise it's a character
					buffer[writePos] = c;
					++writePos;
				}
			}
		}
		else
		{
			// its a character
			buffer[writePos] = c;
			++writePos;
		}
		
		// next character
		++pos;
	}
	
	PrivateWexprStringValue ret;
	ret.value = buffer;
	ret.endIndex = end;
	
	return ret;
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
		if (s_isNotBarewordSafe(c))
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
			self->m_array.list = NULL;
			self->m_array.listCount = rhs->m_array.listCount;
			
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

// returns the part of the buffer remaining
// will load into self, setting up everything. Assumes we're empty/null to start.
static WexprBuffer s_Expression_parseFromBinaryChunk (WexprExpression* self, WexprBuffer data, WexprError* error)
{
	
	if (data.byteSize < (1 + sizeof(uint8_t))) // minimum of 1
	{
		if (error)
		{
			error->message = strdup ("Chunk not big enough for header");
			error->code = WexprErrorCodeBinaryChunkNotBigEnough;
		}
		
		WexprBuffer buf;
		buf.byteSize = 0; buf.data = NULL;
		return buf;
	}
	
	const uint8_t* buf = data.data;
	
	#define BUFCAST(buf, position, type) ((type)((uint8_t*)buf+(position)))
	
	uint64_t size = 0;
	const uint8_t* dataNewPos = wexpr_uvlq64_read(
		BUFCAST(buf, 0, uint8_t*),
		data.byteSize,
		&size
	);
	
	size_t sizeSize = (size_t)(dataNewPos - buf);
	
	uint8_t chunkType = *BUFCAST(buf, sizeSize, uint8_t*);
	
	size_t readAmount = sizeSize + sizeof(uint8_t);
	
	#define RETURN_REST() \
		{ \
			WexprBuffer rest; \
			rest.byteSize = data.byteSize - readAmount; \
			rest.data = (uint8_t*)buf + readAmount; \
			return rest; \
		} while (0)
		
	if (chunkType == WexprExpressionTypeNull)
	{
		// nothing more to do
		wexpr_Expression_changeType(self, WexprExpressionTypeNull);
		
		RETURN_REST();
	}
	
	else if (chunkType == WexprExpressionTypeValue)
	{
		// data is the entire binary data
		wexpr_Expression_changeType(self, WexprExpressionTypeValue);
		wexpr_Expression_valueSetLengthString(self, 
			BUFCAST(buf, readAmount, const char*), size
		);
		
		readAmount += size;
		
		RETURN_REST();
	}
	
	else if (chunkType == WexprExpressionTypeArray)
	{
		// data is child chunks
		wexpr_Expression_changeType(self, WexprExpressionTypeArray);
		
		size_t curPos = 0;
		
		// build children as needed
		while (curPos < size)
		{
			// read a new element
			size_t startSize = size-curPos;
			WexprBuffer inBuf;
			inBuf.data = BUFCAST(buf, readAmount+curPos, const void*);
			inBuf.byteSize = startSize;
			
			WexprExpression* childExpr = wexpr_Expression_createInvalid();
			WexprBuffer remaining = s_Expression_parseFromBinaryChunk(
				childExpr,
				inBuf,
				error
			);
			
			curPos += (startSize - remaining.byteSize);
			
			if (remaining.data == NULL)
			{
				// failure when parsing the array
				WexprBuffer buf;
				buf.byteSize = 0; buf.data = NULL;
				return buf;
			}
			
			// otherwise, add it
			WexprExpressionPrivateArrayElement* lelem = malloc(sizeof(WexprExpressionPrivateArrayElement));
				lelem->expression = childExpr;
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
		
		readAmount += curPos;
		RETURN_REST();
	}
	
	else if (chunkType == WexprExpressionTypeMap)
	{
		// data is key,value chunks
		wexpr_Expression_changeType(self, WexprExpressionTypeMap);
		
		size_t curPos = 0;
		
		// build children as needed
		while (curPos < size)
		{
			// read a new key
			size_t startSize = size-curPos;
			WexprBuffer inBuf;
			inBuf.data = BUFCAST(buf, readAmount+curPos, const void*);
			inBuf.byteSize = startSize;
			
			WexprExpression* keyExpression = wexpr_Expression_createInvalid();
			WexprBuffer remaining = s_Expression_parseFromBinaryChunk(
				keyExpression,
				inBuf,
				error
			);
			
			size_t keySize = (startSize - remaining.byteSize);
			curPos += keySize;
			
			if (remaining.data == NULL)
			{
				// failure when parsing the child
				WexprBuffer buf;
				buf.byteSize = 0; buf.data = NULL;
				return buf;
			}
			
			// now parse the value
			WexprExpression* valueExpr = wexpr_Expression_createInvalid();
			remaining = s_Expression_parseFromBinaryChunk(
				valueExpr,
				remaining,
				error
			);
			
			curPos += (startSize - remaining.byteSize - keySize);
			
			if (remaining.data == NULL)
			{
				// failure when parsing the child
				WexprBuffer buf;
				buf.byteSize = 0; buf.data = NULL;
				return buf;
			}
			
			// now add it
			// both malloc so can free later
			WexprExpressionPrivateMapElement* elem = malloc (sizeof(WexprExpressionPrivateMapElement));
			elem->key = strdup(wexpr_Expression_value(keyExpression));
			elem->value = valueExpr;
			
			hashmap_put(self->m_map.hash, elem->key, elem);
			
			// destroy our key since thats not stored anywhere
			wexpr_Expression_destroy(keyExpression);
		}
		
		readAmount += curPos;
		RETURN_REST();
	}
	
	else if (chunkType == WexprExpressionTypeBinaryData)
	{
		// data is the entire binary data
		// first byte is the compression
		uint8_t compression = *BUFCAST(buf, readAmount, uint8_t*);
		
		if (compression != 0x00)
		{
			if (error)
			{
				error->message = strdup ("Unknown compression method to use");
				error->code = WexprErrorCodeBinaryUnknownCompression;
			}
			
			WexprBuffer buf;
			buf.byteSize = 0; buf.data = NULL;
			return buf;
		}
		
		// raw compression
		wexpr_Expression_changeType(self, WexprExpressionTypeBinaryData);
		wexpr_Expression_binaryData_setValue(self, 
			BUFCAST(buf, readAmount+1, const char*), size-1
		);
		
		readAmount += size;
		
		RETURN_REST();
	}
	
	else
	{
		// unknown type
		if (error)
		{
			error->message = strdup ("Unknown chunk type to read");
			error->code = WexprErrorCodeBinaryChunkNotBigEnough;
		}
		
		WexprBuffer rest;
		rest.byteSize = 0; rest.data = NULL;
		return rest;
	}
	
	#undef BUFCAST
	#undef RETURN_REST
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
			error->line = parserState->line;
			error->column = parserState->column;
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
	// if < we're a binary string
	// otherwise, we're a value.
	
	if (str.size >= 2 && s_StringRef_isEqual(s_StringRef_slice2 (str, 0, 2), s_StringRef_create("#(")))
	{
		// We're an array
		self->m_type = WexprExpressionTypeArray;
		self->m_array.listCount = 0;
		self->m_array.list = NULL;
		
		// move our string forward
		str = s_StringRef_slice(str, 2);
		parserState->column += 2;
		
		// continue building children as needed
		while (true)
		{
			str = s_trimFrontOfString (str, parserState);
			
			if (str.size == 0)
			{
				error->code = WexprErrorCodeArrayMissingEndParen;
				error->message = strdup("An Array was missing its ending paren");
				error->line = parserState->line;
				error->column = parserState->column;
				
				return s_StringRef_createInvalid();
			}
			
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
		parserState->column += 1;
		
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
		parserState->column += 2;
		
		// build our children as needed
		while (true)
		{
			str = s_trimFrontOfString(str, parserState);
			
			if (str.size == 0)
			{
				error->code = WexprErrorCodeMapMissingEndParen;
				error->message = strdup("A Map was missing its ending paren");
				error->line = parserState->line;
				error->column = parserState->column;
				
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
				// keep our previous position just in case the value is bad
				WexprLineNumber prevLine = parserState->line;
				WexprColumnNumber prevColumn = parserState->column;
				
				WexprExpression* keyExpression = wexpr_Expression_createNull();
				str = s_Expression_parseFromString(keyExpression, str, parseFlags, parserState, error);
				
				if (wexpr_Expression_type(keyExpression) != WexprExpressionTypeValue)
				{
					error->code = WexprErrorCodeMapKeyMustBeAValue;
					error->message = strdup("Map keys must be a value");
					error->line = prevLine;
					error->column = prevColumn;
				
					wexpr_Expression_destroy(keyExpression);
					
					return s_StringRef_createInvalid();
				}
				
				WexprExpression* valueExpression = wexpr_Expression_createInvalid();
				str = s_Expression_parseFromString(valueExpression, str, parseFlags, parserState, error);
				
				if (valueExpression->m_type == WexprExpressionTypeInvalid)
				{
					// it wasnt filled in! no key found.
					error->code = WexprErrorCodeMapNoValue;
					error->message = strdup("Map key must have a value");
					error->line = prevLine;
					error->column = prevColumn;
				
					wexpr_Expression_destroy(keyExpression);
					wexpr_Expression_destroy(valueExpression);
					
					return s_StringRef_createInvalid();
				}
				
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
		parserState->column += 1;
		
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
			error->line = parserState->line;
			error->column = parserState->column;
			
			return s_StringRef_createInvalid();
		}
		
		PrivateStringRef refName = s_StringRef_slice2(str, 1, endingBracketIndex-1);
		
		// validate the contents
		bool invalidName = false;
		for (size_t i=0; i < refName.size; ++i)
		{
			char v = refName.ptr[i];
			
			bool isAlpha = (v >= 'a' && v <= 'z') || (v >= 'A' && v <= 'Z');
			bool isNumber = (v >= '0' && v <= '9');
			bool isUnder = (v == '_');
			
			if (i == 0 && (isAlpha || isUnder))
			{}
			else if (i != 0 && (isAlpha || isNumber || isUnder))
			{}
			else
			{
				invalidName = true;
				break;
			}
		}
		
		if (invalidName)
		{
			if (error)
			{
				error->code = WexprErrorCodeReferenceInvalidName;
				error->message = strdup ("A reference doesn't have a valid name");
				error->line = parserState->line;
				error->column = parserState->column;
				
				return s_StringRef_createInvalid();
			}
			
			return s_StringRef_createInvalid();
		}
		
		// move forward
		s_privateParserState_moveForwardBasedOnString(parserState, 
			s_StringRef_slice2(str, 0, endingBracketIndex+1)
		);
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
			error->line = parserState->line;
			error->column = parserState->column;
			
			return s_StringRef_createInvalid();
		}
		
		PrivateStringRef refName = s_StringRef_slice2(str, 2, endingBracketIndex-2);
		
		// move forward
		s_privateParserState_moveForwardBasedOnString(parserState, 
			s_StringRef_slice2(str, 0, endingBracketIndex+1)
		);
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
			error->line = parserState->line;
			error->column = parserState->column;
			
			return s_StringRef_createInvalid();
		}
		
		// copy this into ourself
		s_Expression_copyInto (self, elem->value);
		
		return str;
	}
	
	// null expressions will be treated as a value, and then parsed seperately
	
	else if (
		str.size >= 1 && s_StringRef_isEqual(
			s_StringRef_slice2(str, 0, 1), s_StringRef_create("<")
		)
	)
	{
		// look for the ending >
		size_t endingQuote = s_StringRef_find(str, '>');
		if (endingQuote == s_InvalidIndex)
		{
			// not found
			error->code = WexprErrorCodeBinaryDataNoEnding;
			error->message = strdup ("Tried to find the ending > for binary data, but not found.");
			error->line = parserState->line;
			error->column = parserState->column;
			
			return s_StringRef_createInvalid();
		}
		
		Base64IBuffer inputBuf;
		inputBuf.buffer = str.ptr+1;
		inputBuf.size = endingQuote-1; // -1 for starting quote. ending was not part.
		Base64Buffer outBuf = base64_decode(inputBuf);
		
		if (outBuf.buffer == NULL)
		{
			error->code = WexprErrorCodeBinaryDataInvalidBase64;
			error->message = strdup ("Unable to decode the base64 data.");
			error->line = parserState->line;
			error->column = parserState->column;
			
			return s_StringRef_createInvalid();
		}
		
		self->m_type = WexprExpressionTypeBinaryData;
		self->m_binaryData.data = outBuf.buffer;
		self->m_binaryData.size = outBuf.size;
		
		s_privateParserState_moveForwardBasedOnString (parserState,
			s_StringRef_slice2 (str, 0, endingQuote+1)
		);
		
		return s_StringRef_slice (str, endingQuote+1);
	}
	
	else if (str.size >= 1)// its a value : must be at least one character
	{
		PrivateWexprStringValue val = s_createValueOfString (str, parserState, error);
		
		if (error && error->code != WexprErrorCodeNone)
			return s_StringRef_createInvalid();
		
		// was it a null/nil string?
		if ((strcmp (val.value, "nil") == 0) || (strcmp (val.value, "null") == 0))
		{
			self->m_type = WexprExpressionTypeNull;
			
			// we dont need the value anymore, trash it
			free(val.value);
			val.value = LIBWEXPR_NULLPTR;
		}
		else
		{
			self->m_type = WexprExpressionTypeValue;
			self->m_value.data = val.value;
		}
		
		s_privateParserState_moveForwardBasedOnString (parserState,
			s_StringRef_slice2(
				str, 0, val.endIndex
			)
		);
		
		return s_StringRef_slice (str, val.endIndex);
	}
	
	// otherwise, we have no idea what happened
	return s_StringRef_createInvalid();
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
		size_t newSize = curBufferSize + 4;
		char* newBuffer = realloc(buffer, newSize);
		
		strncpy (newBuffer+curBufferSize, "null", 4);
		return s_stringRef_createFromPointerSize(newBuffer, newSize);
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
	
	else if (type == WexprExpressionTypeBinaryData)
	{
		// binary data - encode as Base64
		const void* buf = wexpr_Expression_binaryData_data(self);
		size_t size = wexpr_Expression_binaryData_size(self);
		
		Base64IBuffer ibuf;
		ibuf.buffer = buf;
		ibuf.size = size;
		
		Base64Buffer outBuf = base64_encode(ibuf);
		size_t newSize = curBufferSize + 2 + outBuf.size;
		char* newBuffer = realloc(buffer, newSize);
		strncpy (newBuffer+curBufferSize, "<", 1); curBufferSize += 1;
		
		strncpy (newBuffer+curBufferSize, outBuf.buffer, outBuf.size);
		curBufferSize += outBuf.size;
		
		strncpy(newBuffer+curBufferSize, ">", 1);
		curBufferSize += 1;
		
		// cleanup our buffer
		free (outBuf.buffer);
		outBuf.buffer = NULL;
		
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
			if (!key)
				continue; // we shouldnt ever get an empty key, but its possible currently in the case of dereffing in a key for some reason : @([a]a b *[a] c)
			
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
	expr->m_type = WexprExpressionTypeInvalid;
	
	PrivateParserState parserState;
	s_privateParserState_init (&parserState);
	
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
			err.line = parserState.line;
			err.column = parserState.column;
		}
		
		if (expr->m_type == WexprExpressionTypeInvalid && err.code == WexprErrorCodeNone)
		{
			// we didnt get an expression and no error currently reported
			err.code = WexprErrorCodeEmptyString;
			err.message = strdup ("No expression found [remained invalid]");
			err.line = parserState.line;
			err.column = parserState.column;
		}
	}
	else
	{
		err.code = WexprErrorCodeInvalidUTF8;
		err.message = strdup ("Invalid UTF8");
	}
	
	// cleanup our parser state
	hashmap_iterate(parserState.aliasHash, &s_freeHashData, NULL);
	
	if (err.code != WexprErrorCodeNone)
	{
		wexpr_Expression_destroy(expr);
		
		if (error)
		{
			WEXPR_ERROR_MOVE(error, &err);
		}
		
		WEXPR_ERROR_FREE (err);
		
		s_privateParserState_free(&parserState);
		
		return NULL;
	}
	
	s_privateParserState_free(&parserState);
	
	return expr;
}

WexprExpression* wexpr_Expression_createFromBinaryChunk (
	const void* data, size_t length, WexprError* error
)
{
	WexprExpression* expr = malloc (sizeof(WexprExpression));
	expr->m_type = WexprExpressionTypeInvalid;
	
	WexprError err = WEXPR_ERROR_INIT();
	
	WexprBuffer inBuf;
	inBuf.data = data;
	inBuf.byteSize = length;
	
	WexprBuffer buf = s_Expression_parseFromBinaryChunk (
		expr, inBuf, &err
	);
	
	(void) buf; // unused, remaining part of buffer
	
	if (err.code != WexprErrorCodeNone)
	{
		wexpr_Expression_destroy (expr);
		expr = NULL;
		
		if (error)
		{
			WEXPR_ERROR_MOVE(error, &err);
		}
		
		WEXPR_ERROR_FREE (err);
	}
	
	return expr;
}

WexprExpression* wexpr_Expression_createInvalid (void)
{
	WexprExpression* expr = malloc (sizeof(WexprExpression));
	expr->m_type = WexprExpressionTypeInvalid;
	
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
	
	else if (self->m_type == WexprExpressionTypeBinaryData)
	{
		free(self->m_binaryData.data);
		self->m_binaryData.size = 0;
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
	
	else if (self->m_type == WexprExpressionTypeBinaryData)
	{
		self->m_binaryData.data = NULL;
		self->m_binaryData.size = 0;
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

WexprMutableBuffer wexpr_Expression_createBinaryRepresentation (WexprExpression* self)
{
	WexprMutableBuffer buf;
	buf.byteSize = 0;
	buf.data = 0;
	
	WexprExpressionType type = wexpr_Expression_type(self);
	
	#define BUFCAST(buf, position, type) ((type)((uint8_t*)buf+(position)))
	
	if (type == WexprExpressionTypeNull)
	{
		size_t sizeSize = wexpr_uvlq64_bytesize(0);
		
		// data is 0x0
		buf.byteSize = sizeSize + sizeof(uint8_t);
		buf.data = realloc(buf.data, buf.byteSize);
		
		wexpr_uvlq64_write ( (uint8_t*)(buf.data) + 0, sizeSize, 0);
		*BUFCAST (buf.data, sizeSize, uint8_t*) = 0x00;
	}
	
	else if (type == WexprExpressionTypeValue)
	{
		const char* val = wexpr_Expression_value(self);
		size_t valLength = strlen(val);
		
		size_t sizeSize = wexpr_uvlq64_bytesize(valLength);
		
		buf.byteSize = sizeSize + sizeof(uint8_t) + valLength;
		buf.data = realloc(buf.data, buf.byteSize);
		
		wexpr_uvlq64_write ((uint8_t*)(buf.data) + 0, sizeSize, valLength);
		*BUFCAST (buf.data, sizeSize, uint8_t*) = 0x01;
		memcpy ((uint8_t*)buf.data + sizeSize + 1, val, valLength);
	}
	
	else if (type == WexprExpressionTypeArray)
	{
		// first pass, figure out the total size
		size_t sizeOfArrayContents = 0;
		const size_t len = wexpr_Expression_arrayCount(self);
		for (size_t i=0; i < len; ++i)
		{
			// TODO: dont create the entire representation twice - ability to ask for size
			WexprMutableBuffer childBuffer = wexpr_Expression_createBinaryRepresentation(
				wexpr_Expression_arrayAt(self, i)
			);
			
			sizeOfArrayContents += childBuffer.byteSize;
			free (childBuffer.data);
		}
		
		size_t sizeSize = wexpr_uvlq64_bytesize(sizeOfArrayContents);
		
		// header
		buf.byteSize = sizeSize + sizeof(uint8_t) + sizeOfArrayContents;
		buf.data = realloc(buf.data, buf.byteSize);
		
		wexpr_uvlq64_write ((uint8_t*)(buf.data) + 0, sizeSize, sizeOfArrayContents);
		*BUFCAST (buf.data, sizeSize, uint8_t*) = 0x02; // write the array buffer
		
		// data
		size_t curPos = sizeSize+1;
		for (size_t i=0; i < len; ++i)
		{
			WexprMutableBuffer childBuffer = wexpr_Expression_createBinaryRepresentation(
				wexpr_Expression_arrayAt(self, i)
			);
			
			memcpy ((uint8_t*)buf.data + curPos, childBuffer.data, childBuffer.byteSize);
			
			free (childBuffer.data);
			curPos += childBuffer.byteSize;
		}
		
		// done
	}
	
	else if (type == WexprExpressionTypeMap)
	{
		// first pass, figure out total size
		size_t sizeOfMapContents = 0;
		const size_t len = wexpr_Expression_mapCount(self);
		
		for (size_t i=0; i < len; ++i)
		{
			// TODO: dont create the entire representation twice - ability to ask for sizes
			const char* mapKey = wexpr_Expression_mapKeyAt(self, i);
			size_t mapKeyLen = strlen(mapKey);
			
			// write the map key as a new value
			size_t keySizeSize = wexpr_uvlq64_bytesize(mapKeyLen);
			size_t keySize = keySizeSize + sizeof(uint8_t) + mapKeyLen;
			sizeOfMapContents += keySize;
			
			// write the map value
			WexprExpression* mapValue = wexpr_Expression_mapValueAt(self, i);
			WexprMutableBuffer childBuffer = wexpr_Expression_createBinaryRepresentation(
				mapValue
			);
			sizeOfMapContents += childBuffer.byteSize;
			free (childBuffer.data);
		}
		
		// second pass, write the header and pairs
		
		size_t sizeSize = wexpr_uvlq64_bytesize(sizeOfMapContents);
		
		// key value pairs
		buf.byteSize = sizeSize + sizeof(uint8_t) + sizeOfMapContents;
		buf.data = realloc(buf.data, buf.byteSize);
		wexpr_uvlq64_write ((uint8_t*)(buf.data) + 0, sizeSize, sizeOfMapContents);
		*BUFCAST (buf.data, sizeSize, uint8_t*) = 0x03; // write the map buffer
		
		size_t curPos = sizeSize+1;
		for (size_t i=0; i < len; ++i)
		{
			const char* mapKey = wexpr_Expression_mapKeyAt(self, i);
			size_t mapKeyLen = strlen(mapKey);
			
			// write the map key as a new value
			size_t keySizeSize = wexpr_uvlq64_bytesize(mapKeyLen);
			size_t keySize = keySizeSize + sizeof(uint8_t) + mapKeyLen;
			
			wexpr_uvlq64_write ((uint8_t*)(buf.data) + curPos, keySizeSize, mapKeyLen);
			*BUFCAST (buf.data, curPos + keySizeSize, uint8_t*) = 0x01;
			memcpy ((uint8_t*)buf.data + curPos + keySizeSize + 1, mapKey, mapKeyLen);
		
			curPos += keySize;
			
			// write the map value
			WexprExpression* mapValue = wexpr_Expression_mapValueAt(self, i);
			WexprMutableBuffer childBuffer = wexpr_Expression_createBinaryRepresentation(
				mapValue
			);
			
			memcpy ((uint8_t*)buf.data + curPos, childBuffer.data, childBuffer.byteSize);
			
			free (childBuffer.data);
			curPos += childBuffer.byteSize;
		}
		
		// done
	}
	
	else if (type == WexprExpressionTypeBinaryData)
	{
		const void* data = wexpr_Expression_binaryData_data(self);
		size_t dataSize = wexpr_Expression_binaryData_size(self);
		
		size_t sizeSize = wexpr_uvlq64_bytesize(dataSize+1); // 1 byte for compression method
		
		buf.byteSize = sizeSize + sizeof(uint8_t) + sizeof(uint8_t) + dataSize;
		buf.data = realloc(buf.data, buf.byteSize);
		
		wexpr_uvlq64_write ((uint8_t*)(buf.data) + 0, sizeSize, dataSize+1);
		
		*BUFCAST (buf.data, sizeSize, uint8_t*) = 0x04;
		*BUFCAST (buf.data, sizeSize+1, uint8_t*) = 0x00; // for now, only raw (no compression)
		memcpy ((uint8_t*)buf.data + sizeSize+1+1, data, dataSize);
	}
	
	#undef BUFCAST

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

// --- BinaryData

const void* wexpr_Expression_binaryData_data (WexprExpression* self)
{
	if (self->m_type != WexprExpressionTypeBinaryData)
		return NULL;
	
	return self->m_binaryData.data;
}

size_t wexpr_Expression_binaryData_size (WexprExpression* self)
{
	if (self->m_type != WexprExpressionTypeBinaryData)
		return 0;
	
	return self->m_binaryData.size;
}

void wexpr_Expression_binaryData_setValue (WexprExpression* self, const void* buffer, size_t byteSize)
{
	if (self->m_type != WexprExpressionTypeBinaryData)
		return;
	
	free(self->m_binaryData.data);
	self->m_binaryData.size = byteSize;
	self->m_binaryData.data = malloc(byteSize);
	if (!self->m_binaryData.data)
		return; // unable to allocate
	
	memcpy (self->m_binaryData.data, buffer, byteSize);
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
