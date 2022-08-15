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
			size_t len;
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

bool wexprSchema_Twine_endsWith (WexprSchemaTwine* self, const char* postfix)
{
	char buffer[512] = {0};
	
	int outLength = 0;
	if (!wexprSchema_Twine_resolveToCString(self, buffer, sizeof(buffer), &outLength))
	{
		fprintf(stderr, "Unable to resolve to buffer for wexprSchema_Twine_endsWith()");
		exit(1);
	}
	
	int len = strlen(postfix);
	
	if (strcmp(buffer+outLength-len, postfix) == 0)
		return true;
	
	return false;
}
