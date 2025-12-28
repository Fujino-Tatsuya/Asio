
#include <iostream>
#include "Server.h"

Server::Server(boost::asio::io_context& io, uint16_t port)
	: m_Acceptor(io, tcp::endpoint(tcp::v4(), port))
{
	DoAccept();
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

bool Server::TryLogin(const std::string& name,
	std::shared_ptr<Session> session)
{
	if (m_list.find(name) != m_list.end())
	{
		session->WriteLoginFailed();

		return false;
	}

	m_list[name] = session;
	std::cout << name << ": login success" << std::endl;
	return true;
}

void Server::BroadCast(pack packet)
{
	for (auto& m : m_list)
	{
		if (strcmp(m.first.c_str(), packet.name) != 0)
		{
			m.second->Send(packet);
		}
	}
}



/////////////////////////////////////////////////////////////////////////////////////////



void Session::Start()
{
	auto self = shared_from_this();
	boost::asio::async_read(m_Socket, boost::asio::buffer(&m_ReadBuf, sizeof(pack)),
		[this, self](boost::system::error_code ec, std::size_t len)
		{
			if (m_Server.TryLogin(m_ReadBuf.name, self))
			{
				Session::WriteLoginSuccess();
				Session::DoRead();
			}
			else
			{
				Session::WriteLoginFailed();
				std::cout << m_ReadBuf.name << ": Login Failed" << std::endl;
			}
		});
}

void Session::DoRead()
{
	auto self = shared_from_this();
	boost::asio::async_read(m_Socket,
		boost::asio::buffer(&m_ReadBuf,sizeof(pack)),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				m_Server.BroadCast(m_ReadBuf);
				DoRead();
			}
			else
			{
				Session::Close();
			}
		}
	);
}

void Session::Close()
{
	boost::system::error_code ec;
	m_Socket.shutdown(tcp::socket::shutdown_both, ec);
	m_Socket.close(ec);
}


void Session::WriteLoginFailed()
{
	pack loginFailed{};
	strcpy_s(loginFailed.name, sizeof(loginFailed.name), "SERVER");
	strcpy_s(loginFailed.msg, sizeof(loginFailed.msg), "LOGIN_FAILED");

	auto self = shared_from_this();
	boost::asio::async_write(
		m_Socket, boost::asio::buffer(&loginFailed, sizeof(pack)),
		[this, self](boost::system::error_code ec, std::size_t /*len*/)
		{
			Session::Close();
		});
}

void Session::WriteLoginSuccess()
{
	pack loginSeccess{};
	strcpy_s(loginSeccess.name, sizeof(loginSeccess.name), "SERVER");
	strcpy_s(loginSeccess.msg, sizeof(loginSeccess.msg), "LOGIN_SUCCESS");

	auto self = shared_from_this();

	boost::asio::async_write(
		m_Socket, boost::asio::buffer(&loginSeccess, sizeof(pack)),
		[this, self, loginSeccess](boost::system::error_code ec, std::size_t /*len*/)
		{
		});
}

void Session::DoWrite()
{
	auto self = shared_from_this();
	boost::asio::async_write(
		m_Socket,
		boost::asio::buffer(&m_WriteQueue.front(), sizeof(pack)),
		[this, self](boost::system::error_code ec, std::size_t)
		{
			if (!ec)
			{
				m_WriteQueue.pop_front();
				if (!m_WriteQueue.empty())
					DoWrite();
			}
			else
			{
				Close();
			}
		}
	);
}

void Session::Send(const pack& packet)
{
	bool writing = !m_WriteQueue.empty();
	m_WriteQueue.push_back(packet);

	if (!writing)
		DoWrite();
}
