//
/// \file libWexprSchema/TypeRef.c
/// \brief A reference to another type
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#include "TypeRef.h"

#include <stdlib.h>

SGLIB_DEFINE_LIST_FUNCTIONS (WexprSchemaPrivateTypeRef, WEXPRSCHEMAPRIVATETYPEREF_COMPARATOR, next)

// --- public Construction/Destruction

WexprSchemaPrivateTypeRef* wexprSchema_PrivateTypeRef_createWithName(const char* name)
{
	WexprSchemaPrivateTypeRef* self = malloc(sizeof(WexprSchemaPrivateTypeRef));
	self->typeName = strdup(name);
	self->type = LIBWEXPR_NULLPTR;
	self->next = LIBWEXPR_NULLPTR;
	
	return self;
}

void wexprSchema_PrivateTypeRef_destroy(WexprSchemaPrivateTypeRef* self)
{
	if (self->typeName) { free(self->typeName); }
	
	free(self);
}

// --- Resolution

void wexprSchema_PrivateTypeRef_resolveWith(WexprSchemaPrivateTypeRef* self, WexprSchemaType* type)
{
	if (!type) { return; }
	
	if (self->typeName) { free(self->typeName); self->typeName = LIBWEXPR_NULLPTR; }
	
	self->type = type;
}

const char* wexprSchema_PrivateTypeRef_name (WexprSchemaPrivateTypeRef* self)
{
	if (self->typeName)
		return self->typeName;
	
	return wexprSchema_Type_name(self->type);
}

bool wexprSchema_PrivateTypeRef_isResolved (WexprSchemaPrivateTypeRef* self)
{
	if (self->type)
		return true;
	
	return false;
}

WexprSchemaType* wexprSchema_PrivateTypeRef_type (WexprSchemaPrivateTypeRef* self)
{
	return self->type;
}
