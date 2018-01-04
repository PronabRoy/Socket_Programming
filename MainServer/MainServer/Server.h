#pragma once
//This is for use "Winsock" function.
#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable : 4996)
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <vector>
#include <WinSock2.h>    
#include <thread>
#include <mutex>
#include "FileInfo.h"
#include "SendAndRecv.h"

#define TOTAL_CONNECTION 100
#define SIZE_OF_MSG 255
 
using std::cout;
using std::endl;
using std::cerr;

//This function belong to "HelperFunction.cpp" file.
//This functin Check two array of char.
bool checkMsg(char*, char*);
void copyMsg(char* input, char* output);
bool openFile(FileInfo&);
bool writeFile(FileInfo& fileInfo);

enum PacketType {
	p_message,
	p_connected,
	p_fileRequest,
	p_unknownFileName,
	p_readyToTransmitFile,
	p_ackOkForFileRequest,
	p_fileTransmiting,
	p_ackForEndFile,  
	p_requestForCam,
	p_setupCam,
	p_ackForCam,
	p_fileTransmitionForCam,
	p_endOfVideoChat
};

struct Connection {
	SOCKET sock;
	FileInfo file;
	std::thread serverThread;
};

class Server
{
public:
	Server(std::string, int);
	bool initSocket();
	void acceptRequest();
	friend void clineHandlarThread(int, Server*);
	~Server();

	int connectedSockNum = 0; 

private:
	std::string _ip;
	int _port;
	sockaddr_in _serverAddr;
	SOCKET _listeing; 
	std::mutex _serverMutex;

	//Initialize ......................................
	//A pointer to the WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSAData  _wsData;
	
	Connection* _connection = new Connection[TOTAL_CONNECTION];
	stream _stream;
	std::vector<char*> _file; 
	
	//Function....................................
	bool processMessage(PacketType , SOCKET );
};

static Server* myServer;





