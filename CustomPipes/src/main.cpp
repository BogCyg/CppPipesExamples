// ---------------------------------------------------
// Created by Boguslaw Cyganek (C) 2024
// ---------------------------------------------------





#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#include "helpers.h"

//import state;




void Payload_PipeTest();
void Payload_PipeTest_Monadic();


void NB_ParallelPipelineTest();




auto main() -> int
{
	print_nl( "\nRun serial pipe - Payload_PipeTest ... " );
	Payload_PipeTest();
	
	print_nl( "\nRun serial pipe with MONADIC std::expected - Payload_PipeTest_Monadic ... " );
	Payload_PipeTest_Monadic();

	print_nl( "\nRun parallel pipe - NB_ParallelPipelineTest ... " );
	NB_ParallelPipelineTest();

	return 0;
}


