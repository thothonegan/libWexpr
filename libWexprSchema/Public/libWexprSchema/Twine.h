//
/// \file libWexprSchema/Twine.h
/// \brief Simple twine class
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#ifndef LIBWEXPRSCHEMA_TWINE_H
#define LIBWEXPRSCHEMA_TWINE_H

#include <libWexpr/Macros.h>

#include <stdbool.h>

LIBWEXPR_EXTERN_C_BEGIN()

typedef enum WexprSchemaTwineChildType
{
	/// Empty string
	WexprSchemaTwineChildTypeEmpty,

	/// This side is pointing at a const char* thats zero terminated
	WexprSchemaTwineChildTypeCString,

	/// This side is pointing at a const char* that we have a fixed length
	WexprSchemaTwineChildTypeCStringWithLength,

	/// This side is pointing at another Twine
	WexprSchemaTwineChildTypeTwine
} WexprSchemaTwineChildType;

typedef union WexprSchemaTwineChild
{
	/// WexprSchemaTwineChildTypeCString: zero terminated cString
	const char* cString;

	/// WexprSchemaTwineChildTypeCString: cstring with length
	struct {
		const char* cString;
		unsigned int length;
	} cStringWithLength;

	/// WexprSchemaTWineChildTypeTwine: The twine we're pointing at
	struct WexprSchemaTwine* twine;
} WexprSchemaTwineChild;

//
/// \brief Simple twine class that stores the concation of objects without taking any ownerhip
/// or performing the concat until needed.
/// Everything is inlined for efficency. Note that everything must be constant once put into
//
typedef struct WexprSchemaTwine
{
	// the left hand side
	WexprSchemaTwineChild lhs;
	WexprSchemaTwineChildType lhsType;

	// the right hand side
	WexprSchemaTwineChild rhs;
	WexprSchemaTwineChildType rhsType;

} WexprSchemaTwine;

//
/// \brief Init the twine as an empty twine
//
static inline void wexprSchema_Twine_init_Empty_Empty (WexprSchemaTwine* self)
{
	self->lhsType = WexprSchemaTwineChildTypeEmpty;
	self->rhsType = WexprSchemaTwineChildTypeEmpty;
}

//
/// \brief Init the twine with the given string and empty
//
static inline void wexprSchema_Twine_init_CStr_Empty (WexprSchemaTwine* self, const char* lhs)
{
	self->lhsType = WexprSchemaTwineChildTypeCString;
	self->lhs.cString = lhs;
	self->rhsType = WexprSchemaTwineChildTypeEmpty;
}

//
/// \brief Init the twine with the given strings
//
static inline void wexprSchema_Twine_init_CStr_CStr (WexprSchemaTwine* self, const char* lhs, const char* rhs)
{
	self->lhsType = WexprSchemaTwineChildTypeCString;
	self->lhs.cString = lhs;
	self->rhsType = WexprSchemaTwineChildTypeCString;
	self->rhs.cString = rhs;
}

//
/// \brief Init the twine with a twine and an empty
//
static inline void wexprSchema_Twine_init_Twine_empty (WexprSchemaTwine* self, WexprSchemaTwine* lhs)
{
	self->lhsType = WexprSchemaTwineChildTypeTwine;
	self->lhs.twine = lhs;
	self->rhsType = WexprSchemaTwineChildTypeEmpty;
}

//
/// \brief Init the twine with a twine and a cstring
//
static inline void wexprSchema_Twine_init_Twine_CStr (WexprSchemaTwine* self, WexprSchemaTwine* lhs, const char* rhs)
{
	self->lhsType = WexprSchemaTwineChildTypeTwine;
	self->lhs.twine = lhs;
	self->rhsType = WexprSchemaTwineChildTypeCString;
	self->rhs.cString = rhs;
}

//
/// \brief Init the twine with a twine and another twine
//
static inline void wexprSchema_Twine_init_Twine_Twine (WexprSchemaTwine* self, WexprSchemaTwine* lhs, WexprSchemaTwine* rhs)
{
	self->lhsType = WexprSchemaTwineChildTypeTwine;
	self->lhs.twine = lhs;
	self->rhsType = WexprSchemaTwineChildTypeTwine;
	self->rhs.twine = rhs;
}

//
/// \brief Resolve the twine into a char buffer.
/// \return Returns true if it succeeds, false if an error occurs
//
bool wexprSchema_Twine_resolveToCString (WexprSchemaTwine* self, char* buffer, int bufferLength, int* outBufferLength);

//
/// \brief Does the resolved twine end with the requested string?
//
bool wexprSchema_Twine_endsWith (WexprSchemaTwine* self, const char* postfix);

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPRSCHEMA_TWINE_H
