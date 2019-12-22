//
/// \file libWexpr/ReferenceTable.c
/// \brief A table of expressions given names
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

#include <libWexpr/ReferenceTable.h>

#include <libWexpr/Expression.h>
#include "ThirdParty/c_hashmap/hashmap.h"

#include <stdlib.h>
#include <string.h>

// privates to WexprReferenceTable
struct WexprReferenceTable
{
	map_t m_hash;
};

typedef struct WexprReferenceTablePrivateMapElement
{
	char* key; // strdup, we own
	WexprExpression* value; // we own
} WexprReferenceTablePrivateMapElement;

static int s_refTable_freeHashData (any_t userData, any_t data)
{
	WexprReferenceTablePrivateMapElement* elem = data;
	free (elem->key);
	wexpr_Expression_destroy(elem->value);
	free (elem);
	
	return MAP_OK; // keep iterating
}

static int s_refTable_freeHashDataAtIndex (any_t userData, any_t data)
{
	// userData is pointer to a size_t representing the index
	size_t* index = (size_t*)userData;
	
	if (*index == 0)
	{
		// do the cleanup for this index - you must destroy after this
		s_refTable_freeHashData(NULL, data);
		return !MAP_OK; // exit
	}
	else
	{
		*index -= 1;
	}
	
	return MAP_OK;
}


typedef struct PrivateRefTableGetKeyValueAtIndex
{
	size_t index; // the index we're requesting
	void* result; // will be null if not found, or a value if found.
} PrivateRefTableGetKeyValueAtIndex;

static int s_refTable_getKeyAtIndex (any_t userData, any_t data)
{
	PrivateRefTableGetKeyValueAtIndex* ud = userData;
	WexprReferenceTablePrivateMapElement* elem = data;
	
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

static int s_refTable_getValueAtIndex (any_t userData, any_t data)
{
	PrivateRefTableGetKeyValueAtIndex* ud = userData;
	WexprReferenceTablePrivateMapElement* elem = data;
	
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

typedef struct PrivateGetIndexOfKey
{
	size_t result; // the result. Wont change if not found.
	size_t tempIndex; // the current index. Should be set to 0
	const char* key; // the key to lookup. cstring, but we dont own.
} PrivateGetIndexOfKey;

static int s_getIndexOfKey (any_t userData, any_t data)
{
	PrivateGetIndexOfKey* ud = (PrivateGetIndexOfKey*) userData;
	WexprReferenceTablePrivateMapElement* elem = data;
	
	if (strcmp (ud->key, elem->key) == 0)
	{
		ud->result = ud->tempIndex;
		
		return !MAP_OK; // exit
	}
	
	ud->tempIndex++; // next index
	
	return MAP_OK;
}

// --- public Construction/Destruction

WexprReferenceTable* wexpr_ReferenceTable_create ()
{
	WexprReferenceTable* ref = malloc (sizeof(WexprReferenceTable));
	ref->m_hash = hashmap_new();
	
	return ref;
}

void wexpr_ReferenceTable_destroy (WexprReferenceTable* self)
{
	// cleanup our hash
	hashmap_iterate (self->m_hash, &s_refTable_freeHashData, NULL);
	hashmap_free (self->m_hash);
	
	// cleanup our memory
	free (self);
}

// --- public Keys/Values

void wexpr_ReferenceTable_setExpressionForKey (
	WexprReferenceTable* self,
	const char* key,
	WexprExpression* expression
)
{
	WexprReferenceTablePrivateMapElement* elem = malloc (sizeof(WexprReferenceTablePrivateMapElement));
	elem->key = strdup (key);
	elem->value = expression;
	
	hashmap_put(self->m_hash, elem->key, elem);
}

void wexpr_ReferenceTable_setExpressionForLengthKey (
	WexprReferenceTable* self,
	const char* key, size_t keyLength,
	WexprExpression* expression
)
{
	WexprReferenceTablePrivateMapElement* elem = malloc (sizeof(WexprReferenceTablePrivateMapElement));
	elem->value = expression;
	elem->key = malloc(keyLength+1);
	memcpy (elem->key, key, keyLength);
	elem->key[keyLength] = 0;
	
	hashmap_put(self->m_hash, elem->key, elem);
}

WexprExpression* wexpr_ReferenceTable_expressionForKey (
	WexprReferenceTable* self,
	const char* key
)
{
	WexprReferenceTablePrivateMapElement* elem = NULL;
	int res = hashmap_get (self->m_hash, (char*)key, (void**) &elem);
	
	if (res == MAP_OK && elem)
	{
		return elem->value;
	}
	
	return NULL;
}

WexprExpression* wexpr_ReferenceTable_expressionForLengthKey (
	WexprReferenceTable* self,
	const char* key, size_t keyLength
)
{
	// key has to be 0 terminated for our hash
	// TODO: use stack functions for this if its short? then we dont have to hit memory
	char* newKey = malloc(keyLength+1);
	memcpy (newKey, key, keyLength);
	newKey[keyLength] = 0; // end terminator
	
	WexprExpression* res = wexpr_ReferenceTable_expressionForKey (self, newKey);
	free (newKey);
	
	return res;
}

void wexpr_ReferenceTable_removeKey (
	WexprReferenceTable* self,
	const char* key
)
{
	size_t index = wexpr_ReferenceTable_indexOfKey(self, key);
	
	// we test against not because we used that to early return 
	if (hashmap_iterate(self->m_hash, &s_refTable_freeHashDataAtIndex, &index) == !MAP_OK)
	{
		hashmap_remove(self->m_hash, (char*) key);
	}
}

void wexpr_ReferenceTable_removeLengthKey (
	WexprReferenceTable* self,
	const char* key, size_t keyLength
)
{
	// key has to be 0 terminated for our hash
	// TODO: use stack functions for this if its short? then we dont have to hit memory
	char* newKey = malloc(keyLength+1);
	memcpy (newKey, key, keyLength);
	newKey[keyLength] = 0; // end terminator
	
	wexpr_ReferenceTable_removeKey(self, newKey);
}

size_t wexpr_ReferenceTable_count (
	WexprReferenceTable* self
)
{
	return hashmap_length (self->m_hash);
}

size_t wexpr_ReferenceTable_indexOfKey (
	WexprReferenceTable* self,
	const char* key
)
{
	PrivateGetIndexOfKey priv;
	priv.result = wexpr_ReferenceTable_count(self);
	priv.key = key;
	priv.tempIndex = 0;
	
	hashmap_iterate(self->m_hash, &s_getIndexOfKey, &priv);
	return priv.result;
}

const char* wexpr_ReferenceTable_keyAtIndex (
	WexprReferenceTable* self,
	size_t index
)
{
	PrivateRefTableGetKeyValueAtIndex val;
	val.index = index;
	val.result = NULL;
	
	hashmap_iterate (self->m_hash, &s_refTable_getKeyAtIndex, &val);
	
	return val.result;
}

WexprExpression* wexpr_ReferenceTable_expressionAtIndex (
	WexprReferenceTable* self,
	size_t index
)
{
	PrivateRefTableGetKeyValueAtIndex val;
	val.index = index;
	val.result = NULL;
	
	hashmap_iterate (self->m_hash, &s_refTable_getValueAtIndex, &val);
	
	return val.result;
}
