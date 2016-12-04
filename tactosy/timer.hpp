#ifndef TACTOSY_TIMER
#define TACTOSY_TIMER

#include <chrono>
#include <thread>


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

        void start(int _interval)
        {
            if (!started)
            {
                inter = _interval;
                started = true;
                runner = thread(&TactosyTimer::workerFunc, this);
            }
        }

        void addTimerHandler(function<void()> &callback)
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
        bool started = false;
        function<void()> callbackFunc;
        int inter = 20;

        void workerFunc()
        {
            chrono::milliseconds interval(inter);

            while (started)
            {
                if (callbackFunc)
                {
                    callbackFunc();
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }
        }
        thread runner;
    };
}

#endif