//
/// \file libWexprSchema/Type.c
/// \brief A single type we validate with the schema
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#include <libWexprSchema/Type.h>

#include <libWexprSchema/PrimitiveType.h>
#include <libWexprSchema/Schema.h>

#include <stdio.h>

#include "../../libWexpr/Private/ThirdParty/sglib/sglib.h"

// --- structures

typedef struct WexprSchemaTypePrivateTypeArrayElement
{
	char* typeName; // we own if exists, will only exist temporarily till we fill out type.
	WexprSchemaType* type;  // we dont own - will be part of the schema that set us up.
	struct WexprSchemaTypePrivateTypeArrayElement* next;
} WexprSchemaTypePrivateTypeArrayElement;

#define WEXPRSCHEMATYPEPRIVATETYPEARRAYELEMENT_COMPARATOR(e1, e2) ( (char*)(e1->type) - (char*)(e2->type))

SGLIB_DEFINE_LIST_PROTOTYPES (WexprSchemaTypePrivateTypeArrayElement, WEXPRSCHEMATYPEPRIVATETYPEARRAYELEMENT_COMPARATOR, next)
SGLIB_DEFINE_LIST_FUNCTIONS (WexprSchemaTypePrivateTypeArrayElement, WEXPRSCHEMATYPEPRIVATETYPEARRAYELEMENT_COMPARATOR, next)

struct WexprSchemaType
{
	//
	/// \brief The name of the type
	//
	char* m_name;

	//
	/// \brief The description of the type
	//
	char* m_description;

	//
	/// \brief The primitive type we're mapped to, if any.
	/// If unknown, we must have a Type that would resolve to the primitive type.
	//
	WexprSchemaPrimitiveType m_primitiveType;

	//
	/// \brief The parent types, of which we need to meet one of them (if any exist).
	//
	WexprSchemaTypePrivateTypeArrayElement* m_types;
};

// --- public Construction/Destruction

WexprSchemaType* wexprSchema_Type_createFromExpression (const char* name, WexprExpression* expression)
{
	WexprSchemaType* self = malloc(sizeof(WexprSchemaType));
	self->m_name = NULL;
	self->m_description = NULL;
	self->m_primitiveType = WexprSchemaPrimitiveTypeUnknown;
	self->m_types = NULL;

	self->m_name = strdup(name);

	WexprExpression* desc = wexpr_Expression_mapValueForKey(expression, "description");
	if (desc)
	{
		self->m_description = strdup(wexpr_Expression_value(desc));
	}

	WexprExpression* primitiveType = wexpr_Expression_mapValueForKey(expression, "primitiveType");
	if (primitiveType) {
		self->m_primitiveType = wexprSchema_PrimitiveType_fromString(wexpr_Expression_value(primitiveType));
	}

	// types
	WexprExpression* types = wexpr_Expression_mapValueForKey(expression, "type");
	if (types) {
		WexprExpressionType typesType = wexpr_Expression_type(types);
		if (typesType == WexprExpressionTypeValue)
		{
			// easy, just add one
			WexprSchemaTypePrivateTypeArrayElement* elem = malloc(sizeof(WexprSchemaTypePrivateTypeArrayElement));
			elem->typeName = strdup(wexpr_Expression_value(types));
			elem->type = NULL;
			elem->next = self->m_types;
			self->m_types = elem;

		}
		else if (typesType == WexprExpressionTypeArray)
		{
			size_t count = wexpr_Expression_arrayCount(types);
			for (size_t i=0; i < count; ++i)
			{
				WexprExpression* expr = wexpr_Expression_arrayAt(types, i);

				WexprSchemaTypePrivateTypeArrayElement* elem = malloc(sizeof(WexprSchemaTypePrivateTypeArrayElement));
				elem->typeName = strdup(wexpr_Expression_value(expr));
				elem->type = NULL;
				elem->next = self->m_types;
				self->m_types = elem;
			}
		}
	}

	// TODO: valueRegex
	// TODO: arrayAllElements
	// TODO: mapProperties
	// TODO: etc

	return self;
}

void wexprSchema_Type_destroy(WexprSchemaType* self)
{
	if (self->m_name)
		free(self->m_name);

	if (self->m_description)
		free(self->m_description);

	if (self->m_types)
	{
		struct sglib_WexprSchemaTypePrivateTypeArrayElement_iterator it;
		for (WexprSchemaTypePrivateTypeArrayElement* list = sglib_WexprSchemaTypePrivateTypeArrayElement_it_init(&it, self->m_types);
			list != NULL; list = sglib_WexprSchemaTypePrivateTypeArrayElement_it_next(&it))
		{
			if (list->typeName)
				free(list->typeName);
			
			free(list);
		}
	}

	free(self);
}

// --- public Loading

bool wexprSchema_Type_resolveWithSchema(WexprSchemaType* self, WexprSchemaSchema* schema, WexprSchemaError** error)
{
	// resolve every type we can
	struct sglib_WexprSchemaTypePrivateTypeArrayElement_iterator it;
	for (WexprSchemaTypePrivateTypeArrayElement* list = sglib_WexprSchemaTypePrivateTypeArrayElement_it_init(&it, self->m_types);
		list != NULL; list = sglib_WexprSchemaTypePrivateTypeArrayElement_it_next(&it))
	{
		if (list->type == NULL && list->typeName != NULL)
		{
			list->type = wexprSchema_Schema_typeWithName(schema, list->typeName);
			if (list->type != NULL)
			{
				free(list->typeName); // not needed anymore
				list->typeName = NULL;
			}
			else
			{
				// failed to resolve
				if (error)
				{
					char errBuffer[256] = {};
					WexprSchemaTwine message;
					wexprSchema_Twine_init_CStr_CStr(&message, "Failed to resolve type: ", list->typeName);
					wexprSchema_Twine_resolveToCString(&message, errBuffer, sizeof(errBuffer), LIBWEXPR_NULLPTR);

					*error = wexprSchema_Error_create(
						WexprSchemaErrorInternal,
						"[schema]",
						errBuffer,
						*error
					);
				}

				return false;
			}
		}
	}
	// success
	return true;
}

