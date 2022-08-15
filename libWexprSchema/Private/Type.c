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
#include <libWexprSchema/TypeInstance.h>

#include "ExternalOnigmo.h"

#include <stdio.h>

#include "../../libWexpr/Private/ThirdParty/c_hashmap/hashmap.h"
#include "../../libWexpr/Private/ThirdParty/sglib/sglib.h"

#include "TypeRef.h"

// Turns on debug output to 
// Format:
// +++ Started a function call
// --- Leaving a function call
// >>> Performing a call into something
// <<< Returned from the call
// ... Other message
//
#define DEBUG_LOGSTDERR 0

// --- structures


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
	WexprSchemaPrivateTypeRef* m_types;
	
	// -------------- Common
	
	// ------------- Value
	
	//
	/// \brief If set, the regular expression the value must match to be valid
	//
	OnigRegexType* m_valueRegex;
	
	//
	/// \brief If set, the string for the regex. strdupd.
	//
	char* m_valueRegexString;
	
	// ------------- Array
	
	//
	/// \brief If set, type definition that all array elements must meet
	//
	WexprSchemaTypeInstance* m_arrayAllElements;
	
	// ------------- Map
	
	//
	/// \brief List of map properties that we have defined
	/// Type is WexprSchemaType_MapProperty
	//
	map_t m_mapProperties;
	
	//
	/// \brief If set, all map values must meet this instance.
	//
	WexprSchemaTypeInstance* m_mapAllProperties;
	
	//
	/// \brief If set, all map keys must meet this instance
	//
	WexprSchemaTypeInstance* m_mapKeyType;
	
	//
	/// \brief Are additional properties outside the 'mapProperties' allowed? Default is false
	//
	bool m_mapAllowAdditionalProperties;
};

typedef struct WexprSchemaType_MapProperty
{
	char* key; // strdup, we own 
	WexprSchemaTypeInstance* value;
} WexprSchemaType_MapProperty;

// --- private internals

static int s_freeMapPropertiesHashData (any_t userData, any_t data)
{
	WexprSchemaType_MapProperty* elem = data;
	free(elem->key);
	return MAP_OK;
}

typedef struct ResolveMapPropertiesParams
{
	WexprSchemaSchema* schema;
	WexprSchemaError** error;
	bool result; // must start true
} ResolveMapPropertiesParams;

static int s_resolveMapProperties (any_t userData, any_t data)
{
	ResolveMapPropertiesParams* params = userData;
	WexprSchemaType_MapProperty* elem = data;
#if DEBUG_LOGSTDERR
	fprintf(stderr, "Resolving map property '%s'\n", elem->key);
#endif

	params->result &= wexprSchema_TypeInstance_resolveWithSchema(elem->value, params->schema, params->error);
	
	return MAP_OK;
}

