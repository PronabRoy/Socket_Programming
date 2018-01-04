#include "Server.h"
 
int main()
{
	Server mainServer("127.0.0.1", 1111);
	
	std::cout << "To add file in server you need to do it manualy" << std::endl;
	std::cout << "Goto MainServer >> file folder." << std::endl;
	std::cout << "put any file there and in Server class Constructor " << std::endl;
	std::cout << "use this commend  _file.push_back(file name);\n" << std::endl;
	
	if (!mainServer.initSocket())
		return 0;

	mainServer.acceptRequest();


 	std::cin.get();
	return 0;
}

/******************************************************
To Connect Different user form diffrent computer in diffrent rouer :-
>> 1st go to router .
>> Port Forwarding (virtual server).
>> Add New.
>> Service of Internal Port = "Port".
>> Ip Address = IPV4 (Goto ipconfig).


*********************************************
ipconfig of ipconfig /all.
netstat -an --->>> to see all open port in your computer.
tracert	(google.com or any IPV4)	--->>> It show step by step route a packet takes to the destination.
nslookup (Domain name or IPV4)	--->>> This command give domain name or ip address stored in DNS server.
******************************************************/
