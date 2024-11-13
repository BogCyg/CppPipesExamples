// ---------------------------------------------------
// Created by Boguslaw Cyganek (C) 2024
// ---------------------------------------------------





#include <iostream>
#include <array>
#include <list>
#include <vector>
#include <string>

#include <expected>

#include <cassert>
#include <format>
#include <numeric>
#include <algorithm>
#include <filesystem>

#include <iterator>
#include <sstream>
#include <fstream>
#include <print>
#include <tuple>

#include <variant>
#include <ranges>


using namespace std::literals;

namespace fs = std::filesystem;



// ===================================================================
// Version of the pipe-line with std::expected



namespace VectorsPipeTest
{


	enum class ENormErr { kEmptyVec, kZeroSum, kWrongVals };

	template < typename T, template < typename > typename Cont = std::vector >
	using NormExpected = std::expected< Cont< T >, ENormErr >; 

	// Takes an FP vector and returns its normalized E2 version
	// Good only for floating-point types
	template <	std::floating_point T, 
					template < typename > typename Cont = std::vector, 
					auto kThresh = 1e-76 >
	auto normalize( Cont< T > v ) -> NormExpected< T, Cont >
	{
		if( not v.size() )
			return std::unexpected( ENormErr::kEmptyVec );

		auto denom = std::inner_product( v.begin(), v.end(), v.begin(), decltype( v )::value_type() );

		if( denom < kThresh )	// check the denominator
			return std::unexpected( ENormErr::kZeroSum );
		else if( std::isinf( denom ) || std::isnan( denom ) )
			return std::unexpected( ENormErr::kWrongVals );

		std::transform( v.begin(), v.end(), v.begin(), [ sq = std::sqrt( denom ) ] ( auto x ) { return x / sq; } );
		return v;
	}


	using DType = double;		
	using DVec = std::vector< DType >;		
	using VecOfVec = std::vector< DVec >;

	using Matrix = VecOfVec;

	using PathVec = std::vector< std::filesystem::path >;


	enum class LoadErr { kNoData, kWrongPath, kCannotOpen, kWrongData };
	using load_exp = std::expected< PathVec, LoadErr >;



	using vec_vec_exp = std::expected< VecOfVec, ENormErr >;

	enum class DistErr { kZeroLen, kWrongData };
	using dist_exp = std::expected< Matrix, DistErr >;


	enum class PathErr { kEmpty };		// no path provided
	using path_exp = std::expected< std::filesystem::path, PathErr >;

	// traverse and collect all paths in this directory of files with the "accept_ext" extension
	load_exp load_paths( path_exp && pe, const std::filesystem::path & accept_ext )
	{
		if( ! pe )	// if no objects to process, then exit passing an error
			return std::unexpected( LoadErr::kWrongData );

		if( ! fs::exists( * pe ) || ! fs::is_directory( * pe ) )
			return std::unexpected( LoadErr::kWrongPath );		// exit if wrong path (not existing or not a dir)

		// Iterate through the directory
		load_exp retExp {};
		for( const auto & file_obj : fs::directory_iterator( * pe ) )
				if( fs::is_regular_file( file_obj ) )	
					if( file_obj.path().extension().string().contains( accept_ext.string() ) )
						retExp->push_back( file_obj.path() );

		return retExp->size() > 0 ? retExp : std::unexpected { LoadErr::kNoData };
	}





	// open all files and read the vectors 
	vec_vec_exp load_vectors( load_exp && le )
	{
		if( ! le )	// if no objects to process, then exit passing an error
			return std::unexpected( ENormErr::kEmptyVec );

		// open each file and read vectors
		VecOfVec	retVecs;
		for( const auto & f : * le )
		{
			if( std::ifstream inFile( f ); inFile.is_open() )
			{
				for( std::string str; std::getline( inFile, str ) && str.length() > 0; )	// read the entire line into the string
				{
					std::istringstream istr( str );
					using DType_Iter = std::istream_iterator< DType >;

					retVecs.emplace_back(	std::make_move_iterator( DType_Iter{ istr } ), 
													std::make_move_iterator( DType_Iter{} ) );
				}
			}
		}

		return retVecs.size() > 0 ? vec_vec_exp { retVecs } : std::unexpected( ENormErr::kEmptyVec );
	}


	vec_vec_exp vec_normalize( vec_vec_exp && vve )	
	{
		if( ! vve )	// if no objects to process, then exit with an error
			return std::unexpected( ENormErr::kEmptyVec );

		for( auto && v : * vve )
			if( auto env = normalize< DVec::value_type, std::vector >( v ); env.has_value() )
				v = env.value();
			else
				return std::unexpected( env.error() );		// stop immediately and pass the error out

		return vve;
	}

					
	// Computes a cosine distance between vectors
	// We assume that the input vectors are already normalized
	dist_exp comp_distance( vec_vec_exp && vve )
	{
		if( ! vve )	// if no objects to process, then exit passing an error
			return std::unexpected( DistErr::kWrongData );

		const auto kColsRows { vve->size() };	// it's a square matrix
		if( kColsRows == 0 )
			return std::unexpected( DistErr::kZeroLen );

		Matrix distances( kColsRows, DVec( kColsRows, DVec::value_type {} ) );

		for( auto r { DVec::size_type {} }; r < kColsRows; ++ r )
		{
			const auto & v = ( * vve )[ r ];
			for( auto c { r + 1 }; c < kColsRows; ++ c )
			{	
				distances[ r ][ c ] = std::inner_product( v.begin(), v.end(), ( * vve )[ c ].begin(), std::remove_cvref_t< decltype(v) >::value_type {} );
				//distances[ c ][ r ] = distances[ r ][ c ];
			}

		}

		return distances;
	}


