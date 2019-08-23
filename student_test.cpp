#include "range.hpp"

#include <vector>
#include <list>
#include "catch.hpp"

int increment( int x ) { return x + 1; }

struct Increment {
    int operator()( int x ) const { return x + 1; }
};

bool even( int x ) {
    return x % 2 == 0;
}

struct Even {
    int operator()( int x ) const {
        return x % 2 == 0;
    }
};

int plus( int a, int b ) { return a + b; }

struct IntPair { int a, b; };
IntPair incrementPair( IntPair x ) { return { x.a + 1, x.b + 1 }; }

template < typename R1, typename R2 >
void checkRangeEqual( R1 expected, R2 value ) {
    auto it = value.begin();
    for ( const auto& x : expected ) {
        REQUIRE( x == *it );
        // Check iterator properties
        auto c = it;
        REQUIRE( c == it++ );
        ++c;
        REQUIRE( c == it );
    }
    REQUIRE( it == value.end() );
}

struct DummyRange {
    using value_type = int;
    using iterator = int *;
    using const_iterator = iterator;
    using difference_type = int;

    iterator begin() const { return nullptr; }
    iterator end() const { return nullptr; }
};

template < typename Range, typename Result >
void checkIteratorTraits( Range /*unused*/, Result /*unused*/ ) {
    using ItTrait = std::iterator_traits< typename Range::iterator >;
    CHECK( std::is_same_v< typename ItTrait::value_type, Result > );
    CHECK( std::is_same_v< typename ItTrait::pointer, const Result * > );
    CHECK( std::is_same_v< typename ItTrait::reference, const Result & > );
    CHECK( std::is_same_v< typename ItTrait::iterator_category, std::forward_iterator_tag > );

    // Iterator is default-constructible
    typename Range::iterator it;
}

template < typename Range, typename Result >
void checkRangeTypedefs( Range a, Result b ) {
    CHECK( std::is_same_v< typename Range::value_type, Result > );
    CHECK( std::is_same_v< typename Range::iterator, typename Range::const_iterator > );
    CHECK( std::is_integral_v< typename Range::difference_type > );
    checkIteratorTraits( a, b );
}

TEST_CASE( "Typedefs are valid" ) {
    std::vector< int > i;
    std::vector< std::string > s;
    SECTION( "map" ) {
        auto intRange = map( i, increment );
        checkRangeTypedefs( intRange, int{} );

        auto sRange = map( i, []( int x ){
            return std::to_string( x );
        } );
        checkRangeTypedefs( sRange, std::string{} );
    }

    SECTION( "filter" ) {
        auto intRange = filter( i, even );
        checkRangeTypedefs( intRange, int{} );

        auto sRange = filter( s, []( std::string ) {
            return true;
        } );
        checkRangeTypedefs( sRange, std::string{} );
    }

    SECTION( "take" ) {
        auto intRange = take( i, 42 );
        checkRangeTypedefs( intRange, int{} );

        auto sRange = take( s, 10 );
        checkRangeTypedefs( sRange, std::string{} );
    }

    SECTION( "zip" ) {
        auto range = zip( i, s );
        checkRangeTypedefs( range, std::pair< int, std::string>{} );
    }

    SECTION( "zipWith" ) {
        auto intRange = zipWith( i, i, []( int a, int b ) {
            return a + b;
        } );
        checkRangeTypedefs( intRange, int{} );

        auto tupleRange = zipWith( i, s,
            []( int, std::string) {
                return std::tuple<>();
            } );
        checkRangeTypedefs( tupleRange, std::tuple<>{} );
    }

    SECTION( "inifiniteSequence" ) {
        auto intRange = infiniteSequence( int{} );
        checkRangeTypedefs( intRange, int{} );

        auto charRange = infiniteSequence( char{} );
        checkRangeTypedefs( charRange, char{} );
    }

    SECTION( "simple range" ) {
        auto intRange = range( int{} );
        checkRangeTypedefs( intRange, int{} );

        auto charRange = range( char{} );
        checkRangeTypedefs( charRange, char{} );
    }

    SECTION( "advanced range" ) {
        auto intRange = range( int{}, int{} );
        checkRangeTypedefs( intRange, int{} );

        auto charRange = range( char{}, char{} );
        checkRangeTypedefs( charRange, char{} );
    }

    SECTION( "enumerate" ) {
        auto intRange = enumerate( i );
        checkRangeTypedefs( intRange, std::pair< size_t, int >{} );

        auto sRange = enumerate( s );
        checkRangeTypedefs( sRange, std::pair< size_t, std::string >{} );
    }
}

