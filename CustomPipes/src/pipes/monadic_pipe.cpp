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
// Version of the pipe-line with MONADIC std::expected




namespace Monadic_VectorsPipeTest
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



	using index_val = std::tuple< Matrix::size_type, Matrix::size_type, DType >;
	using max_exp = std::expected< index_val, DistErr >;

	using common_errors		= std::variant< PathErr, LoadErr, ENormErr, DistErr >;

	using path_com_exp		= std::expected< std::filesystem::path,	common_errors >;
	using load_com_exp		= std::expected< PathVec,						common_errors >;
	using vec_vec_com_exp	= std::expected< VecOfVec,						common_errors >;
	using dist_com_exp		= std::expected< Matrix,						common_errors >;
	using max_com_exp			= std::expected< index_val,					common_errors >;


	// traverse and collect all paths in this directory of files with the "accept_ext" extension
	load_exp load_paths( std::filesystem::path && pe, const std::filesystem::path & accept_ext = "txt"sv )
	{
		if( ! fs::exists( pe ) || ! fs::is_directory( pe ) )
			return std::unexpected( LoadErr::kWrongPath );		// exit if wrong path (not existing or not a dir)

		// Iterate through the directory
		load_exp retExp {};
		for( const auto & file_obj : fs::directory_iterator( pe ) )
				if( fs::is_regular_file( file_obj ) )	
					if( file_obj.path().extension().string().contains( accept_ext.string() ) )
						retExp->push_back( file_obj.path() );

		return retExp->size() > 0 ? retExp : std::unexpected { LoadErr::kNoData };
	}

	load_com_exp load_paths_common( std::filesystem::path && pe, const std::filesystem::path & accept_ext = "txt" )
	{

		if( ! fs::exists( pe ) || ! fs::is_directory( pe ) )
			return std::unexpected( LoadErr::kWrongPath );		// exit if wrong path (not existing or not a dir)

		// Iterate through the directory
		load_exp retExp {};
		for( const auto & file_obj : fs::directory_iterator( pe ) )
				if( fs::is_regular_file( file_obj ) )	
					if( file_obj.path().extension().string().contains( accept_ext.string() ) )
						retExp->push_back( file_obj.path() );

		return retExp->size() > 0 ? retExp : std::unexpected { LoadErr::kNoData };
	}





	// open all files and read the vectors 
	vec_vec_exp load_vectors( PathVec && le )
	{
		// open each file and read vectors
		VecOfVec	retVecs;
		for( const auto & f : le )
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

	vec_vec_com_exp load_vectors_common( PathVec && le )
	{
		// open each file and read vectors
		VecOfVec	retVecs;
		for( const auto & f :le )
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

	vec_vec_exp vec_normalize( VecOfVec && vve )	
	{

		for( auto && v : vve )
			if( auto env = normalize< DVec::value_type, std::vector >( v ); env.has_value() )
				v = env.value();
			else
				return std::unexpected( env.error() );		// stop immediately and pass the error out

		return vve;
	}

	vec_vec_com_exp vec_normalize_common( VecOfVec && vve )	
	{
		for( auto && v : vve )
			if( auto env = normalize< DVec::value_type, std::vector >( v ); env.has_value() )
				v = env.value();
			else
				return std::unexpected( env.error() );		// stop immediately and pass the error out

		return vve;
	}


	// Computes a cosine distance between vectors
	// We assume that the input vectors are already normalized
	dist_exp comp_distance( VecOfVec && vve )
	{
		const auto kColsRows { vve.size() };	// it's a square matrix
		if( kColsRows == 0 )
			return std::unexpected( DistErr::kZeroLen );

		Matrix distances( kColsRows, DVec( kColsRows, DVec::value_type {} ) );

		for( auto r { DVec::size_type {} }; r < kColsRows; ++ r )
		{
			const auto & v = ( vve )[ r ];
			for( auto c { r + 1 }; c < kColsRows; ++ c )
				distances[ r ][ c ] = std::inner_product( v.begin(), v.end(), ( vve )[ c ].begin(), std::remove_cvref_t< decltype(v) >::value_type {} );
		}

		return distances;
	}


	// Computes a cosine distance between vectors
	// We assume that the input vectors are already normalized
	dist_com_exp comp_distance_common( VecOfVec && vve )
	{
		const auto kColsRows { vve.size() };	// it's a square matrix
		if( kColsRows == 0 )
			return std::unexpected( DistErr::kZeroLen );

		Matrix distances( kColsRows, DVec( kColsRows, DVec::value_type {} ) );

		for( auto r { DVec::size_type {} }; r < kColsRows; ++ r )
		{
			const auto & v = ( vve )[ r ];
			for( auto c { r + 1 }; c < kColsRows; ++ c )
				distances[ r ][ c ] = std::inner_product( v.begin(), v.end(), ( vve )[ c ].begin(), std::remove_cvref_t< decltype(v) >::value_type {} );
		}

		return distances;
	}


	max_exp find_max( Matrix && de )
	{
		const auto kColsRows { de.size() };	// it's a square matrix
		assert( kColsRows > 0 );
		assert( (de)[ 0 ].size() > 0 );

		constexpr DType kNoneVal { std::numeric_limits< DType >::lowest() };
		index_val ret { 0u, 0u, kNoneVal };
		for( Matrix::size_type r {}; r < kColsRows; ++ r )
		{
			for( Matrix::size_type c { r + 1 }; c < kColsRows; ++ c )
			{
				auto val = (de)[ r ][ c ];
				assert( val >= -1.1 && val <= +1.1 );

				if( std::get< 2 >( ret ) < val )
					ret = { r, c, val };
			}
		}

		return std::get< 2 >( ret ) != kNoneVal ? max_exp { ret } : std::unexpected( DistErr::kWrongData );
	}

	max_com_exp find_max_common( Matrix && de )
	{
		const auto kColsRows { de.size() };	// it's a square matrix
		assert( kColsRows > 0 );
		assert( (de)[ 0 ].size() > 0 );

		constexpr DType kNoneVal { std::numeric_limits< DType >::lowest() };
		index_val ret { 0u, 0u, kNoneVal };
		for( Matrix::size_type r {}; r < kColsRows; ++ r )
		{
			for( Matrix::size_type c { r + 1 }; c < kColsRows; ++ c )
			{
				auto val = (de)[ r ][ c ];
				assert( val >= -1.1 && val <= +1.1 );

				if( std::get< 2 >( ret ) < val )
					ret = { r, c, val };
			}
		}

		return std::get< 2 >( ret ) != kNoneVal ? max_exp { ret } : std::unexpected( DistErr::kWrongData );
	}


	// https://en.cppreference.com/w/cpp/utility/variant/visit
	// helper type for the visitor
	template<class... Ts>
	struct overloaded : Ts... { using Ts::operator()...; };


	void GenPipeTest_Monadic()
	{
		const auto kFileExt { "txt"sv };

		auto result =	path_com_exp( ".\\..\\data"sv )	
							.and_then( [ ext = kFileExt ] ( auto && pe ) { return load_paths_common( std::move( pe ), ext ); } )
							.and_then( load_vectors_common )
							.and_then( vec_normalize_common )
							.and_then( comp_distance_common )		
							.and_then( find_max_common )

							.and_then( [] ( auto && r )
									{
										auto [ x, y, v ] = r;
										std::println( "success @ idx=({},{}; val={:.3f})\n", x, y, v );

										return max_com_exp {};		
									}
								)

							.or_else(	[] ( auto && err )
									{
										// err is a std::variant that holds different error types
										std::visit(	overloaded {
															[]( PathErr pe )	{ std::println( "PathErr #{}",	static_cast<int>(pe) ); },
															[]( LoadErr le )	{ std::println( "LoadErr #{}",	static_cast<int>(le) ); },
															[]( ENormErr ne ) { std::println( "ENormErr #{}",	static_cast<int>(ne) ); },
															[]( DistErr de )	{ std::println( "DistErr #{}",	static_cast<int>(de) ); }
														}, err );



										return max_com_exp( std::unexpected( DistErr::kWrongData ) );		// propagate the error further on
									}
								)

														;
		std::println( "typeid( result ).name() == {}", typeid( result ).name() );



		std::vector vals { 10, 11, 2, -3, 4, 55, 0, 0, -4, 10, -8 };

		for( auto p : vals	| std::views::filter( []( auto a ){ return a > 0; } ) 
									| std::views::transform( []( auto a ){ return 2 * a; } ) )
				std::cout << p << ", ";


		std::println( "\n\n" );
	}



}












