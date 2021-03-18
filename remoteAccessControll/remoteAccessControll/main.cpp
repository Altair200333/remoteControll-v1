#include <filesystem>
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
		else if (cmds[0] == "send")
		{
			if (cmds.size() != 2)
			{
				std::cout << "What do you want to send\n";
			}
			else
			{
				if (!exists_test3(cmds[1]) || std::filesystem::is_directory(cmds[1]))
				{
					std::cout<< "File " + cmds[1] + " not found bruh\n";
				}
				else
				{
					auto data = readFile(cmds[1].c_str());
					auto fileSize = intToBytes(data.size());
					auto filenameLength = intToBytes(cmds[1].length());
					
					std::vector<char> result = {'s', 'e', 'n', 'd'};

					std::string str = cmds[1];
					std::vector<char> filename;
					std::copy(str.begin(), str.end(), std::back_inserter(filename));
					
					result.insert(result.end(), filenameLength.begin(), filenameLength.end());
					result.insert(result.end(), filename.begin(), filename.end());

					result.insert(result.end(), fileSize.begin(), fileSize.end());
					result.insert(result.end(), data.begin(), data.end());

					auto response = client.sendData(result);
					std::string resp = !response.empty() ? std::string(response.data()) : "<empty>";
					std::cout << resp << "\n";
				}
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