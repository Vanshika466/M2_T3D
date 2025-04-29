#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <chrono>
#include <fstream>
#include <tuple>
#include <sstream>
#include <string>

// Shared resources
std::queue<std::tuple<std::string, std::string, int>> trafficQueue;
std::unordered_map<std::string, int> trafficData;
std::mutex queueMutex, dataMutex;
std::condition_variable cv;
bool done = false;

// Function to parse data from external file (Producer)
void producer(const std::string& filename)
{
    std::ifstream fileStream(filename);
    if (!fileStream.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        done = true;
        cv.notify_all();
        return;
    }

    std::string line;
    while (std::getline(fileStream, line)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));  // Simulate real-time delay

        std::istringstream lineStream(line);
        std::string timestamp, light, carCountStr;

        std::getline(lineStream, timestamp, ',');
        std::getline(lineStream, light, ',');
        std::getline(lineStream, carCountStr, ',');

        int carCount = std::stoi(carCountStr);

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            trafficQueue.push(std::make_tuple(timestamp, light, carCount));
        }

        cv.notify_one();  // Notify consumer that new data is available
        std::cout << "[Producer] Read Traffic Data: (" << timestamp << ", " << light << ", " << carCount << " cars)\n";
    }

    done = true;
    cv.notify_all();  // Notify all waiting threads
}

// Function to process traffic data (Consumer)
void consumer()
{
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [] { return !trafficQueue.empty() || done; });

        while (!trafficQueue.empty()) {
            auto data = trafficQueue.front();
            trafficQueue.pop();
            lock.unlock();

            std::string timestamp = std::get<0>(data);
            std::string light = std::get<1>(data);
            int carCount = std::get<2>(data);

            {
                std::lock_guard<std::mutex> dataLock(dataMutex);
                trafficData[light] += carCount;
            }

            std::cout << "[Consumer] Processed: " << timestamp << " - " << light << " (" << carCount << " cars)\n";
            lock.lock();
        }

        if (done && trafficQueue.empty()) break;
    }

    std::cout << "\n--- Top Congested Traffic Lights ---\n";
    for (const auto& entry : trafficData) {
        std::cout << entry.first << ": " << entry.second << " cars passed\n";
    }
    std::cout << "-----------------------------------\n";
}

// Main function
int main()
{
    std::string filename = "traffic_data.txt";

    std::thread producerThread(producer, filename);
    std::thread consumerThread(consumer);

    producerThread.join();
    consumerThread.join();

    return 0;
}
