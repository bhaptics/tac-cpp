#ifndef TACTOSY_TIMER
#define TACTOSY_TIMER

#include <chrono>
#include <thread>
#include <deque>
#include <mutex>
#include <future>

namespace tactosy
{
    using namespace std;
    class TactosyTimer
    {
    public:
        ~TactosyTimer()
        {
            stop();
        }

        void start()
        {
            if (!started)
            {
                started = true;
                runner = std::thread(&TactosyTimer::workerFunc, this);
            }
        }

        void addTimerHandler(std::function<void()> &callback)
        {
            callbackFunc = callback;
        }

        void stop()
        {
            started = false;
            if (runner.joinable())
            {
                runner.join();
            }
        }
    private:
        std::atomic<bool> started = false;
        std::function<void()> callbackFunc;
        int interval = 20;
        int sleepTime = 1;

        std::chrono::steady_clock::time_point prev;

        void workerFunc()
        {
            while (started)
            {
                std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
                
                int values = std::chrono::duration_cast<std::chrono::milliseconds>(current - prev).count();
    
                if (callbackFunc && values >= interval )
                {
                    prev = current;
                    callbackFunc();
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            }
        }
        std::thread runner;
    };
}

#endif