// --- public Properties

const char* wexprSchema_Type_name (WexprSchemaType* self)
{
	return self->m_name;
}

const char* wexprSchema_Type_description (WexprSchemaType* self)
{
	return self->m_description;
}

unsigned int wexprSchema_Type_possibleTypesCount (WexprSchemaType* self)
{
	if (self->m_types == 0)
		return 0;

	return sglib_WexprSchemaTypePrivateTypeArrayElement_len(self->m_types);
}

WexprSchemaType* wexprSchema_Type_typeAt (WexprSchemaType* self, unsigned int index)
{
	struct sglib_WexprSchemaTypePrivateTypeArrayElement_iterator it;
	for (WexprSchemaTypePrivateTypeArrayElement* list = sglib_WexprSchemaTypePrivateTypeArrayElement_it_init(&it, self->m_types);
		list != NULL; list = sglib_WexprSchemaTypePrivateTypeArrayElement_it_next(&it))
	{
		if (index == 0)
			return list->type;
	}

	return NULL;
}

WexprSchemaPrimitiveType wexprSchema_Type_primitiveTypes (WexprSchemaType* self)
{
	if (self->m_primitiveType != WexprSchemaPrimitiveTypeUnknown)
		return self->m_primitiveType;

	WexprSchemaPrimitiveType pt = WexprSchemaPrimitiveTypeUnknown;

	unsigned int count = wexprSchema_Type_possibleTypesCount(self);
	for (unsigned int i=0; i < count; ++i)
	{
		WexprSchemaType* t = wexprSchema_Type_typeAt(self, i);
		pt |= wexprSchema_Type_primitiveTypes(t);
	}

	return pt;
}

// --- public Validation

bool wexprSchema_Type_validateObject (
	WexprSchemaType* self,
	WexprSchemaTwine* objectPath,
	WexprExpression* expression,
	WexprSchemaError** error
)
{
	// check that the primitive types are set.
	WexprSchemaPrimitiveType primitive = wexprSchema_Type_primitiveTypes(self);
	WexprExpressionType expressionType = wexpr_Expression_type(expression);

	if (!wexprSchema_PrimitiveType_matchesExpressionType(primitive, expressionType))
	{
		if (error)
		{
			char errBuffer[256] = {};
			wexprSchema_Twine_resolveToCString(objectPath, errBuffer, sizeof(errBuffer), LIBWEXPR_NULLPTR);

			WexprSchemaTwine errorMessage1, errorMessage2, errorMessage3;
			wexprSchema_Twine_init_CStr_CStr(&errorMessage1,
				"Expression didnt match primitive type: was ", wexpr_ExpressionType_toString(expressionType)
			);

			wexprSchema_Twine_init_Twine_CStr(&errorMessage2,
				&errorMessage1, " but expected "
			);

			WexprSchemaTwine primTwine = wexprSchema_PrimitiveType_toTwine(primitive);
			wexprSchema_Twine_init_Twine_Twine(&errorMessage3,
				&errorMessage2, &primTwine
			);

			char messageBuffer[256]= {};
			wexprSchema_Twine_resolveToCString(&errorMessage3, messageBuffer, sizeof(messageBuffer), LIBWEXPR_NULLPTR);

			*error = wexprSchema_Error_create(
				WexprSchemaErrorInternal,
				errBuffer, messageBuffer,
				*error
			);
		}

		return false;
	}

	// check its types if it has any.
	// At least one needs to be successful.
	unsigned int typeCount= wexprSchema_Type_possibleTypesCount(self);
	if (typeCount > 0)
	{
		WexprSchemaError* typeErrors = NULL;
		WexprSchemaType* parentTypeSelected = NULL;

		for (unsigned int i=0; i < typeCount; ++i)
		{
			WexprSchemaType* t = wexprSchema_Type_typeAt(self, i);
			bool success =  wexprSchema_Type_validateObject(t, objectPath, expression, &typeErrors);
			if (success)
			{
				parentTypeSelected = t;
				break;
			}
		}

		if (parentTypeSelected == NULL)
		{
			if (error)
			{
				char errBuffer[256] = {};
				wexprSchema_Twine_resolveToCString(objectPath, errBuffer, sizeof(errBuffer), LIBWEXPR_NULLPTR);

				*error = wexprSchema_Error_create(
					WexprSchemaErrorInternal,
					errBuffer,
					"Does not match possible types, possibilities were:",
					typeErrors
				);
			}
			else
			{
				wexprSchema_Error_destroy(typeErrors); // nowhere to go
			}

			return false;
		}

		// success - cleanup errors
		if (typeErrors)
		{
			wexprSchema_Error_destroy(typeErrors);
		}
	}

	// Then resolve our type specific rules if applicable
	// (based on the wexpr primitive)

	// TODO - we assume correct atm
	return true;
}
