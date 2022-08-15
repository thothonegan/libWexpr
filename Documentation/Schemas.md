
Wexpr schemas


Schemas are based loosely on json-schema, but applied to Wexpr.

A schema file is a wexpr file that is used to validate another wexpr file, making sure it meets the given conditions.
Basically, similar to type safety for a configuration file.

Schema files themselves are validatable via the schema's schema.
Schemas can be specified either externally, or using an internal $schema on the root object of the file.

libWexprSchema is the reference library for handling schemas, and WexprTool has support to validate them.


A schema contains:
- an Id used to 