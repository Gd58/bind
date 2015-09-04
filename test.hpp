#ifndef TEST_HPP
#define TEST_HPP
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "bind.hpp"
BOOST_AUTO_TEST_CASE( bind_test )
{
    bind( [](int a,int b){ BOOST_CHECK_EQUAL( a, 1 ); BOOST_CHECK_EQUAL( b, 2 ); }, 1, place_holder< 0 >( ) )( 2 );
    auto f = bind( foldl( ), 100, place_holder< 0 >( ) );
    BOOST_CHECK_EQUAL( f( 2, 3, 450 ), 555 );
}
#endif // TEST_HPP
