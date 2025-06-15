#pragma once
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <unordered_map>

namespace {
    std::string getCurrentTimeUTC() {
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        std::tm utcTime;
        gmtime_s(&utcTime, &nowTime);
        auto sinceEpoch = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(sinceEpoch) % 1000;
        std::ostringstream oss;
        oss << std::put_time(&utcTime, "%Y-%m-%d %H:%M:%S")
            << "." << std::setfill('0') << std::setw(3) << static_cast<int>(millis.count());
        return oss.str();
    }
}

class MetrixWriter {
private:
	std::string filename;
    std::mutex fileMutex;

public:
	MetrixWriter(const std::string& filename): filename(filename) {

	}

	void write(const std::string& metrixText) {
        std::unique_lock<std::mutex> lock(fileMutex);
        std::ofstream outFile(filename, std::ios::app);
        if (outFile.is_open()) {
            outFile << metrixText << std::endl;
            outFile.close();
        } else {
            outFile.close();
            throw std::runtime_error("Cant open log file");
        }
	}
};


class MetrixPoller {
private:
    std::mutex eventMutex;
    bool mustPoll = false;
    std::unordered_map<std::string, std::string> events;
    MetrixWriter& writer;
    std::thread pollingThread;

    void _polling() {
        while (mustPoll) {
            writeMetrix();
            std::chrono::seconds timeToSleep(1);
            std::this_thread::sleep_for(timeToSleep);
        }
    }

    void writeMetrix() {
        std::unique_lock<std::mutex> lock(eventMutex);
        if (events.empty()) {
            return;
        }
        std::string resultString = getCurrentTimeUTC() + ' ';
        for (auto event: events) {
            resultString += '"' + event.first + '"' + ' ' + event.second + ' ';
        }
        writer.write(resultString);
        events.clear();
    }

public:
    MetrixPoller(MetrixWriter& writer): writer(writer) {

    }

    ~MetrixPoller() {
        stopPolling();
    }

    void startPolling() {
        if (mustPoll) {
            return;
        }
        mustPoll = true;
        pollingThread = std::thread([this] { this->_polling(); });
    }

    void stopPolling() {
        if (!mustPoll) {
            return;
        }
        mustPoll = false;
        pollingThread.join();
        writeMetrix();
    }

    template<typename T>
    void writeEvent(const std::string& eventName, const T value) {
        std::unique_lock<std::mutex> lock(eventMutex);
        events[eventName] = std::to_string(value);
    }
};