#ifndef TACTOSY_TIMER
#define TACTOSY_TIMER

#include <chrono>
#include <thread>
#include <deque>
#include <mutex>
#include <future>


namespace tactosy
{
    class TactosyTimer
    {
    public:
        ~TactosyTimer()
        {
            stop();
        }

        void start(int _interval)
        {
            if (!started)
            {
                interval = _interval;
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
        std::deque<std::packaged_task<void()>> tasks;

        void workerFunc()
        {
            while (started)
            {
                if (callbackFunc)
                {
                    callbackFunc();
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }
        }
        std::thread runner;
    };
}

#endif