#include "../src/tactosy.hpp"

using namespace tactosy;
int main() 
{
    TactosyManager manager;
    manager.registerFeedback("path", "feedback/Pathmode.tactosy");
    manager.registerFeedback("dot", "feedback/Dotmode.tactosy");
    manager.registerFeedback("undefined", "feedback/Undefined.tactosy");
    manager.registerFeedback("head", "feedback/head.tactosy");
    manager.registerFeedback("wrong", "feedback/Wrongformat.tactosy");
    manager.init();
   
    Sleep(300);

    manager.submitRegistered("undefined");
	
    // USE tactosy studio feedback file
    manager.submitRegistered("dot");
    Sleep(2000);

    manager.submitRegistered("path");

    Sleep(3000);

    manager.submitRegistered("head");

    Sleep(3000);

    // Manual control
    vector<uint8_t> values(20, 0);
    values[3] = 100;
    manager.submit("dotTest", Right, values, 300);

    Sleep(500);

    vector<PathPoint> points;
    PathPoint point(0.8f, 0.6f, 50);
    points.push_back(point);

    manager.submit("pathTest", Right, points, 300);
    Sleep(500);


    // Long running test
    vector<uint8_t> onValues(20, 0);
    onValues[3] = 100;
    onValues[2] = 100;
    onValues[1] = 100;
    vector<uint8_t> noneValues(20, 0);

    for(int i = 0 ; i < 1000 ; i++)
    {
        manager.submit("dotTest", Right, onValues, 20);
        manager.submit("dotLeft", Left, onValues, 20);

        Sleep(40);
        manager.submit("dotTest", Right, noneValues, 20);
        manager.submit("dotLeft", Left, noneValues, 20);
        Sleep(40);
    }

    manager.destroy();
    return 0;
}
