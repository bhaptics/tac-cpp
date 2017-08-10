#include "../src/tactosy.hpp"
#include "../src/thirdparty/json.hpp"
#include "../src/common/timer.hpp"
#include "../src/common/model.hpp"
#include "../src/common/util.hpp"

using namespace tactosy;

using easywsclient::WebSocket;
using nlohmann::json;

static WebSocket::pointer webSocket = NULL;

void handle_message(const char* message)
{

	/*std::string temp = message;
	nlohmann::json j = nlohmann::json::parse(message.c_str());
	int pos = atoi((j.at("Position").get<string>()).c_str());
	string val = j.at("Values").get<string>();
	stringstream stream(val.substr(1,val.size()-2));
	string item;
	vector<uint8_t> values;
	while (std::getline(stream,item,','))
	{
		values.push_back(atoi(item.c_str()));
	}
	HapticFeedback tempFeedback((Position)pos,values,FeedbackMode::DOT_MODE );*/

	printf(message);
	printf("\n");

}

HapticPlayer *HapticPlayer::hapticManager = 0;

int main() 
{
	/**
#ifdef _WIN32
	INT rc;
	WSADATA wsaData;

	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc) {
		printf("WSAStartup Failed.\n");
		return 1;
	}
#endif

	//webSocket = WebSocket::from_url("ws://localhost:8126/foo");//ws://echo.websocket.org/
	webSocket = WebSocket::create("echo.websocket.org", 80, "");
	assert(webSocket);
	webSocket->send("goodbye");
	webSocket->send("hello");
	while (webSocket->getReadyState() != WebSocket::CLOSED) {
		webSocket->poll();
		webSocket->dispatch(handle_message);
	}
	delete webSocket;
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
	//*/
	//**
	
	//HapticPlayer wow;
	//HapticPlayer neener;
	//HapticPlayer::instance()->dispatchFunctionVar = handle_message;
	//HapticPlayer::feedbackBuffer = {};
	HapticPlayer::instance()->registerFeedback("path", "feedback/Pathmode.tactosy");
	HapticPlayer::instance()->registerFeedback("dot", "feedback/Dotmode.tactosy");
	HapticPlayer::instance()->registerFeedback("undefined", "feedback/Undefined.tactosy");
	HapticPlayer::instance()->registerFeedback("head", "feedback/head.tactosy");
	HapticPlayer::instance()->registerFeedback("wrong", "feedback/Wrongformat.tactosy");
	HapticPlayer::instance()->registerFeedback("Headache", "feedback/Headache.tactosy");
	HapticPlayer::instance()->registerFeedback("Head2", "feedback/Head2.tactosy");
	HapticPlayer::instance()->init();
	HapticPlayer::instance()->init();
	HapticPlayer::instance()->init();
	//HapticPlayer::instance()->Enable();
 //   Sleep(300);



	////while (true) {
	//Sleep(200);
	////}
 //   HapticPlayer::instance()->submitRegistered("undefined");
 //   // USE tactosy studio feedback file
 //   HapticPlayer::instance()->submitRegistered("dot");
 //   Sleep(2000);
 //   HapticPlayer::instance()->submitRegistered("path");
 //   Sleep(3000);
 //   HapticPlayer::instance()->submitRegistered("Headache");
 //   Sleep(2000);
 //   HapticPlayer::instance()->submitRegistered("Headache");
 //   Sleep(2000);
 //   HapticPlayer::instance()->submitRegistered("head");
 //   Sleep(3000);
 //   // Manual control
 //   vector<uint8_t> values(20, 0);
 //   values[3] = 100;
 //   HapticPlayer::instance()->submit("dotTest", Right, values, 300);
	////HapticPlayer::instance()->Check(handle_message);
 //   Sleep(500);

 //   vector<PathPoint> points;
 //   PathPoint point(0.8f, 0.6f, 50);
 //   points.push_back(point);

 //   HapticPlayer::instance()->submit("pathTest", Right, points, 300);
 //   Sleep(500);


 //   // Long running test
 //   vector<uint8_t> onValues(20, 0);
 //   onValues[3] = 100;
 //   onValues[2] = 100;
 //   onValues[1] = 100;
 //   vector<uint8_t> noneValues(20, 0);


	////while (true)
	//{
	//	HapticPlayer::instance()->submit("HeadHit", Head, { PathPoint(.2, .3, 100) }, 100);
	//	Sleep(200);
	//}
	HapticPlayer::instance()->registerFeedback("FireBallLeft", "feedback/Fireball_Left.tactosy");
	HapticPlayer::instance()->registerFeedback("FireBallRight", "feedback/Fireball_Right.tactosy");
	for (int i = 0; i < 5000; i++)
	{

		HapticPlayer::instance()->submitRegistered("FireBallLeft");
		
		//HapticPlayer::instance()->submitRegistered("Head2");
		//HapticPlayer::instance()->submitRegistered("path");
		Sleep(20);
		HapticPlayer::instance()->submitRegistered("FireBallRight");
	}
	Sleep(10000);
	//HapticPlayer::instance()->submit("pathTest", Right, points, 300);
	//Sleep(500);
	//HapticPlayer::instance()->submit("pathTest", Left, points, 300);
	//Sleep(500);


    HapticPlayer::instance()->destroy();
    return 0;//*/
}
