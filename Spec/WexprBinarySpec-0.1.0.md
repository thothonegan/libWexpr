Wexpr Binary Spec 0.1.0
======================

Based on Wexpr Spec 0.1

Refer to [the text spec](WexprSpec.md) for the general concepts and ideas.

This format provides a method for storing a wexpr expression as binary for faster loading,
parsing, as needed. It may also be referred to as the 'ExpressionDataStore' format in older implementations (WolfWexpr 1, ruby), though this is slightly different.

Most typing should be straight forward, using names from C (uint8_t, etc).
Special types include:
* UVLQ64 : An unsigned 64bit [Variable Length Quantity](https://en.wikipedia.org/wiki/Variable-length_quantity) . The MSB for each byte is either 0 (which means this is the last octet) or 1 (which means continue to the next octet).


The standard extension is .bwexpr. All multibyte values are big endian.

The binary format currently has the following limitations:
- References are not in the binary format. Being a parsing construct, it is removed before implementation.
- Comments are not in the binary format. Similar reasoning to References.

The format is a chunked format geared towards faster reading, at the speed of some backtracing while writing.
With smaller files, the binary spec will probably end up bigger then the original text files (mainly due to size markers).

High level:
- Header
- Chunks... (generally at least an expression chunk). Currently no other ones are defined.

Header (20 bytes)
-----------------

| Name       | Type       | Comments                                               |
| ---------- | ---------- | ------------------------------------------------------ |
| magic      | uint8_t[8] | must be '0x83' 'BWEXPR' '0x0A'                         |
| version    | uint32_t   | Version of the spec it matches. See below.             |
| reserved   | uint8_t[8] | Reserved for future use. Currently should all be 0x00. |

version comments:  The general format is (reserved2)(major2)(minor2)(patch2). So for example a possible 5.2.1 version would be 00050201 decimal value.

Current versions:
0.1.0 - 0x00001000
0.0.1 - 0x00000001

Chunks
-------

Chunks are structured in the given format:

| Name | Type     | Comments                                                        |
| ---- | -------- | --------------------------------------------------------------- |
| size | UVLQ64   | Size of the data field in bytes,                                |
| type | uint8_t  | Type of the chunk. Unknown chunks should be ignored.            |
| data | bytes... | The data within the chunk.                                      |

Chunks have the currently defined values:

- 0x00 - Expression : Null
- 0x01 - Expression : Value
- 0x02 - Expression : Map
- 0x03 - Expression : Array
- 0x04 - Expression : BinaryData
- 0x7F and below - Reserved for future use by the spec.
- 0x80 and up - Reserved for per-user or experimental use.

The only required chunk is one and only one root expression chunk. Other chunks can be added as needed. Currently we have none defined, but might eventually have such as a comment chunk, or other arbitary data that wants to be associated with the file.

Expression : Null Chunk - 0x00
--------------------------------

Data is empty. Represents a null wexpr expression.

Example:

```
[0][0x00]
```

Expression : Value Chunk - 0x01
---------------------------------

Data contains the string for a value chunk. Size is UTF8 string length with no null terminator.
Data within the value must be UTF-8 safe.

Example:

```
[15][0x01]["High Correction"]
```

Expression : Map Chunk - 0x02
-------------------------------

Data is a list of pair of key/value expressions. Size contains all child chunks.

Example (indentation for readability):

```
[22][0x02]
	[6][0x01]["Engine"]
	[4][0x01]["Wolf"]
```

Expression : Array Chunk - 0x03
---------------------------------

Data is a list of values. Size contains all child chunks.

Example (indentation for readability):

```
[21][0x03]
	[1][0x01]["1"]
	[1][0x01]["2"]
	[1][0x01]["3"]
```

Expression : Binary Data Chunk - 0x04
---------------------------------------

Data stores binary data directly.

Format of the data:

| Name        | Type     | Comments                                |
| ----------- | -------- | --------------------------------------- |
| compression | uint8_t  | The method the data is compressed with. |
| data        | bytes... | The compressed data.                    |


The method for compressing data is also provided with the given possible values:
- 0x00 - No compression (raw).
- 0x01 - INFLATE/zlib compression

Example:
```
[9][0x04][0x00][0x83 0x42 0x57 0x45 0x58 0x50 0x52 0x0A]
```


Changes
---------------
* 0.1 - Switched to a VLQ64 encoding instead of a fixed for sizes. Allows 64bit files (along with better compression of small values).
* 0.0.1 - Original version
