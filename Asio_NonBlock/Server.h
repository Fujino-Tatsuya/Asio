#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Session;

class Server
{
public:
	Server(boost::asio::io_context& io, uint16_t port);

	bool TryLogin(const std::string& name, std::shared_ptr<Session> session);

	void BroadCast(const std::string& msg);
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
	void Deliver(std::string msg);
private:

	void DoWrite(std::size_t length);

	

	tcp::socket m_Socket;
	char m_Data[1024];

	std::array<char, 1024> m_ReadBuf;

	Server& m_Server;
};