	using index_val = std::tuple< Matrix::size_type, Matrix::size_type, DType >;
	using max_exp = std::expected< index_val, DistErr >;

	max_exp find_max( dist_exp && de )
	{
		if( ! de )	// if no objects to process, then exit passing an error
			return std::unexpected( DistErr::kWrongData );

		const auto kColsRows { de->size() };	// it's a square matrix
		assert( kColsRows > 0 );
		assert( (*de)[ 0 ].size() > 0 );

		constexpr DType kNoneVal { std::numeric_limits< DType >::lowest() };
		index_val ret { 0u, 0u, kNoneVal };
		for( Matrix::size_type r {}; r < kColsRows; ++ r )
		{
			for( Matrix::size_type c { r + 1 }; c < kColsRows; ++ c )
			{
				auto val = (*de)[ r ][ c ];
				assert( val >= -1.1 && val <= +1.1 );

				if( std::get< 2 >( ret ) < val )
					ret = { r, c, val };
			}
		}

		return std::get< 2 >( ret ) != kNoneVal ? max_exp { ret } : std::unexpected( DistErr::kWrongData );
	}




	// ===================================================================
	// Version of the pipe-line with std::expected

	template < typename T >
	concept is_expected = requires( T t )
	{
		typename T::value_type;		// type requirement – nested member name exists
		typename T::error_type;		// type requirement – nested member name exists

		requires std::is_constructible_v< bool, T >;		// std::convertible_to< T, bool > will not work - ok, there is a conversion but this is a CONTEXTUAL CONVERSION!
		requires std::same_as< decltype( * t ), typename T::value_type & >;
		requires std::same_as< std::remove_cvref_t< decltype( * t ) >, typename T::value_type >;

		requires std::constructible_from< T, std::unexpected< typename T::error_type > >; 
	};

	// The second, more general version allows passing in and out std::expected with possibly different types
	// In this version all functions in the pipe are called and there is their responsibility to check if the passed object has and object or an error
	template < typename T, typename E, typename Function >
	requires std::invocable< Function, std::expected< T, E > >
				&& is_expected< typename std::invoke_result_t< Function, std::expected< T, E > > >
	constexpr auto operator | ( std::expected< T, E > && ex, Function && f ) -> typename std::invoke_result_t< Function, std::expected< T, E > >
	{
		return std::invoke( std::forward< Function >( f ), std::forward< std::expected< T, E > >( ex ) );
	}

	template < typename T, typename E, typename Function >
	requires std::invocable< Function, std::expected< T, E > >
				&& is_expected< typename std::invoke_result_t< Function, std::expected< T, E > > >
	constexpr auto operator | ( const std::expected< T, E > & ex, Function && f ) -> typename std::invoke_result_t< Function, std::expected< T, E > >
	{
		return std::invoke( std::forward< Function >( f ), ex );
	}
	// ===================================================================



	void GenPipeTest()
	{
		auto cur_dir = fs::current_path();

		// Here we create our CUSTOM PIPE
		auto result =	path_exp( ".\\..\\data"sv ) 
							| [] ( auto && pe ) { return load_paths( std::move( pe ), "txt"sv ); }
							| load_vectors 
							| vec_normalize 
							| comp_distance
							| find_max 
							| []	( auto && exp_2_check ) 
									{
										if( exp_2_check )
										{
											auto [ x, y, v ] = * exp_2_check;
											std::println( "success @ idx=({},{}; val={:.3f})\n", x, y, v );
										}
										else
											switch( exp_2_check.error() )
											{
												case DistErr::kWrongData:
													std::println( "DistErr::kWrongData\n" );
													break;
												case DistErr::kZeroLen:
													std::println( "DistErr::kZeroLen\n" );
													break;
												default :
													assert( false );
													break;																		
											}

										return exp_2_check;
									}
		;
		

		std::println( "typeid( result ).name() == {}", typeid( result ).name() );


		// No problem using ranges at the same scope
		std::vector vals { 10, 11, 2, -3, 4, 55, 0, 0, -4, 10, -8 };

		for( auto p : vals	| std::views::filter( []( auto a ){ return a > 0; } ) 
									| std::views::transform( []( auto a ){ return 2 * a; } ) )
				std::cout << p << ", ";


		std::println( "\n\n" );
	}


}



// =====================================================================================












