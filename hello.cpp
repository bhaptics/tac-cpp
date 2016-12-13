#include "src/tactosy.hpp"

using namespace tactosy;
int main() 
{
    TactosyManager manager;
    manager.registerFeedback("path", "feedback/Pathmode.tactosy");
    manager.registerFeedback("dot", "feedback/Dotmode.tactosy");
    manager.registerFeedback("undefined", "feedback/Undefined.tactosy");
    manager.init();
   
    Sleep(300);

    manager.sendSignal("undefined");
	
    // USE tactosy studio feedback file
    manager.sendSignal("dot");
    Sleep(2000);
    manager.sendSignal("path");

    Sleep(3000);

    // Manual control
    vector<uint8_t> values(20, 0);
    values[3] = 100;
    manager.sendSignal("dotTest", Right, values, 300);

    Sleep(500);

    vector<Point> points;
    Point point(0.8f, 0.6f, 0.5f);
    points.push_back(point);

    manager.sendSignal("pathTest", Right, points, 300);
    Sleep(500);


    // Long running test
    vector<uint8_t> onValues(20, 0);
    onValues[3] = 100;
    onValues[2] = 100;
    onValues[1] = 100;
    vector<uint8_t> noneValues(20, 0);

    for(int i = 0 ; i < 1000 ; i++)
    {
        manager.sendSignal("dotTest", Right, onValues, 20);

        Sleep(40);
        manager.sendSignal("dotTest", Right, noneValues, 20);
        Sleep(40);
    }

    manager.destroy();
    return 0;
}
