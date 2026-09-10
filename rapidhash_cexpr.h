//
//	Copyright(c) 2026 Nathan Cornaud
// 
//	SPDX-License-Identifier: MIT
//

/*
 *	rapidhash v3 implementation compatible with C++20 compile-time execution.
 * 
 *	Based on Nicolas De Carli's implementation :
 *  https://github.com/Nicoshev/rapidhash
 */

#pragma once

#include <concepts>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <span>
#include <array>
#include <string_view>
#include <bit>


/*
 *  Unrolled macro.
 *  Improves large input speed, but increases code size and worsens small input speed.
 *
 *  RAPIDHASH_COMPACT: Normal behavior.
 *  RAPIDHASH_UNROLLED:
 * 
 */
#ifndef RAPIDHASH_UNROLLED
# define RAPIDHASH_COMPACT
#elif defined(RAPIDHASH_COMPACT)
# error "cannot define RAPIDHASH_COMPACT and RAPIDHASH_UNROLLED simultaneously."
#endif

/*
 *	Protection macro, alters behaviour of rapid_mum multiplication function.
 *
 *  RAPIDHASH_FAST: Normal behavior, max speed.
 *  RAPIDHASH_PROTECTED: Extra protection against entropy loss.
 */
#ifndef RAPIDHASH_PROTECTED
# define RAPIDHASH_FAST
#elif defined(RAPIDHASH_FAST)
# error "cannot define RAPIDHASH_PROTECTED and RAPIDHASH_FAST simultaneously."
#endif

/*
 *	This consteval implementation adds the RAPIDHASH_CEXPR_FORCE_PORTABLE_MUL macro.
 * 
 *	It is used to force the worst-case (slowest) path (compiling with MSVC or any other compiler
 *	without a custom 128 bits type) on any compiler (for testing and benchmarking).
 */

/*
 *	This consteval implementation adds the RAPIDHASH_CEXPR_ALLOW_CLASS_REPR macro.
 *
 *	It is used to allow the raw hashing of structures/classes representations through the API.
 *	It is unrecommended and deactivated by default because the hashing of these is not guaranteed
 *	to be stable accross builds, versions and execution environments.
 * 
 *	It may not cause any issues if the hashes do not cross the process barrier, but if you use them
 *	for a protocol or binary format, it is strongly advised to create your own semantically correct
 *	hasher using the allowed primitives (hashing per field using the result of the previous one as
 *	the seed for the next one).
 */

namespace rapidhash_cexpr::detail
{
	inline constexpr std::array<std::uint64_t, 8> rapidhash_secret =
	{
		0x2d358dccaa6c78a5,
		0x8bb84b93962eacc9,
		0x4b33a62ed433d4a3,
		0x4d5a2da51de1aa47,
		0xa0761d6478bd642f,
		0xe7037ed1a0b428db,
		0x90ed1765281c388c,
		0xaaaaaaaaaaaaaaaa
	};

	[[nodiscard]]
	consteval std::uint64_t rapid_mum(
		std::uint64_t a,
		std::uint64_t b,
		std::uint64_t& result_high
	) noexcept
	{
#	if defined(__SIZEOF_INT128__) && !defined(RAPIDHASH_CEXPR_FORCE_PORTABLE_MUL)

		unsigned __int128 r = static_cast<unsigned __int128>(a) * b;

#		ifdef RAPIDHASH_PROTECTED
			result_high = static_cast<std::uint64_t>(r >> 64) ^ b;
			return static_cast<std::uint64_t>(r) ^ a;
#		else
			result_high = static_cast<std::uint64_t>(r >> 64);
			return static_cast<std::uint64_t>(r);

#		endif

#	else

		const auto a0 = std::uint32_t(a);
		const auto a1 = std::uint32_t(a >> 32);
		const auto b0 = std::uint32_t(b);
		const auto b1 = std::uint32_t(b >> 32);

		const std::uint64_t p00 = std::uint64_t(a0) * b0;
		const std::uint64_t p01 = std::uint64_t(a0) * b1;
		const std::uint64_t p10 = std::uint64_t(a1) * b0;
		const std::uint64_t p11 = std::uint64_t(a1) * b1;

		const std::uint64_t middle =
			(p00 >> 32) +
			(p01 & 0xffffffffu) +
			(p10 & 0xffffffffu);

#		ifdef RAPIDHASH_PROTECTED
			result_high = (p11 + (p01 >> 32) + (p10 >> 32) + (middle >> 32)) ^ b;
			return ((middle << 32) | (p00 & 0xffffffffu)) ^ a;
#		else
			result_high = p11 + (p01 >> 32) + (p10 >> 32) + (middle >> 32);
			return (middle << 32) | (p00 & 0xffffffffu);
#		endif

#	endif
	}