// Validate a value object whos rules should match the current SchemaType
static bool s_wexprSchema_Type_validateValue(
	WexprSchemaType* self,
	WexprSchemaTwine* objectPath,
	WexprExpression* expression,
	WexprSchemaError** error
)
{
#if DEBUG_LOGSTDERR
	char debugLogBuffer[1024] = {};
	int outDebugBufferLength = 0;
	
	wexprSchema_Twine_resolveToCString(objectPath, debugLogBuffer, sizeof(debugLogBuffer), &outDebugBufferLength);
	fprintf(stderr, "+++ %s: Validate value\n", debugLogBuffer);
#endif
	
	const char* exprValue = wexpr_Expression_value(expression);
	int exprValueLen = ((exprValue == NULL) ? 0 : strlen(exprValue));
	bool success = true;
	
	if (self->m_valueRegex)
	{
		#if DEBUG_LOGSTDERR
			fprintf(stderr, "    Validate '%s' against regex: %s\n", exprValue, self->m_valueRegexString);
		#endif
		bool regexSuccess = true;
		
		OnigRegion* region = onig_region_new();
		
		OnigPosition r = onig_search(self->m_valueRegex,
			(const unsigned char*) exprValue, (const unsigned char*) (exprValue + exprValueLen),
			(const unsigned char*) exprValue, (const unsigned char*) (exprValue + exprValueLen),
			region,
			ONIG_OPTION_NONE
		);
		
		if (r != 0) // < 0 is error, > 0 means not matched at start of string
			regexSuccess = false;
		else if (region->num_regs == 0)
			regexSuccess = false;
		else if (region->end[0] != exprValueLen)
			regexSuccess = false; // didnt cover the whole string
		
		if (regexSuccess)
		{
		#if DEBUG_LOGSTDERR
			fprintf(stderr, "    Regex success\n");
		#endif
		}
		
		if (!regexSuccess)
		{
		#if DEBUG_LOGSTDERR
			fprintf(stderr, "    Regex failure \n");
		#endif
			if (error)
			{
				char errBuffer[256] = {0};
				wexprSchema_Twine_resolveToCString(objectPath, errBuffer, sizeof(errBuffer), LIBWEXPR_NULLPTR);
				
				char msgBuffer[1024] = {0};
				sprintf(msgBuffer, "Value '%s' does not meet required regex '%s'", exprValue, self->m_valueRegexString
				);

				*error = wexprSchema_Error_create(
					WexprSchemaErrorInternal,
					errBuffer,
					msgBuffer,
					LIBWEXPR_NULLPTR,
					*error
				);
			}
		}
		
		onig_region_free(region, 1);
		success &= regexSuccess;
	}
	
	#if DEBUG_LOGSTDERR
		fprintf(stderr, "--- Overall done: %d\n", success);
	#endif
	return success;
}

// Validate an array object whos rules should match the current SchemaType
static bool s_wexprSchema_Type_validateArray (
	WexprSchemaType* self,
	WexprSchemaTwine* objectPath,
	WexprExpression* expression,
	WexprSchemaError** error
)
{
#if DEBUG_LOGSTDERR
	char debugLogBuffer[1024] = {};
	int outDebugBufferLength = 0;
	
	//wexprSchema_Twine_resolveToCString(objectPath, debugLogBuffer, sizeof(debugLogBuffer), &outDebugBufferLength);
	fprintf(stderr, "+++ Validate array\n");
#endif
	
	// check the array types
	bool success = true;
	if (self->m_arrayAllElements)
	{
		size_t count = wexpr_Expression_arrayCount(expression);
		
#if DEBUG_LOGSTDERR
	fprintf(stderr, "    Validating array all elements\n");
#endif
		for (size_t i=0; i < count; ++i)
		{
			char buf[32] = {0};
			sprintf(buf, "[%zu]", i);
			
			WexprSchemaTwine indexObjectPath;
			wexprSchema_Twine_init_Twine_CStr(&indexObjectPath, objectPath, buf);
			
			WexprExpression* childExpr = wexpr_Expression_arrayAt(expression, i);
			bool res = wexprSchema_TypeInstance_validateObject(
				self->m_arrayAllElements,
				&indexObjectPath,
				childExpr,
				error
			);
			success &= res;
		}
	}
	
	return success;
}

typedef struct PrivateValidateMapPropertyParams
{
	WexprSchemaTwine* parentObjectPath;
	WexprExpression* parentExpression;
	WexprSchemaError** error;
	bool validateAgainstAll;
	bool result;
} PrivateValidateMapPropertyParams;

