#ifndef TACTOSY_HPP
#define TACTOSY_HPP

#include "easywsclient.hpp"
#include "timer.hpp"
#include "model.hpp"
#include "util.hpp"
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>

using namespace std;
using easywsclient::WebSocket;

namespace tactosy
{
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))  
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))  
    class TactosyManager
    {
        unique_ptr<WebSocket> ws;
        
        map<string, FeedbackSignal> _registeredSignals;
        map<string, FeedbackSignal> _activeSignals;
        int _currentTime = 0;
        int _interval = 20;
        int _motorSize = 20;
        TactosyTimer timer;

        void playFeedback(TactosyFeedback &feedback)
        {
            vector<uint8_t> message(_motorSize + 2, 0);
            if (feedback.mode == PATH_MODE)
            {
                message[0] = 1;
            }
            else if (feedback.mode == DOT_MODE)
            {
                message[0] = 2;
            }

            if (feedback.position == Left)
            {
                message[1] = 1;
            }
            else if (feedback.position == Right)
            {
                message[1] = 2;
            }

            for (int i = 0; i < _motorSize; i++)
            {
                message[i + 2] = feedback.values[i];
            }

            send(message);
        }

        void send(vector<uint8_t> &message) const
        {
            ws->sendBinary(message);
            ws->poll(100);
        }

        void doRepeat()
        {
            if (_activeSignals.size() == 0)
            {
                if (_currentTime > 0)
                {
                    _currentTime = 0;
                    vector<uint8_t> bytes(_motorSize, 0);
                    TactosyFeedback feedback(Right, bytes, DOT_MODE);
                    TactosyFeedback Leftfeedback(Left, bytes, DOT_MODE);
                    playFeedback(feedback);
                    playFeedback(Leftfeedback);
                }

                return;
            }

            vector<uint8_t> dotModeSignalLeft(_motorSize, 0);
            vector<uint8_t> dotModeSignalRight(_motorSize, 0);
            vector<uint8_t> pathModeSignalLeft(_motorSize, 0);
            vector<uint8_t> pathModeSignalRight(_motorSize, 0);

            vector<string> expiredSignals;

            bool dotModeLeftActive = false;
            bool dotModeRightActive = false;
            bool pathModeActiveLeft = false;
            bool pathModeActiveRight = false;

            for (auto keyPair : _activeSignals)
            {
                auto key = keyPair.first;
                auto signalData = keyPair.second;

                if (signalData.StartTime > _currentTime || signalData.StartTime < 0)
                {
                    signalData.StartTime = _currentTime;
                }

                int timePast = _currentTime - signalData.StartTime;

                if (timePast > signalData.EndTime)
                {
                    expiredSignals.push_back(key);
                }
                else
                {
                    if (Util::containsKey(timePast, signalData.HapticFeedback))
                    {
                        auto hapticFeedbackData = signalData.HapticFeedback[timePast];
                        for (auto &feedback : hapticFeedbackData)
                        {
                            if (feedback.mode == PATH_MODE && feedback.position == Left)
                            {
                                int prevSize = pathModeSignalLeft[0];

                                vector<uint8_t> data = feedback.values;
                                int size = data[0];
                                if (prevSize + size > 6)
                                {
                                    continue;
                                }

                                pathModeSignalLeft[0] = prevSize + size;

                                for (int i = prevSize; i < prevSize + size; i++)
                                {
                                    pathModeSignalLeft[3 * i + 1] = data[3 * (i - prevSize) + 1];
                                    pathModeSignalLeft[3 * i + 2] = data[3 * (i - prevSize) + 2];
                                    pathModeSignalLeft[3 * i + 3] = data[3 * (i - prevSize) + 3];
                                }

                                pathModeActiveLeft = true;
                            }
                            else if (feedback.mode == PATH_MODE && feedback.position == Right)
                            {
                                int prevSize = pathModeSignalRight[0];

                                vector<uint8_t> data = feedback.values;
                                int size = data[0];
                                if (prevSize + size > 6)
                                {
                                    continue;
                                }

                                pathModeSignalRight[0] = prevSize + size;

                                for (int i = prevSize; i < prevSize + size; i++)
                                {
                                    pathModeSignalRight[3 * i + 1] = data[3 * (i - prevSize) + 1];
                                    pathModeSignalRight[3 * i + 2] = data[3 * (i - prevSize) + 2];
                                    pathModeSignalRight[3 * i + 3] = data[3 * (i - prevSize) + 3];
                                }

                                pathModeActiveRight = true;
                            }
                            else if (feedback.mode == DOT_MODE && feedback.position == Left)
                            {
                                for (int i = 0; i < _motorSize; i++)
                                {
                                    dotModeSignalLeft[i] += feedback.values[i];
                                }

                                dotModeLeftActive = true;
                            }
                            else if (feedback.mode == DOT_MODE && feedback.position == Right)
                            {
                                for (int i = 0; i < _motorSize; i++)
                                {
                                    dotModeSignalRight[i] += feedback.values[i];
                                }
                                dotModeRightActive = true;
                            }
                        }
                    }
                }
            }
            for (auto &key : expiredSignals)
            {
                _activeSignals.erase(key);
            }

            if (dotModeLeftActive)
            {
                TactosyFeedback feedback(Left, dotModeSignalLeft, DOT_MODE);
                playFeedback(feedback);
            }
            else if (pathModeActiveLeft)
            {
                TactosyFeedback feedback(Left, pathModeSignalLeft, PATH_MODE);
                playFeedback(feedback);
            }
            else
            {
                vector<uint8_t> values(_motorSize, 0);
                TactosyFeedback feedback(Left, values, DOT_MODE);
                playFeedback(feedback);
            }

            if (dotModeRightActive)
            {
                TactosyFeedback feedback(Right, dotModeSignalRight, DOT_MODE);
                playFeedback(feedback);
            }
            else if (pathModeActiveRight)
            {
                TactosyFeedback feedback(Right, pathModeSignalRight, PATH_MODE);
                playFeedback(feedback);
            }
            else
            {
                vector<uint8_t> values(_motorSize, 0);
                TactosyFeedback feedback(Right, values, DOT_MODE);
                playFeedback(feedback);
            }

            _currentTime += _interval;
        }

        void callbackFunc()
        {
            try
            {
                doRepeat();
            }
            catch (int e)
            {
                int  a = e + 1;
            }
        }


    public:
        void registerFeedback(string key, string path)
        {
            TactosyFile file = Util::parse(path);
            FeedbackSignal signal(file);
            _registeredSignals[key] = signal;
        }

        void init()
        {
            function<void()> callback = std::bind(&TactosyManager::callbackFunc, this);
            timer.addTimerHandler(callback);
            timer.start(_interval);

#ifdef _WIN32
            INT rc;
            WSADATA wsaData;

            rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (rc) {
                printf("WSAStartup Failed.\n");
                return;
            }
#endif

            ws = unique_ptr<WebSocket>(WebSocket::create());
            assert(ws);

            vector<uint8_t> values(_motorSize, 0);

            TactosyFeedback feedback(Right, values, DOT_MODE);
            playFeedback(feedback);
        }

        void sendSignal(string key, Position position, vector<uint8_t> &motorBytes, int durationMillis)
        {
            TactosyFeedback feedback(position, motorBytes, DOT_MODE);
            FeedbackSignal signal(feedback, durationMillis, _interval);
            _activeSignals[key] = signal;
        }

        void sendSignal(string key, Position position, vector<Point> &points, int durationMillis)
        {
            if (points.size() > 6 || points.size() <= 0)
            {
                printf("number of points should be [1 ~ 6]");
                return;
            }

            vector<uint8_t> bytes(_motorSize, 0);
            bytes[0] = points.size();
            for (size_t i = 0; i < points.size(); i++)
            {
                bytes[3 * i + 1] = static_cast<int>(MIN(40, MAX(0, points[i].x * 40))); // x
                bytes[3 * i + 2] = static_cast<int>(MIN(30, MAX(0, points[i].y * 30)));
                bytes[3 * i + 3] = static_cast<int>(MIN(100, MAX(0, points[i].intensity * 100)));
            }

            TactosyFeedback feedback(position, bytes, PATH_MODE);
            FeedbackSignal signal(feedback, durationMillis, _interval);
            _activeSignals[key] = signal;
        }

        void sendSignal(string key, double intensity, double duration)
        {
            if (!Util::containsKey(key, _registeredSignals))
            {
                printf("Key : %s is not registered.", key.c_str());

                return;
            }

            if (duration < 0.01 || duration > 100)
            {
                printf("not allowed duration %f", duration);
                return;
            }

            if (intensity < 0.01f || intensity > 100)
            {
                printf("not allowed intensity %f", duration);
                return;
            }

            FeedbackSignal signal = _registeredSignals[key];

            FeedbackSignal copiedFeedbackSignal = FeedbackSignal::Copy(signal, _interval, intensity, duration);

            _activeSignals[key] = copiedFeedbackSignal;
        }

        void sendSignal(string key)
        {
            if (!Util::containsKey(key, _registeredSignals))
            {
                printf("Key : %s is not registered.", key.c_str());

                return;
            }

            auto signal = _registeredSignals[key];

            signal.StartTime = -1;
            if (!Util::containsKey(key, _activeSignals))
            {
                _activeSignals[key] = signal;
            }
        }

        bool isPlaying()
        {
            return _activeSignals.size() > 0;
        }

        bool isPlaying(string key)
        {
            return Util::containsKey(key, _activeSignals);
        }

        void turnOff()
        {
            _activeSignals.clear();
        }

        void turnOff(string key)
        {
            if (!Util::containsKey(key, _activeSignals))
            {
                printf("feedback with key( %s ) is not playing.", key.c_str());
                return;
            }

            _activeSignals.erase(key);
        }

        void destroy()
        {
            vector<uint8_t> values(_motorSize, 0);
            TactosyFeedback feedback(All, values, DOT_MODE);
            playFeedback(feedback);

            ws->close();
        }
    };
}

#endif