	[[nodiscard]]
	consteval std::uint64_t rapid_mix(std::uint64_t a, std::uint64_t b) noexcept
	{
		std::uint64_t high;
		auto low = rapid_mum(a, b, high);
		return low ^ high;
	}

	template <typename T>
	concept ByteType =
		(std::is_integral_v<std::remove_cv_t<T>> &&
			sizeof(std::remove_cv_t<T>) == 1) ||
		std::same_as<std::remove_cv_t<T>, std::byte>;

	/// Forces reading in little-endian (as in the official implementation).
	template <typename R, ByteType T>
		requires std::same_as<R, std::uint64_t> ||
		std::same_as<R, std::uint32_t>
	[[nodiscard]]
	consteval R rapid_read(const T* p) noexcept
	{
		R value = 0;

		for (std::size_t i = 0; i != sizeof(R); ++i)
		{
			// So the branching gets resolved once and disappears from the hot loop.
			if constexpr (std::same_as<std::remove_cv_t<T>, std::byte>)
			{
				value |= static_cast<R>(std::to_integer<unsigned char>(p[i])) << (i * 8);
			}
			else
			{
				value |= static_cast<R>(static_cast<unsigned char>(p[i])) << (i * 8);
			}
		}

		return value;
	}

	/// Classic rapidhash v3.
	template <ByteType T, std::size_t N>
	[[nodiscard]]
	consteval std::uint64_t rapidhash_internal(std::span<const T, N> input, std::uint64_t seed = 0) noexcept
	{
		auto p = input.data();
		std::uint64_t len = input.size_bytes();
		auto i = len;
		std::uint64_t a = 0, b = 0;

		seed ^= rapid_mix(seed ^ rapidhash_secret[2], rapidhash_secret[1]);
		if (len <= 16)
		{
			if (len >= 4)
			{
				seed ^= len;
				if (len >= 8)
				{
					a = rapid_read<std::uint64_t>(p);
					b = rapid_read<std::uint64_t>(p + len - 8);
				}
				else
				{
					a = rapid_read<std::uint32_t>(p);
					b = rapid_read<std::uint32_t>(p + len - 4);
				}
			}
			else if (len > 0)
			{
				a = (static_cast<std::uint64_t>(static_cast<unsigned char>(p[0])) << 45) | static_cast<unsigned char>(p[len - 1]);
				b = static_cast<unsigned char>(p[len >> 1]);
			}
			else
				a = b = 0;
		}
		else
		{
			if (i > 112)
			{
				auto s1 = seed, s2 = seed, s3 = seed, s4 = seed, s5 = seed, s6 = seed;

#				ifdef RAPIDHASH_COMPACT
					do
					{
						seed = rapid_mix(rapid_read<std::uint64_t>(p) ^ rapidhash_secret[0],
							rapid_read<std::uint64_t>(p + 8) ^ seed);
						s1 = rapid_mix(rapid_read<std::uint64_t>(p + 16) ^ rapidhash_secret[1],
							rapid_read<std::uint64_t>(p + 24) ^ s1);
						s2 = rapid_mix(rapid_read<std::uint64_t>(p + 32) ^ rapidhash_secret[2],
							rapid_read<std::uint64_t>(p + 40) ^ s2);
						s3 = rapid_mix(rapid_read<std::uint64_t>(p + 48) ^ rapidhash_secret[3],
							rapid_read<std::uint64_t>(p + 56) ^ s3);
						s4 = rapid_mix(rapid_read<std::uint64_t>(p + 64) ^ rapidhash_secret[4],
							rapid_read<std::uint64_t>(p + 72) ^ s4);
						s5 = rapid_mix(rapid_read<std::uint64_t>(p + 80) ^ rapidhash_secret[5],
							rapid_read<std::uint64_t>(p + 88) ^ s5);
						s6 = rapid_mix(rapid_read<std::uint64_t>(p + 96) ^ rapidhash_secret[6],
							rapid_read<std::uint64_t>(p + 104) ^ s6);
						p += 112;
						i -= 112;
					} while (i > 112);

#				else
					while (i > 224)
					{
						seed = rapid_mix(rapid_read<std::uint64_t>(p) ^ rapidhash_secret[0], rapid_read<std::uint64_t>(p + 8) ^ seed);
						s1 = rapid_mix(rapid_read<std::uint64_t>(p + 16) ^ rapidhash_secret[1], rapid_read<std::uint64_t>(p + 24) ^ s1);
						s2 = rapid_mix(rapid_read<std::uint64_t>(p + 32) ^ rapidhash_secret[2], rapid_read<std::uint64_t>(p + 40) ^ s2);
						s3 = rapid_mix(rapid_read<std::uint64_t>(p + 48) ^ rapidhash_secret[3], rapid_read<std::uint64_t>(p + 56) ^ s3);
						s4 = rapid_mix(rapid_read<std::uint64_t>(p + 64) ^ rapidhash_secret[4], rapid_read<std::uint64_t>(p + 72) ^ s4);
						s5 = rapid_mix(rapid_read<std::uint64_t>(p + 80) ^ rapidhash_secret[5], rapid_read<std::uint64_t>(p + 88) ^ s5);
						s6 = rapid_mix(rapid_read<std::uint64_t>(p + 96) ^ rapidhash_secret[6], rapid_read<std::uint64_t>(p + 104) ^ s6);
						seed = rapid_mix(rapid_read<std::uint64_t>(p + 112) ^ rapidhash_secret[0], rapid_read<std::uint64_t>(p + 120) ^ seed);
						s1 = rapid_mix(rapid_read<std::uint64_t>(p + 128) ^ rapidhash_secret[1], rapid_read<std::uint64_t>(p + 136) ^ s1);
						s2 = rapid_mix(rapid_read<std::uint64_t>(p + 144) ^ rapidhash_secret[2], rapid_read<std::uint64_t>(p + 152) ^ s2);
						s3 = rapid_mix(rapid_read<std::uint64_t>(p + 160) ^ rapidhash_secret[3], rapid_read<std::uint64_t>(p + 168) ^ s3);
						s4 = rapid_mix(rapid_read<std::uint64_t>(p + 176) ^ rapidhash_secret[4], rapid_read<std::uint64_t>(p + 184) ^ s4);
						s5 = rapid_mix(rapid_read<std::uint64_t>(p + 192) ^ rapidhash_secret[5], rapid_read<std::uint64_t>(p + 200) ^ s5);
						s6 = rapid_mix(rapid_read<std::uint64_t>(p + 208) ^ rapidhash_secret[6], rapid_read<std::uint64_t>(p + 216) ^ s6);
						p += 224;
						i -= 224;
					}
					if (i > 112)
					{
						seed = rapid_mix(rapid_read<std::uint64_t>(p) ^ rapidhash_secret[0], rapid_read<std::uint64_t>(p + 8) ^ seed);
						s1 = rapid_mix(rapid_read<std::uint64_t>(p + 16) ^ rapidhash_secret[1], rapid_read<std::uint64_t>(p + 24) ^ s1);
						s2 = rapid_mix(rapid_read<std::uint64_t>(p + 32) ^ rapidhash_secret[2], rapid_read<std::uint64_t>(p + 40) ^ s2);
						s3 = rapid_mix(rapid_read<std::uint64_t>(p + 48) ^ rapidhash_secret[3], rapid_read<std::uint64_t>(p + 56) ^ s3);
						s4 = rapid_mix(rapid_read<std::uint64_t>(p + 64) ^ rapidhash_secret[4], rapid_read<std::uint64_t>(p + 72) ^ s4);
						s5 = rapid_mix(rapid_read<std::uint64_t>(p + 80) ^ rapidhash_secret[5], rapid_read<std::uint64_t>(p + 88) ^ s5);
						s6 = rapid_mix(rapid_read<std::uint64_t>(p + 96) ^ rapidhash_secret[6], rapid_read<std::uint64_t>(p + 104) ^ s6);
						p += 112;
						i -= 112;
					}

#				endif

				seed ^= s1;
				s2 ^= s3;
				s4 ^= s5;
				seed ^= s6;
				s2 ^= s4;
				seed ^= s2;
			}

			if (i > 16)
			{
				seed = rapid_mix(rapid_read<std::uint64_t>(p) ^ rapidhash_secret[2],
					rapid_read<std::uint64_t>(p + 8) ^ seed);
				if (i > 32)
				{
					seed = rapid_mix(rapid_read<std::uint64_t>(p + 16) ^ rapidhash_secret[2],
						rapid_read<std::uint64_t>(p + 24) ^ seed);
					if (i > 48)
					{
						seed = rapid_mix(rapid_read<std::uint64_t>(p + 32) ^ rapidhash_secret[1],
							rapid_read<std::uint64_t>(p + 40) ^ seed);
						if (i > 64)
						{
							seed = rapid_mix(rapid_read<std::uint64_t>(p + 48) ^ rapidhash_secret[1],
								rapid_read<std::uint64_t>(p + 56) ^ seed);
							if (i > 80)
							{
								seed = rapid_mix(rapid_read<std::uint64_t>(p + 64) ^ rapidhash_secret[2],
									rapid_read<std::uint64_t>(p + 72) ^ seed);
								if (i > 96)
								{
									seed = rapid_mix(rapid_read<std::uint64_t>(p + 80) ^ rapidhash_secret[1],
										rapid_read<std::uint64_t>(p + 88) ^ seed);
								}
							}
						}
					}
				}
			}

			a = rapid_read<std::uint64_t>(p + i - 16) ^ i;
			b = rapid_read<std::uint64_t>(p + i - 8);
		}

		a ^= rapidhash_secret[1];
		b ^= seed;
		std::uint64_t high;
		a = rapid_mum(a, b, high);
		return rapid_mix(a ^ rapidhash_secret[7],
			high ^ rapidhash_secret[1] ^ i);
	}

