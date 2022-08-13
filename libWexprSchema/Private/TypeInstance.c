//
/// \file libWexprSchema/TypeInstance.c
/// \brief An instance of a type (usually for a given property)
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#include <libWexprSchema/TypeInstance.h>

#include "TypeRef.h"

struct WexprSchemaTypeInstance
{
	char* m_description;
	WexprSchemaPrivateTypeRef* m_type;
};

WexprSchemaTypeInstance* wexprSchema_TypeInstance_createFromExpression (WexprExpression* expression)
{
	WexprSchemaTypeInstance* self = malloc(sizeof(WexprSchemaTypeInstance));
	self->m_description = LIBWEXPR_NULLPTR;
	self->m_type = LIBWEXPR_NULLPTR;
	
	WexprExpression* desc = wexpr_Expression_mapValueForKey(expression, "description");
	if (desc)
	{
		self->m_description = strdup(wexpr_Expression_value(desc));
	}
	
	WexprExpression* typeName = wexpr_Expression_mapValueForKey(expression, "type");
	if (typeName)
	{
		self->m_type = wexprSchema_PrivateTypeRef_createWithName(wexpr_Expression_value(typeName));
	}
	
	return self;
}

void wexprSchema_TypeInstance_destroy (WexprSchemaTypeInstance* self)
{
	if (self->m_type) { wexprSchema_PrivateTypeRef_destroy(self->m_type); }
	if (self->m_description) { free(self->m_description); }
	
	free(self); // done
}

// --- public Resolution

bool wexprSchema_TypeInstance_resolveWithSchema(WexprSchemaTypeInstance* self, struct WexprSchemaSchema* schema, WexprSchemaError** error)
{
	if (wexprSchema_PrivateTypeRef_isResolved(self->m_type))
	{
		return true;
	}
	
	const char* name = wexprSchema_PrivateTypeRef_name(self->m_type);
	WexprSchemaType* resolvedType = wexprSchema_Schema_typeWithName(schema, name);
	if (resolvedType)
		wexprSchema_PrivateTypeRef_resolveWith(self->m_type, resolvedType);
	else
	{
		// failed to resolve
		if (error)
		{
			char errBuffer[256] = {};
			WexprSchemaTwine message;
			wexprSchema_Twine_init_CStr_CStr(&message, "Failed to resolve type: ", self->m_type->typeName);
			wexprSchema_Twine_resolveToCString(&message, errBuffer, sizeof(errBuffer), LIBWEXPR_NULLPTR);

			*error = wexprSchema_Error_create(
				WexprSchemaErrorInternal,
				"[schema]",
				errBuffer,
				LIBWEXPR_NULLPTR,
				*error
			);
		}

		return false;
	}
	
	return true;
}
