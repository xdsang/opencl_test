#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>
#include <chrono>
using namespace std::chrono;
#include <ctime>
#include <vector>
#include <utility>
#include <limits>

using std::vector;
using std::pair;


extern void printBuf(const unsigned char *buf, int len);

namespace shr
{
class Timer {
private:
    time_point<steady_clock> start_time, end_time;

public:
    void start() {
        start_time = steady_clock::now();
    }

    void elapse() {
        end_time = steady_clock::now();
        duration<double> elapsed_seconds = end_time - start_time;

        int hours = duration_cast<std::chrono::hours>(elapsed_seconds).count();
        int minutes = duration_cast<std::chrono::minutes>(elapsed_seconds).count() % 60;
        int seconds = duration_cast<std::chrono::seconds>(elapsed_seconds).count() % 60;

        std::cout << "Elapsed time: " << hours << " hours " << minutes << " minutes " << seconds << " seconds" << "\n";
    }

    double elapse(vector<pair<double, double>> &timeInfo)
    {
        auto tick_tock = getTickTock(timeInfo);
        return tick_tock.second - tick_tock.first;
    }

    pair<double, double> getTickTock(vector<pair<double, double>> &timeInfo)
    {
        // Get the first start thread tick time, and the last thread tock time
        double tick = std::numeric_limits<double>::max(), tock = 0;
        for (auto &t : timeInfo)
        {
            if (t.first < tick)
            {
                tick = t.first;
            }
            if (t.second > tock)
            {
                tock = t.second;
            }
        }

        return std::make_pair(tick, tock);
    }

    // Get the time point
    double seconds()
    {
        return duration_cast<duration<double>>(steady_clock::now().time_since_epoch()).count();
    }

    void showCurrentTime()
    {
        std::time_t now = std::time(0);
        std::string timeString = std::ctime(&now);
        std::cout << "Current time: " << timeString << "\n";
    }
};
} // end of namespace shr

// Remove null characters ('\0') from strings.
inline void trimString(std::string &str)
{
    size_t pos = str.find('\0');

    if (pos != std::string::npos)
    {
        str.erase(pos);
    }
}

inline uint64_t roundToMultipleOf(uint64_t number, uint64_t base, uint64_t maxValue)
{
    uint64_t n = (number > maxValue) ? maxValue : number;
    return (n / base) * base;
}

void populateArray(float *ptr, uint64_t N);
std::string outputDigits(long long count, uint length);

inline double getSpeedInGbps(const size_t data_size_InBytes, const double elapse_in_sec, const int loop_count)
{
    double speed = data_size_InBytes * 8 / 1024.0f / 1024 / 1024 / ( elapse_in_sec/ loop_count);
    return speed;
}

#endif // __UTILS_H__