TEST_CASE( "Range can accept number of sources" ) {
    std::vector< int > v1, v2;
    std::list< int > l1, l2;
    DummyRange d1, d2;
    SECTION( "map" ) {
        map( v1, increment );
        map( l1, increment );
        map( d1, increment );
        map( v1, Increment() );
        map( v1, []( int x ){ return x + 1; } );
    }

    SECTION( "filter" ) {
        filter( v1, even );
        filter( l1, even );
        filter( d1, even );
        filter( v1, Even() );
        filter( v1, []( int x ){ return x % 2 == 0; } );
    }

    SECTION( "take" ) {
        take( v1, 10 );
        take( l1, 10 );
        take( d1, 10 );
    }

    SECTION( "zip" ) {
        zip( v1, v2 );
        zip( l1, l2 );
        zip( d1, d2 );
        zip( v1, l2 );
        zip( v1, d2 );
        zip( l1, d2 );
    }

    SECTION( "zipWith" ) {
        zipWith( v1, v2, plus );
        zipWith( l1, l2, plus );
        zipWith( d1, d2, plus );
        zipWith( v1, l2, plus );
        zipWith( v1, d2, plus );
        zipWith( l1, d2, plus );
        zipWith( v1, v2, std::plus< int >() );
        zipWith( v1, v2, []( int a , int b ) {
            return a + b;
        } );
    }
}

TEST_CASE( "map basic properties" ) {
    std::vector< int > empty;
    std::vector< int > ints = { 1, 2, 3, 4, 5 };
    std::vector< std::string > strings = {
        "I", "am", "the", "one", "who", "knocks!"
    };

    SECTION( "increment empty" ) {
        checkRangeEqual( empty, map( empty, increment ) );
        checkRangeEqual( empty, empty | map( increment ) );
    }

    SECTION( "increment ints" ) {
        std::vector< int > out;
        std::transform( ints.begin(), ints.end(), std::back_inserter( out ),
            increment );
        checkRangeEqual( out, map( ints, increment ) );
        checkRangeEqual( out, ints | map( increment ) );
    }

    SECTION( "make upper case" ) {
        auto f = []( std::string s ) {
            std::transform( s.begin(), s.end(), s.begin(),
                []( char c ) { return std::toupper( c ); } );
            return s;
        };

        std::vector< std::string > out;
        std::transform( strings.begin(), strings.end(), std::back_inserter( out ),
            f );
        checkRangeEqual( out, map( strings, f ) );
        checkRangeEqual( out, strings | map( f ) );
    }

    SECTION( "convert to string" ) {
        auto f = []( int x ) { return std::to_string( x ); };
        std::vector< std::string > out;
        std::transform( ints.begin(), ints.end(), std::back_inserter( out ), f );
        checkRangeEqual( out, map( ints, f ) );
        checkRangeEqual( out, ints | map( f ) );
    }
}

TEST_CASE( "filter basic properties" ) {
    std::vector< int > empty;
    std::vector< int > ints = { 1, 2, 3, 4, 5 };
    std::vector< std::string > strings = {
        "I", "am", "the", "one", "who", "knocks!"
    };

    SECTION( "filter empty" ) {
        checkRangeEqual( empty, filter( empty, even ) );
        checkRangeEqual( empty, empty | filter( even ) );
    }

    SECTION( "filter ints" ) {
        std::vector< int > out;
        std::copy_if( ints.begin(), ints.end(), std::back_inserter( out ), even );
        checkRangeEqual( out, filter( ints, even ) );
        checkRangeEqual( out, ints | filter( even ) );
    }

    SECTION( "filter strings" ) {
        auto f = []( const std::string& s ) {
            return s == "the";
        };

        std::vector< std::string > out;
        std::copy_if( strings.begin(), strings.end(), std::back_inserter( out ), f );
        checkRangeEqual( out, filter( strings, f ) );
        checkRangeEqual( out, strings | filter( f ) );
    }
}

