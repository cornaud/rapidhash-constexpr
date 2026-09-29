# rapidhash v3 for compile-time evaluation

A portable, accurate, header-only C++20 implementation of **rapidhash v3** designed for
**compile-time** (constexpr) evaluation.

> [!NOTE]
>
> This implementation is **consteval** only.
> It is meant to be paired with a runtime implementation for such uses.
>
> The reference runtime implementation is available here:
> https://github.com/Nicoshev/rapidhash

Portable across most platforms:
* MSVC compatibility
* Forces little-endian

Tested:
* Full output compatibility testing against canonical results
* Complete API testing
* Compiler and macro modifier matrix

Covers all of the variants:
* Micro and Nano support
* Protected modification support
* Unrolled macro modifier support

Convenient API:
* Small, idiomatic API
* Supports raw byte buffers and strings
* Allows the hashing of C++ object representations
* Direct API for hashing array representations

## Usage

The API and its full documentation can be found at the bottom of the header file.
It is accessible through the `rapidhash_cexpr` namespace.

The API is divided into 3 families of functions:
* `rapidhash_raw`: for the hashing of arrays of byte-like types.
* `rapidhash_str`: for the hashing of strings, they don't include the null terminator.
* `rapidhash`: for the hashing of C++ object representations, such as integers and even structures
(when allowed). An overload for arrays of these is also available.

Examples of usage can be found in `tests/api_test.h`.