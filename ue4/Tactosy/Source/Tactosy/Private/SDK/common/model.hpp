#ifndef TACTOSY_MODEL
#define TACTOSY_MODEL

#include <map>

namespace tactosy
{
    using namespace std;

    enum Position {
        All = 0, Left = 1, Right = 2,
        Vest = 3,
        Head = 4,
        VestFront = 201, VestBack = 202,
        GloveLeft = 203, GloveRight = 204,
        Custom1 = 251, Custom2 = 252, Custom3 = 253, Custom4 = 254
    };

    enum FeeddbackMode {
        PATH_MODE,
        DOT_MODE
    };

    struct DotPoint
    {
        int index;
        int intensity;
        DotPoint(int _index, int _intensity)
        {
            index = _index;
            intensity = _intensity;
        }
    };

    struct PathPoint
    {
        float x;
        float y;
        int intensity;

        PathPoint(float _x, float _y, int _intensity)
        {
            x = _x;
            y = _y;
            intensity = _intensity;
        }
    };

    struct HapticFeedbackFrame
    {
        vector<PathPoint> pathPoints;
        vector<DotPoint> dotPoints;
        int texture;
        Position position;

        HapticFeedbackFrame(Position _pos, const vector<PathPoint> &_pathPoints)
        {
            position = _pos;
            pathPoints = _pathPoints;
            texture = 0;
        }

        HapticFeedbackFrame(Position _pos, const vector<DotPoint> &_dotPoints)
        {
            position = _pos;
            dotPoints = _dotPoints;
            texture = 0;
        }
    };

    struct HapticFeedback
    {
        Position position;
        FeeddbackMode mode;
        vector<uint8_t> values;

        HapticFeedback(Position _pos, const int _val[], FeeddbackMode _mod)
        {
            position = _pos;
            mode = _mod;
            values.assign(20, 0);
            for (int i = 0; i < 20; i++)
            {
                values[i] = _val[i];
            }
        }

        HapticFeedback(Position _pos, const vector<uint8_t> &_val, FeeddbackMode _mod)
        {
            position = _pos;
            mode = _mod;
            values.assign(20, 0);
            for (int i = 0; i < 20; i++)
            {
                values[i] = _val[i];
            }
        }

        HapticFeedback() : position(), mode()
        {
            values.assign(20, 0);
        }
    };

    struct TactosyFile
    {
        int intervalMillis;
        int size;
        int durationMillis;
        map<int, vector<HapticFeedback>> feedback;

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

    class BufferedHapticFeedback
    {
    public:
        BufferedHapticFeedback() {}
        map<int, vector<HapticFeedback>> feedbackMap;
        int StartTime;
        int EndTime;

        BufferedHapticFeedback(HapticFeedback feedback, int durationMillis, int interval)
        {
            int i;

            for (i = 0; i < durationMillis / interval; i++)
            {
                if (feedback.position == All)
                {

                    HapticFeedback left(Left, feedback.values, feedback.mode);
                    HapticFeedback right(Right, feedback.values, feedback.mode);
                    HapticFeedback vBack(VestBack, feedback.values, feedback.mode);
                    HapticFeedback vFront(VestFront, feedback.values, feedback.mode);
                    HapticFeedback head(Head, feedback.values, feedback.mode);
                    int time = i * interval;
                    feedbackMap[time] = { left, right, vBack, vFront, head };
                }
                else
                {
                    feedbackMap[i * interval] = { feedback };
                }


            }
            StartTime = -1;
            vector<HapticFeedback> f;
            int bytes[20] = { 0 };
            if (feedback.position == All)
            {
                HapticFeedback left(Left, bytes, feedback.mode);
                HapticFeedback right(Right, bytes, feedback.mode);
                HapticFeedback vBack(VestBack, bytes, feedback.mode);
                HapticFeedback vFront(VestFront, bytes, feedback.mode);
                HapticFeedback head(Head, bytes, feedback.mode);
                f = { left, right, vBack, vFront, head };
            }
            else
            {
                f = { feedback };

            }

            feedbackMap[i * interval] = f;
            EndTime = i * interval;
        }

        BufferedHapticFeedback(TactosyFile tactosyFile)
        {
            StartTime = -1;
            EndTime = tactosyFile.durationMillis;
            feedbackMap = tactosyFile.feedback;
        }

        static BufferedHapticFeedback Copy(const BufferedHapticFeedback &signal, int interval, float intensityRatio, float durationRatio)
        {
            BufferedHapticFeedback feedbackSignal;
            feedbackSignal.EndTime = static_cast<int>(signal.EndTime * durationRatio / interval * interval) + interval;
            feedbackSignal.StartTime = -1;

            int time;
            for (time = 0; time < feedbackSignal.EndTime; time += interval)
            {
                int keyTime = static_cast<int>(time / durationRatio) / interval*interval;

                if (Common::containsKey(keyTime, signal.feedbackMap)) // contains
                {
                    vector<HapticFeedback> tactosyFeedbacks = signal.feedbackMap.at(keyTime);

                    vector<HapticFeedback> copiedFeedbacks(tactosyFeedbacks.size());

                    for (size_t i = 0; i < tactosyFeedbacks.size(); i++)
                    {
                        HapticFeedback tactosyFeedback = tactosyFeedbacks[i];
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
                        HapticFeedback feedback(tactosyFeedback.position, values, tactosyFeedback.mode);
                        copiedFeedbacks[i] = feedback;
                        feedbackSignal.feedbackMap[time] = copiedFeedbacks;
                    }
                }
            }

            return feedbackSignal;
        }

    };
}

#endif