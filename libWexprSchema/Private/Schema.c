//
/// \file libWexprSchema/Schema.c
/// \brief A wexpr schema
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#include <libWexprSchema/Schema.h>

#include <libWexprSchema/Twine.h>
#include <libWexprSchema/Type.h>
#include <libWexpr/libWexpr.h>


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../libWexpr/Private/ThirdParty/sglib/sglib.h"

static bool s_stringStartsWith (const char* haystack, size_t haystackLength, const char* needle, size_t needleLength)
{
	if (haystackLength < needleLength) // cant fit
		return false;

	for (size_t i=0; i < needleLength; ++i)
	{
		if (haystack[i] != needle[i])
			return false;
	}

	return true;
}

// --- structures

typedef struct WexprSchemaPrivateTypeArrayElement
{
	WexprSchemaType* type; // we own
	struct WexprSchemaPrivateTypeArrayElement* next;
} WexprSchemaPrivateTypeArrayElement;

#define WEXPRSCHEMAPRIVATETYPEARRAYELEMENT_COMPARATOR(e1, e2) ( (char*)(e1->type) - (char*)(e2->type))

SGLIB_DEFINE_LIST_PROTOTYPES (WexprSchemaPrivateTypeArrayElement, WEXPRSCHEMAPRIVATETYPEARRAYELEMENT_COMPARATOR, next)
SGLIB_DEFINE_LIST_FUNCTIONS (WexprSchemaPrivateTypeArrayElement, WEXPRSCHEMAPRIVATETYPEARRAYELEMENT_COMPARATOR, next)

typedef struct WexprSchemaPrivateTypeSchemaMapElement
{
	char* key; // strdup, we own
	WexprSchemaSchema* schema; // we own

	struct WexprSchemaPrivateTypeSchemaMapElement* next;
} WexprSchemaPrivateTypeSchemaMapElement;

#define WEXPRSCHEMAPRIVATETYPESCHEMAMAPELEMENT_COMPARATOR(e1, e2) ( (char*)(e1->key) - (char*)(e2->key))

SGLIB_DEFINE_LIST_PROTOTYPES (WexprSchemaPrivateTypeSchemaMapElement, WEXPRSCHEMAPRIVATETYPESCHEMAMAPELEMENT_COMPARATOR, next)
SGLIB_DEFINE_LIST_FUNCTIONS (WexprSchemaPrivateTypeSchemaMapElement, WEXPRSCHEMAPRIVATETYPESCHEMAMAPELEMENT_COMPARATOR, next)

struct WexprSchemaSchema
{
	//
	/// \brief Callbacks used for this schema
	//
	WexprSchemaSchema_Callbacks m_callbacks;
	
	//
	/// \brief The ID of the schema
	//
	char* m_id;
	
	//
	/// \brief The version of the schema we've loaded, or 0 if unknown.
	//
	int m_schemaVersion;

	//
	/// \brief The title of the schema
	//
	char* m_title;
	
	//
	/// \brief The description of the schema
	//
	char* m_description;
	
	// The schemas we're connected to
	WexprSchemaPrivateTypeSchemaMapElement* m_referenceSchemas;
	
	// types that are a part of this schema (linked list)
	WexprSchemaPrivateTypeArrayElement* m_types;
	
	// rootType - links to something inside m_types or m_referenceSchemas
	WexprSchemaType* m_rootType;
	
};

static void* s_defaultAlloc (void* allocatorUserData, size_t size)
{
	(void)allocatorUserData;
	
	return malloc(size);
}

static void s_defaultDealloc (void* allocatorUserData, void* ptr)
{
	(void)allocatorUserData;
	
	free(ptr);
}

static const char* s_defaultPathForSchemaID (void* userData, const char* schemaID)
{
	(void)userData;
	return LIBWEXPR_NULLPTR;
}

