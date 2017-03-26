/**
 * Compile-time sieve of Eratosthenes.
 *
 *
 * Copyright Dr Robert H Crowston, 2017, all rights reserved.
 * Use and redistribution is permitted under the BSD Licence available at https://opensource.org/licenses/bsd-license.php.
 *
 * Presently, this will only build with gcc-6.3.1 or later. To build, invoke g++ with the -std=c++17 flag.
 *
 * Bugs:
 *  o  This is not a true implementation of Eratosthenes’ method because every factor will be checked even if it was
 *     already encountered as a multiple of a lower factor.
 *  o  Array size is twice as big as necessary since evens are omitted.
 *  o  Won't build on clang.
 *
 */


#include <array>
#include <cstdint>
#include <type_traits>
#include <utility>    

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
	using table = std::array<bool, Size>;

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
		return { lhs[Is] | rhs[Is] ... };
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

	/*template <uint_t Size, uint_t Factor, 
		typename = std::enable_if_t</* Factor is on composite list *//* >
	>
	struct merged_factor_table
	{
		constexpr static auto get()
			-> table<Size>
		{
			// This factor already checked because it is a multiple of a previous factor; skip this factor and go to the next factor.
			return merged_factor_table<Size, Factor+2>::get();
		}
	};*/

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
	static_assert(!check<7 >(0));
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
	return rhc::primes::check<101>(num);
}

