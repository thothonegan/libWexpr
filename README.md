# libWexpr [MIT]

Wexpr is a simple configuration language, similar to lisps's S expressions or JSON.
Designed to be readable, while being quick and easy to parse.

Example file can be found in [simple.wexpr](Examples/simple.wexpr).

Some editor syntax files are provided under [Syntax](Syntax/) - most editors choosing 'lisp' for .wexpr files will also be close to correct for basic highlighting.

Basic syntax
--------------------

```
; this is a comment

;(--
	This is a block comment
--)

"This is a value"
This_Is_Also_A_Value
1.0 ; and this
true ; and this - basically anything thats a single string or quoted

#(a b c) ; this is an array. Ordered
@(k1 v1 k2 v2) ; this is a map of key-value pairs. Not ordered.

[asdf] 20 ; this is a reference. In this case 'asdf' is 20.
*[asdf] ; this places a reference. Would place '20' here in the file.
```


Related Repositories
--------------------

- Atom syntax - https://github.com/thothonegan/language-wexpr/
- Lua implementation - https://github.com/tyraindreams/libWexpr.lua
