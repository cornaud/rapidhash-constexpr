//
//	Copyright(c) 2026 Nathan Cornaud
// 
//	SPDX-License-Identifier: MIT
//

#pragma once

#include "../rapidhash_cexpr.h"

#include <array>
#include <cstdint>
#include <string_view>


// This macro modifier only extends the API surface without altering it, so there is no reason not
// to define it for all tests.
#ifndef RAPIDHASH_CEXPR_ALLOW_CLASS_REPR
	#error "RAPIDHASH_CEXPR_ALLOW_CLASS_REPR macro modifier needs to be defined to test the full API." 
#endif

constexpr std::array<std::uint8_t, 9> kRapidhashRawTestInput =
	{ 0, 1, 2, 4, 8, 16, 32, 64, 128 };

constexpr std::string_view kRapidhashStrTestInput =
	"The quick brown fox jumps over the lazy dog!";

constexpr std::uint64_t kRapidhashObjTestInput =
	0xDEADBEEFDEADBEEF;

struct ClassTestMock
{
	std::uint64_t field1;
	std::uint64_t field2;
};
// There is no reason for the compiler to add padding, but that's for documentation purposes.
static_assert(sizeof(ClassTestMock) == sizeof(std::uint64_t) * 2);

constexpr ClassTestMock kRapidhashClassTestInput =
	{ 0xDEADBEEFDEADBEEF, 0xCAFEBABECAFEBABE };

/*-----------------------------------------------------------------------------------------------*/

using namespace rapidhash_cexpr;

#ifndef RAPIDHASH_PROTECTED
	static_assert(rapidhash_raw(std::span{ kRapidhashRawTestInput })
		== UINT64_C(11621751150061859480));
	static_assert(rapidhash_raw_micro(std::span{ kRapidhashRawTestInput })
		== UINT64_C(11621751150061859480));
	static_assert(rapidhash_raw_nano(std::span{ kRapidhashRawTestInput })
		== UINT64_C(11621751150061859480));

	static_assert(rapidhash_str(kRapidhashStrTestInput)
		== UINT64_C(7081045857440208797));
	static_assert(rapidhash_str_micro(kRapidhashStrTestInput)
		== UINT64_C(7081045857440208797));
	static_assert(rapidhash_str_nano(kRapidhashStrTestInput)
		== UINT64_C(7081045857440208797));

	static_assert(rapidhash(kRapidhashObjTestInput)
		== UINT64_C(3347823127960748911));
	static_assert(rapidhash_micro(kRapidhashObjTestInput)
		== UINT64_C(3347823127960748911));
	static_assert(rapidhash_nano(kRapidhashObjTestInput)
		== UINT64_C(3347823127960748911));

	static_assert(rapidhash(kRapidhashClassTestInput)
		== UINT64_C(11791412416827551821));
	static_assert(rapidhash_micro(kRapidhashClassTestInput)
		== UINT64_C(11791412416827551821));
	static_assert(rapidhash_nano(kRapidhashClassTestInput)
		== UINT64_C(11791412416827551821));

#else
	static_assert(rapidhash_raw(std::span{ kRapidhashRawTestInput })
		== UINT64_C(17668022348940638732));
	static_assert(rapidhash_raw_micro(std::span{ kRapidhashRawTestInput })
		== UINT64_C(17668022348940638732));
	static_assert(rapidhash_raw_nano(std::span{ kRapidhashRawTestInput })
		== UINT64_C(17668022348940638732));

	static_assert(rapidhash_str(kRapidhashStrTestInput)
		== UINT64_C(5502392529543867157));
	static_assert(rapidhash_str_micro(kRapidhashStrTestInput)
		== UINT64_C(5502392529543867157));
	static_assert(rapidhash_str_nano(kRapidhashStrTestInput)
		== UINT64_C(5502392529543867157));

	static_assert(rapidhash(kRapidhashObjTestInput)
		== UINT64_C(5946730404678310576));
	static_assert(rapidhash_micro(kRapidhashObjTestInput)
		== UINT64_C(5946730404678310576));
	static_assert(rapidhash_nano(kRapidhashObjTestInput)
		== UINT64_C(5946730404678310576));

	static_assert(rapidhash(kRapidhashClassTestInput)
		== UINT64_C(7709455843466862430));
	static_assert(rapidhash_micro(kRapidhashClassTestInput)
		== UINT64_C(7709455843466862430));
	static_assert(rapidhash_nano(kRapidhashClassTestInput)
		== UINT64_C(7709455843466862430));

#endif