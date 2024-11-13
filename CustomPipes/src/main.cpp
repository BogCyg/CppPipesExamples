// ---------------------------------------------------
// Created by Boguslaw Cyganek (C) 2024
// ---------------------------------------------------





#include <print>




namespace VectorsPipeTest
{
	void GenPipeTest();
}


namespace Monadic_VectorsPipeTest
{
	void GenPipeTest_Monadic();
}


void NB_ParallelPipelineTest();





auto main() -> int
{
	std::println( "\n=================\nRun VectorsPipeTest::GenPipeTest() ... " );
	VectorsPipeTest::GenPipeTest();

	std::println( "\n=================\nRun Monadic_VectorsPipeTest::GenPipeTest_Monadic() ... " );
	Monadic_VectorsPipeTest::GenPipeTest_Monadic();

	std::println( "\n=================\nRun parallel pipe - NB_ParallelPipelineTest ... " );
	NB_ParallelPipelineTest();

	return 0;
}