//
/// \brief Create a string containing the contents of the path. Will be null terminated, and must not contain nulls.
/// \note Allocated with callbacks
//
static char* s_createStringFromLocation (const char* path, WexprSchemaSchema_Callbacks* callbacks, WexprSchemaError** error)
{
	if (strstr(path, "http") == path)
	{
		// TODO: curl
		if (error) {
			*error = wexprSchema_Error_create(
				WexprSchemaErrorInternal,
				"/",
				"Unable to load schema from http/https",
				LIBWEXPR_NULLPTR,
				LIBWEXPR_NULLPTR
			);
		}

		return LIBWEXPR_NULLPTR;
	}
	else
	{
		FILE* f = fopen(path, "rb");
		if (f == LIBWEXPR_NULLPTR) {
			if (error) {
				char buffer[1024] = {0};
				sprintf(buffer, "Unable to open file %s: %s",
					path,
					strerror(errno)
				);

				*error = wexprSchema_Error_create(
					WexprSchemaErrorInternal,
					"/",
					buffer,
					LIBWEXPR_NULLPTR,
					LIBWEXPR_NULLPTR
				);
			}
			return LIBWEXPR_NULLPTR;
		}

		fseek(f, 0, SEEK_END);
		long int end = ftell(f);
		fseek(f, 0, SEEK_SET);
		
		char* buf = callbacks->alloc(callbacks->allocatorUserData, end+1);
		if (buf == LIBWEXPR_NULLPTR)
		{
			if (error) {
				*error = wexprSchema_Error_create(
					WexprSchemaErrorInternal,
					"/",
					"Unable to allocate memory",
					LIBWEXPR_NULLPTR,
					LIBWEXPR_NULLPTR
				);
			}
			return LIBWEXPR_NULLPTR;
		}
		fread(buf, end, 1, f);
		fclose(f);
		
		buf[end] = 0; // terminate
		return buf;
	}
}

static bool s_loadFromSchemaID (WexprSchemaSchema* self, const char* schemaID, WexprSchemaError** error)
{
	WexprSchemaSchema_Callbacks* callbacks = &(self->m_callbacks);
	const char* path = callbacks->pathForSchemaID(self->m_callbacks.pathForSchemaIDUserData, schemaID);
	if (path == LIBWEXPR_NULLPTR)
		path = schemaID;
	
	char* str = s_createStringFromLocation(path, callbacks, error);
	if (str == LIBWEXPR_NULLPTR)
		return false;
	
	WexprError err = WEXPR_ERROR_INIT();
	
	WexprExpression* wexpr = wexpr_Expression_createFromLengthString(
		str, strlen(str), WexprParseFlagNone, &err
	);
	
	if (!wexpr)
	{
		if (error)
		{
			*error = wexprSchema_Error_create(
				WexprSchemaErrorInternal,
				"/",
				"Error when loading schema wexpr",
				LIBWEXPR_NULLPTR,
				LIBWEXPR_NULLPTR
			); 
		}
		return false;
	}

	WEXPR_ERROR_FREE(err);
	callbacks->dealloc(callbacks->allocatorUserData, str);
	
	// check the schema version first
	WexprExpression* schema = wexpr_Expression_mapValueForKey(wexpr, "$schema");
	if (schema) {
		const char* str = wexpr_Expression_value(schema);
		if (strcmp(str, "https://wexpr.hackerguild.com/versions/1.schema.wexpr") == 0)
		{
			self->m_schemaVersion = 1;
		}
		else
		{
			if (error)
			{
				*error = wexprSchema_Error_create(
					WexprSchemaErrorInternal,
					"/",
					"Schema's schema was unknown",
					LIBWEXPR_NULLPTR,
					LIBWEXPR_NULLPTR
				); 
			}
			return false;
		}
	}

	// now load the rest
	WexprExpression* id = wexpr_Expression_mapValueForKey(wexpr, "$id");
	if (id) {
		self->m_id = LIBWEXPR_STRDUP(wexpr_Expression_value(id));
	}

	WexprExpression* title = wexpr_Expression_mapValueForKey(wexpr, "title");
	if (title) {
		self->m_title = LIBWEXPR_STRDUP(wexpr_Expression_value(title));
	}

	WexprExpression* description = wexpr_Expression_mapValueForKey(wexpr, "description");
	if (description) {
		self->m_description = LIBWEXPR_STRDUP(wexpr_Expression_value(description));
	}

	// types
	// first pass - loading the type definitions
	WexprExpression* types = wexpr_Expression_mapValueForKey(wexpr, "$types");
	if (types) {
		size_t numberOfTypes = wexpr_Expression_mapCount(types);
		for (size_t i=0; i < numberOfTypes; ++i)
		{
			const char* key = wexpr_Expression_mapKeyAt(types, i);
			WexprExpression* typeExpr = wexpr_Expression_mapValueAt(types, i);
			WexprSchemaType* type = wexprSchema_Type_createFromExpression(key, typeExpr);

			WexprSchemaPrivateTypeArrayElement* elem = malloc(sizeof(WexprSchemaPrivateTypeArrayElement));
			elem->type = type;
			elem->next = self->m_types;
			self->m_types = elem;
		}
	}

	// second pass - resolve the types as needed
	struct sglib_WexprSchemaPrivateTypeArrayElement_iterator it;
	for (WexprSchemaPrivateTypeArrayElement* list = sglib_WexprSchemaPrivateTypeArrayElement_it_init(&it, self->m_types);
		list != NULL; list = sglib_WexprSchemaPrivateTypeArrayElement_it_next(&it))
	{
		if (!wexprSchema_Type_resolveWithSchema(list->type, self, error))
		{
			return false;
		}
	}

	// then resolve the root type.
	WexprExpression* rootType = wexpr_Expression_mapValueForKey(wexpr, "rootType");
	if (rootType) {
		self->m_rootType = wexprSchema_Schema_typeWithName(self, wexpr_Expression_value(rootType));
	}

	// and we've pulled everything
	wexpr_Expression_destroy(wexpr);
	return true;
}

