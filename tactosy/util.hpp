#ifndef TACTOSY_UTIL
#define TACTOSY_UTIL

#include <fstream>
#include <string>
#include <map>
#include "model.hpp"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;
namespace tactosy
{
    class Util
    {
    public:
        static string readFile(string path)
        {
            ifstream file(path);
            string str;
            string file_contents;
            while (getline(file, str))
            {
                file_contents += str;
                file_contents.push_back('\n');
            }

            return file_contents;
        }

        template<class T, class V>
        static bool containsKey(T key, map<T, V> &mapData)
        {
            return mapData.find(key) != mapData.end();
        }

        static TactosyFile parse(string path)
        {
            string jsonStr = readFile(path);

            TactosyFile file;

            json tactosyFileJson = json::parse(jsonStr.c_str());

            int intervalMillis = tactosyFileJson["intervalMillis"].get<int>();
            int size = tactosyFileJson["size"].get<int>();
            int durationMillis = tactosyFileJson["durationMillis"].get<int>();

            file.durationMillis = durationMillis;
            file.size = size;
            file.intervalMillis = intervalMillis;

            map<string, json> feedbacks = tactosyFileJson["feedback"];

            for (auto& kv : feedbacks) {
                int key = stoi(kv.first);
                json value = kv.second;

                for (auto& tactosyFeedbackJson : value) {
                    TactosyFeedback feedback;

                    string mode = tactosyFeedbackJson["mode"].get<string>();
                    string position = tactosyFeedbackJson["position"].get<string>();
                    auto values = tactosyFeedbackJson["values"];

                    int index = 0;
                    for (auto& val : values)
                    {
                        feedback.values[index] = val.get<int>();
                        index++;
                    }

                    if ("PATH_MODE" == mode)
                    {
                        feedback.mode = PATH_MODE;
                    }
                    else if ("DOT_MODE")
                    {
                        feedback.mode = DOT_MODE;
                    }
                    else
                    {
                        throw "unknown mode : " + mode;
                    }

                    if ("Right" == position)
                    {
                        feedback.position = Right;
                    }
                    else if ("Left" == position)
                    {
                        feedback.position = Left;
                    }
                    else if ("All" == position)
                    {
                        feedback.position = All;
                    }
                    else
                    {
                        throw "unkown position " + position;
                    }

                    file.feedback[key].push_back(feedback);
                }
            }

            return file;
        }
    };
}

#endif