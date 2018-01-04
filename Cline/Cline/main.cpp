#include "Client.h"			

int main()
{

	std::string ip = "127.0.0.1";
	int port = 1111;
	
	std::cout << "To Download any File form Server 1st Wtite -> File then enter" << std::endl;
	std::cout << "Then Write file full name then enter" << std::endl;
	std::cout << "For Example : File (Enter) C++.png" << std::endl;
	std::cout << "See Downloaded File in Cline\file folder\n" << std::endl;
	std::cout << "To Open Camera in Server side write -> cam" << std::endl;
	std::cout << "Press Ecs to close camera\n" << std::endl;

	Client client(ip, port);
	if (client.connectServer() == true)
		client.sendNRecv();
	else
	{
		cout << "Unable to connect server" << endl; 
	}
	
		
	std::cin.get();
	return 0;
}







