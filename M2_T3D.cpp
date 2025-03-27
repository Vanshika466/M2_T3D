// Step 1: Include the necessary library for input-output operations
#include <iostream>

// Step 2: Include the library for file handling
#include <fstream>

// Step 3: Include the library for string stream operations
#include <sstream>

// Step 4: Include the library to use vectors
#include <vector>

// Step 5: Include the library to use queues
#include <queue>

// Step 6: Include the library for unordered_map (hash table)
#include <unordered_map>

// Step 7: Include the library to use threads
#include <thread>

// Step 8: Include the library to use mutex for thread synchronization
#include <mutex>

// Step 9: Include the library for condition variables
#include <condition_variable>

// Step 10: Include the library for sorting algorithms
#include <algorithm>

// Step 11: Use the standard namespace to avoid using 'std::' repeatedly
using namespace std;

// Step 12: Declare a queue to store traffic data as pairs (Traffic Light ID, Car Count)
queue<pair<int, int>> trafficQueue;

// Step 13: Declare a mutex for synchronizing access to the shared queue
mutex mtx;

// Step 14: Declare a condition variable to coordinate between producer and consumer threads
condition_variable cv;

// Step 15: Declare a boolean flag to indicate when data processing is done
bool done = false;

// Step 16: Define the producer function to read traffic data from a file
void producer(const string &filename) {
    
    // Step 17: Open the file containing traffic data
    ifstream file(filename);

    // Step 18: Check if the file failed to open
    if (!file) {
        
        // Step 19: Print an error message
        cerr << "Error: Unable to open file " << filename << endl;
        
        // Step 20: Set done to true since no data will be processed
        done = true;

        // Step 21: Notify consumer threads that no more data will be added
        cv.notify_all();

        // Step 22: Exit the function
        return;
    }

    // Step 23: Declare a string to hold each line of data
    string line;

    // Step 24: Read each line from the file
    while (getline(file, line)) {

        // Step 25: Create a string stream from the line
        stringstream ss(line);

        // Step 26: Declare variables to store parsed data
        string timestamp, idStr, countStr;

        // Step 27: Extract timestamp (not used)
        getline(ss, timestamp, ',');

        // Step 28: Extract traffic light ID as a string
        getline(ss, idStr, ',');

        // Step 29: Extract car count as a string
        getline(ss, countStr, ',');

        // Step 30: Convert traffic light ID to an integer
        int id = stoi(idStr);

        // Step 31: Convert car count to an integer
        int count = stoi(countStr);

        {
            // Step 32: Lock the mutex before modifying the queue
            unique_lock<mutex> lock(mtx);

            // Step 33: Add the traffic data to the queue
            trafficQueue.push({id, count});
        }

        // Step 34: Notify one consumer thread that new data is available
        cv.notify_one();

        // Step 35: Simulate a small delay
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    // Step 36: Set done to true when all data is processed
    done = true;

    // Step 37: Notify all consumer threads that data reading is finished
    cv.notify_all();
}

// Step 38: Define the consumer function to process the traffic data
void consumer(int topN) {
    
    // Step 39: Create a hash map to store total cars per traffic light
    unordered_map<int, int> congestionMap;

    // Step 40: Start an infinite loop for data processing
    while (true) {
        
        // Step 41: Declare a variable to hold data from the queue
        pair<int, int> trafficData;

        {
            // Step 42: Lock the mutex before accessing the queue
            unique_lock<mutex> lock(mtx);

            // Step 43: Wait until the queue is not empty or processing is done
            cv.wait(lock, [] { return !trafficQueue.empty() || done; });

            // Step 44: Exit if queue is empty and producer is done
            if (trafficQueue.empty() && done) break;

            // Step 45: Get the front element from the queue
            trafficData = trafficQueue.front();

            // Step 46: Remove the processed element from the queue
            trafficQueue.pop();
        }

        // Step 47: Update the congestion count for the traffic light
        congestionMap[trafficData.first] += trafficData.second;
    }

    // Step 48: Convert map to a vector for sorting
    vector<pair<int, int>> sortedTraffic(congestionMap.begin(), congestionMap.end());

    // Step 49: Sort traffic data in descending order of car count
    sort(sortedTraffic.begin(), sortedTraffic.end(), [](auto &a, auto &b) {
        return a.second > b.second;
    });

    // Step 50: Print header message
    cout << "Top " << topN << " Most Congested Traffic Lights:\n";

    // Step 51: Loop to print the top N congested traffic lights
    for (int i = 0; i < min(topN, (int)sortedTraffic.size()); ++i) {
        
        // Step 52: Print traffic light ID and total car count
        cout << "Traffic Light " << sortedTraffic[i].first << " -> " << sortedTraffic[i].second << " cars\n";
    }
}

// Step 53: Define the main function to start producer and consumer threads
int main() {

    // Step 54: Define the file name containing traffic data
    string filename = "traffic_data.txt";

    // Step 55: Define the number of top congested traffic lights to display
    int topN = 5;

    // Step 56: Start the producer thread
    thread producerThread(producer, filename);

    // Step 57: Start the consumer thread
    thread consumerThread(consumer, topN);

    // Step 58: Wait for the producer thread to finish execution
    producerThread.join();

    // Step 59: Wait for the consumer thread to finish execution
    consumerThread.join();

    // Step 60: Return 0 to indicate successful program execution
    return 0;
}
