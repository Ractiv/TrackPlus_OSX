#include "ipc.h"

IPC::IPC(const string self_name_in)
{
	self_name = self_name_in;

	if (!directory_exists(ipc_path))
	{
		create_directory(ipc_path);
		cout << "ipc directory created" << endl;
	}
	else
	{
		delete_all_files(ipc_path);
		cout << "ipc directory loaded" << endl;
	}
}

void IPC::update()
{
	vector<string> file_name_vec = list_files_in_directory(ipc_path);
	for (string file_name_current : file_name_vec)
		if (file_name_current.size() >= self_name.size())
		{
			const string file_name = file_name_current.substr(0, self_name.size());
			const string file_name_id = file_name_current.substr(self_name.size(), file_name_current.size());

			if (file_name == self_name)
			{
#ifdef _WIN32
                Sleep(10);
#elif __APPLE__
                
                usleep(10);
#endif
			
				vector<string> lines = read_text_file(ipc_path + "\\" + file_name_current);
				delete_file(ipc_path + "\\" + file_name_current);
				vector<string> message_vec = split_string(lines[0], "!");
				const string message_head = message_vec[0];
				const string message_body = message_vec[1];

				if (lines.size() == 1)
				{
					if (command_map.count(message_head))
						command_map[message_head](message_body, file_name_id);
				}
				else if (lines.size() > 1)
				{
					const int id = atoi(lines[1].c_str());
					if (response_map_checker[id] == true)
					{
						response_map_checker[id] = false;
						response_map[id](message_body);
					}
				}
			}
		}
}

int IPC::send_message(const string recipient, const string message_head, const string message_body)
{
	const string file_name = recipient;
	vector<string> file_name_vec = list_files_in_directory(ipc_path);
	bool file_count_array[100];
	int file_count = 0;

	for (string file_name_current : file_name_vec)
		if (file_name_current.size() >= file_name.size())
		{
			const string file_name_substring = file_name_current.substr(0, file_name.size());
			if (file_name_substring == file_name)
			{
				const string used_file_num_str = file_name_current.substr(file_name.size(), file_name_current.size());
				const int used_file_num = atoi(used_file_num_str.c_str());
				file_count_array[used_file_num] = true;
				++file_count;
			}
		}

	if (file_count < 100)
	{
		int file_count_current = 0;

		while (file_count_array[file_count_current] == true)
			++file_count_current;

		write_string_to_file(ipc_path + "\\" + file_name + to_string(file_count_current), message_head + "!" + message_body);	
		return file_count_current;
	}
	return -1;
}

void IPC::get_response(const string recipient, const string message_head, const string message_body,
					   function<void (const string message_body)> callback)
{
	const int id = send_message(recipient, message_head, message_body);

	if (id != -1)
	{
		response_map[id] = callback;
		response_map_checker[id] = true;
	}
}

void IPC::map_function(const string message_head, function<void (const string message_body, const string id)> callback)
{
	command_map[message_head] = callback;
}

void IPC::open_udp_channel(const string recipient)
{
	UDP* udp_ptr = &udp;
	bool* udp_ready_ptr = &udp_ready;
	get_response(recipient, "open udp channel", "", [udp_ptr, udp_ready_ptr](const string message_body)
	{
		const int port = atoi(message_body.c_str());
		udp_ptr->set_port(port);
		*udp_ready_ptr = true;
	});
}

void IPC::send_udp_message(const string message_head, const string message_body)
{
	if (udp_ready)
		udp.send_message(message_head + "!" + message_body);
}