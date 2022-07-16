//
/// \file libWexprSchema/Twine.c
/// \brief Simple twine class
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#include <libWexprSchema/Twine.h>

#include <string.h>

static bool s_writeOutChild (WexprSchemaTwineChildType type, WexprSchemaTwineChild* child, char* buffer, int bufferLength, int* outBufferLength)
{
	switch (type)
	{
		case WexprSchemaTwineChildTypeEmpty:
		{
			if (outBufferLength) { *outBufferLength = 0; }
			return true;
		}

		case WexprSchemaTwineChildTypeCStringWithLength:
		case WexprSchemaTwineChildTypeCString:
		{
			int len;
			const char* buf;

			if (type == WexprSchemaTwineChildTypeCStringWithLength)
			{
				len = child->cStringWithLength.length;
				buf = child->cStringWithLength.cString;
			}
			else
			{
				buf = child->cString;
				len = strlen(child->cString);
			}

			if (len > bufferLength) {
				if (outBufferLength) { *outBufferLength = 0; }
				return false;
			}

			memcpy(buffer, child->cString, len);
			if (outBufferLength) { *outBufferLength = len; }
			return true;
		}

		case WexprSchemaTwineChildTypeTwine:
		{
			return wexprSchema_Twine_resolveToCString(child->twine, buffer, bufferLength, outBufferLength);
		}

		default:
			// unable to determine type
			return false;
	}
}

// ---- members

bool wexprSchema_Twine_resolveToCString (WexprSchemaTwine* self, char* buffer, int bufferLength, int* outBufferLength)
{
	int leftCurWritten = 0;

	// left side
	bool success = s_writeOutChild(self->lhsType, &(self->lhs), buffer, bufferLength, &leftCurWritten);
	if (!success)
	{
		if (outBufferLength) *outBufferLength = leftCurWritten;
		return false;
	}

	// right side
	int rightCurWritten = 0;
	success = s_writeOutChild(self->rhsType, &(self->rhs), buffer+leftCurWritten, bufferLength-leftCurWritten, &rightCurWritten);

	if (outBufferLength) *outBufferLength = leftCurWritten + rightCurWritten;

	return success;
}
