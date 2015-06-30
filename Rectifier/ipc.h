#pragma once

#include <iostream>
#include <functional>
#include <map>
#include <unistd.h>
#include "globals.h"
#include "filesystem.h"
#include "string_functions.h"
#include "udp.h"

using namespace std;

class IPC
{
public:
	string self_name;

	function<void (const string)> response_map[100];

	bool response_map_checker[100];
	
	std::map<string, function<void (const string, const string)>> command_map;

	UDP udp;

	bool udp_ready = false;

	IPC(const string self_name_in);
	void update();
	int send_message(const string recipient, const string message_head, const string message_body);
	void get_response(const string recipient, const string message_head, const string message_body, 
					  function<void (const string message_body)> callback);
	void map_function(const string message_head, function<void (const string message_body, const string id)> callback);
	void open_udp_channel(const string recipient);
	void send_udp_message(const string message_head, const string message_body);
};