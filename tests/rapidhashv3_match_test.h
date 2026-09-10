//
//	Copyright(c) 2026 Nathan Cornaud
// 
//	SPDX-License-Identifier: MIT
//

#pragma once

#include "../rapidhash_cexpr.h"

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>


// This macro modifier only adds more branches to test without modifying others, so there is no
// reason not to define it for all tests (also, it only affects the classic version).
#ifndef RAPIDHASH_UNROLLED
	#error "RAPIDHASH_UNROLLED macro modifier needs to be defined to test the full algorithm." 
#endif

// All bits are set to 1 so all of them are digested. This way we can catch if the algorithm
// doesn't process correctly a specific part of the seed.
constexpr std::uint64_t kTestSeed = UINT64_MAX;

constexpr std::size_t kMaxKeySize = 897;

constexpr auto test_key = [] {
	std::array<std::uint8_t, kMaxKeySize> key{};
	key.fill(static_cast<std::uint8_t>(0xFF));
	return key;
}();

// rapidhash algorithms treat data differently depending on the key length, so we need to test all
// possible branches by adjusting it.

struct MatchTestEntry
{
	// Size of the key in bytes.
	const std::size_t key_size;

	// Result of the canonical algorithm for the same input.
	const std::uint64_t expected_result;
};


#ifndef RAPIDHASH_PROTECTED
	/// Fast version's hashes.
	constexpr auto kRapidhashTests = std::to_array<MatchTestEntry>
	(
		{
			{ 0,	UINT64_C(11140877522690980840) },
			{ 1,	UINT64_C(9251219562908335114) },
			{ 3,	UINT64_C(9855541361249270571) },
			{ 4,	UINT64_C(16053422642684411695) },
			{ 7,	UINT64_C(6829848028857839105) },
			{ 8,	UINT64_C(1674115573101822818) },
			{ 16,	UINT64_C(6887163819122108532) },
			{ 17,	UINT64_C(10560720991023676734) },
			{ 32,	UINT64_C(7526751371883728689) },
			{ 33,	UINT64_C(16786768215918839256) },
			{ 48,	UINT64_C(9745953224250731655) },	// Nano's last in-boundary hash.
			{ 49,	UINT64_C(13168672448780669553) },
			{ 64,	UINT64_C(4665151007981721119) },
			{ 65,	UINT64_C(11902017547541073991) },	
			{ 80,	UINT64_C(13445179613622457919) },	// Nano's last in-boundary hash.
			{ 81,	UINT64_C(17020541547615768859) },
			{ 96,	UINT64_C(4031136537890506306) },
			{ 97,	UINT64_C(15042590471307266296) },
			{ 112,	UINT64_C(14255831598678046013) },
			{ 113,	UINT64_C(915505647382813063) },
			{ 224,	UINT64_C(7845576733303907292) },
			{ 225,	UINT64_C(8414212197772991758) },
			{ 336,	UINT64_C(4777339466429327366) },
			{ 337,	UINT64_C(886311968889344095) },
			{ 448,	UINT64_C(568954917701723228) },
			{ 449,	UINT64_C(7298001394582076224) },
			{ 672,	UINT64_C(17891627798677646466) },
			{ 673,	UINT64_C(18171837725067603561) },
			{ 784,	UINT64_C(17840175707310819059) },
			{ 785,	UINT64_C(16789280020887996903) },
			{ 896,	UINT64_C(3723178991198336620) },
			{ 897,	UINT64_C(5314690915440935096) },
		}
	);

	/*
	 *	Micro and Nano versions should match the classic version's hashes up to a certain key size,
	 *	which also happens to be right out of the last branching in their respective implementation.
	 *	For those reasons we test one key size outside of those limits for each algorithm
	 *	(80 to 81 marks the limit for Micro and 48 to 49 for Nano).
	 */

	constexpr MatchTestEntry kOutOfBoundTestMicro =
	{
		81,
		UINT64_C(420530629328136403)
	};

	constexpr MatchTestEntry kOutOfBoundTestNano =
	{
		49,
		UINT64_C(706321860845937056)
	};

#else
	/// Protected version's hashes
	/// (contains the hashes of Nano because we only need to test for one version).
	constexpr auto kRapidhashTests = std::to_array<MatchTestEntry>
	(
		{
			{ 0,	UINT64_C(12058417598163660915) },
			{ 1,	UINT64_C(1033430555000069945) },
			{ 3,	UINT64_C(8142942729113398040) },
			{ 4,	UINT64_C(7794017408696824316) },
			{ 7,	UINT64_C(14559166778982862856) },
			{ 8,	UINT64_C(7786490067445414955) },
			{ 16,	UINT64_C(14183153642459654301) },
			{ 17,	UINT64_C(2124688490172508608) },
			{ 32,	UINT64_C(13027415969370110270) },
			{ 33,	UINT64_C(13053718693051117860) },
			{ 48,	UINT64_C(7571253273798065008) },
		}
	);

	constexpr MatchTestEntry kOutOfBoundTestNano =
	{
		49,
		UINT64_C(1721829565746737837)
	};

#endif // RAPIDHASH_FAST

/*-----------------------------------------------------------------------------------------------*/