	/// Micro rapidhash v3.
	template <ByteType T, std::size_t N>
	[[nodiscard]]
	consteval std::uint64_t rapidhash_micro_internal(std::span<const T, N> input, std::uint64_t seed = 0) noexcept
	{
		auto p = input.data();
		std::uint64_t len = input.size_bytes();
		auto i = len;
		std::uint64_t a = 0, b = 0;

		seed ^= rapid_mix(seed ^ rapidhash_secret[2], rapidhash_secret[1]);
		if (len <= 16)
		{
			if (len >= 4)
			{
				seed ^= len;
				if (len >= 8)
				{
					a = rapid_read<std::uint64_t>(p);
					b = rapid_read<std::uint64_t>(p + len - 8);
				}
				else
				{
					a = rapid_read<std::uint32_t>(p);
					b = rapid_read<std::uint32_t>(p + len - 4);
				}
			}
			else if (len > 0)
			{
				a = (static_cast<std::uint64_t>(static_cast<unsigned char>(p[0])) << 45) | static_cast<unsigned char>(p[len - 1]);
				b = static_cast<unsigned char>(p[len >> 1]);
			}
			else
				a = b = 0;
		}
		else
		{
			if (i > 80)
			{
				auto s1 = seed, s2 = seed, s3 = seed, s4 = seed;
				do
				{
					seed = rapid_mix(rapid_read<std::uint64_t>(p) ^ rapidhash_secret[0],
						rapid_read<std::uint64_t>(p + 8) ^ seed);
					s1 = rapid_mix(rapid_read<std::uint64_t>(p + 16) ^ rapidhash_secret[1],
						rapid_read<std::uint64_t>(p + 24) ^ s1);
					s2 = rapid_mix(rapid_read<std::uint64_t>(p + 32) ^ rapidhash_secret[2],
						rapid_read<std::uint64_t>(p + 40) ^ s2);
					s3 = rapid_mix(rapid_read<std::uint64_t>(p + 48) ^ rapidhash_secret[3],
						rapid_read<std::uint64_t>(p + 56) ^ s3);
					s4 = rapid_mix(rapid_read<std::uint64_t>(p + 64) ^ rapidhash_secret[4],
						rapid_read<std::uint64_t>(p + 72) ^ s4);
					p += 80;
					i -= 80;
				} while (i > 80);
				seed ^= s1;
				s2 ^= s3;
				seed ^= s4;
				seed ^= s2;
			}
			if (i > 16)
			{
				seed = rapid_mix(rapid_read<std::uint64_t>(p) ^ rapidhash_secret[2], rapid_read<std::uint64_t>(p + 8) ^ seed);
				if (i > 32)
				{
					seed = rapid_mix(rapid_read<std::uint64_t>(p + 16) ^ rapidhash_secret[2], rapid_read<std::uint64_t>(p + 24) ^ seed);
					if (i > 48)
					{
						seed = rapid_mix(rapid_read<std::uint64_t>(p + 32) ^ rapidhash_secret[1], rapid_read<std::uint64_t>(p + 40) ^ seed);
						if (i > 64)
						{
							seed = rapid_mix(rapid_read<std::uint64_t>(p + 48) ^ rapidhash_secret[1], rapid_read<std::uint64_t>(p + 56) ^ seed);
						}
					}
				}
			}
			a = rapid_read<std::uint64_t>(p + i - 16) ^ i;  b = rapid_read<std::uint64_t>(p + i - 8);
		}

		a ^= rapidhash_secret[1];
		b ^= seed;
		std::uint64_t high;
		a = rapid_mum(a, b, high);
		return rapid_mix(a ^ rapidhash_secret[7], high ^ rapidhash_secret[1] ^ i);
	}

