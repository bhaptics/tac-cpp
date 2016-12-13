#ifndef TACTOSY_UTIL
#define TACTOSY_UTIL

#include <fstream>
#include <string>
#include <map>
#include "model.hpp"
#include "thirdparty/json.hpp"

namespace tactosy
{
    using namespace std;
    using json = nlohmann::json;
    class Util
    {
    public:
        static string readFile(const string& path)
        {
            ifstream file(path);

            if (!file.good())
            {
                throw runtime_error("file not exists : " + path);
            }

            string str;
            string file_contents;
            while (getline(file, str))
            {
                file_contents += str;
                file_contents.push_back('\n');
            }

            return file_contents;
        }

        static TactosyFile parse(const string& path)
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
                    else if ("DOT_MODE" == mode)
                    {
                        feedback.mode = DOT_MODE;
                    }
                    else
                    {
                        throw runtime_error("unknown mode : " + mode);
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
                        throw runtime_error("unkown position " + position);
                    }

                    file.feedback[key].push_back(feedback);
                }
            }

            return file;
        }
    };
}

#endif