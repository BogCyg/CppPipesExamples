// ==========================================================================
//
// Oprogramowanie napisane przez prof. Bogusława Cyganka do użytku z książką:
// ==> Programowanie w języku C++. Wprowadzenie dla inżynierów. PWN 2023 <==
//
// Oprogramowanie jest dostarczane w stanie takim, w jakim jest, i do celów edukacyjnych
// bez żadnych gwarancji ani odpowiedzialności za jego użycie w jakiejkolwiek aplikacji.
//
// ==========================================================================
//
// Software written by Boguslaw Cyganek (C) to be used with the book:
// INTRODUCTION TO PROGRAMMING WITH C++ FOR ENGINEERS
// Published by Wiley, 2020
//
// The software is supplied as is and for educational purposes
// without any guarantees nor responsibility of its use in any application. 
//
// ==========================================================================




#pragma once


namespace CppBook
{



#include <vector>
#include <cassert>
#include <stdexcept>			// std::out_of_range

#include <iterator>


// The range utility. Allows combination of the indexed
// and range based for loops.
// Template type deduction is used.
// Examples:
//
//	for( auto i : range( 123 ) )
//		cout << i << " ";
//	...
//	for( auto i : range( 0.0, 256.0, 16.5 ) ) 
//	{
//		for( auto j : range( -2, 16, 3 ) ) 
//			ostr << j << ", ";
//		ostr << endl << i << endl;
//	}
//
//
template < typename T = size_t >
class range 
{
		const T kFrom, kEnd, kStep;

     public:

		///////////////////////////////////////////////////////////
		// Konstruktor
		///////////////////////////////////////////////////////////
		//
		// WEJŚCIE:
		//		from - Liczba rozpoczynająca ciąg.
		//		end - eneruj liczby do, ale z pominięciem tej liczby.
		//		step - Różnica między każdą kolejną liczbą w ciągu (nie może być 0).
		//
		// UWAGI:
		//		Jeśli step = 0, zgłaszany jest wyjątek std::out_of_range
		//
		range( const T from, const T end, const T step = 1 ) 
			: kFrom( from ), kEnd( end ), kStep( step ) 
		{
			assert( kStep != 0 );
			if( kStep == 0 )
				throw std::out_of_range( "step param must not be 0" );
		}

		// Domyślnie from==0, step==1
		range( const T end ) 
			: kFrom( 0 ), kEnd( end ), kStep( 1 ) 
		{
			assert( kEnd > 0 );
		}

	private:

		class range_iter
		{
			T fVal {};
			const T kStep {};
		public:

			range_iter( const T v, const T step ) : fVal( v ), kStep( step ) {}
			operator T  () const						{ return fVal; }
			operator const T & ()					{ return fVal; }
			const T operator * () const			{ return fVal; }
			const range_iter & operator ++ ()	{ fVal += kStep; return * this; }


			// Common to all iterators - add these to allow std::copy etc.
			using difference_type		= T;
			using value_type				= T;
			using pointer					= T*;
			using reference				= T&;
			using iterator_category		= std::forward_iterator_tag;
			using iterator_concept		= std::forward_iterator_tag;


			bool operator == ( const range_iter & ri ) const
			{
				return ! operator != ( ri );
			}

			bool operator != ( const range_iter & ri ) const
			{	
				// To jest podchwytliwa część – podczas pracy iterator
				// sprawdza koniec wywołując !=, więc musi to być traf w celu zatrzymania;
				// Proste != nie zadziała, gdy podczas zwiększania kStart N razy o kSteps
				// przekroczy wartość kEnd. Stąd też poniższy warunek:
				return kStep > 0 ? fVal < ri.fVal : fVal > ri.fVal;	
			}      											
		};													

	public:

        const range_iter begin()	{ return range_iter( kFrom, kStep ); }
        const range_iter end()		{ return range_iter( kEnd,	kStep ); }

    public:

		// Konwersja do dowolnego vector< T >
		operator std::vector< T > ( void ) 
		{
			auto p = [this]( auto v ){ for( T i {kFrom}; i<kEnd; i+=kStep ) v.push_back(i); return v; };
			auto n = [this]( auto v ){ for( T i {kFrom}; i>kEnd; i+=kStep ) v.push_back(i); return v; };

			return kStep > 0 ? p( std::vector< T >() ) : n( std::vector< T >() );	// use RVO here
		}
};




// ---------------------------------------------------
// Let's introduce some template type deduction guides

range( const size_t ) -> range< size_t >;

range( const unsigned int ) -> range< size_t >;

range( const int ) -> range< int >;

range( char ) -> range< int >;

//range( const char ) -> range< int >;




}	// end of the CppBookCode namespace



