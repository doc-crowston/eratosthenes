/**
 * Compile-time sieve of Eratosthenes.
 *
 *
 * Copyright Dr Robert H Crowston, 2017, all rights reserved.
 * Use and redistribution is permitted under the BSD Licence available at https://opensource.org/licenses/bsd-license.php.
 *
 * Presently, this will only build with gcc-7.0.0 (experimental version) or later. To build, invoke g++ with the -std=c++17 flag.
 * Or, see it in action here: https://godbolt.org/g/eMocgO
 *
 * Bugs and to do:
 *  o  This is not a true implementation of Eratosthenes’ method because every factor will be checked even if it was
 *     already encountered as a multiple of a lower factor.
 *  o  Array size is twice as big as necessary since evens are omitted.
 *
 */

#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <type_traits>
#include <utility> 

namespace rhc
{
	namespace detail
	{
		template <typename T,
			typename = std::enable_if_t<std::is_integral<T>::value>
		>
		constexpr T ceil(float number)
		{	// Note: std::ceil is not constexpr, so own implementation here.
			return (static_cast<float>(static_cast<T>(number)) == number) ?
				static_cast<T>(number) : static_cast<T>(number) + ((number > 0) ? 1 : 0);
		}
	}

	using std::size_t;

	template <size_t Size>
	class bit_array
	{
		constexpr static size_t bits = Size;
		constexpr static size_t bytes = detail::ceil<size_t>(static_cast<float>(bits) / CHAR_BIT);
		std::byte storage[bytes];	// Raw backing storage.
		
		static constexpr size_t index_to_byte (const size_t index)
		{	// Logical index to byte map.
			return index/CHAR_BIT;
		}
		static constexpr size_t index_to_offset (const size_t index)
		{	// Get the offset within one byte.
			return index - CHAR_BIT*(index/CHAR_BIT);
		}

		public:
		constexpr bit_array() : storage{} { ; }
		template <typename T,
			typename = std::enable_if_t<std::is_integral<T>::value>
		>
		constexpr bit_array (const T integer)
			: storage{*reinterpret_cast<const std::byte*>(&integer)}
		{ ; }
		constexpr bit_array (const std::initializer_list<bool>& list) : storage {}
		{
				assert(list.size() <= bits);
				size_t index = 0;
				for (const auto& bit : list)
				{
						storage[index_to_byte(index)] |= std::byte(bit << index_to_offset(index));
						++index;
				}
		}
		constexpr bool operator[](const std::size_t index) const
		{
			return static_cast<bool>(
				(storage[index_to_byte(index)] >> index_to_offset(index)) & std::byte(0x1)
			);
		}
	}; // End of class bit_array.
} // End of namespace rhc.

namespace rhc::primes
{
	using uint_t = std::uintmax_t;
	using index_t = std::size_t;
	using std::size_t;
	
	// I omit storing the primality of 0, 1, and the even numbers because each is trivially known.
	// Little helper to remap from number to array index.
	constexpr index_t to_index (const uint_t number)
	{
		return (number-3)/2;
	}
	// And the reverse.
	constexpr uint_t to_number (const index_t idx)
	{
		return idx*2 + 3;
	}

	template <uint_t Size>
	using table = rhc::bit_array<Size>;

	template <uint_t Size, uint_t Factor, index_t ... Is>
	constexpr auto get_factor_table(std::index_sequence<Is ...> ) 
		-> table<Size>
	{	// NB. this line does not compile on Clang 3.9.1.
		return { to_number(Is) % Factor == 0 && to_number(Is) > Factor ... };
	}

	template <uint_t Size, index_t ... Is>
	constexpr auto merge_factors(const table<Size> lhs, const table<Size> rhs, std::index_sequence<Is ...> )
		-> table<Size>
	{
		return { static_cast<bool>(lhs[Is] | rhs[Is]) ... };
	}

	template <uint_t Size, uint_t Factor
		//,typename = std::enable_if_t</*Factor is not on composite list*/>		 
	>
	struct merged_factor_table
	{
		constexpr static auto get()
			-> table<Size>
		{	
			using Indices = std::make_index_sequence<Size>;
			return merge_factors(
				get_factor_table<Size, Factor>(Indices()),
				merged_factor_table<Size, Factor+2>::get(),
				Indices()
			);
		}
	};

	template <uint_t Size>
	struct merged_factor_table<Size, Size>
	{
		constexpr static auto get()
			-> table<Size>
		{
			return get_factor_table<Size, Size>(std::make_index_sequence<Size>());
		}
	};

	template <size_t Size>
	constexpr bool check(const uint_t num)
	{
		//constexpr size_t HalfSize = Size/2;
		constexpr table<Size> composites = merged_factor_table<Size, 3>::get(); 
		if (num == 0 || num == 1) 	return false;
		if (num == 2)				return true;
		if (num % 2 == 0)			return false;
		return !composites[to_index(num)];
	}
	
	// Arbitrary check list.
	static_assert(!check<17>(0));
	static_assert(!check<7 >(1));
	static_assert( check<7 >(2));
	static_assert( check<7 >(3));
	static_assert(!check<7 >(4));
	static_assert( check<71>(5));
	static_assert(!check<71>(6));
	static_assert( check<71>(7));
	static_assert( check<71>(29));
	static_assert(!check<71>(33));

} // namespace rhc::primes.

bool is_prime(const rhc::primes::uint_t num)
{
	return rhc::primes::check<257>(num);
}
