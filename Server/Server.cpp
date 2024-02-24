#include<cstdint>
#include<memory>
#include<string>
#include<vector>
#include"IServer.h"
#include"../common/connection.h"
#include"../common/message.h"
#include"../common/tsqueue.h"


class CustomServer : public server_interface<CustomMsgTypes>{
public:
	std::multimap<uint32_t,uint32_t> pairConnection;
	CustomServer(uint16_t nPort) : server_interface<CustomMsgTypes>(nPort){

	}
	void sendToClient(std::shared_ptr<connection<CustomMsgTypes>> client, message<CustomMsgTypes>& msg){
		for(auto &i : pairConnection){
			if(i.first == client->GetID()){
				for(auto &con : m_deqConnections){
					if(con->GetID() == i.second){
						msg << i.first;
						con->Send(msg);
					}
				}
			}
		}
	}


protected:
	virtual bool OnClientConnect(std::shared_ptr<connection<CustomMsgTypes>> client){
		message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr<connection<CustomMsgTypes>> client){
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr<connection<CustomMsgTypes>> client, message<CustomMsgTypes>& msg){
		switch (msg.header.id){
			case CustomMsgTypes::ServerPing:{
				std::cout << "[" << client->GetID() << "]: Server Ping\n";

				// Simply bounce message back to client
				client->Send(msg);
				break;
			}
			case CustomMsgTypes::MessageAll:{
				std::cout << "[" << client->GetID() << "]: Message All\n";
				// // Construct a new message and send it to all clients
				// message<CustomMsgTypes> msgTemp;
				// msgTemp.header.id = CustomMsgTypes::MessageAll;
				msg << client->GetID();
				MessageAllClients(msg, client);
				break;
			}
			case CustomMsgTypes::ConnectToIP:{
				std::string IP;
				std::string port;
				bool check = false;
				for(auto i : msg.body){
					if(i == ':'){
						check = true;
						continue;
					}
					if(check == false){
						IP.push_back(i);
					}
					else{
						port.push_back(i);
					}
				}
				std::cout << "[" << client->GetID() << "]: Would like to connect to another\n";
				
				bool checkConnect = false;
				message<CustomMsgTypes> response;
				std::string content;
				for(auto &i : m_deqConnections){
						if(i->getEndPoint() == asio::ip::tcp::endpoint(asio::ip::address::from_string(IP),std::stoi(port))){
							pairConnection.insert(std::pair<uint32_t,uint32_t>(client->GetID(),i->GetID()));
							content = "Connected with Client " + IP + ":" +port;
							response.header.id = CustomMsgTypes::ConnectToIP;
							for(auto &i : content){
								response << i;
							}
							client->Send(response);
							checkConnect = true;
							break;
						}
				}
				if(!checkConnect){
					content = "Failed to connect client: " + IP + ":" +port;
					response.header.id = CustomMsgTypes::ConnectToIP;
					for(auto &i : content){
						response << i;
					}
					client->Send(response);
				}
				break;
			}
			case CustomMsgTypes::TextMessage : {
				std::cout<<"["<<client->GetID()<<"] sent a message\n"<<std::flush;
				sendToClient(client,msg);
				break;
			}
			case CustomMsgTypes::FileTranfer : {
				std::cout<<"["<<client->GetID()<<"] sent a file:"<<msg.fileName<<"\n"<<std::flush;
				sendToClient(client,msg);
				break;
			}
		}
	}
};

int main()
{
	CustomServer server(60000); 
	server.Start();

	while (1){
		server.Update(-1, true);
	}
	
	return 0;
}