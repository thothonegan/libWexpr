 
PCRE compatible regular expression - see regex101.compatible
mode : /sg

Thanks to Alexander Clay

```
(?'whitespace'[\s]+)|(?'bcomment';\(--.*--\))|(?'lcomment';[^\n]*)|(?'string'\"(?:[^\\\\\"]+|\\\\.)*\")|(?'number'[-]?[0-9]+[.]?[0-9]*)|(?'word'[^<>*#@\(\);\[\]\r\n \t]+)|(?'referencedefinition'\*\[[a-zA-Z_][a-zA-Z0-9_]*\])|(?'reference'\[[a-zA-Z_][a-zA-Z0-9_]*\])|(?'binary'<[a-zA-Z0-9\+\/=]+>)|(?'array'\#\()|(?'map'\@\()|(?'close'\))
```
