// ---------------------------------------------------
// Created by Boguslaw Cyganek (C) 2024
// ---------------------------------------------------



#include <iostream>
#include <ranges>
#include <string>
#include <vector>
#include <sstream>
#include <format>
#include <algorithm>
#include <array>
#include <numeric>
#include <cassert>
#include <any>
#include <optional>
#include <functional>
#include <ctime>

#include <expected>
#include <iomanip>			// for std::quoted

#include "helpers.h"
#include "range.h"

#include <random>
#include <print>


import payload;



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


template < typename T, typename E, typename Function >
requires std::invocable< Function, T >
			&& is_expected< typename std::invoke_result_t< Function, T > >
constexpr auto operator | ( std::expected< T, E > && ex, Function && f ) -> typename std::invoke_result_t< Function, T >
{
	return ex ? std::invoke( std::forward< Function >( f ), * std::forward< std::expected< T, E > >( ex ) ) : ex;
}

//
//// We have a data structure to process
//struct Payload
//{
//	std::string	fStr	{};
//	int			fVal	{};
//};
//
//// Some error types just for the example
//enum class OpErrorType : unsigned char { kInvalidInput, kOverflow, kUnderflow };
//
//// For the pipe-line operation - the expected type is Payload,
//// while the 'unexpected' is OpErrorType
//using PayloadOrError = std::expected< Payload, OpErrorType >;


PayloadOrError Payload_Proc_1( PayloadOrError && s )
{
	if( ! s )
		return s;
	++ s->fVal;
	s->fStr += " proc by 1,";
	std::cout << "I'm in Payload_Proc_1, s = " << s->fStr << "\n";
	return s;
}

PayloadOrError Payload_Proc_2( PayloadOrError && s )
{
	if( ! s )
		return s;
	++ s->fVal;
	s->fStr += " proc by 2,";
	std::cout << "I'm in Payload_Proc_2, s = " << s->fStr << "\n";

	// Emulate the error, at least once in a while ...
	std::mt19937 rand_gen( std::random_device {} () );
	return ( rand_gen() % 2 ) ? s : 
					std::unexpected { rand_gen() % 2 ? OpErrorType::kOverflow : OpErrorType::kUnderflow };
}

PayloadOrError Payload_Proc_3( PayloadOrError && s )
{
	if( ! s )
		return s;
	++ s->fVal;
	s->fStr += " proc by 3,";
	std::cout << "I'm in Payload_Proc_3, s = " << s->fStr << "\n";
	return s;
}


void Payload_PipeTest()
{
	static_assert( is_expected< PayloadOrError > );	// a short-cut to verify the concept

	// ----------------------------------------------------------------------------------------------------------------
	// Here we create our CUSTOM PIPE
	auto res = 	PayloadOrError { Payload { "Start string ", 42 } } | Payload_Proc_1 | Payload_Proc_2 | Payload_Proc_3 ;
	// ----------------------------------------------------------------------------------------------------------------
	 
	if( res )
		print_nl( "Success! Result of the pipe: fStr == ", res->fStr, " fVal == ", res->fVal );
	else
		switch( res.error() )
		{
			case OpErrorType::kInvalidInput: print_nl( "Error: OpErrorType::kInvalidInput" ); break;
			case OpErrorType::kOverflow:		print_nl( "Error: OpErrorType::kOverflow" );		break;
			case OpErrorType::kUnderflow:		print_nl( "Error: OpErrorType::kUnderflow" );		break;
			default: print_nl( "That's really an unexpected error ..." );							break;
		}
}


// This is a version of Payload_PipeTest but this time with the monadic interface of std::expected
void Payload_PipeTest_Monadic()
{

	auto res = 	PayloadOrError { Payload { "Start string ", 42 } }

		.and_then( Payload_Proc_1 )
		.and_then( Payload_Proc_2 )
		.and_then( Payload_Proc_3 )
		
		.or_else(	[]( const auto & err )
						{
							switch( err )
							{
								case OpErrorType::kInvalidInput: print_nl( "Error: OpErrorType::kInvalidInput" );	break;
								case OpErrorType::kOverflow:		print_nl( "Error: OpErrorType::kOverflow" );			break;
								case OpErrorType::kUnderflow:		print_nl( "Error: OpErrorType::kUnderflow" );		break;
								default: print_nl( "That's really an unexpected error ..." );								break;
							}
							return PayloadOrError( std::unexpected( err ) );		// propagate the error further on
							//return PayloadOrError( Payload() );		// but we can also fix the situation and return a value
						}
		);


		print_nl( "typeid( res ).name() == ", typeid( res ).name() );

		if( res )
			print_nl(  "Success! Result of the pipe: fStr == ", res->fStr, " fVal == ", res->fVal );
}













