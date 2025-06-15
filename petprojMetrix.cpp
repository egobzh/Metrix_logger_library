#include <iostream>
#include <thread>
#include "Metrix.h"

void emulateRPS(MetrixPoller& poller) {
	// this function emulate event
	std::srand(std::time({}));
	const int randomValue = std::rand() % (100 - 30 + 1) + 30;
	poller.writeEvent("HTTP requests RPS", randomValue);
}

void emulateTST(MetrixPoller& poller) {
	// this function emulate event
	std::srand(std::time({}));
	const int randomValue = std::rand() % (5 + 1);
	poller.writeEvent("Technical support tickets", randomValue);
}

void emulate(MetrixPoller& poller) {
	// emulate events 15 seconds
	for (int i = 0; i < 15; i++) {
		emulateTST(poller);
		emulateRPS(poller);
		std::chrono::seconds timeToSleep(1);
		std::this_thread::sleep_for(timeToSleep);
	}
}

int main() {
	// USING GUIDE

	// Create writer. One writer can be used in many pollers.
	MetrixWriter writer("testlog.log");

	// Create poller.
	MetrixPoller poller(writer);

	// Start polling events.
	poller.startPolling();

	// Also you can stop polling. Destructor also stop polling.
	// poller.stopPolling();

	// Write metric. Metric value must be converted to std::string.
	poller.writeEvent("Test tasks completed:)", 1);

	// this is events emulation in new thread
	std::thread t = std::thread(emulate, std::ref(poller));
	std::cout << "start logging" << std::endl;
	t.join();
	std::cout << "logging finished" << std::endl;
}
