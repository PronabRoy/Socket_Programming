#include "Server.h" 


Server::Server(std::string ip, int port)
{
	this->_ip = ip;
	this->_port = port;
	myServer = this;

	_file.push_back("Song0.mp4");
	_file.push_back("Song1.mp4");
	_file.push_back("Song2.mp4");
	_file.push_back("images.jpg");
	_file.push_back("C++.png"); 
	_file.push_back("Cline.rar");
}

bool Server::initSocket()
{
	//The highest version of Windows Sockets specification that the caller can use
	WORD var = MAKEWORD(2, 1);
	//The WSAStartup function initiates use of the Winsock DLL by a process
	//The WSAStartup function is called to initiate use of WS2_32.dll.
	//If successful this function return a non-zero value.
	int wsOk = WSAStartup(var, &_wsData);
	if (wsOk != 0)
	{
		cerr << "Can't initialize wsData" << endl;
		return 0;
	}
	else
		cout << "Initialize wsData successfully" << endl;
		
	_serverAddr.sin_family = AF_INET;					 // host byte order 
	_serverAddr.sin_port = htons(_port);
	_serverAddr.sin_addr.s_addr = htons(INADDR_ANY);	///use my IP adr

	//Create socket ................................
	_listeing = socket(AF_INET, SOCK_STREAM, 0);
	if (_listeing == INVALID_SOCKET)
	{
		cout << "Can't create Socket!" << endl;
		return false;
	}

	//Bind socket...................................
	if (bind(_listeing, (sockaddr*)&_serverAddr, sizeof(_serverAddr)) == INVALID_SOCKET)
	{
		cout << "Binding Problem!" << endl;
		return false;
	}

	//Listening......................................
	if (listen(_listeing, SOMAXCONN) == SOCKET_ERROR)
	{
		cout << "Listening problem!" << endl;
		return false;
	}


	return true;
}

void Server::acceptRequest()
{
	//Wait for a connection form cline .......................
	//........................................................................
	sockaddr_in cline;
	int clineSize = sizeof(cline);
	for (int i = 0; i < TOTAL_CONNECTION; i++)
	{
		SOCKET clineSocket = accept(_listeing, (sockaddr*)&cline, &clineSize);

		//To convert binary IP to string: inet_noa() 
		cout << "Ip addr : " <<  inet_ntoa(cline.sin_addr) << endl;

		if (clineSocket == 0)
		{
			cout << "Faild to accept cline request!" << endl;
		}
		else
		{ 
			PacketType p = PacketType::p_connected;
			send(clineSocket, (char*)&p, sizeof(p), 0);
			_connection[i].sock = clineSocket;
			connectedSockNum++;
			//Creating server thread for eatch client.
			_connection[i].serverThread = std::thread(clineHandlarThread, i, this);
		}
	}

}
 
void clineHandlarThread(int index, Server* server)
{ 
	PacketType packetType;
	while (true)
	{ 
		if (!recvAll(server->_connection[index].sock, (char*)&packetType, sizeof(packetType)))
			break;
		 
		std::lock_guard<std::mutex> lockGuard(server->_serverMutex);
		if (!server->processMessage(packetType, index)) 
			break;  

	}

	cout << "Lost clint: " << index << endl;
	//close socket 
	closesocket(server->_connection[index].sock);	
}

