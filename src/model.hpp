#ifndef TACTOSY_MODEL
#define TACTOSY_MODEL

#include <map>
#include <vector>

namespace tactosy
{
    using namespace std;

    enum Position {
        Left,
        Right,
        All
    };

    enum FeeddbackMode {
        PATH_MODE,
        DOT_MODE
    };

    struct TactosyFeedback
    {
        Position position;
        FeeddbackMode mode;
        vector<uint8_t> values;

        TactosyFeedback(Position _pos, const int _val[], FeeddbackMode _mod)
        {
            position = _pos;
            mode = _mod;
            values.assign(20, 0);
            for (int i = 0; i < 20; i++)
            {
                values[i] = _val[i];
            }
        }

        TactosyFeedback(Position _pos, const vector<uint8_t> &_val, FeeddbackMode _mod)
        {
            position = _pos;
            mode = _mod;
            values.assign(20, 0);
            for (int i = 0; i < 20; i++)
            {
                values[i] = _val[i];
            }
        }

        TactosyFeedback() : position(), mode()
        {
            values.assign(20, 0);
        }
    };

    struct Point
    {
        float x;
        float y;
        float intensity;

        Point(float _x, float _y, float _intensity)
        {
            x = _x;
            y = _y;
            intensity = _intensity;
        }
    };

    struct TactosyFile
    {
        int intervalMillis;
        int size;
        int durationMillis;
        map<int, vector<TactosyFeedback>> feedback;

    };

    class Common
    {
    public:
        template<class T, class V>
        static bool containsKey(T key, const map<T, V> &mapData)
        {
            return mapData.find(key) != mapData.end();
        }
    };

    class FeedbackSignal
    {
    public:
        FeedbackSignal() {}
        map<int, vector<TactosyFeedback>> HapticFeedback;
        int StartTime;
        int EndTime;

        FeedbackSignal(TactosyFeedback feedback, int durationMillis, int interval)
        {
            int i;

            for (i = 0; i < durationMillis / interval; i++)
            {
                if (feedback.position == All)
                {

                    TactosyFeedback left(Left, feedback.values, feedback.mode);
                    TactosyFeedback right(Right, feedback.values, feedback.mode);
                    vector<TactosyFeedback> feedbacks = { left, right };
                    int time = i * interval;
                    HapticFeedback[time] = { left, right };
                }
                else
                {
                    HapticFeedback[i*interval] = { feedback };
                }


            }
            StartTime = -1;
            vector<TactosyFeedback> f;
            int bytes[20] = { 0 };
            if (feedback.position == All)
            {
                TactosyFeedback left(Left, bytes, feedback.mode);
                TactosyFeedback right(Right, bytes, feedback.mode);
                f = { left, right };
            }
            else
            {
                f = { feedback };

            }

            HapticFeedback[i * interval] = f;
            EndTime = i * interval;
        }

        FeedbackSignal(TactosyFile tactosyFile)
        {
            StartTime = -1;
            EndTime = tactosyFile.durationMillis;
            HapticFeedback = tactosyFile.feedback;
        }

        static FeedbackSignal Copy(const FeedbackSignal &signal, int interval, float intensityRatio, float durationRatio)
        {
            FeedbackSignal feedbackSignal;
            feedbackSignal.EndTime = static_cast<int>(signal.EndTime * durationRatio / interval * interval) + interval;
            feedbackSignal.StartTime = -1;

            int time;
            for (time = 0; time < feedbackSignal.EndTime; time += interval)
            {
                int keyTime = static_cast<int>(time / durationRatio) / interval*interval;

                if (Common::containsKey(keyTime, signal.HapticFeedback)) // contains
                {
                    vector<TactosyFeedback> tactosyFeedbacks = signal.HapticFeedback.at(keyTime);

                    vector<TactosyFeedback> copiedFeedbacks(tactosyFeedbacks.size());

                    for (size_t i = 0; i < tactosyFeedbacks.size(); i++)
                    {
                        TactosyFeedback tactosyFeedback = tactosyFeedbacks[i];
                        int values[20];
                        if (tactosyFeedback.mode == DOT_MODE)
                        {
                            for (size_t valueIndex = 0; valueIndex < tactosyFeedback.values.size(); valueIndex++)
                            {
                                int val = static_cast<int>(tactosyFeedback.values[valueIndex] * intensityRatio);
                                if (val > 100)
                                {
                                    val = 100;
                                }
                                else if (val < 0)
                                {
                                    val = 0;
                                }

                                values[valueIndex] = val;
                            }
                        }
                        else if (tactosyFeedback.mode == PATH_MODE)
                        {
                            for (size_t valueIndex = 0; valueIndex < tactosyFeedback.values.size(); valueIndex++)
                            {
                                values[valueIndex] = tactosyFeedback.values[valueIndex];
                            }

                            for (int index = 0; index < 6; index++)
                            {
                                int realIndex = 3 + index * 3;
                                int val = static_cast<int>(tactosyFeedback.values[realIndex] * intensityRatio);
                                if (val > 100)
                                {
                                    val = 100;
                                }
                                else if (val < 0)
                                {
                                    val = 0;
                                }

                                values[realIndex] = val;
                            }
                        }
                        TactosyFeedback feedback(tactosyFeedback.position, values, tactosyFeedback.mode);
                        copiedFeedbacks[i] = feedback;
                        feedbackSignal.HapticFeedback[time] = copiedFeedbacks;
                    }
                }
            }

            return feedbackSignal;
        }

    };
}

#endif