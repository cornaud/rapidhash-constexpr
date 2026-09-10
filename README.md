# rapidhash v3 for compile-time evaluation

A portable, accurate, header-only C++20 implementation of **rapidhash v3** designed for
**compile-time** (constexpr) evaluation.

> [!NOTE]
>
> This implementation is **consteval** only.
> It is meant to be paired with a runtime implementation for such uses.
>
> The reference runtime implementation is available here :
> https://github.com/Nicoshev/rapidhash

Portable across most platforms :
* No compiler intrinsics
* Forces little-endian

Tested :
* Full output compatibility testing against canonical results
* Complete API testing
* Compiler and macro modifier matrix

Covers all of the variants :
* Micro and Nano support
* Protected modification support
* Original macro modifiers support

Convenient API :
* Small, idiomatic API
* Supports raw byte buffers and strings
* Allows the hashing of C++ object representations