#define OBJECTPATH_APPEND(varName, prevObjectPath, name) \
WexprSchemaTwine varName##_withSlash; \
WexprSchemaTwine varName; \
if (wexprSchema_Twine_endsWith(prevObjectPath, "/")) \
{ \
	wexprSchema_Twine_init_Twine_empty(&varName##_withSlash, prevObjectPath); \
} \
else \
{ \
	wexprSchema_Twine_init_Twine_CStr(&varName##_withSlash, prevObjectPath, "/"); \
} \
wexprSchema_Twine_init_Twine_CStr(&varName, &varName##_withSlash, name)


static int s_validateMapProperty (any_t userData, any_t data)
{
	PrivateValidateMapPropertyParams* params = userData;
	WexprSchemaType_MapProperty* elem = data;
	
	OBJECTPATH_APPEND(objectPath, params->parentObjectPath, elem->key);
	WexprSchemaError* e = LIBWEXPR_NULLPTR;
	
	bool res = wexprSchema_TypeInstance_validateObject(
		elem->value,
		&objectPath,
		wexpr_Expression_mapValueForKey(params->parentExpression, elem->key),
		&e
	);
	
	if (!res) {
		if (params->error)
		{
			char objectPathStr[256] = {0};
			wexprSchema_Twine_resolveToCString(&objectPath, objectPathStr, sizeof(objectPathStr), LIBWEXPR_NULLPTR);
			
			char buffer[512] = {0};
			sprintf(buffer, "Error when validating map property: %s", elem->key);
			
			*(params->error) = wexprSchema_Error_create(
				WexprSchemaErrorInternal,
				objectPathStr,
				buffer,
				e,
				*(params->error)
			);
		}
	}
	
	params->result &= res;
	
	return MAP_OK;
}

// Validate a map object whos rules should match the current SchemaType
static bool s_wexprSchema_Type_validateMap (
	WexprSchemaType* self,
	WexprSchemaTwine* objectPath,
	WexprExpression* expression,
	WexprSchemaError** error
)
{
#if DEBUG_LOGSTDERR
	char debugLogBuffer[1024] = {};
	int outDebugBufferLength = 0;
	
	//wexprSchema_Twine_resolveToCString(objectPath, debugLogBuffer, sizeof(debugLogBuffer), &outDebugBufferLength);
	fprintf(stderr, "+++ Validate map\n");
#endif
	
	bool success = true;
	
	// we need to check it multiple ways
	// - we need to check via the properties we have . This will make sure all of our rules apply correct.
	// TODO
	PrivateValidateMapPropertyParams p;
	p.error = error;
	p.parentExpression = expression;
	p.parentObjectPath = objectPath;
	p.result = true;
	hashmap_iterate(self->m_mapProperties, &s_validateMapProperty, &p);
	success &= p.result;
	
	// - we need to check all properties against the all if we have it
	if (self->m_mapAllProperties || self->m_mapKeyType)
	{
		size_t count = wexpr_Expression_mapCount(expression);
		for (size_t i=0; i < count; ++i)
		{
			const char* key = wexpr_Expression_mapKeyAt(expression, i);
			
			OBJECTPATH_APPEND(keyObjectPath, objectPath, key);
				
			if (self->m_mapKeyType)
			{
				// create a fake expression we can test against
				WexprExpression* keyValue = wexpr_Expression_createValue(key);
				
				bool res = wexprSchema_TypeInstance_validateObject(
					self->m_mapKeyType,
					&keyObjectPath,
					keyValue,
					error
				);
				success &= res;
				
				wexpr_Expression_destroy(keyValue);
			}
			
			if (self->m_mapAllProperties)
			{
#if DEBUG_LOGSTDERR
				fprintf(stderr, "--- Testing against mapAllProperties: %s\n", key);
#endif
			
				WexprExpression* value = wexpr_Expression_mapValueAt(expression, i);
				
				bool res = wexprSchema_TypeInstance_validateObject(
					self->m_mapAllProperties,
					&keyObjectPath,
					value,
					error
				);
				success &= res;
			}
		}
	}
	
	// - we  need to check via expression, if we're not allowed to have unknown properties
	// since we've checked all our rules, we need to just make sure we have rules for each one
	if (!self->m_mapAllProperties && !self->m_mapAllowAdditionalProperties)
	{
		// make sure we have the equivilants
		size_t count = wexpr_Expression_mapCount(expression);
		for (size_t i=0; i < count; ++i)
		{
			const char* key = wexpr_Expression_mapKeyAt(expression, i);
			
			// can we identify the key in our rules?
			bool foundKey = false;
			any_t unusedRes;
			if (MAP_OK == hashmap_get(self->m_mapProperties, (char*) key, &unusedRes))
			{
				foundKey = true;
			}
			
			// if not, error it out
			if (!foundKey)
			{
				if (error)
				{
					char errBuffer[256] = {0};
					wexprSchema_Twine_resolveToCString(objectPath, errBuffer, sizeof(errBuffer), LIBWEXPR_NULLPTR);
					
					char msgBuffer[1024] = {0};
					sprintf(msgBuffer, "Map has additional property which wasnt allowed: %s", key);

					*error = wexprSchema_Error_create(
						WexprSchemaErrorInternal,
						errBuffer,
						msgBuffer,
						LIBWEXPR_NULLPTR,
						*error
					);
				}
				
				success = false;
			}
		}
	}
	
	return success;
}

// --- public Construction/Destruction

WexprSchemaType* wexprSchema_Type_createFromExpression (const char* name, WexprExpression* expression)
{
#if DEBUG_LOGSTDERR
			fprintf(stderr, "Type_createFromExpression('%s') ---- Creating \n", name);
#endif
			
	WexprSchemaType* self = malloc(sizeof(WexprSchemaType));
	self->m_name = NULL;
	self->m_description = NULL;
	self->m_primitiveType = WexprSchemaPrimitiveTypeUnknown;
	self->m_types = NULL;
	self->m_valueRegex = NULL;
	self->m_valueRegexString = NULL;

	self->m_name = LIBWEXPR_STRDUP(name);
	self->m_mapProperties = hashmap_new();
	self->m_arrayAllElements = NULL;
	self->m_mapAllowAdditionalProperties = false;
	self->m_mapAllProperties = NULL;
	self->m_mapKeyType = NULL;

	WexprExpression* desc = wexpr_Expression_mapValueForKey(expression, "description");
	if (desc)
	{
		self->m_description = LIBWEXPR_STRDUP(wexpr_Expression_value(desc));
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
#if DEBUG_LOGSTDERR
			fprintf(stderr, "Type_createFromExpression('%s') Adding single possible type: %s\n", name, wexpr_Expression_value(types));
#endif
			// easy, just add one
			WexprSchemaPrivateTypeRef* elem = wexprSchema_PrivateTypeRef_createWithName(
				wexpr_Expression_value(types)
			);
			elem->next = self->m_types;
			self->m_types = elem;

		}
		else if (typesType == WexprExpressionTypeArray)
		{
			size_t count = wexpr_Expression_arrayCount(types);
			for (size_t i=0; i < count; ++i)
			{
				WexprExpression* expr = wexpr_Expression_arrayAt(types, i);

#if DEBUG_LOGSTDERR
			fprintf(stderr, "Type_createFromExpression('%s') Adding possible type: %s\n", name, wexpr_Expression_value(expr));
#endif
				WexprSchemaPrivateTypeRef* elem = wexprSchema_PrivateTypeRef_createWithName(
					wexpr_Expression_value(expr)
				);
				elem->next = self->m_types;
				self->m_types = elem;
			}
		}
	}

	// --- common
	
	// --- value
	
	WexprExpression* valueRegexExpr = wexpr_Expression_mapValueForKey(expression, "valueRegex");
	if (valueRegexExpr)
	{
		const char* str = wexpr_Expression_value(valueRegexExpr);
		self->m_valueRegexString = LIBWEXPR_STRDUP(str);
		
#if DEBUG_LOGSTDERR
			fprintf(stderr, "Type_createFromExpression('%s') Found regex %s\n", name, str);
#endif
		OnigErrorInfo einfo;
		memset (&einfo, 0, sizeof(einfo));
		int r = onig_new(&(self->m_valueRegex),
			(const unsigned char*) str,
			(const unsigned char*) (str + strlen(str)),
			ONIG_OPTION_DEFAULT, ONIG_ENCODING_UTF8, ONIG_SYNTAX_DEFAULT,
			&einfo
		);
		
		if (r != ONIG_NORMAL)
		{
			fprintf(stderr, "Schema regex failed to compile");
			abort();
		}
	}
	
	// --- array 
	WexprExpression* arrayAllElementsExpr = wexpr_Expression_mapValueForKey(expression, "arrayAllElements");
	if (arrayAllElementsExpr)
	{
#if DEBUG_LOGSTDERR
		fprintf(stderr, "Type_createFromExpression('%s') Found arrayAllElements\n", name);
#endif
		self->m_arrayAllElements = wexprSchema_TypeInstance_createFromExpression(
			arrayAllElementsExpr
		);
	}
	
	// --- map
	WexprExpression* mapPropertiesExpr = wexpr_Expression_mapValueForKey(expression, "mapProperties");
	if (mapPropertiesExpr)
	{
		size_t numberOfElements = wexpr_Expression_mapCount(mapPropertiesExpr);
		for (size_t i=0; i < numberOfElements; ++i)
		{
			const char* keyName = wexpr_Expression_mapKeyAt(mapPropertiesExpr, i);
			WexprExpression* value = wexpr_Expression_mapValueAt(mapPropertiesExpr, i);
			
			// SchemaTypeDefinition
			WexprSchemaType_MapProperty* prop = malloc(sizeof(WexprSchemaType_MapProperty));
			prop->key = LIBWEXPR_STRDUP(keyName);
			prop->value = wexprSchema_TypeInstance_createFromExpression(value);
			
#if DEBUG_LOGSTDERR
			fprintf(stderr, "Type_createFromExpression('%s') Found mapProperties key %s\n", name, keyName);
#endif
			hashmap_put(self->m_mapProperties, prop->key, prop);
		}
	}
	
	WexprExpression* mapKeyTypeExpr = wexpr_Expression_mapValueForKey(expression, "mapValueForKey");
	if (mapKeyTypeExpr)
	{
#if DEBUG_LOGSTDERR
		fprintf(stderr, "Type_createFromExpression('%s') Found mapKeyExpression\n", name);
#endif
		self->m_mapKeyType = wexprSchema_TypeInstance_createFromExpression(
			mapKeyTypeExpr
		);
	}
	
	WexprExpression* mapAllProperties = wexpr_Expression_mapValueForKey(expression, "mapAllProperties");
	if (mapAllProperties)
	{
#if DEBUG_LOGSTDERR
			fprintf(stderr, "Type_createFromExpression('%s') Found mapAllProperties\n", name);
#endif
		self->m_mapAllProperties = wexprSchema_TypeInstance_createFromExpression(
			mapAllProperties
		);
	}
	
	WexprExpression* mapAllowAdditionalPropertiesExpr = wexpr_Expression_mapValueForKey(expression, "mapAllowAdditionalProperties");
	if (mapAllowAdditionalPropertiesExpr)
	{
#if DEBUG_LOGSTDERR
			fprintf(stderr, "Type_createFromExpression('%s') Found mapAllowAdditionalProperties\n", name);
#endif
		const char* str = wexpr_Expression_value(mapAllowAdditionalPropertiesExpr);
		if (strcmp(str, "true") == 0)
		{
#if DEBUG_LOGSTDERR
			fprintf(stderr, "Type_createFromExpression('%s') Enabled mapAllowAdditionalProperties\n", name);
#endif
			self->m_mapAllowAdditionalProperties = true;
		}
	}
	
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
		struct sglib_WexprSchemaPrivateTypeRef_iterator it;
		for (WexprSchemaPrivateTypeRef* list = sglib_WexprSchemaPrivateTypeRef_it_init(&it, self->m_types);
			list != NULL; list = sglib_WexprSchemaPrivateTypeRef_it_next(&it))
		{
			if (list->typeName)
				wexprSchema_PrivateTypeRef_destroy(list);
		}
	}

	if (self->m_valueRegex)
	{
		onig_free(self->m_valueRegex);
		self->m_valueRegex = LIBWEXPR_NULLPTR;
	}
	
	if (self->m_valueRegexString)
	{
		free(self->m_valueRegexString);
	}
	
	if (self->m_arrayAllElements)
	{
		wexprSchema_TypeInstance_destroy(self->m_arrayAllElements);
	}
	
	if (self->m_mapAllProperties)
	{
		wexprSchema_TypeInstance_destroy(self->m_mapAllProperties);
	}
	
	if (self->m_mapKeyType)
	{
		wexprSchema_TypeInstance_destroy(self->m_mapKeyType);
	}
	
	hashmap_iterate(self->m_mapProperties, &s_freeMapPropertiesHashData, NULL);
	hashmap_free(self->m_mapProperties);
	self->m_mapAllProperties = LIBWEXPR_NULLPTR;
	
	free(self);
}

// --- public Loading

bool wexprSchema_Type_resolveWithSchema(WexprSchemaType* self, WexprSchemaSchema* schema, WexprSchemaError** error)
{
#if DEBUG_LOGSTDERR
	fprintf(stderr, "wexprSchema_Type_resolveWithSchema('%s') Resolving types...\n", self->m_name);
#endif
	
	// resolve every type we can
	struct sglib_WexprSchemaPrivateTypeRef_iterator it;
	for (WexprSchemaPrivateTypeRef* list = sglib_WexprSchemaPrivateTypeRef_it_init(&it, self->m_types);
		list != NULL; list = sglib_WexprSchemaPrivateTypeRef_it_next(&it))
	{
		if (!wexprSchema_PrivateTypeRef_isResolved(list))
		{
			const char* name = wexprSchema_PrivateTypeRef_name(list);
			if (name)
			{
				WexprSchemaType* resolvedType = wexprSchema_Schema_typeWithName(schema, name);
				if (resolvedType)
					wexprSchema_PrivateTypeRef_resolveWith(list, resolvedType);
				else
				{
					// failed to resolve
					if (error)
					{
						char errBuffer[256] = {0};
						WexprSchemaTwine message;
						wexprSchema_Twine_init_CStr_CStr(&message, "Failed to resolve type: ", list->typeName);
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
			}
		}
	}
	
	// resolve the map properties
	ResolveMapPropertiesParams p;
	p.error = error;
	p.schema = schema;
	p.result = true;
	
	hashmap_iterate(self->m_mapProperties, &s_resolveMapProperties, &p);
	if (!p.result)
		return false;
	
	// resolve the array
	if (self->m_arrayAllElements)
	{
		bool r = wexprSchema_TypeInstance_resolveWithSchema(
			self->m_arrayAllElements,
			schema,
			error
		);
		if (!r)
			return false;
	}
	
	// resolve the key
	if (self->m_mapKeyType)
	{
		bool r = wexprSchema_TypeInstance_resolveWithSchema(
			self->m_mapKeyType,
			schema,
			error
		);
		if (!r)
			return false;
	}
	
	// resolve the all
	if (self->m_mapAllProperties)
	{
		bool r = wexprSchema_TypeInstance_resolveWithSchema(
			self->m_mapAllProperties,
			schema,
			error
		);
		if (!r)
			return false;
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

	return sglib_WexprSchemaPrivateTypeRef_len(self->m_types);
}

WexprSchemaType* wexprSchema_Type_typeAt (WexprSchemaType* self, unsigned int index)
{
	struct sglib_WexprSchemaPrivateTypeRef_iterator it;
	for (WexprSchemaPrivateTypeRef* list = sglib_WexprSchemaPrivateTypeRef_it_init(&it, self->m_types);
		list != NULL; list = sglib_WexprSchemaPrivateTypeRef_it_next(&it))
	{
		if (index == 0)
			return list->type;
		
		index -= 1;
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
#if DEBUG_LOGSTDERR
	char debugLogBuffer[1024] = {};
	int outDebugBufferLength = 0;
	
	wexprSchema_Twine_resolveToCString(objectPath, debugLogBuffer, sizeof(debugLogBuffer), &outDebugBufferLength);
	fprintf(stderr, "+++ Trying to validate object: %.*s against %s\n", outDebugBufferLength, debugLogBuffer, wexprSchema_Type_name(self));
#endif
	
	// check that the primitive types are set.
	WexprSchemaPrimitiveType primitive = wexprSchema_Type_primitiveTypes(self);
	WexprExpressionType expressionType = (expression ? wexpr_Expression_type(expression) : WexprExpressionTypeInvalid);
	
#if DEBUG_LOGSTDERR
	{
		WexprSchemaTwine twine = wexprSchema_PrimitiveType_toTwine(primitive);
		wexprSchema_Twine_resolveToCString(&twine, debugLogBuffer, sizeof(debugLogBuffer), &outDebugBufferLength);
		fprintf(stderr, "... Trying to match schema primitive type %.*s (%d) against type %s\n",
			outDebugBufferLength, debugLogBuffer,
			primitive,
			wexpr_ExpressionType_toString(expressionType)
		);
	}
#endif
	
	if (!wexprSchema_PrimitiveType_matchesExpressionType(primitive, expressionType))
	{
#if DEBUG_LOGSTDERR
		{
			fprintf(stderr, "  failed to match schema primitive types\n");
		}
#endif

		if (error)
		{
			char errBuffer[256] = {0};
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

			char messageBuffer[256]= {0};
			wexprSchema_Twine_resolveToCString(&errorMessage3, messageBuffer, sizeof(messageBuffer), LIBWEXPR_NULLPTR);

			*error = wexprSchema_Error_create(
				WexprSchemaErrorInternal,
				errBuffer, messageBuffer,
				LIBWEXPR_NULLPTR,
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
			
#if DEBUG_LOGSTDERR
			{
				fprintf(stderr, ">>> Testing against possible type: %s ...\n",
					wexprSchema_Type_name(t)
				);
			}
#endif

			bool success =  wexprSchema_Type_validateObject(t, objectPath, expression, &typeErrors);
			
#if DEBUG_LOGSTDERR
			{
				fprintf(stderr, "<<< Finished testing against possible type: %s : %s\n",
					wexprSchema_Type_name(t),
					(success ? "success" : "failed")
				);
			}
#endif

			if (success)
			{
				parentTypeSelected = t;
				break;
			}
		}

		if (parentTypeSelected == NULL)
		{
			#if DEBUG_LOGSTDERR
			{
				fprintf(stderr, "--- No type matched\n");
			}
			#endif
			
			if (error)
			{
				char errBuffer[256] = {0};
				wexprSchema_Twine_resolveToCString(objectPath, errBuffer, sizeof(errBuffer), LIBWEXPR_NULLPTR);

				*error = wexprSchema_Error_create(
					WexprSchemaErrorInternal,
					errBuffer,
					"Does not match possible types. Reasons for each possible type follows.",
					typeErrors,
					*error
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
	if (primitive & WexprSchemaPrimitiveTypeNull)
	{
		#if DEBUG_LOGSTDERR
			fprintf(stderr, "... Running null tests (none)\n");
		#endif
	}
	
	if (primitive & WexprSchemaPrimitiveTypeArray)
	{
		#if DEBUG_LOGSTDERR
			fprintf(stderr, "... Running array tests\n");
		#endif
			
		if (!s_wexprSchema_Type_validateArray(self, objectPath, expression, error))
			return false;
	}
	
	if (primitive & WexprSchemaPrimitiveTypeBinaryData)
	{
		#if DEBUG_LOGSTDERR
			fprintf(stderr, "... Running binary data tests (none)\n");
		#endif
	}
	
	if (primitive & WexprSchemaPrimitiveTypeMap)
	{
		#if DEBUG_LOGSTDERR
			fprintf(stderr, "... Running map tests\n");
		#endif
	
		if (!s_wexprSchema_Type_validateMap(self, objectPath, expression, error))
			return false;
	}
	
	if (primitive & WexprSchemaPrimitiveTypeValue)
	{
		#if DEBUG_LOGSTDERR
			fprintf(stderr, "... Running value tests\n");
		#endif
			
		if (!s_wexprSchema_Type_validateValue(self, objectPath, expression, error))
		{
		#if DEBUG_LOGSTDERR
			fprintf(stderr, "--- Value tests failed\n");
		#endif
			return false;
		}
	}
	
	#if DEBUG_LOGSTDERR
	{
		wexprSchema_Twine_resolveToCString(objectPath, debugLogBuffer, sizeof(debugLogBuffer), &outDebugBufferLength);
		fprintf(stderr, "--- %s %s: All rules matched - success\n", self->m_name, debugLogBuffer);
	}
	#endif
			
	return true;
}