// --- public Construction/Destructino

WexprSchemaSchema* wexprSchema_Schema_createFromSchemaID(const char* schemaID, WexprSchemaSchema_Callbacks* callbacks, WexprSchemaError** error)
{
	// defaults
	WexprSchemaSchema_Callbacks c = {
		/* alloc = */             &s_defaultAlloc,
		/* dealloc = */           &s_defaultDealloc,
		/* pathForSchemaID = */   &s_defaultPathForSchemaID,
		/* allocatorUserData = */ LIBWEXPR_NULLPTR,
		/* pathForSchemaIDUserData = */ LIBWEXPR_NULLPTR
	};

	// move over the values that were set
	if (callbacks != LIBWEXPR_NULLPTR)
	{
		if (callbacks->alloc) { c.alloc = callbacks->alloc; }
		if (callbacks->dealloc) { c.dealloc = callbacks->dealloc; }
		if (callbacks->pathForSchemaID) { c.pathForSchemaID = callbacks->pathForSchemaID; }
		if (callbacks->allocatorUserData) { c.allocatorUserData = callbacks->allocatorUserData; }
		if (callbacks->pathForSchemaIDUserData) { c.pathForSchemaIDUserData = callbacks->pathForSchemaIDUserData; } 
	}

	WexprSchemaSchema* self = c.alloc(c.allocatorUserData, sizeof(WexprSchemaSchema));
	self->m_callbacks = c;
	self->m_title = NULL;
	self->m_description = NULL;
	self->m_id = NULL;
	self->m_schemaVersion = 0;
	self->m_types = NULL;
	self->m_rootType = NULL;
	self->m_referenceSchemas = NULL;

	// perform the load
	if (!s_loadFromSchemaID (self, schemaID, error))
	{
		wexprSchema_Schema_destroy(self);
		return LIBWEXPR_NULLPTR;
	}

	return self;
}

