// simple_webserver.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <WinSock2.h>
#include <fstream>

#pragma comment(lib,"ws2_32.lib")

#define FAST_CGI_IP "127.0.0.1"
#define FAST_CGI_PORT 9000


void catHTML(SOCKET *client, char *filename);
void catJPG(SOCKET *client, char *filename);
void catPNG(SOCKET *client, char *filename);
void catICO(SOCKET *client, char *filename);
void catPHP(SOCKET *client, char *filename, const char *query);
void sendError(SOCKET *client);
void sendData(SOCKET *client, char *filename);
void requestHandle(SOCKET *client);
void errorMessage(const char *message);

int main()
{
	WORD version = MAKEWORD(1, 1);
	WSADATA wsa;
	if (WSAStartup(version, &wsa) != 0)
	{
		errorMessage("WSA error");
	}

	SOCKADDR_IN serveraddr;
	SOCKADDR_IN clientaddr;
	SOCKET listenfd;
	SOCKET clientfd;
	
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serveraddr.sin_port = htons(80);

	listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (listenfd == -1)
	{
		errorMessage("socket error");
	}
	if (bind(listenfd, (sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		errorMessage("bind error");
	}
	if (listen(listenfd, 1) < 0)
	{
		errorMessage("listen error");
	}
	std::cout << "Start" << std::endl;
	while (true)
	{
		int len = sizeof(clientaddr);
		if ((clientfd = accept(listenfd, (sockaddr *)&clientaddr, &len)) < 0)
		{
			errorMessage("accept error");
		}
		//memset(buf, 0, 1024);
		//recv(clientfd, buf, 1024, 0);
		//cout << buf << endl;
		//send(clientfd, respone, strlen(respone), 0);
		//closesocket(clientfd);
		requestHandle(&clientfd);
	}
	return 0;
}

void catHTML(SOCKET * client, char * filename)
{
	char buf[1024];
	std::ifstream file;

	char status[] = "HTTP/1.0 200 OK\r\n";
	char header[] = "Server: Simple WebServer\r\nContent-Type: text/html\r\n\r\n";

	file.open(filename, std::ios::in);
	if (!file.is_open())
	{
		status[9] = '4';
		status[11] = '4';
		errorMessage("open error");
		std::cout << file.rdstate() << std::endl;
	}

	send(*client, status, 18, 0);
	send(*client, header, sizeof(header), 0);

	while (file.peek() != EOF)
	{
		file.read(buf, 1024);
		send(*client, buf, 1024, 0);
	}
	file.close();
}

void catJPG(SOCKET * client, char * filename)
{
	char buf[1024];
	std::ifstream file;

	char status[] = "HTTP/1.0 200 OK\r\n";
	char header[] = "Server: Simple WebServer\r\nContent-Type: image/jpeg\r\n\r\n";

	file.open(filename, std::ios::in|std::ios::binary);
	if (!file.is_open())
	{
		status[9] = 4;
		status[11] = 4;
	}

	send(*client, status, 18, 0);
	send(*client, header, sizeof(header), 0);

	while (file.peek() != EOF)
	{
		file.read(buf, 1024);
		send(*client, buf, 1024, 0);
	}
	file.close();
}

void catPNG(SOCKET * client, char * filename)
{
	char buf[1024];
	std::ifstream file;

	char status[] = "HTTP/1.0 200 OK\r\n";
	char header[] = "Server: Simple WebServer\r\nContent-Type: image/png\r\n\r\n";

	file.open(filename, std::ios::in);
	if (!file.is_open())
	{
		status[9] = 4;
		status[11] = 4;
		errorMessage("open error");
	}

	send(*client, status, 18, 0);
	send(*client, header, sizeof(header), 0);

	while (file.peek() != EOF)
	{
		file.read(buf, 1024);
		send(*client, buf, 1024, 0);
	}
	file.close();
}

void catICO(SOCKET * client, char * filename)
{
	char buf[1024];
	std::ifstream file;

	char status[] = "HTTP/1.0 200 OK\r\n";
	char header[] = "Server: Simple WebServer\r\nContent-Type: image/x-icon\r\n\r\n";

	file.open(filename, std::ios::in);
	if (!file.is_open())
	{
		status[9] = 4;
		status[11] = 4;
		errorMessage("open error");
	}

	send(*client, status, 18, 0);
	send(*client, header, sizeof(header), 0);

	while (file.peek() != EOF)
	{
		file.read(buf, 1024);
		send(*client, buf, 1024, 0);
	}
	file.close();
}

void catPHP(SOCKET * client, char * filename, const char * query)
{
	SOCKET cgi;
	SOCKADDR_IN addr;

	char status[] = "HTTP/1.0 200 OK\r\n";
	char header[] = "Server: Simple WebServer\r\n";

	cgi = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (cgi == -1)
	{
		errorMessage("FAST-CGI Socket error");
		sendError(client);
		return;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(FAST_CGI_IP);
	addr.sin_port = htons(FAST_CGI_PORT);

	if (connect(cgi, (sockaddr*)&addr, sizeof(addr)) < 0)
	{
		errorMessage("FAST-CGI Connect error");
		sendError(client);
		return;
	}

	FCGI_BeginRequest beginRequest;
	beginRequest.header = makeHeader(FCGI_Request_Type::FCGI_BEGIN_REQUEST, 1, sizeof(beginRequest.body), 0);
	beginRequest.body = makeBeginRequestBody(FCGI_Role::FCGI_RESPONDER);
	if (send(cgi, (char*)&beginRequest, sizeof(beginRequest), 0) < 0)
	{
		errorMessage("FAST-CGI Send error");
		sendError(client);
		closesocket(cgi);
		return;
	}

	char msg[100];
	strcpy_s(msg, "D:\\wamp\\www\\");
	strcat_s(msg, filename);
	char *params[][2] = { {(char*)"SCRIPT_FILENAME",msg},{(char*)"REQUEST_METHOD",(char*)"GET"},{(char*)"QUERY_STRING",(char*)query},{(char*)"",(char*)""} };
	int contentLength, paddingLength;
	FCGI_Params *paramsMsg;
	for (size_t i = 0; params[i][0] != ""; i++)
	{
		contentLength = strlen(params[i][0]) + strlen(params[i][1]) + 2;
		paddingLength = (contentLength % 8) == 0 ? 0 : 8 - (contentLength % 8);
		paramsMsg = (FCGI_Params*)::operator new(sizeof(FCGI_Params) + contentLength + paddingLength);
		paramsMsg->keyLength = (unsigned char)strlen(params[i][0]);
		paramsMsg->valueLength = (unsigned char)strlen(params[i][1]);
		paramsMsg->header = makeHeader(FCGI_Request_Type::FCGI_PARAMS, 1, contentLength, paddingLength);
		memset(paramsMsg->data, 0, contentLength + paddingLength);
		memcpy(paramsMsg->data, params[i][0], strlen(params[i][0]));
		memcpy(paramsMsg->data + strlen(params[i][0]), params[i][1], strlen(params[i][1]));
		if (send(cgi, (char*)paramsMsg, 8 + contentLength + paddingLength, 0) < 0)
		{
			errorMessage("FAST-CGI Send error 2");
			sendError(client);
			closesocket(cgi);
			return;
		}
		delete paramsMsg;
	}

	FCGI_Header stdinHeader;
	stdinHeader = makeHeader(FCGI_Request_Type::FCGI_STDIN, 1, 0, 0);
	send(cgi, (char*)&stdinHeader, 8, 0);

	FCGI_Header responseHeader;
	char *message;
	if (recv(cgi, (char*)&responseHeader, sizeof(responseHeader), 0) < 0)
	{
		errorMessage("FAST-CGI Recv error");
		sendError(client);
		closesocket(cgi);
		return;
	}
	if (responseHeader.type == FCGI_Request_Type::FCGI_STDOUT)
	{
		contentLength = ((int)responseHeader.contentLength1 << 8) + (int)responseHeader.contentLength0;
		message = new char[contentLength];
		recv(cgi, message, contentLength, 0);

        send(*client, status, sizeof(status), 0);
	    send(*client, header, sizeof(header), 0);
		send(*client, message, contentLength, 0);
		
		delete message;
	}

	closesocket(cgi);
}

void sendError(SOCKET * client)
{
	char status[] = "HTTP/1.0 400 OK\r\n";
	char header[] = "Server: Simple WebServer\r\nContent-Type: text/html\r\n\r\n";

	send(*client, status, 18, 0);
	send(*client, header, sizeof(header), 0);
}

void sendData(SOCKET * client, char * filename)
{
	char file[40];
	char type[10];
	strcpy_s(file, filename);

	char *context;
	strtok_s(file, ".", &context);
	char *temp = strtok_s(NULL, ".?", &context);
	if (temp != nullptr)strcpy_s(type, temp);
	else strcpy_s(type, "");

	if (strcmp(type, "html") == 0)
	{
		catHTML(client, filename);
	}
	else if (strcmp(type, "jpg") == 0)
	{
		catJPG(client, filename);
	}
	else if (strcmp(type, "png") == 0)
	{
		catPNG(client, filename);
	}
	else if (strcmp(type, "php") == 0)
	{

		if (strstr(filename, "?") != NULL)
		{
			char query[40];
			char *context;
			strcpy_s(file, strtok_s(filename, "?", &context));
			temp = strtok_s(NULL, "?", &context);
			if (temp != nullptr)strcpy_s(query, temp);
			catPHP(client, filename, query);
		}
		else
		{
			catPHP(client, filename, "");
		}
	}
	else
	{
		//sendError(client);
	}
}

void requestHandle(SOCKET *client)
{
	char buf[1024];
	char method[10];
	char filename[40];

	recv(*client, buf, 1024, 0);

	if (strstr(buf, "HTTP/") == NULL)
	{
		closesocket(*client);
		return;
	}

	char *context;
	errorMessage("debuging");
	//std::cout << buf << std::endl;
	strcpy_s(method, strtok_s(buf, " /", &context));           //strtok only single thread
	strcpy_s(filename, strtok_s(NULL, " /", &context));
	
	if (strcmp(method,"GET")==0)
	{
		
		sendData(client, filename);
	}
	else
	{
		//closesocket(*client);
		//return;
	}
	closesocket(*client);
	std::cout << std::endl;
}

void errorMessage(const char *message)
{
	std::cout << message << std::endl;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
