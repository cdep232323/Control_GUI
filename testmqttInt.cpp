/*
	g++ -o testBool testmqtt.cpp -I /usr/include/tasutils/io -I /usr/include/tasutils/messaging -lTASmessaging -lTASjson -lmosquitto -lpthread -lprotobuf
*/

#include <unistd.h>
#include <time.h>
// #include <statusmessages.pb.h>
// #include <tetxmessages.pb.h>
// #include "tasmessagesutils.h"

#include <mqtt_helper.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
// #include <google/protobuf/util/json_util.h>
// #include "messagesutils.h"
using std::ostringstream;
using std::string;
using std::cerr;
using std::endl;

using mqtt_helper::MessageData;
using mqtt_helper::MqttHelper;
using mqtt_helper::MqttHelperConfig;


#define SLEEP_TIME 5 //sec


int main(int argc, char** argv) {
	char Topic[256];
	int state = 0;
	double value = 0.0;
	int value1 = 0;
	long int value2 = 0;
	bool ON = true, OFF = false, out = false;


	if(argc == 3) { //int -> bool
		state = atoi(argv[2]);
		strcpy(Topic, argv[1]);
		printf("topic: %s\n", Topic);
	}
	else {
		printf("Usage: ./test <Topic> <value>\n");
		return 1;
	}

	MqttHelperConfig config;
	config.publishQoS = 2;
	config.clientID = "mqtt_tester";
	config.subscribeTopics = {};

	// config.verbose = true;
	MqttHelper client;
	if(!client.configure(config)) {
		return 1;
	}
	if(!client.start(0, 0)) {
		return 1;
	}

	if(argc == 3) {
		out = (state>0) ? true:false;
		client.publishBool(Topic, out);
	}

	client.stop(0, 1e9);

	return 0;
}