bool Server::processMessage(PacketType packetType, SOCKET index)
{ 
	int sendData = 0;
	PacketType pacTyp;
	

	char msg[SIZE_OF_MSG];
	//_file type of message we get from clint.
	switch (packetType)
	{
		case p_message:
		{
			//First received message form a cline and send that message all of it's cline.
			if (!recvAll(_connection[index].sock, msg, sizeof(msg)))
				return false;
			//Send all of its cline
			for (int i = 0; i < connectedSockNum; i++)
			{
				if (i != index)
				{
					//First send type of PacketType that is sending and then send actual message.
					sendAll(_connection[i].sock, (char*)&packetType, sizeof(packetType));
					sendAll(_connection[i].sock, msg, sizeof(msg));
				}
			}
			break;
		}

		case p_requestForCam:
		{
			//For stream
			_stream.cap = cv::VideoCapture(0);


			//For stream setup.............................. 
			if (!_stream.cap.read(_stream.frame))
				std::cout << "Faild to read image!" << endl;

			_stream.frameDataSize = _stream.frame.total() * _stream.frame.elemSize();	//Total size of data for one frame.
			_stream.c = new char[_stream.frameDataSize];
			_stream.height = _stream.frame.size().height;
			_stream.width = _stream.frame.size().width; 
			
			pacTyp = PacketType::p_setupCam;
			sendAll(_connection[index].sock, (char*)&pacTyp, sizeof(pacTyp));

			//Sending setup data...................
			sendAll(_connection[index].sock, (char*)&_stream.height, sizeof(int));
			sendAll(_connection[index].sock, (char*)&_stream.width, sizeof(int));
			sendAll(_connection[index].sock, (char*)&_stream.frameDataSize, sizeof(int));
			break;
		}

		case p_ackForCam:
		{
			//For stream.............................. 
			if (!_stream.cap.read(_stream.frame))
				std::cout << "Faild to read image!" << endl;

			memcpy(_stream.c, _stream.frame.data, _stream.frameDataSize);

			pacTyp = PacketType::p_fileTransmitionForCam;
			sendAll(_connection[index].sock, (char*)&pacTyp, sizeof(pacTyp));

			sendAll(_connection[index].sock, _stream.c, _stream.frameDataSize);
			break;
		}

		case p_endOfVideoChat:
		{
			_stream.cap.~VideoCapture();
			std::cout << "End off Video chat" << std::endl;
			break;
		}
		
		case p_fileRequest:
		{ 
			if(recv(_connection[index].sock, _connection[index].file.fileName, sizeof(_connection[index].file.fileName), 0) == SOCKET_ERROR)
				return false;

			//_file have all avalable file in server.So we need to loop through all file with client requested file.
			for (int i = 0; i < _file.size(); i++)
			{
				if (checkMsg(_connection[index].file.fileName, _file[i]))
				{  
					//First send acknolwagement that we are ready to send data, and then send the name of file we are sending.
					pacTyp = PacketType::p_readyToTransmitFile;
					sendAll(_connection[index].sock, (char*)&pacTyp, sizeof(pacTyp));
					sendAll(_connection[index].sock, _connection[index].file.fileName, sizeof(_connection[index].file.fileName));  
					return true;
				} 
			}

			pacTyp = PacketType::p_unknownFileName;
			sendAll(_connection[index].sock, (char*)&pacTyp, sizeof(pacTyp));
			break;
		}

		case p_ackOkForFileRequest:
		{    
			//Sending file
			std::cout << "sending .........." << std::endl;
			if (!openFile(_connection[index].file))
				break;

			std::cout << _connection[index].file.remainingByte << std::endl;

			pacTyp = PacketType::p_fileTransmiting;
			sendAll(_connection[index].sock, (char*)&pacTyp, sizeof(pacTyp));
			sendAll(_connection[index].sock, _connection[index].file.memory, sizeof(_connection[index].file.memory));
			sendAll(_connection[index].sock, (char*)&_connection[index].file.remainingByte, sizeof(_connection[index].file.remainingByte));						//send remaining byte.
			sendAll(_connection[index].sock, (char*)&_connection[index].file.fileSize, sizeof(_connection[index].file.fileSize));
			break;
		}//File images.jpg

		case p_ackForEndFile:
		{
			pacTyp = PacketType::p_ackForEndFile;
			sendAll(_connection[index].sock, (char*)&pacTyp, sizeof(pacTyp));

			_connection[index].file.empty(); 
			break;
		}

		default:
		{
			cout << "Can't process this message!" << endl;
			break;
		}
		

	}

	return true;

}

Server::~Server()
{ 
	for (int index = 0; index < connectedSockNum; index++)
	{
		if (_connection[index].file.inputFile.is_open())
			_connection[index].file.inputFile.close();

		if (_connection[index].file.outputFile.is_open())
			_connection[index].file.outputFile.close();
	}
	//Free all heap memory.
	delete[] _connection;
	
}

/***************************************************
sendto() and recvfrom() - DGRAM style

int sendto(int sockfd, const void *msg, int len, int flags, 
const struct sockaddr *to, int tolen); – to is a pointer to a 
struct sockaddr which contains the 
destination IP and port – tolen is sizeof(struct sockaddr) 

● int recvfrom(int sockfd, void *buf, int len, int flags, 
struct sockaddr *from, int *fromlen); – from is a pointer
to a local struct sockaddr that will be filled with IP address
and port of the originating machine – fromlen will contain length of address stored in from
*****************************************************/
