#include <iostream>
#include<chrono>
#include<cstdint>
#include<string>
#include<vector>
#include<fstream>
#include<algorithm>
#include"IClient.h"
#include"../common/message.h"


class CustomClient : public client_interface<CustomMsgTypes>{
private:
	std::vector<uint32_t> connectedID;
public:
	void MessageAll(const std::string &content){
		message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageAll;	
		for(auto &i : content){
			msg << i;
		}	
		Send(msg);
	}

	void ConnectMsg(const std::string &destination){
		message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ConnectToIP;
		for(auto i : destination){
			msg << i;
		}
		Send(msg);
	}
	void sendTextMsg(const std::string &content){
		message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::TextMessage;
		for(auto i : content){
			msg << i;
		}
		Send(msg);
	}
	void sendFile(const std::string &path,const std::string &fileName){
		message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::FileTranfer;
		msg.fileName = fileName;
		std::ifstream file(path, std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Failed to open file.\n";
		}
		std::vector<char> buffer(std::istreambuf_iterator<char>(file),{});
		file.close();
		for(auto i : buffer){
			msg << i;
		}
		Send(msg);
	}

	void readMsg(message<CustomMsgTypes> &msg) {
		std::string content;
		uint32_t ID;
		msg >> ID;
		for(auto i : msg.body){
			content.push_back(i);
		}
		std::cout<<"["<<ID<<"]: "<<std::flush;
		std::cout<<content<<"\n"<<std::flush;
	}
};



int main(){
	CustomClient c;
	c.Connect("127.0.0.1", 60000);

	std::thread thr([&](){
		while (true){
				if (c.IsConnected()){
					if (!c.Incoming().empty()){
						auto msg = c.Incoming().pop_front().msg;
						switch (msg.header.id){
							case CustomMsgTypes::ServerAccept:{
								// Server has responded to a ping request				
								std::cout << "Server Accepted Connection\n"<<std::flush;
								std::cout<<"Please enter your IP which you want to connect\n"<<std::flush;
								break;
							}
							case CustomMsgTypes::ServerMessage:{
								// Server has responded to a ping request	
								uint32_t clientID;
								msg >> clientID;
								std::cout << "Hello from [" << clientID << "]\n";
								break;
							}
							case CustomMsgTypes::MessageAll : {
								c.readMsg(msg);
								break;
							}
							case CustomMsgTypes::ConnectToIP:{
								std::string content;
								for(auto i : msg.body){
									content.push_back(i);
								}
								std::cout<<content<<"\n";
								break;
							}
							case CustomMsgTypes::TextMessage:{
								// std::string content;
								// uint32_t ID;
								// msg >> ID;
								// for(auto i : msg.body){
								// 	content.push_back(i);
								// }
								// std::cout<<"["<<ID<<"]: "<<std::flush;
								// std::cout<<content<<"\n"<<std::flush;
								c.readMsg(msg);
								break;
							}
							case CustomMsgTypes::FileTranfer:{
								std::vector<char> content;
								uint32_t ID;
								msg >> ID;
								for(auto i : msg.body){
									content.push_back(i);
								}
								// std::string path = "/mnt/c/Users/DO MINH DUC/Desktop/";
								path += msg.fileName;
								std::ofstream outputFile(path,std::ios::binary);

								if (!outputFile.is_open()) {
									std::cerr << "Failed to open output file." << std::endl;								}

								for (char c : content) {
									outputFile.put(c);
								}
								
								outputFile.close();
								path.clear();
								std::cout<<"["<<ID<<"]:Sent a file with name:"<<msg.fileName<<"\n"<<std::flush;
								break;
							}
						}
					}
				}
				else{
					std::cout << "Server Down\n";
					break;
				}
			}
	});

	thr.detach();


	std::string ip;
	std::getline(std::cin,ip);
	c.ConnectMsg(ip);

	while (true){
		//std::cout<<"Please enter your cmd\n";
		std::string cmd;
		std::string type;
		int start = 0;
		int stop = 0;
		std::getline(std::cin,cmd);
		for(size_t i=0;i<cmd.size();i++){
			if(cmd[i] == ' '){
				stop = i - 1;
				for(size_t j = start ;j <= stop;j++){
					type.push_back(cmd[j]);
				}
				break;
			}
		}
		start = stop + 2;
		if(type == "SENDTEXT"){
			std::string content;
			for(size_t i = start;i<cmd.size();i++){
				content.push_back(cmd[i]);
			}
			c.sendTextMsg(content);
		}
		else if(type == "SENDTEXTALL"){
			std::string content;
			for(size_t i = start;i<cmd.size();i++){
				content.push_back(cmd[i]);
			}
			c.MessageAll(content);
		}
		else if(type == "SENDFILE"){
			std::string path;
			std::string fileName;
			for(size_t i = start;i<cmd.size();i++){
				path.push_back(cmd[i]);
			}
			for(size_t i = path.size() - 1 ;i>=0;i--){
				if(path[i] == '/' || path[i] == '\\') {
					break;
				}
				fileName.push_back(path[i]);
			}
			std::reverse(fileName.begin(),fileName.end());
			c.sendFile(path,fileName);
		}
		else if(type == "CONNECT"){
			std::string newIP;
			for(size_t i = start;i<cmd.size();i++){
				newIP.push_back(cmd[i]);
			}
			c.ConnectMsg(newIP);
		}
		else{
			std::cout<<"Error cmd\n";
		}
	}
	
	return 0;
}