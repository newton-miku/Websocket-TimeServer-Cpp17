//#include "stdafx.h"
#include <iostream>
#include <thread>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using namespace std;
typedef websocketpp::server<websocketpp::config::asio> WebsocketServer;
typedef WebsocketServer::message_ptr message_ptr;
typedef websocketpp::server<websocketpp::config::asio> server;
using ConnectionHdl = websocketpp::connection_hdl;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
server m_server;
string mssg = "Time:";
std::vector<ConnectionHdl> connections;
bool running = false;

void OnOpen(WebsocketServer* server, websocketpp::connection_hdl hdl, std::vector<ConnectionHdl> *connections)
{
	string mssg = u8"连接成功Time:";
	mssg.append(std::to_string(time(0)));
	server->send(hdl, mssg, websocketpp::frame::opcode::text);
	server::connection_ptr con = m_server.get_con_from_hdl(hdl);
	string client_ip_port = con->get_remote_endpoint();
	connections->push_back(hdl);
	cout << "have client connected" << client_ip_port << endl;
}
bool equal_connection_hdl(ConnectionHdl& a, ConnectionHdl& b) {
	return a.lock() == b.lock();
}
void remove_connection(std::vector<ConnectionHdl>* connections,
	ConnectionHdl& hdl) {
	auto equal_connection_hdl_predicate = std::bind(&equal_connection_hdl, hdl, ::_1);
	connections->erase(std::remove_if(std::begin(*connections), std::end(*connections),
		equal_connection_hdl_predicate),std::end(*connections));
}

void OnClose(WebsocketServer* server, websocketpp::connection_hdl hdl, std::vector<ConnectionHdl>* connections)
{
	cout << "have client disconnected" << endl;
	remove_connection(connections, hdl);

}

string GBK_2_UTF8(string gbkStr)
{
	string outUtf8 = "";
	int n = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
	WCHAR* str1 = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char* str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	outUtf8 = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return outUtf8;
}

void OnMessage(WebsocketServer* server, websocketpp::connection_hdl hdl, message_ptr msg, std::vector<ConnectionHdl>* connections)
{
	server::connection_ptr con = m_server.get_con_from_hdl(hdl);
	string client_ip_port = con->get_remote_endpoint();
	string strMsg = msg->get_payload();
	cout << "收到来自" << client_ip_port << "的：" << strMsg << endl;

	string strRespon = "receive: ";
	strRespon.append(strMsg);
	//strRespon += "\n";
	server->send(hdl, (strRespon), websocketpp::frame::opcode::text);
	server->send(hdl, GBK_2_UTF8(mssg), websocketpp::frame::opcode::text);
}
void setTime(PVOID pParam) {
	WebsocketServer* servers = (server*) pParam;
	while (running) {
		mssg = "这是一个测试消息:";
		mssg += "<br>Time:";
		mssg.append(std::to_string(time(0)));
		//mssg = GBK_2_UTF8(mssg);
		for (auto& connection : connections) {
			servers->send(connection, GBK_2_UTF8(mssg), websocketpp::frame::opcode::text);
			this_thread::sleep_for(chrono::seconds(1));
		}
	}
}
void serverFunc() {
	m_server.set_access_channels(websocketpp::log::alevel::none); // 设置打印的日志等级

	m_server.init_asio(); // 初始化asio

	m_server.set_open_handler(bind(&OnOpen, &m_server, ::_1, &connections)); // 绑定websocket连接到来时的回调函数
	m_server.set_close_handler(bind(&OnClose, &m_server, ::_1, &connections)); // 绑定websocket连接断开时的回调函数
	m_server.set_message_handler(bind(&OnMessage, &m_server, ::_1, ::_2, &connections)); // 绑定websocket连接有消息到来时的回调函数
	m_server.listen(8090);
	m_server.start_accept();
	m_server.run();
}
int main1()
{
	running = true;
	//SetConsoleOutputCP(65001);
	/*
	thread time_th(setTime, &m_server);
	time_th.detach();
	/**/
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE); //获取标准输出句柄
	SetConsoleTextAttribute(hOutput, FOREGROUND_GREEN); //设置文本颜色红色
	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(setTime), &m_server, NULL, nullptr);
	SetLayeredWindowAttributes(GetConsoleWindow(), RGB(12,12,12), 0, LWA_COLORKEY);
	//默认黑窗口的透明
	SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 600, 400, SWP_NOMOVE );
	//设置窗口置顶
	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(serverFunc), &m_server, NULL, nullptr);
	//serverFunc();//不可在子函数中创建子线程并分离
	//printf("服务器运行中");
	/*WebsocketServer          server;

	// Set logging settings
	server.set_access_channels(websocketpp::log::alevel::all);
	server.clear_access_channels(websocketpp::log::alevel::frame_payload);
	server.set_access_channels(websocketpp::log::alevel::none); // 设置打印的日志等级

	// Initialize ASIO
	server.init_asio();

	// Register our open handler
	server.set_open_handler(bind(&OnOpen, &server, ::_1));

	// Register our close handler
	server.set_close_handler(bind(&OnClose, &server, _1));

	// Register our message handler
	server.set_message_handler(bind(&OnMessage, &server, _1, _2));


	//Listen on port 2152
	server.listen(8090);

	//Start the server accept loop
	server.start_accept();

	//Start the ASIO io_service run loop
	server.run();
	*/

	return 0;
}
void main() {
	/*
	thread ser_th(serverFunc);
	ser_th.detach();
	/**/
	main1();
	while (1) {
		cout << "服务器正在运行中" << endl;
		this_thread::sleep_for(chrono::seconds(5));
	}
}