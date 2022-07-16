//
/// \file libWexprSchema/Schema.c
/// \brief A wexpr schema
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#include <libWexprSchema/Schema.h>

#include <libWexpr/libWexpr.h>

#include <stdio.h>
#include <stdlib.h>

#include "../../libWexpr/Private/ThirdParty/sglib/sglib.h"

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
	/// \brief The schema the schema is following
	/// Can be null if its the root schema.
	//
	WexprSchemaSchema* m_schema;
	
	//
	/// \brief The title of the schema
	//
	char* m_title;
	
	//
	/// \brief The description of the schema
	//
	char* m_description;
	
	// referenceSchemas
	
	// types
	
	// rootType
	
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
/// \brief Create a string containing the contents of the path
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
				"Unable to load schema from http/https"
			);
		}

		return LIBWEXPR_NULLPTR;
	}
	else
	{
		FILE* f = fopen(path, "rb");
		fseek(f, 0, SEEK_END);
		long int end = ftell(f);
		fseek(f, 0, SEEK_SET);
		
		char* buf = callbacks->alloc(callbacks->allocatorUserData, end);
		if (buf == LIBWEXPR_NULLPTR)
		{
			if (error) {
				*error = wexprSchema_Error_create(
					WexprSchemaErrorInternal,
					"/",
					"Unable to allocate memory"
				);
			}
			return LIBWEXPR_NULLPTR;
		}
		fread(buf, end, 1, f);
		fclose(f);
		
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
	
	
	
	WEXPR_ERROR_FREE(err);
	callbacks->dealloc(callbacks->allocatorUserData, str);
	
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
	
	if (self->m_schema)
	{
		wexprSchema_Schema_destroy(self->m_schema);
	}
	
	if (self->m_title)
		callbacks->dealloc(callbacks->allocatorUserData, self->m_title);
	
	if (self->m_description)
		callbacks->dealloc(callbacks->allocatorUserData, self->m_description);
	
	self->m_callbacks.dealloc(self->m_callbacks.allocatorUserData, self);
}

// --- public Validation

bool wexprSchema_Schema_validateExpression(WexprExpression* expression, WexprSchemaError** error)
{
	if (error) {
		*error = wexprSchema_Error_create(
			WexprSchemaErrorInternal,
			"/",
			"Unable to validate"
		);
	
		return false;
	}
	
	return false;
}
