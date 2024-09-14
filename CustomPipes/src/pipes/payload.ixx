// ---------------------------------------------------
// Created by Boguslaw Cyganek (C) 2024
// ---------------------------------------------------


export module payload;




import <string>;
import <expected>;



// We have a data structure to process
export struct Payload
{
	std::string	fStr	{};
	int			fVal	{};
};

// Some error types just for the example
export enum class OpErrorType : unsigned char { kInvalidInput, kOverflow, kUnderflow };

// For the pipe-line operation - the expected type is Payload,
// while the 'unexpected' is OpErrorType
export using PayloadOrError = std::expected< Payload, OpErrorType >;



