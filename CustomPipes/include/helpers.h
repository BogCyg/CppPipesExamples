// ---------------------------------------------------
// Created by Boguslaw Cyganek (C) 2023-2024
// 
// New software to the book:
// Introduction to Programming with C++ for Engineers
// 
// ---------------------------------------------------



#include <iostream>
#include <concepts>


// Adapted from the book: Stroustrup B.: A Tour of C++.
template< typename P >
concept printable = requires( P p ) 
{
   { std::cout << p } -> std::same_as< std::ostream & >;
};



template < printable ... P >
auto & print( P && ... params )
{
   return ( ( std::cout << std::forward< P >( params ) ), ... );
}

template < printable ... P >
auto & print_nl( P && ... params )
{
   return print( std::forward< P >( params ) ... ) << '\n';
}


						 


