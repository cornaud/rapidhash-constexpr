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

## Benchmarks

Due to some limitations in compile-time execution context, this implementation's flow
rate is from **30% to 50%** slower on small keys. However, on large key sizes, it is
almost identical to the reference implementation.

Performances are expected to be worse when compiling with MSVC, due to the absence of a 128 bits intrinsic type.

This was measured using xxHash's [Open Source benchmark program](https://github.com/Cyan4973/xxHash/tree/release/tests/bench).

This is the main reason why you should pair this library, optimized for compile-time
workloads, with an [optimized runtime implementation](https://github.com/Nicoshev/rapidhash).
