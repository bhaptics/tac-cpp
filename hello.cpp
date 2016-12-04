#include<iostream>
#include "tactosy/tactosy.hpp"

using namespace tactosy;
int main() 
{
	TactosyManager manager;
	manager.registerFeedback("test", "feedback/Test2.tactosy");
	manager.registerFeedback("dot", "feedback/Dotmode.tactosy");
	manager.init();
	Sleep(1000);
//	manager.sendSignal("dot");
//	Sleep(2000);
//	manager.sendSignal("test");
//
//	Sleep(3000);
//	vector<uint8_t> values(20, 0);
//	values[3] = 100;
//	manager.sendSignal("dotTest", Right, values, 300);
//
//	Sleep(500);
//
//	vector<Point> points;
//	Point point(0.8, 0.6, 1);
//	points.push_back(point);
//
//	manager.sendSignal("pathTest", Right, points, 300);
//	Sleep(500);

	vector<uint8_t> onValues(20, 0);
	onValues[3] = 100;
	vector<uint8_t> noneValues(20, 0);

    for(int i = 0 ; i < 5 ; i++)
	{
		manager.sendSignal("dotTest", Right, onValues, 20);

		Sleep(40);

		manager.sendSignal("dotTest", Right, noneValues, 20);
		Sleep(40);

		printf("repeated: %d\n", i);
	}

	manager.destroy();
    return 0;
}