TEST_CASE( "zipWith basic properties" ) {
    std::vector< int > empty;
    std::vector< int > ints1 = { 1, 2, 3, 4, 5, 6 };
    std::vector< int > ints2 = { 4, 5, 8, 2 };
    std::string str1 = "abcd";
    std::string str2 = "efghijk";

    SECTION( "zipWith ints and empty" ) {
        checkRangeEqual( empty, zipWith( empty, ints1, std::plus< int >() ) );
        checkRangeEqual( empty, zipWith( ints1, empty, std::plus< int >() ) );
    }

    SECTION( "zipWith ints with plus" ) {
        int counter = 0;
        for ( int x : zipWith( ints1, ints2, std::plus< int >() ) ) {
            REQUIRE( x == ints1[ counter ] + ints2[ counter ] );
            counter++;
        }
        REQUIRE( counter == 4 );

        counter = 0;
        for ( int x : zipWith( ints2, ints1, std::plus< int >() ) ) {
            REQUIRE( x == ints1[ counter ] + ints2[ counter ] );
            counter++;
        }
        REQUIRE( counter == 4 );
    }

    SECTION( "concat letters from strings" ) {
        std::vector< std::string > out = {
            "ae", "bf", "cg", "dh"
        };
        checkRangeEqual( out, zipWith( str1, str2, []( char a, char b ) {
            std::string s;
            s.push_back( a );
            s.push_back( b );
            return s;
        } ) );
    }
}

TEST_CASE( "zip basic properties" ) {
    std::vector< int > empty;
    std::vector< int > ints1 = { 1, 2, 3, 4, 5, 6 };
    std::vector< int > ints2 = { 4, 5, 8, 2 };
    std::vector< std::string > strings = {
        "I", "am", "the", "one", "who", "knocks!"
    };

    SECTION( "zip empty" ) {
        for ( auto x : zip( ints1, empty ) ) {
            x = x; // To trick Wall
            REQUIRE( false );
        }
    }

    SECTION( "zip ints" ) {
        int counter = 0;
        for ( auto x : zip( ints1, ints2 ) ) {
            REQUIRE( x == std::make_pair( ints1[ counter ], ints2[ counter ] ) );
            counter++;
        }
        REQUIRE( counter == 4 );
    }

    SECTION( "zip ints and strings" ) {
        int counter = 0;
        for ( const auto& x : zip( ints1, strings ) ) {
            REQUIRE( x == std::make_pair( ints1[ counter ], strings[ counter ] ) );
            counter++;
        }
        REQUIRE( counter == 6 );
    }
}

TEST_CASE( "range basic properties" ) {
    SECTION( "range( 5 )" ) {
        int counter = 0;
        for ( int x : range( 5 ) ) {
            REQUIRE( counter == x );
            counter++;
        }
        REQUIRE( counter == 5 );
    }

    SECTION( "range( 1, 10, 2 )" ) {
        int counter = 1;
        for ( int x : range( 1, 10, 2 ) ) {
            REQUIRE( x == counter );
            counter += 2;
        }
        REQUIRE( counter == 11 );
    }

    SECTION( "range( 2, -5, -2 )" ) {
        int counter = 2;
        for ( int x : range( 2, -5, -2 ) ) {
            REQUIRE( x == counter );
            counter -= 2;
        }
        REQUIRE( counter == -6 );
    }

    SECTION( "inifinite sequence, diff 1" ) {
        int counter = 7;
        for ( int x : infiniteSequence( 7 ) ) {
            REQUIRE( x == counter );
            counter++;
            if ( counter >= 20 )
                break;
        }
        REQUIRE( counter == 20 );
    }

    SECTION( "inifinite sequence, diff -1" ) {
        int counter = 7;
        for ( int x : infiniteSequence( 7, -1 ) ) {
            REQUIRE( x == counter );
            counter--;
            if ( counter <= -5 )
                break;
        }
        REQUIRE( counter == -5 );
    }

    SECTION( "inifinite sequence, diff 2" ) {
        int counter = 7;
        for ( int x : infiniteSequence( 7, 2 ) ) {
            REQUIRE( x == counter );
            counter += 2;
            if ( counter >= 21 )
                break;
        }
        REQUIRE( counter == 21 );
    }
}