using namespace rapidhash_cexpr::detail;

consteval bool test_match_classic(MatchTestEntry test_entry)
{
	auto current_key = std::span{ test_key }.first(test_entry.key_size);
	std::uint64_t output = rapidhash_internal(current_key, kTestSeed);
	if (output == test_entry.expected_result)
	{
		return true;
	}
	return false;
}

consteval bool test_match_micro(MatchTestEntry test_entry)
{
	auto current_key = std::span{ test_key }.first(test_entry.key_size);
	std::uint64_t output = rapidhash_micro_internal(current_key, kTestSeed);
	if (output == test_entry.expected_result)
	{
		return true;
	}
	return false;
}

consteval bool test_match_nano(MatchTestEntry test_entry)
{
	auto current_key = std::span{ test_key }.first(test_entry.key_size);
	std::uint64_t output = rapidhash_nano_internal(current_key, kTestSeed);
	if (output == test_entry.expected_result)
	{
		return true;
	}
	return false;
}

/*-----------------------------------------------------------------------------------------------*/

// For these modifiers, we only need to test for one version (like Nano), as errors on the others
// would be catched by the individual tests.
#ifndef RAPIDHASH_PROTECTED
	static_assert(test_match_classic(kRapidhashTests[0]));
	static_assert(test_match_classic(kRapidhashTests[1]));
	static_assert(test_match_classic(kRapidhashTests[2]));
	static_assert(test_match_classic(kRapidhashTests[3]));
	static_assert(test_match_classic(kRapidhashTests[4]));
	static_assert(test_match_classic(kRapidhashTests[5]));
	static_assert(test_match_classic(kRapidhashTests[6]));
	static_assert(test_match_classic(kRapidhashTests[7]));
	static_assert(test_match_classic(kRapidhashTests[8]));
	static_assert(test_match_classic(kRapidhashTests[9]));
	static_assert(test_match_classic(kRapidhashTests[10]));
	static_assert(test_match_classic(kRapidhashTests[11]));
	static_assert(test_match_classic(kRapidhashTests[12]));
	static_assert(test_match_classic(kRapidhashTests[13]));
	static_assert(test_match_classic(kRapidhashTests[14]));
	static_assert(test_match_classic(kRapidhashTests[15]));
	static_assert(test_match_classic(kRapidhashTests[16]));
	static_assert(test_match_classic(kRapidhashTests[17]));
	static_assert(test_match_classic(kRapidhashTests[18]));
	static_assert(test_match_classic(kRapidhashTests[19]));
	static_assert(test_match_classic(kRapidhashTests[20]));
	static_assert(test_match_classic(kRapidhashTests[21]));
	static_assert(test_match_classic(kRapidhashTests[22]));
	static_assert(test_match_classic(kRapidhashTests[23]));
	static_assert(test_match_classic(kRapidhashTests[24]));
	static_assert(test_match_classic(kRapidhashTests[25]));
	static_assert(test_match_classic(kRapidhashTests[26]));
	static_assert(test_match_classic(kRapidhashTests[27]));
	static_assert(test_match_classic(kRapidhashTests[28]));
	static_assert(test_match_classic(kRapidhashTests[29]));
	static_assert(test_match_classic(kRapidhashTests[30]));
	static_assert(test_match_classic(kRapidhashTests[31]));

	static_assert(test_match_micro(kRapidhashTests[0]));
	static_assert(test_match_micro(kRapidhashTests[1]));
	static_assert(test_match_micro(kRapidhashTests[2]));
	static_assert(test_match_micro(kRapidhashTests[3]));
	static_assert(test_match_micro(kRapidhashTests[4]));
	static_assert(test_match_micro(kRapidhashTests[5]));
	static_assert(test_match_micro(kRapidhashTests[6]));
	static_assert(test_match_micro(kRapidhashTests[7]));
	static_assert(test_match_micro(kRapidhashTests[8]));
	static_assert(test_match_micro(kRapidhashTests[9]));
	static_assert(test_match_micro(kRapidhashTests[10]));
	static_assert(test_match_micro(kRapidhashTests[11]));
	static_assert(test_match_micro(kRapidhashTests[12]));
	static_assert(test_match_micro(kRapidhashTests[13]));
	static_assert(test_match_micro(kRapidhashTests[14]));

	static_assert(test_match_micro(kOutOfBoundTestMicro));

#endif

	static_assert(test_match_nano(kRapidhashTests[0]));
	static_assert(test_match_nano(kRapidhashTests[1]));
	static_assert(test_match_nano(kRapidhashTests[2]));
	static_assert(test_match_nano(kRapidhashTests[3]));
	static_assert(test_match_nano(kRapidhashTests[4]));
	static_assert(test_match_nano(kRapidhashTests[5]));
	static_assert(test_match_nano(kRapidhashTests[6]));
	static_assert(test_match_nano(kRapidhashTests[7]));
	static_assert(test_match_nano(kRapidhashTests[8]));
	static_assert(test_match_nano(kRapidhashTests[9]));
	static_assert(test_match_nano(kRapidhashTests[10]));

	static_assert(test_match_nano(kOutOfBoundTestNano));