
#include <iostream>
#include "Server.h"

Server::Server(boost::asio::io_context& io, uint16_t port)
	: m_Acceptor(io, tcp::endpoint(tcp::v4(), port))
{
	DoAccept();
}

bool Server::TryLogin(const std::string& name,
	std::shared_ptr<Session> session)
{
	if (m_list.find(name) != m_list.end())
		return false;

	m_list[name] = session;
	session->DoRead();
	std::cout << name << " login success" << std::endl;
	return true;
}

void Server::BroadCast(const std::string& msg)
{
	for (auto& Session : m_list)
	{
		Session.second->Deliver(msg);
	}
}

void Server::DoAccept()
{
	m_Acceptor.async_accept(
		[this](boost::system::error_code ec, tcp::socket socket)
		{
			if (!ec)
			{
				auto session =
					std::make_shared<Session>(std::move(socket), *this);
				session->Start();
			}

			DoAccept();
		});
}



/////////////////////////////////////////////////////////////////////////////////////////



void Session::Start()
{
	auto self = shared_from_this();
	m_Socket.async_read_some(boost::asio::buffer(m_ReadBuf),
		[this, self](boost::system::error_code ec, std::size_t len)
		{
			std::string name(m_ReadBuf.data(), len);
			if (m_Server.TryLogin(name, self))
			{
				DoRead();
			}
			else
			{
				std::cout << "login Failed" << std::endl;
			}
		});
}

void Session::DoRead()
{
	auto self = shared_from_this();
	m_Socket.async_read_some(
		boost::asio::buffer(m_Data),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				DoWrite(length);
			}
			else
			{
				// Client disconnected
			}
		}
	);
}

void Session::DoWrite(std::size_t length)
{
	auto self = shared_from_this();
	boost::asio::async_write(
		m_Socket,
		boost::asio::buffer(m_Data, length),
		[this, self](boost::system::error_code ec, std::size_t /*len*/)
		{
			if (!ec)
			{
				DoRead(); // 계속 읽기
			}
		}
	);
}

void Session::Deliver(std::string msg)
{

}