void wexprSchema_Schema_destroy(WexprSchemaSchema* self)
{
	WexprSchemaSchema_Callbacks* callbacks = &(self->m_callbacks);
	
	if (self->m_id)
		callbacks->dealloc(callbacks->allocatorUserData, self->m_id);
	
	if (self->m_title)
		callbacks->dealloc(callbacks->allocatorUserData, self->m_title);
	
	if (self->m_description)
		callbacks->dealloc(callbacks->allocatorUserData, self->m_description);
	
	// ignore rootType - it'll index into self->m_types

	if (self->m_referenceSchemas)
	{
		struct sglib_WexprSchemaPrivateTypeSchemaMapElement_iterator it;
		for (WexprSchemaPrivateTypeSchemaMapElement* list = sglib_WexprSchemaPrivateTypeSchemaMapElement_it_init(&it, self->m_referenceSchemas);
			list != NULL; list = sglib_WexprSchemaPrivateTypeSchemaMapElement_it_next(&it))
		{
			free(list->key);
			wexprSchema_Schema_destroy(list->schema);
			free(list);
		}
	}

	if (self->m_types)
	{
		struct sglib_WexprSchemaPrivateTypeArrayElement_iterator it;
		for (WexprSchemaPrivateTypeArrayElement* list = sglib_WexprSchemaPrivateTypeArrayElement_it_init(&it, self->m_types);
			list != NULL; list = sglib_WexprSchemaPrivateTypeArrayElement_it_next(&it))
		{
			wexprSchema_Type_destroy(list->type);
			free(list);
		}
	}

	self->m_callbacks.dealloc(self->m_callbacks.allocatorUserData, self);
}

// --- public 

WexprSchemaType* WexprSchema_Schema_rootType (WexprSchemaSchema* self)
{
	return self->m_rootType;
}

WexprSchemaType* wexprSchema_Schema_typeWithName(WexprSchemaSchema* self, const char* name)
{
	const char* moduleSplit = strstr(name, "::");
	if (moduleSplit == NULL)
	{
		// its a local type - look through our local names
		struct sglib_WexprSchemaPrivateTypeArrayElement_iterator it;
		for (WexprSchemaPrivateTypeArrayElement* list = sglib_WexprSchemaPrivateTypeArrayElement_it_init(&it, self->m_types);
			list != NULL; list = sglib_WexprSchemaPrivateTypeArrayElement_it_next(&it))
		{
			const char* typeName = wexprSchema_Type_name(list->type);
			if (strcmp(typeName, name) == 0)
				return list->type;
		}

		return NULL;
	}

	// has a module split - lookup the module first
	const char* typeName = moduleSplit+2;
	long moduleNameLength = (moduleSplit - name);

	struct sglib_WexprSchemaPrivateTypeSchemaMapElement_iterator it;
	for (WexprSchemaPrivateTypeSchemaMapElement* list = sglib_WexprSchemaPrivateTypeSchemaMapElement_it_init(&it, self->m_referenceSchemas);
		list != NULL; list = sglib_WexprSchemaPrivateTypeSchemaMapElement_it_next(&it))
	{
		if (s_stringStartsWith(
			list->key, strlen(list->key),
			name, moduleNameLength))
		{
			return wexprSchema_Schema_typeWithName(list->schema, typeName);
		}
	}

	return NULL; // not found
}

// --- public Validation

bool wexprSchema_Schema_validateExpression(WexprSchemaSchema* self, WexprExpression* expression, WexprSchemaError** error)
{
	WexprSchemaTwine objectPath;
	wexprSchema_Twine_init_CStr_Empty(&objectPath, "/");

	// get the root type
	WexprSchemaType* rootType = WexprSchema_Schema_rootType(self);
	if (!rootType)
	{
		char errBuffer[256] = {0};
		wexprSchema_Twine_resolveToCString(&objectPath, errBuffer, sizeof(errBuffer), LIBWEXPR_NULLPTR);

		*error = wexprSchema_Error_create(
			WexprSchemaErrorInternal,
			errBuffer,
			"No root type in schema to compare to",
			LIBWEXPR_NULLPTR,
			*error
		);
	
		return false;
	}

	bool success = wexprSchema_Type_validateObject(
		rootType,
		&objectPath,
		expression,
		error
	);

	if (!success)
	{
		if (error)
		{
			char errBuffer[256] = {0};
			wexprSchema_Twine_resolveToCString(&objectPath, errBuffer, sizeof(errBuffer), LIBWEXPR_NULLPTR);

			*error = wexprSchema_Error_create(
				WexprSchemaErrorInternal,
				errBuffer,
				"Unable to validate",
				*error,
				LIBWEXPR_NULLPTR
			);
		}
		return false;
	}
	
	return true;
}