	/// Nano rapidhash v3.
	template <ByteType T, std::size_t N>
	[[nodiscard]]
	consteval std::uint64_t rapidhash_nano_internal(std::span<const T, N> input, std::uint64_t seed = 0) noexcept
	{
		auto p = input.data();
		std::uint64_t len = input.size_bytes();
		auto i = len;
		std::uint64_t a = 0, b = 0;

		seed ^= rapid_mix(seed ^ rapidhash_secret[2], rapidhash_secret[1]);
		if (len <= 16)
		{
			if (len >= 4)
			{
				seed ^= len;
				if (len >= 8)
				{
					a = rapid_read<std::uint64_t>(p);
					b = rapid_read<std::uint64_t>(p + len - 8);
				}
				else
				{
					a = rapid_read<std::uint32_t>(p);
					b = rapid_read<std::uint32_t>(p + len - 4);
				}
			}
			else if (len > 0)
			{
				a = (static_cast<std::uint64_t>(static_cast<unsigned char>(p[0])) << 45) | static_cast<unsigned char>(p[len - 1]);
				b = static_cast<unsigned char>(p[len >> 1]);
			}
			else
				a = b = 0;
		}
		else
		{
			if (i > 48)
			{
				auto s1 = seed, s2 = seed;
				do
				{
					seed = rapid_mix(rapid_read<std::uint64_t>(p) ^ rapidhash_secret[0],
						rapid_read<std::uint64_t>(p + 8) ^ seed);
					s1 = rapid_mix(rapid_read<std::uint64_t>(p + 16) ^ rapidhash_secret[1],
						rapid_read<std::uint64_t>(p + 24) ^ s1);
					s2 = rapid_mix(rapid_read<std::uint64_t>(p + 32) ^ rapidhash_secret[2],
						rapid_read<std::uint64_t>(p + 40) ^ s2);
					p += 48;
					i -= 48;
				} while (i > 48);
				seed ^= s1;
				seed ^= s2;
			}
			if (i > 16)
			{
				seed = rapid_mix(rapid_read<std::uint64_t>(p) ^ rapidhash_secret[2], rapid_read<std::uint64_t>(p + 8) ^ seed);
				if (i > 32)
				{
					seed = rapid_mix(rapid_read<std::uint64_t>(p + 16) ^ rapidhash_secret[2], rapid_read<std::uint64_t>(p + 24) ^ seed);
				}
			}
			a = rapid_read<std::uint64_t>(p + i - 16) ^ i;  b = rapid_read<std::uint64_t>(p + i - 8);
		}

		a ^= rapidhash_secret[1];
		b ^= seed;
		std::uint64_t high;
		a = rapid_mum(a, b, high);
		return rapid_mix(a ^ rapidhash_secret[7], high ^ rapidhash_secret[1] ^ i);
	}

} // namespace rapidhash_cexpr::detail


