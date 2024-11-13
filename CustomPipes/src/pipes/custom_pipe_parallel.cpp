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

#include <numeric>
#include <cassert>

#include <functional>
#include <ctime>

#include <expected>
#include <iomanip>			// for std::quoted

#include <random>
#include <print>

#include <queue> 
#include <mutex> 
#include <condition_variable>


import payload;

// ===================================================================

 





// -----------------------------------------------------------


template < typename Elem >
class TSynchroQueue
{

public:

	using value_type = Elem;

	using ExpectedElem = std::expected< Elem, bool >;		// TO DO: change bool to enum and add some STOP conditions for the pipeline to stop processing

public:

	void push( Elem && in_elem )
	{
		{
			std::unique_lock	theLock( fMutex );
			fQueue.emplace( in_elem );
		}

		fCondVar.notify_one();	// we call notify_one when the mutex is already released - otherwise the notified thread
	}									// can be woken up only to try to lock still locked mutex

	ExpectedElem pop( void )
	{
		std::unique_lock	theLock( fMutex );

		//if( fQueue.empty() )
		//	return ExpectedElem( std::unexpected( false ) );

		// Block waiting until the queue is not empty
		// std::condition_variable operates only with std::unique_lock<std::mutex>
		// This is the key point - if I'm here then I possessed the mutex. However, if I stay here we ALL WOULD BE BLOCKED,
		// since no other thread can push anything to this queue. The solution is to give up my thread and realease the mutex
		// until the condition is true - in this case, this is that the queue is not longer empty.
		fCondVar.wait( theLock, [ this ]() { return not fQueue.empty(); } );	

		// OK, we have something to pop and to return
		auto out_elem = fQueue.front();
		fQueue.pop();
		return ExpectedElem( out_elem );
	}

	// This is not thread safe!
	auto size() const { return fQueue.size(); }

public:

	// This blocks also creation of the copy and move constructors, as well as the copy assignment.
	// However, the default constructor and destructor are created by the compiler.
	// See the book by N. Jousuttis.
	TSynchroQueue & operator = ( TSynchroQueue && ) = delete;

private:

	std::queue< Elem >			fQueue;

	std::mutex						fMutex;

	std::condition_variable		fCondVar;



};





using PaylodOrErrorProcFun = std::function< void ( PayloadOrError & in_elem, PayloadOrError & out_elem ) >;





using NB_PayloadOrError_Queue = TSynchroQueue< PayloadOrError >;

using NB_PayloadOrError_Queue_SS = std::shared_ptr< NB_PayloadOrError_Queue >;




template< typename Elem >
using NB_ParallelPipeFun = std::function< void ( TSynchroQueue< Elem > & in_q, TSynchroQueue< Elem > & out_q ) >;

using NB_ParallelPipeFun_4_PayloadOrError = NB_ParallelPipeFun< PayloadOrError >;


using namespace std::string_view_literals;
constexpr	auto		kStopToken			{ "STOP!"sv };


void		NB_ParPipe_Fun_Loop( NB_PayloadOrError_Queue_SS in_q, NB_PayloadOrError_Queue_SS out_q, PaylodOrErrorProcFun && theCartridgeFun )
{
	auto th_id = std::this_thread::get_id();

	for( ;; )
	{

		auto pop_elem_or_none = in_q->pop();
		//if( not pop_elem_or_none )
		//{
		//	using namespace std::chrono_literals;
		//	std::this_thread::sleep_for( 200ms );
		//	continue;
		//}

		assert( pop_elem_or_none );

		if( pop_elem_or_none->value().fStr == kStopToken )
		{
			out_q->push( std::move( pop_elem_or_none->value() ) );		// pass the STOP token
			break;																		// and exit the thread
		}
		else
		{
			PayloadOrError outElem;
			theCartridgeFun( * pop_elem_or_none, outElem );		// do some action with the pop'ed element
			out_q->push( std::move( outElem ) );
		}

	}

}



auto operator | ( NB_PayloadOrError_Queue_SS in_queue, PaylodOrErrorProcFun && f ) -> NB_PayloadOrError_Queue_SS
{
	NB_PayloadOrError_Queue_SS		out_queue_sp( std::make_shared< NB_PayloadOrError_Queue >() );

	std::jthread	theThread( NB_ParPipe_Fun_Loop, in_queue, out_queue_sp, std::move( f ) );
	auto th_id = theThread.get_id();
	theThread.detach();	// Let it run separately

	return out_queue_sp;

}



void echo_fun( auto & a, auto & b )
{
	b = a; /*just a copy*/ 
}

void echo_fun2( PayloadOrError & a, PayloadOrError & b )
{
	b = a; /*just a copy*/ 
}


void add_2( PayloadOrError & a, PayloadOrError & b )
{
	b = a;
	b->fStr += "_2";
	b->fVal += 2;
}


void add_3( PayloadOrError & a, PayloadOrError & b )
{
	b = a;
	b->fStr += "_3";
	b->fVal += 3;
}


void NB_ParallelPipelineTest()
{
	NB_PayloadOrError_Queue_SS		theFirstQueue( std::make_shared< NB_PayloadOrError_Queue >() );

	theFirstQueue->push( Payload{ "The quick", 5 } );
	theFirstQueue->push( Payload{ "brown", 6 } );
	theFirstQueue->push( Payload{ "fox", 7 } );
	theFirstQueue->push( Payload{ std::string( kStopToken ), 13});		// this is our custom communication with the threads

	auto init_size = theFirstQueue->size();


	std::println( "init_size = {}", init_size );


	NB_PayloadOrError_Queue_SS zeroQueue( std::make_shared< NB_PayloadOrError_Queue >() );


	auto out_q_SS = theFirstQueue | add_2 | add_3 | add_2;
	//uto out_q_SS = zeroQueue | add_2 | add_3 | add_2;

	// Here the above pipeline is already running and waiting for the objects to process
	// This is done in the following loop - objects from theFirstQueue are moved to zeroQueue

	for( ; out_q_SS->size() < init_size; )
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for( 200ms );		// emulate some actions
	
		if( theFirstQueue->size() > 0 )			// theFirstQueue is served only by the main thread
			if( auto elem = theFirstQueue->pop(); elem )
				zeroQueue->push( elem->value() );
	}

	// If here, then all objects are in the pipeline and processed by the threads.
	// In the following loop we process whatever is available on the out queue.

	for( ; out_q_SS->size() > 0; )
	{
		auto ret_e = out_q_SS->pop();
		if( ret_e )
			if( const auto & s = ret_e->value().fStr; s == kStopToken )
				std::println( "The STOP token detected" );
			else
				std::println( "fStr = {}", s );
	}


}
















