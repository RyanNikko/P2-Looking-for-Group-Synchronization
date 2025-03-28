#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#ifdef _WIN32
void moveCursorTo(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
#endif

using namespace std;

mutex mtx;
condition_variable cv;
queue<int> waitingParties;
vector<int> dungeonSlots;
vector<size_t> roomPartyCount;
vector<int> roomTotalTime;
size_t activeDungeons = 0;
size_t maxDungeons;
size_t partiesWaiting = 0;
size_t leftoverTanks = 0;
size_t leftoverHealers = 0;
size_t leftoverDPS = 0;
bool running = true;

void getUserInput(size_t& t, size_t& h, size_t& d, int& t1, int& t2) {
    //These snippets aims to validate if the user input is a positive integer.
    auto readPositiveInteger = [](const string& prompt) -> size_t {
        int value;
        while (true) {
            cout << prompt;
            if (cin >> value && value >= 0) {
                return static_cast<size_t>(value);
            }
            else {
                cout << "Invalid input! Please enter a positive integer." << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
        };
    
    //Same here for t1.
    auto readTimeValue = [](const string& prompt) -> int {
        int value;
        while (true) {
            cout << prompt;
            if (cin >> value && value > 0) {
                return value;
            }
            else {
                cout << "Invalid input! Please enter a positive integer." << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
        };

    maxDungeons = readPositiveInteger("Enter max number of parties in the dungeon: ");
    t = readPositiveInteger("Enter number of tanks: ");
    h = readPositiveInteger("Enter number of healers: ");
    d = readPositiveInteger("Enter number of DPS: ");
    t1 = readTimeValue("Enter fastest clear time (in seconds): ");

    //When asking for t2, it must not exceed 15 and must be greater than t1.
    do {
        t2 = readTimeValue("Enter slowest clear time (must be greater than minimum and max 15 seconds): ");
        if (t2 > 15 || t2 <= t1) {
            cout << "Invalid input! Slowest clear time must be greater than the minimum and cannot exceed 15. Try again." << endl;
        }
    } while (t2 > 15 || t2 <= t1);
}

//Simulates a party going through the dungeon.
void dungeonRun(size_t roomNumber, int clearTime) {
    {
        lock_guard<mutex> lock(mtx);
#ifdef _WIN32
        moveCursorTo(0, roomNumber);
#endif
        cout << "Dungeon Room [" << roomNumber << "]: Active: time to clear " << clearTime << " seconds      " << flush;
    }

    this_thread::sleep_for(chrono::seconds(clearTime));

    {
        lock_guard<mutex> lock(mtx);
        activeDungeons--;
        dungeonSlots[roomNumber - 1] = 0;
        roomTotalTime[roomNumber - 1] += clearTime;
#ifdef _WIN32
        moveCursorTo(0, roomNumber);
#endif
        cout << "Dungeon Room [" << roomNumber << "]: Empty                                      " << flush;

        if (waitingParties.empty() && activeDungeons == 0) {
            running = false;
            cv.notify_all();
        }
    }

    cv.notify_one();
}

void dungeonManager() {
    //Handles party assignments to empty dungeon rooms.
    while (true) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [] { return (!waitingParties.empty() && activeDungeons < maxDungeons) || !running; });

        if (!running && waitingParties.empty() && activeDungeons == 0) {
            break;
        }

        if (!waitingParties.empty() && activeDungeons < maxDungeons) {
            int clearTime = waitingParties.front();
            waitingParties.pop();
            activeDungeons++;
            partiesWaiting--;

#ifdef _WIN32
            moveCursorTo(0, maxDungeons + 1);
#endif
            cout << "Parties waiting in queue: " << partiesWaiting << "     " << flush;

            size_t roomNumber = 0;
            for (size_t i = 0; i < maxDungeons; i++) {
                if (dungeonSlots[i] == 0) {
                    roomNumber = i + 1;
                    dungeonSlots[i] = clearTime;
                    roomPartyCount[i]++;
                    roomTotalTime[i] += clearTime;
                    break;
                }
            }

            thread(dungeonRun, roomNumber, clearTime).detach();
        }
    }
}

int main() {
    //Initialize values. Also set up random seed for t1 and t2.
    srand(time(0));
    size_t t, h, d;
    int t1, t2;

    //Grab user input.
    getUserInput(t, h, d, t1, t2);

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    //Initialize vectors to track dungeon statistics.
    dungeonSlots.resize(maxDungeons, 0);
    roomPartyCount.resize(maxDungeons, 0);
    roomTotalTime.resize(maxDungeons, 0);

    //Now we form valid parties, assigning each one with a clear time.
    size_t partiesFormed = 0;
    while (t >= 1 && h >= 1 && d >= 3) {
        t--;
        h--;
        d -= 3;
        int clearTime = t1 + rand() % (t2 - t1 + 1);
        waitingParties.push(clearTime);
        partiesFormed++;
    }

    //Set up leftovers to be printed later and the parties waiting in queue.
    leftoverTanks = t;
    leftoverHealers = h;
    leftoverDPS = d;
    partiesWaiting = partiesFormed;

    //Ends program early should no parties be formed.
    if (partiesWaiting == 0) {
        cout << "No valid parties could be formed!!! Raid cancelled..." << endl;
        return 0;
    }

    //Start of display while program is ongoing.
    cout << "Total valid parties formed: " << partiesWaiting << endl;
    
    //Initial display for dungeon rooms/instances.
    for (size_t i = 1; i <= maxDungeons; i++) {
        cout << "Dungeon Room [" << i << "]: Empty" << endl;
    }

    //Begin raid.
    thread manager(dungeonManager);
    manager.join();

//For printing data recorded when dungeon raid was ongoing.
#ifdef _WIN32
    moveCursorTo(0, maxDungeons + 2);
#else
    cout << endl;
#endif
    cout << endl;
    cout << "All parties have cleared their dungeon runs." << endl << endl;

    cout << "=== Dungeon Raid Summary ===" << endl;

    for (size_t i = 0; i < maxDungeons; i++) {
        cout << "Dungeon Room [" << i + 1 << "] hosted " << roomPartyCount[i] << " parties." << endl;
        cout << "Total time spent in this room: " << roomTotalTime[i] << " seconds" << endl << endl;
    }

    cout << "# of players unable to form a valid party:" << endl;
    cout << "Tanks: " << leftoverTanks << endl;
    cout << "Healers: " << leftoverHealers << endl;
    cout << "DPS: " << leftoverDPS << endl;

    return 0;
}