/*-----------------------------------------------------------------------------------------------*/


namespace rapidhash_cexpr
{
	/*
	 *	For raw buffers decomposed in bytes
	 *	(a span of : char, std::int8_t, std::uint8_t, ..., std::byte).
	 *
	 *	This avoids any copy but requires the input to be natively typed as an array of bytes.
	 *
	 *	You shouldn't use this interface for strings, as there is a more convenient one taking
	 *	a std::string_view that you can construct implicitly. This also avoid ambiguity with the
	 *	null terminator.
	 */

	template <detail::ByteType T, std::size_t N>
	[[nodiscard]]
	consteval std::uint64_t rapidhash_raw(std::span<const T, N> input, std::uint64_t seed = 0)
	{
		return detail::rapidhash_internal(input, seed);
	}

	template <detail::ByteType T, std::size_t N>
	[[nodiscard]]
	consteval std::uint64_t rapidhash_raw_micro(std::span<const T, N> input, std::uint64_t seed = 0)
	{
		return detail::rapidhash_micro_internal(input, seed);
	}

	template <detail::ByteType T, std::size_t N>
	[[nodiscard]]
	consteval std::uint64_t rapidhash_raw_nano(std::span<const T, N> input, std::uint64_t seed = 0)
	{
		return detail::rapidhash_nano_internal(input, seed);
	}

