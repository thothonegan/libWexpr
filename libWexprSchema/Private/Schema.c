//
/// \file libWexprSchema/Schema.c
/// \brief A wexpr schema
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#include <libWexprSchema/Schema.h>

#include <stdlib.h>

struct WexprSchemaSchema
{
	WexprSchemaSchema_Callbacks m_callbacks;
};

static void* s_defaultAlloc (void* allocatorUserData, size_t size)
{
	return malloc(size);
}

static void s_defaultDealloc (void* allocatorUserData, void* ptr)
{
	free(ptr);
}

static const char* s_defaultPathForSchemaID (void* userData, const char* schemaID)
{
	(void)userData;
	return LIBWEXPR_NULLPTR;
}

static bool s_loadFromSchemaID (WexprSchemaSchema* self, const char* schemaID, WexprSchemaError** error)
{
	if (error) {
		*error = wexprSchema_Error_create(
			WexprSchemaErrorInternal,
			"/",
			"Unable to load schema"
		);
	}

	return false;
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
	self->m_callbacks.dealloc(self->m_callbacks.allocatorUserData, self);
}

// --- public Validation

bool wexprSchema_Schema_validateExpression(WexprExpression* expression, WexprSchemaError** error)
{
	return false;
}
