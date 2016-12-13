#ifndef TACTOSY_HPP
#define TACTOSY_HPP

#include "thirdparty/easywsclient.hpp"
#include "common/timer.hpp"
#include "common/model.hpp"
#include "common/util.hpp"

#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>

namespace tactosy
{
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))  
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))  
    using namespace std;
    using easywsclient::WebSocket;
    class TactosyManager
    {
        unique_ptr<WebSocket> ws;
        
        map<string, FeedbackSignal> _registeredSignals;
        map<string, FeedbackSignal> _activeSignals;
        mutex mtx;
        int _currentTime = 0;
        int _interval = 20;
        int _motorSize = 20;
        TactosyTimer timer;

        void playFeedback(const TactosyFeedback &feedback)
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

        void send(const vector<uint8_t> &message) const
        {
            if (ws->getReadyState() != WebSocket::OPEN)
            {
                
            } else
            {
                ws->sendBinary(message);
                ws->poll();
            }
        }

        void updateActive(const string &key, const FeedbackSignal& signal)
        {
            mtx.lock();
            _activeSignals[key] = signal;
            mtx.unlock();
        }

        void remove(const string &key)
        {
            mtx.lock();
            _activeSignals.erase(key);
            mtx.unlock();
        }

        void removeAll()
        {
            mtx.lock();
            _activeSignals.clear();
            mtx.unlock();
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

            mtx.lock();
            for (auto keyPair = _activeSignals.begin(); keyPair != _activeSignals.end(); ++keyPair)
            {
                auto key = keyPair->first;

                if (keyPair->second.StartTime > _currentTime || keyPair->second.StartTime < 0)
                {
                    keyPair->second.StartTime = _currentTime;
                }

                int timePast = _currentTime - keyPair->second.StartTime;

                if (timePast > keyPair->second.EndTime)
                {
                    expiredSignals.push_back(key);
                }
                else
                {
                    if (Common::containsKey(timePast, keyPair->second.HapticFeedback))
                    {
                        auto hapticFeedbackData = keyPair->second.HapticFeedback.at(timePast);
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

            mtx.unlock();

            for (auto &key : expiredSignals)
            {
                remove(key);
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
            doRepeat();
        }


    public:
        int registerFeedback(const string &key, const string &path)
        {
            try
            {
                TactosyFile file = Util::parse(path);
                FeedbackSignal signal(file);
                _registeredSignals[key] = signal;

                return 0;
            } catch(exception &e)
            {
                printf("Exception : %s\n", e.what());

                return -1;
            }
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

        void sendSignal(const string &key, Position position, const vector<uint8_t> &motorBytes, int durationMillis)
        {
            TactosyFeedback feedback(position, motorBytes, DOT_MODE);
            FeedbackSignal signal(feedback, durationMillis, _interval);
            updateActive(key, signal);
        }

        void sendSignal(const string &key, Position position, const vector<Point> &points, int durationMillis)
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
            updateActive(key, signal);
        }

        void sendSignal(const string &key, float intensity, float duration)
        {
            if (!Common::containsKey(key, _registeredSignals))
            {
                printf("Key : %s is not registered.", key.c_str());

                return;
            }

            if (duration < 0.01f || duration > 100.0f)
            {
                printf("not allowed duration %f", duration);
                return;
            }

            if (intensity < 0.01f || intensity > 100.0f)
            {
                printf("not allowed intensity %f", duration);
                return;
            }

            FeedbackSignal signal = _registeredSignals.at(key);

            FeedbackSignal copiedFeedbackSignal = FeedbackSignal::Copy(signal, _interval, intensity, duration);
            updateActive(key, copiedFeedbackSignal);
        }

        void sendSignal(const string &key)
        {
            if (!Common::containsKey(key, _registeredSignals))
            {
                printf("Key : '%s' is not registered.", key.c_str());

                return;
            }

            auto signal = _registeredSignals.at(key);

            signal.StartTime = -1;
            if (!Common::containsKey(key, _activeSignals))
            {
                updateActive(key, signal);
            }
            
        }

        bool isPlaying()
        {
            return _activeSignals.size() > 0;
        }

        bool isPlaying(const string &key)
        {
            return Common::containsKey(key, _activeSignals);
        }

        void turnOff()
        {
            removeAll();
        }

        void turnOff(const string &key)
        {
            if (!Common::containsKey(key, _activeSignals))
            {
                printf("feedback with key( %s ) is not playing.", key.c_str());
                return;
            }

            remove(key);
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