TEST_CASE( "basic enumerate properties" ) {
    std::string text = "ABCD";
    SECTION( "enumerate string" ) {
        int counter = 0;
        for ( auto [i, c] : enumerate( text ) ) {
            REQUIRE( text[ counter ] == c );
            REQUIRE( counter == i );
            counter++;
        }
    }

    SECTION( "enumerate string with pipe" ) {
        int counter = 0;
        for ( auto [i, c] : text | enumerate() ) {
            REQUIRE( text[ counter ] == c );
            REQUIRE( counter == i );
            counter++;
        }
    }
}

TEST_CASE( "basic take properties" ) {
    std::string text = "ABCD";
    SECTION( "take 5" ) {
        int counter = 0;
        for ( char c : take( text, 5 ) ) {
            REQUIRE( c == text[ counter ] );
            counter++;
        }
        REQUIRE( counter == 4 );
    }

    SECTION( "take 2" ) {
        int counter = 0;
        for ( char c : take( text, 2 ) ) {
            REQUIRE( c == text[ counter ] );
            counter++;
        }
        REQUIRE( counter == 2 );
    }

    SECTION( "take 0" ) {
        int counter = 0;
        for ( char c : take( text, 0 ) ) { // NOLINT
            REQUIRE( false );
        }
        REQUIRE( counter == 0 );
    }

    SECTION( "take infinite sequence" ) {
        int counter = 0;
        for ( int x : take( infiniteSequence( 0 ), 5 ) ) {
            REQUIRE( x == counter );
            counter++;
        }
        REQUIRE( counter == 5 );
    }

    SECTION( "take infinite sequence with pipe" ) {
        int counter = 0;
        for ( int x : infiniteSequence( 0 ) | take( 5 ) ) {
            REQUIRE( x == counter );
            counter++;
        }
        REQUIRE( counter == 5 );
    }
}

TEST_CASE("infiniteSequence | filter") {
	auto range = infiniteSequence(0) | filter( even );
	int counter = 0;
	for ( int x : range ) {
		CHECK( counter == x );
		counter += 2;
		if ( counter == 10 ) {
			break;
		}
	}
}

TEST_CASE( "Ranges are chainable" ) {
    SECTION( "Trivial" ) {
        std::vector< int > s1 = { 1, 2, 3, 4, 5 };
        auto range = s1 | filter( even ) | map( []( int x ){ return x + 42; } );
        int counter = 44;
        for ( int x : range ) {
            CHECK( counter == x );
            counter += 2;
        }
        REQUIRE( counter == 48 );
    }

	SECTION("map | filter") {
		std::vector< int > s = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		auto range = s | filter( even ) | map( []( int x ) { return x + 1; } );
		int counter = 1;
		for ( int x : range ) {
			CHECK( counter == x );
			counter += 2;
		}

		REQUIRE( counter == 13 );
	}

	SECTION("map | filter | take") {
		std::vector< int > s = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		auto range = s | map( []( int x ){ return x + 10; } ) | filter( even ) | take(3);
		int counter = 10;
		for ( int x : range ) {
			CHECK( counter == x );
			counter += 2;
		}
		REQUIRE( counter == 16 );
	}

	SECTION("map | filter | take 2") {
		std::vector< int > s = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		auto range = s | map( []( int x ){ return x + 10; } ) | filter( []( int x ) { return x % 2 == 1; }) | take(3);
		int counter = 11;
		for ( int x : range ) {
			CHECK( counter == x );
			counter += 2;
		}
		REQUIRE( counter == 17 );
	}

	SECTION("infiniteSequence | map | filter | take") {
		auto range = infiniteSequence(0) | map([](int x) { return x + 10; }) | filter(even) | take(3);
		int counter = 10;
		for (int x : range) {
			CHECK(counter == x);
			counter += 2;
		}
		REQUIRE(counter == 16);
	}

	SECTION("range | map | filter | take") {
		auto r = range(0, 10, 1) | map([](int x) { return x + 10; }) | filter(even) | take(3);
		int counter = 10;
		for (int x : r) {
			CHECK(counter == x);
			counter += 2;
		}
		REQUIRE(counter == 16);
	}

	SECTION("range | map | filter | take with infiniteSequence zip") {
		auto r = range(0, 10, 1) | map([](int x) { return x + 10; }) | filter(even) | take(3);
		auto rWithZip = zip(infiniteSequence(0), r);
		int counterSecond = 10;
		int counterFirst = 0;
		for (auto x : rWithZip) {
			CHECK(counterFirst == x.first);
			CHECK(counterSecond == x.second);
			counterFirst += 1;
			counterSecond += 2;
		}
		REQUIRE(counterFirst == 3);
		REQUIRE(counterSecond == 16);
	}
}