	/*
	 *	For strings
	 *	(anything implicitly constructible to a std::string_view :
	 *	literals, char*, char[], std::string, ..., std::string_view itself).
	 *
	 *	These functions do not perform any copy.
	 * 
	 *	However there could be some overhead on large data due to implicit construction from the
	 *	given type to std::string_view, depending on what it is.
	 * 
	 *	They do not include the null-terminator in the hash.
	 */

	[[nodiscard]]
	consteval std::uint64_t rapidhash_str(std::string_view stv, std::uint64_t seed = 0)
	{
		return detail::rapidhash_internal(
			std::span<const char>{ stv.data(), stv.size() },
			seed
		);
	}

	[[nodiscard]]
	consteval std::uint64_t rapidhash_str_micro(std::string_view stv, std::uint64_t seed = 0)
	{
		return detail::rapidhash_micro_internal(
			std::span<const char>{ stv.data(), stv.size() },
			seed
		);
	}

	[[nodiscard]]
	consteval std::uint64_t rapidhash_str_nano(std::string_view stv, std::uint64_t seed = 0)
	{
		return detail::rapidhash_nano_internal(
			std::span<const char>{ stv.data(), stv.size() },
			seed
		);
	}

	/*
	 *	For C++ objects
	 *	(int, float, ..., structs, classes)
	 * 
	 *	This API allows you to hash more complex typed objects, not only raw bytes, at compilation.
	 * 
	 * 	This API is only compatible with types that are :
	 *		* trivially copyable.
	 *		* not a union type.
	 *		* not a pointer type.
	 *		* not a pointer to member type.
	 *		* not a volatile-qualified type.
	 *		* not a structure/class (unless explicitly allowed).
	 * 
	 *	If you are aware of stability issues with hashing raw representations of structs/classes
	 *	(padding, pointer members, compiler-specific layout...)
	 *	you can allow their hashing through this API by defining :
	 *	RAPIDHASH_CEXPR_ALLOW_CLASS_REPR
	 *
	 *	These functions create a single copy of the object about to be hashed. For arrays, a single
	 *	copy of its entirety is created.
	 *	It may create a slight overhead on large objects.
	 */

