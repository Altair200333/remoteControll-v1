#include <iostream>
#include <stdio.h>
#include <string>
#include <windows.h>
#include <WS2tcpip.h>
#include <sstream>
#include <fstream>

#include "SocketClient.h"

std::vector<std::string> split(std::string str)
{
	std::istringstream iss(str);
	std::vector<std::string> tokens;
	std::copy(std::istream_iterator<std::string>(iss),
		std::istream_iterator<std::string>(),
		back_inserter(tokens));
	return tokens;
}

int main()
{
    SocketClient client;
    client.init();

	std::cout << "Enter commands( -h for help)\n";
	
	bool running = true;
	while(running)
	{
		std::cout << "$>>";
		std::string command;
		
		std::getline(std::cin >> std::ws, command);
		
		auto cmds = split(command);
		if(cmds[0] == "con")
		{
			if(cmds.size() == 2)
			{
				client.address = cmds[1];
			}
			else if(cmds.size() == 3)
			{
				client.port = cmds[2];
			}
			client.remConnect();
		}
		else if(command == "discon")
		{
			client.closeConnection();
		}
		else if(command == "recon")
		{
			client.closeConnection();
			client.remConnect();
		}
		else if (command == "exit")//HALT
		{
			running = false;
		}
		else if (cmds[0] == "get")
		{
			auto response = client.sendData(command);
			if(response[0] == 1)
			{
				int size = *reinterpret_cast<int*>(&response[1]);

				std::cout << "File received; bytes - "<<size<<"\n";

				std::ofstream file(cmds[1], std::ios::binary);
				file.write(&response[5], size);
			}
			else
			{
				std::string resp = !response.empty() ? std::string(response.data()) : "<empty>";
				std::cout << resp << "\n";
			}
		}
		else
		{
			auto recv = client.sendData(command);
			std::string resp = !recv.empty()? std::string(recv.data()):"<empty>";
			std::cout << resp << "\n";
		}
	}
    client.closeConnection();
    return 0;
}