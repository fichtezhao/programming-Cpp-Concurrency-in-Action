// try.cpp : Defines the entry point for the console application.
//

//class condition_variable;
//
//Condition variable
//A condition variable is an object able to block the calling thread
//until notified to resume.
//
//It uses a unique_lock(over a mutex) to lock the thread 
//when one of its wait functions is called.The thread remains 
//blocked until woken up by another thread that calls a notification
//function on the same condition_variable object.
//
//Objects of type condition_variable always use unique_lock<mutex> to wait : 
//for an alternative that works with any kind of lockable type, see condition_variable_any


// condition_variable example
#include <iostream>           // std::cout
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void slaves(int id) {
	std::unique_lock<std::mutex> lck(mtx);
	//do sth #1
	std::cout << "thread " << id << " creation" <<'\n';
	// wait for lord notify, then block the current process
	cv.wait(lck);
	// freedom, continue process
	std::cout << "thread " << id << " notified " << "\n";
}

void do_something() {

	std::unique_lock<std::mutex> lck(mtx);
	for (int i = 0; i < 100; i++)
	{
		cv.notify_one();
		std::cout << i <<" slaves are released" << "\r";
	}
	std::cout << "all slaves are released, yeap!\n";
	//release slaves
	//cv.notify_all();
}

int main()
{
	std::thread threads[10];
	// spawn 10 threads:
	// crearted orderly
	for (int i = 0; i < 10; ++i)
		threads[i] = std::thread(slaves, i);

	std::cout << "10 threads ready to race...\n";
	do_something(); // go!

	for (auto& th : threads) th.join();
	std::cout << "good threads" << std::endl;

	return 0;
}