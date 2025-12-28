#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <boost/asio.hpp>
#include <deque>

using boost::asio::ip::tcp;

struct pack
{
	char name[32];
	char msg[256];
};

class Session;

class Server
{
public:
	Server(boost::asio::io_context& io, uint16_t port);

	bool TryLogin(const std::string& name, std::shared_ptr<Session> session);

	void BroadCast(pack packet);
private:
	void DoAccept();

	std::unordered_map<std::string, std::shared_ptr<Session>> m_list;
	tcp::acceptor m_Acceptor;
};



//////////////////////////////////////////////////////////////////////////////////////////


class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(tcp::socket socket, Server& server)
		: m_Socket(std::move(socket)),
		m_Server(server)
	{
	}

	void Start();
	void DoRead();
	void WriteLoginFailed();
	void WriteLoginSuccess();

	
	void Send(const pack& packet);
	void DoWrite();

private:
	void Close();

	tcp::socket m_Socket;

	pack m_ReadBuf;

	Server& m_Server;

	std::deque<pack> m_WriteQueue;
	bool m_Writing = false;
};