 
PCRE compatible regular expression - see regex101.compatible
mode : /sg

Thanks to Alexander Clay

```
(?'whitespace'[\s]+)|(?'bcomment';\(--.*--\))|(?'lcomment';[^\n]*)|(?'string'\"(?:[^\\\\\"]+|\\\\.)*\")|(?'number'-?\d*\.?\d)|(?'word'[^<>"*#@\(\);\[\]\r\n \t]+)|(?'referencedefinition'\*\[[a-zA-Z_][\w]*\])|(?'reference'\[[a-zA-Z_][a-zA-Z0-9_]*\])|(?'binary'<[a-zA-Z0-9\+\/=]+>)|(?'array'\#\()|(?'map'\@\()|(?'close'\))
```