	template <typename T>
	concept HashableObject =
#		ifndef RAPIDHASH_CEXPR_ALLOW_CLASS_REPR
			!std::is_class_v<T> &&
#		endif

		/// These are the restrictions for std::bit_cast to be constexpr
		///	(see https://en.cppreference.com/cpp/numeric/bit_cast).
		std::is_object_v<T> &&
		!std::is_union_v<T> &&
		!std::is_pointer_v<T> &&
		!std::is_member_pointer_v<T> &&
		!std::is_volatile_v<T> &&
		std::is_trivially_copyable_v<T>;

	template <HashableObject T>
	[[nodiscard]]
	consteval std::uint64_t rapidhash(const T& input, std::uint64_t seed = 0)
	{
		auto bytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(input);
		return detail::rapidhash_internal(std::span<const std::byte>{ bytes }, seed);
	}

	template <HashableObject T>
	[[nodiscard]]
	consteval std::uint64_t rapidhash_micro(const T& input, std::uint64_t seed = 0)
	{
		auto bytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(input);
		return detail::rapidhash_micro_internal(std::span<const std::byte>{ bytes }, seed);
	}

	template <HashableObject T>
	[[nodiscard]]
	consteval std::uint64_t rapidhash_nano(const T& input, std::uint64_t seed = 0)
	{
		auto bytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(input);
		return detail::rapidhash_nano_internal(std::span<const std::byte>{ bytes }, seed);
	}

	/*
	 *	Array of objects
	 */

	template <HashableObject T, std::size_t N>
	[[nodiscard]]
	consteval std::uint64_t rapidhash(const T (&input)[N], std::uint64_t seed = 0)
	{
		auto bytes = std::bit_cast<std::array<std::byte, (sizeof(T) * N)>>(input);
		return detail::rapidhash_internal(std::span<const std::byte>{ bytes }, seed);
	}

	template <HashableObject T, std::size_t N>
	[[nodiscard]]
	consteval std::uint64_t rapidhash_micro(const T(&input)[N], std::uint64_t seed = 0)
	{
		auto bytes = std::bit_cast<std::array<std::byte, (sizeof(T) * N)>>(input);
		return detail::rapidhash_micro_internal(std::span<const std::byte>{ bytes }, seed);
	}

	template <HashableObject T, std::size_t N>
	[[nodiscard]]
	consteval std::uint64_t rapidhash_nano(const T(&input)[N], std::uint64_t seed = 0)
	{
		auto bytes = std::bit_cast<std::array<std::byte, (sizeof(T) * N)>>(input);
		return detail::rapidhash_nano_internal(std::span<const std::byte>{ bytes }, seed);
	}

} // namespace rapidhash_cexpr