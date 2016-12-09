#include<iostream>
#include "tactosy/tactosy.hpp"

using namespace tactosy;
int main() 
{
	TactosyManager manager;
	manager.registerFeedback("test", "feedback/Test2.tactosy");
	manager.registerFeedback("dot", "feedback/Dotmode.tactosy");
	manager.init();
	Sleep(300);
	manager.sendSignal("dot");
	Sleep(2000);
	manager.sendSignal("test");

    Sleep(3000);

	vector<uint8_t> values(20, 0);
	values[3] = 100;
	manager.sendSignal("dotTest", Right, values, 300);

	Sleep(500);

	vector<Point> points;
	Point point(0.8f, 0.6f, 0.5f);
	points.push_back(point);

	manager.sendSignal("pathTest", Right, points, 300);
	Sleep(500);

	vector<uint8_t> onValues(20, 0);
	onValues[3] = 20;
    onValues[2] = 20;
    onValues[1] = 20;
	vector<uint8_t> noneValues(20, 0);

    for(int i = 0 ; i < 1000 ; i++)
	{
		manager.sendSignal("dotTest", Right, onValues, 20);

		Sleep(40);

//        for(int j = 0 ; j < 1000 ; j++)
//        {
		    manager.sendSignal("dotTest", Right, noneValues, 20);
//        }
//            printf("%d\n", i);
		Sleep(40);
	}

	manager.destroy();
    return 0;
}
