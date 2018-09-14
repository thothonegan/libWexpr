#!/usr/bin/env lua

package.path ='?.lua;' .. package.path
libWexpr = require("libWexpr") -- put libWexpr.lua in the same dir as this file

path = arg[1]

local file = io.open(path, "rb")
local content = file:read("*a")
file:close()

local res,err = libWexpr:decode(content)

if res == nil and err ~= nil then
	print("Error: " .. err)
	os.exit(1)
end
