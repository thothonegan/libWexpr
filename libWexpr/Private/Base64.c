//
/// \file libWexpr/Base64.c
/// \brief Base64 support
//
// #LICENSE_BEGIN:MIT#
// 
// Copyright (c) 2017-2018, Kenneth Perry (thothonegan)
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// #LICENSE_END#
// 

#include "Base64.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

// --- static

const char* s_base64Table = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/"
;

bool s_isValidBase64Character (char c)
{
	return (c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z')
		|| (c >= '0' && c <= '9')
		|| (c == '+')
		|| (c == '/')
	;
}

uint8_t s_indexOfBase64Character (uint8_t b)
{
	if (b >= 'A' && b <= 'Z') return b - 'A';
	else if (b >= 'a' && b <= 'z') return (b - 'a') + 26;
	else if (b >= '0' && b <= '9') return (b - '0') + 52;
	else if (b == '+') return 62;
	else if (b == '/') return 63;
	
	else
	{
		return 0xFF; // invalid
	}
}

void s_base64EncodeAndAppend (char* buffer, uint8_t input[3], size_t amountToWrite)
{
	uint8_t outputBytes[4] = {
		(uint8_t) ( (input[0] & 0xFC) >> 2),
		(uint8_t) (((input[0] & 0x03) << 4) + ((input[1] & 0xF0) >> 4)),
		(uint8_t) (((input[1] & 0x0F) << 2) + ((input[2] & 0xC0) >> 6)),
		(uint8_t) ( (input[2] & 0x3F))
	};
	
	
	for (size_t i=0; i < amountToWrite; ++i)
	{
		buffer[i] = s_base64Table[outputBytes[i]];
	}
}

void s_base64DecodeAndAppend (void* buffer, uint8_t input[4], size_t amountToWrite)
{
	uint8_t* b = buffer;
	
	uint8_t decodedBytes[3] = {
		(uint8_t) (  (input[0]         << 2) + ((input[1] & 0x30) >> 4)),
		(uint8_t) ( ((input[1] & 0x0F) << 4) + ((input[2] & 0x3C) >> 2)),
		(uint8_t) ( ((input[2] & 0x03) << 6) + input[3])
	};
	
	for (size_t i=0; i < amountToWrite; ++i)
		b[i] = decodedBytes[i];
}

// --- main

Base64Buffer base64_decode (Base64IBuffer buf)
{
	Base64Buffer res;
	
	// estimate size : every 4 bytes of text becomes 3 bytes binary.
	res.size = buf.size * 3 / 4 + 1;
	res.buffer = malloc(res.size);
	
	if (!res.buffer)
		return res; // buffer is null so it's invalid
	
	const uint8_t* bufBuf = buf.buffer;
	
	size_t lenRemaining = buf.size;
	size_t inPos = 0; // position in buf.buffer
	
	uint8_t outputBuffer[4];
	size_t outBufferPos = 0; // position in outputBuffer
	size_t outPos = 0; // position in res.buffer
	
	while (lenRemaining > 0
		&& ( bufBuf[inPos] != '=')
	)
	{
		--lenRemaining; // handling one
		
		if (!s_isValidBase64Character (bufBuf[inPos]))
		{
			Base64Buffer r;
			r.buffer = NULL; r.size = 0;
			return r; // invalid string
		}
		
		outputBuffer[outBufferPos++] = bufBuf[inPos];
		inPos++;
		
		if (outBufferPos == 4)
		{
			// swap our output buffer with the positions
			for (size_t i=0; i < 4 ; ++i)
				outputBuffer[i] = s_indexOfBase64Character (outputBuffer[i]);
			
			s_base64DecodeAndAppend((uint8_t*)res.buffer + outPos, outputBuffer, 3);
			outPos += 3;
			outBufferPos = 0;
		}
	}
	
	if (outBufferPos != 0)
	{
		for (size_t j=outBufferPos; j < 4; ++j)
			outputBuffer[j] = 0; // empty the rest
		
		// swap our output buffer with the positions
		for (size_t i=0; i < 4 ; ++i)
			outputBuffer[i] = s_indexOfBase64Character (outputBuffer[i]);
		
		s_base64DecodeAndAppend((uint8_t*)res.buffer + outPos, outputBuffer, outBufferPos-1);
		outPos += outBufferPos-1;
	}
	
	if (outPos < res.size)
	{
		// we didnt fill up the entire buffer, just cut the size down
		res.size = outPos;
	}
	
	return res;
}

Base64Buffer base64_encode (Base64IBuffer buf)
{
	Base64Buffer res;
	
	// estimated size : every 3 bytes becomes 4 bytes
	res.size = 4 * ((buf.size / 3) + 1); // 4*ceil(n/3)
	res.buffer = malloc(res.size);
	
	if (!res.buffer)
		return res; // buffer is null so its invalid
	
	size_t remaining = buf.size;
	uint8_t inputBytes[3];
	
	size_t curInInputBytes = 0; // current position in inputBytes
	size_t curInInput = 0; // current position in buf.buffer
	size_t curInOutput = 0; // current position in res.buffer
	
	while (remaining--)
	{
		inputBytes[curInInputBytes] = *( (uint8_t*)buf.buffer + curInInput);
		curInInputBytes++;
		curInInput++;
		
		if (curInInputBytes == 3)
		{
			// filled up - encode
			s_base64EncodeAndAppend((char*)res.buffer + curInOutput, inputBytes, 4);
			curInOutput += 4;
			curInInputBytes = 0;
		}
	}
	
	// out of bytes - 0 the rest out
	if (curInInputBytes != 0)
	{
		for (size_t j = curInInputBytes; j < 3; ++j)
		{
			inputBytes[j] = 0;
		}
		
		s_base64EncodeAndAppend((char*)res.buffer + curInOutput, inputBytes, curInInputBytes+1);
		curInOutput += curInInputBytes+1;
		
		// append padding
		while (curInInputBytes++ < 3)
		{
			((uint8_t*)res.buffer)[curInOutput] = '=';
			curInOutput++;
		}
	}
	
	// make sure its output size is correct
	res.size = curInOutput;
	
	return res; // success
}