TEST_CASE( "Ranges end" ) {
	SECTION( "ZipWith" ) {
		std::vector< int > s;
		std::vector< int > s1;

		auto range = zipWith(s, s1, []( int x, int y) { return x + y; });
		REQUIRE( range.begin() == range.end());

		s1.push_back(1);

		auto range1 = zipWith(s, s1, []( int x, int y) { return x + y; });
		REQUIRE( range1.begin() == range1.end());

		auto range2 = zipWith(s1, s, []( int x, int y) { return x + y; });
		REQUIRE( range2.begin() == range2.end());

		s.push_back(1);
		s1.push_back(2);

		auto range3 = zipWith(s, s1, plus);
		auto it3 = range3.begin();
		++it3;
		REQUIRE( it3 == range3.end() );

		auto range4 = zipWith(s1, s, plus);
		auto it4 = range4.begin();
		++it4;
		REQUIRE( it4 == range4.end() );
	}
}

TEST_CASE( "operator->") {
	std::vector< std::string > s {"A", "B", "C", "D", "E"};
	auto r = map(s, []( std::string x ){ return (x + 'X'); });
	auto it = r.begin();
	while (it != r.end()) {
		CHECK( it->find('X') == 1 );
		++it;
	}
}


TEST_CASE( "map to 1 | filter false | take 1 is empty" ) {
	std::vector< int > input1;
	auto f1 = []( int ) { return 1; };
	auto f2 = []( int ) { return false; };

	input1 = { 0 };

	auto range1 = input1 | map( f1 ) | filter( f2 ) | take( 1 );
	for ( auto x : range1 ) { // NOLINT
		CHECK(false);
	}

	CHECK(range1.begin() == range1.end());
}

TEST_CASE( "map | filter | take") {
	SECTION( "non-empty" ) {
		std::vector< int > input1;
		auto f1 = []( int x ) { return x % 2; };

		std::vector< int > expected = { 0 };

		for ( size_t i = 1; i < 11; ++i) {
			input1 = { 0 };
			auto range1 = input1 | map( f1 ) | filter( even ) | take( i );
			checkRangeEqual( expected, range1 );

			for ( int j =  0; j < 10; ++j) {
				input1.push_back(1);
				auto range1 = input1 | map( f1 ) | filter( even ) | take( i );
				checkRangeEqual( expected, range1 );
			}
		}

		input1 = { 0 };
		for ( size_t i = 2; i < 12; ++i) {
			input1 = expected;
			auto range1 = input1 | map( f1 ) | filter( even ) | take( i );
			checkRangeEqual( expected, range1 );

			for ( int j =  0; j < 10; ++j) {
				input1.push_back(1);
				auto range1 = input1 | map( f1 ) | filter( even ) | take( i );
				checkRangeEqual( expected, range1 );
			}

			input1.push_back(0);
			expected.push_back(0);
		}

		input1 = { 0 };
		expected = {};
		for ( size_t i = 0; i < 10; ++i) {
			auto range1 = input1 | map( f1 ) | filter( even ) | take( i );
			checkRangeEqual( expected, range1 );

			for ( int j =  0; j < 10; ++j) {
				input1.push_back(1);
				auto range1 = input1 | map( f1 ) | filter( even ) | take( i );
				checkRangeEqual( expected, range1 );
			}

			std::remove(input1.begin(), input1.end(), 1);
			input1.push_back(0);
			expected.push_back(0);

		}
	}

	SECTION( "empty" ) {
		std::vector< int > input1;
		auto f1 = []( int ) { return 1; };
		auto f2 = []( int ) { return false; };

		std::vector< int > empty = { };

		for ( size_t i = 1; i < 11; ++i) {
			input1 = { 0 };
			auto range1 = input1 | map( f1 ) | filter( f2 ) | take( i );
			checkRangeEqual( empty, range1 );

			for ( int j =  0; j < 10; ++j) {
				input1.push_back(0);
				auto range1 = input1 | map( f1 ) | filter( f2 ) | take( i );
				checkRangeEqual( empty, range1 );
			}
		}
	}
}
