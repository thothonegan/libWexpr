//
/// \file libWexprSchema/TypeInstance.c
/// \brief An instance of a type (usually for a given property)
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#include <libWexprSchema/TypeInstance.h>

#include "TypeRef.h"

#define DEBUG_LOGSTDERR 0

struct WexprSchemaTypeInstance
{
	char* m_description;
	WexprSchemaPrivateTypeRef* m_type;
	
	// If true, this specific property is optional and can be null/invalid.
	bool m_optional;
};

WexprSchemaTypeInstance* wexprSchema_TypeInstance_createFromExpression (WexprExpression* expression)
{
	WexprSchemaTypeInstance* self = malloc(sizeof(WexprSchemaTypeInstance));
	self->m_description = LIBWEXPR_NULLPTR;
	self->m_type = LIBWEXPR_NULLPTR;
	self->m_optional = false;
	
	WexprExpression* desc = wexpr_Expression_mapValueForKey(expression, "description");
	if (desc)
	{
		self->m_description = LIBWEXPR_STRDUP(wexpr_Expression_value(desc));
	}
	
	WexprExpression* typeName = wexpr_Expression_mapValueForKey(expression, "type");
	if (typeName)
	{
		self->m_type = wexprSchema_PrivateTypeRef_createWithName(wexpr_Expression_value(typeName));
	}
	
	WexprExpression* optionalExpr = wexpr_Expression_mapValueForKey(expression, "optional");
	if (optionalExpr)
	{
		const char* optionalStr = wexpr_Expression_value(optionalExpr);
		
		if (strcmp(optionalStr, "true") == 0)
		{
			self->m_optional = true;
		}
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
#if DEBUG_LOGSTDERR
	fprintf (stderr, "TypeInstance - Resolving with schema : name = %s\n", name);
#endif
	
	WexprSchemaType* resolvedType = wexprSchema_Schema_typeWithName(schema, name);
	if (resolvedType)
		wexprSchema_PrivateTypeRef_resolveWith(self->m_type, resolvedType);
	else
	{
		// failed to resolve
		if (error)
		{
			char errBuffer[256] = {0};
			WexprSchemaTwine message;
			wexprSchema_Twine_init_CStr_CStr(&message, "Failed to resolve type: ", self->m_type->typeName);
			wexprSchema_Twine_resolveToCString(&message, errBuffer, sizeof(errBuffer), LIBWEXPR_NULLPTR);

			*error = wexprSchema_Error_create(
				WexprSchemaErrorInternal,
				"[schema:typeinstance]",
				errBuffer,
				LIBWEXPR_NULLPTR,
				*error
			);
		}

#if DEBUG_LOGSTDERR
	fprintf (stderr, "TypeInstance - FAILED TO RESOLVE\n");
#endif
		return false;
	}
	
	if (!wexprSchema_PrivateTypeRef_isResolved(self->m_type))
	{
		fprintf(stderr, "INTERNAL ERROR: TypeInstance: Success on resolve, yet we're not resolved.\n");
		abort();
	}
	
#if DEBUG_LOGSTDERR
	fprintf (stderr, "TypeInstance - Succeeded resolution\n");
#endif
	return true;
}

// --- public Validation

bool wexprSchema_TypeInstance_validateObject (
	WexprSchemaTypeInstance* self,
	WexprSchemaTwine* objectPath,
	WexprExpression* expression,
	WexprSchemaError** error
)
{
	if (!wexprSchema_PrivateTypeRef_isResolved(self->m_type))
	{
		char errBuffer[256] = {0};
		wexprSchema_Twine_resolveToCString(objectPath, errBuffer, sizeof(errBuffer), LIBWEXPR_NULLPTR);
					
		char msgBuffer[512] = {0};
		sprintf(msgBuffer, "Type for type instance not resolved: %s", wexprSchema_PrivateTypeRef_name(self->m_type));
		
		*error = wexprSchema_Error_create(
			WexprSchemaErrorInternal,
			errBuffer,
			msgBuffer,
			LIBWEXPR_NULLPTR,
			*error
		);
		
		return false;
	}
	
	WexprExpressionType expressionType = (expression ? wexpr_Expression_type(expression) : WexprExpressionTypeInvalid);
	
	// special case, if expressionType is null or invalid and we're not required, we can short circuit
	if (self->m_optional && (expressionType == WexprExpressionTypeNull || expressionType == WexprExpressionTypeInvalid))
	{
		return true;
	}
	
	// resolve it against the type
	return wexprSchema_Type_validateObject(
		wexprSchema_PrivateTypeRef_type(self->m_type),
		objectPath,
		expression,
		error
	);
}
