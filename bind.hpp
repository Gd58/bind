#ifndef BIND_HPP
#define BIND_HPP
#include <cassert>
#include <algorithm>
#include <type_traits>
#include <tuple>
#include <iostream>
namespace bind
{
	template< int placeholder >
	struct place_holder{ };
	template< typename K, typename ... RES >
	struct binded_function
	{
		template< typename ... >
		struct GET_LEAST_NUM_ARG
		{ static constexpr size_t value = 0; };
		template< typename T, typename ... ARG >
		struct GET_LEAST_NUM_ARG< T, ARG ... >
		{ static constexpr size_t value = GET_LEAST_NUM_ARG< ARG ... >::value; };
		template< size_t T, typename ... ARG >
		struct GET_LEAST_NUM_ARG< place_holder< T >, ARG ... >
		{ static constexpr size_t value = T > GET_LEAST_NUM_ARG< ARG ... >::value ? T : GET_LEAST_NUM_ARG< ARG ... >::value; };
		static constexpr size_t LEAST_NUM_ARG = GET_LEAST_NUM_ARG< RES ... >::value;
		K k;
		struct tag{ };
		template< typename NNEXT, size_t pos >
		struct parameter_delegate
		{
			typedef NNEXT NEXT;
			template< typename ... BEFORE >
			struct wrapper
			{
				template< typename ... ORIGINAL >
				struct wrapper2
				{
					template< typename SFINAE, typename ... REMAINING >
					static auto function(
							const binded_function * that,
							const parameter_delegate * pd,
							const BEFORE & ... before,
							const typename std::enable_if< ! std::is_same< void, typename SFINAE::NEXT >::value, tag >::type & t,
							const ORIGINAL & ... original,
							const tag &,
							const REMAINING & ... rem )
					{
						return std::decay< decltype( * pd->next ) >::type::
								template wrapper< BEFORE ..., std::tuple_element< pos, std::tuple< ORIGINAL ... > >::type >::
								template wrapper2< ORIGINAL ... >::template function< typename SFINAE::NEXT >(
									that, pd->next, before ..., std::get< pos >( std::tie( original ... ) ), t, original ..., t, rem ... ); }
					template< typename SFINAE, typename ... REMAINING >
					static auto function(
							const binded_function * that,
							const parameter_delegate *,
							const BEFORE & ... before,
							const typename std::enable_if< std::is_same< void, typename SFINAE::NEXT >::value, tag >::type &,
							const ORIGINAL & ... original,
							const tag &,
							const REMAINING & ... rem )
					{ return that->k( before ..., std::get< pos >( std::tie( original ... ) ), rem ... ); }
				};
			};
			NEXT * next;
			~parameter_delegate( ) { clean_if_necessary< NEXT >( ); }
			template< typename SFINAE >
			typename std::enable_if< ! std::is_same< void, SFINAE >::value >::type clean_if_necessary( ) { delete next; }
			template< typename SFINAE >
			typename std::enable_if< std::is_same< void, SFINAE >::value >::type clean_if_necessary( ) { }
			template< typename SFINAE, typename ... ARG  >
			typename std::enable_if< ! std::is_same< void, SFINAE >::value >::type new_if_necessary( const ARG & ... arg ) { next = new NEXT( arg ... ); }
			template< typename SFINAE, typename ... ARG >
			typename std::enable_if< std::is_same< void, SFINAE >::value >::type new_if_necessary( const ARG & ... ) { }
			template< typename ... ARG >
			parameter_delegate( const ARG & ... arg ) { new_if_necessary< NEXT >( arg ... ); }
		};
		template< typename NNEXT, typename T >
		struct constant_delegate
		{
			typedef NNEXT NEXT;
			T value;
			NEXT * next;
			template< typename ... BEFORE >
			struct wrapper
			{
				template< typename ... ORIGINAL >
				struct wrapper2
				{
					template< typename SFINAE, typename ... REMAINING >
					static auto function(
							const binded_function * that,
							const constant_delegate * cd,
							const BEFORE & ... before,
							const typename std::enable_if< ! std::is_same< void, typename SFINAE::NEXT >::value, tag >::type & t,
							const ORIGINAL & ... original,
							const tag &,
							const REMAINING & ... rem )
					{ return std::decay< decltype( * cd->next ) >::type::
								template wrapper< BEFORE ..., T >::template wrapper2< ORIGINAL ... >::template function< typename SFINAE::NEXT >(
									that, cd->next, before ..., cd->value, t, original ..., t, rem ... ); }
					template< typename SFINAE, typename ... REMAINING >
					static auto function(
							const binded_function * that,
							const constant_delegate * cd,
							const BEFORE & ... before,
							const typename std::enable_if< std::is_same< void, typename SFINAE::NEXT >::value, tag >::type &,
							const ORIGINAL & ...,
							const tag &,
							const REMAINING & ... rem )
					{ return that->k( before ..., cd->value, rem ... ); }
				};
			};
			~constant_delegate( ) { clean_if_necessary< NEXT >( ); }
			template< typename SFINAE >
			typename std::enable_if< ! std::is_same< void, SFINAE >::value >::type clean_if_necessary( ) { delete next; }
			template< typename SFINAE >
			typename std::enable_if< std::is_same< void, SFINAE >::value >::type clean_if_necessary( ) { }
			template< typename SFINAE, typename ... ARG  >
			typename std::enable_if< ! std::is_same< void, SFINAE >::value >::type new_if_necessary( const ARG & ... arg ) { next = new NEXT( arg ... ); }
			template< typename SFINAE, typename ... ARG >
			typename std::enable_if< std::is_same< void, SFINAE >::value >::type new_if_necessary( const ARG & ... ) { }
			template< typename F, typename ... ARG >
			constant_delegate( const F & f, const ARG & ... arg ) : value( f ) { new_if_necessary< NEXT, ARG ... >( arg ... ); }
			template< size_t F, typename ... ARG >
			constant_delegate( const place_holder< F > &, const ARG & ... arg ) : constant_delegate( arg ... ) { }
		};
		template< typename ... >
		struct GET_DELEGATE
		{ typedef void type; };
		template< typename T, typename ... ARG >
		struct GET_DELEGATE< T, ARG ... >
		{ typedef constant_delegate< typename GET_DELEGATE< ARG ... >::type, T > type; };
		template< size_t T, typename ... ARG >
		struct GET_DELEGATE< place_holder< T >, ARG ... >
		{ typedef parameter_delegate< typename GET_DELEGATE< ARG ... >::type, T > type; };
		typedef typename GET_DELEGATE< RES ... >::type DELEGATE;
		DELEGATE d;
		template< typename ... ARG >
		struct operator_delegate
		{
			const binded_function * that;
			std::tuple< const ARG & ... > data;
			template< size_t NUM_ARG_EATENED, typename FIRST_REMAINING, typename ... REST_REMAINING >
			auto function(
					const ARG & ... bef,
					const typename std::enable_if< NUM_ARG_EATENED != LEAST_NUM_ARG, FIRST_REMAINING >::type &,
					const REST_REMAINING & ... rem ) const
			{ return function< NUM_ARG_EATENED + 1, REST_REMAINING ... >( that, bef ..., rem ... ); }
			template< size_t NUM_ARG_EATENED, typename FIRST_REMAINING, typename ... REST_REMAINING >
			auto function(
					const ARG & ... bef,
					const typename std::enable_if< NUM_ARG_EATENED == LEAST_NUM_ARG, FIRST_REMAINING >::type &,
					const REST_REMAINING & ... rem ) const
			{ return second_function< ARG ... >::function( that, this, bef ..., rem ... ); }
			template< typename ... BEFORE >
			struct second_function
			{
				template< typename ... AFTER >
				static auto function(
						const binded_function * that,
						const operator_delegate *,
						const BEFORE & ... bef,
						const AFTER & ... aft )
				{ return DELEGATE::template wrapper< >::template wrapper2< BEFORE ... >::template function< DELEGATE >(
								that, & that->d, tag( ), bef ..., tag( ), aft ... ); }
			};
			operator_delegate( const binded_function * that, const ARG & ... arg ) : that( that ), data( std::tie( arg ... ) ) { }
			auto operator ( )( const ARG & ... arg ) const { return function< 0, ARG ... >( arg ..., arg ... ); }
		};
		template< typename ... ARG >
		auto operator ( )( const ARG & ... arg ) const { return operator_delegate< ARG ... >( this, arg ... )( arg ... ); }
		template< typename ... ARG >
		binded_function( const K & k, const ARG & ... arg ) : k( k ), d( arg ... ) { }
	};
	template< typename T, typename ... ARG >
	auto bind( const T & t, const ARG & ... arg ) { return binded_function< T, ARG ... >( t, arg ... ); }
	struct output
	{
		auto operator ( )( ) const { }
		template< typename T, typename ... ARG >
		auto operator ( )( const T & t, const ARG & ... arg ) const
		{
			std::cout << t << std::endl;
			( *this )( arg ... );
		}
	};
	void bound_example( )
	{
		bind( [](int a,int b){ assert( a == 1 && b == 2 ); }, 1, place_holder< 0 >( ) )( 2 );
		auto f = bind( output( ), 100, place_holder< 0 >( ) );
		f( 2, 3, 450 );
	}
}
#endif // BIND_HPP
