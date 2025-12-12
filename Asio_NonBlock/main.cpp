#include <boost/asio.hpp>
#include <iostream>
#include <memory>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(tcp::socket socket)
		: m_Socket(std::move(socket))
	{
	}

	void Start()
	{
		DoRead();
	}

private:
	void DoRead()
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

	void DoWrite(std::size_t length)
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

	tcp::socket m_Socket;
	char m_Data[1024];
};


class Server
{
public:
	Server(boost::asio::io_context& io, uint16_t port)
		: m_Acceptor(io, tcp::endpoint(tcp::v4(), port))
	{
		DoAccept();
	}

private:
	void DoAccept()
	{
		m_Acceptor.async_accept(
			[this](boost::system::error_code ec, tcp::socket socket)
			{
				if (!ec)
				{
					std::make_shared<Session>(std::move(socket))->Start();
				}

				DoAccept(); // 계속 accept
			}
		);
	}

	tcp::acceptor m_Acceptor;
};


int main()
{
	try
	{
		boost::asio::io_context io;
		Server server(io, 7777);

		std::cout << "[Async Echo] Server listening on port 7777...\n";
		io.run(); // 이벤트 루프 시작
	}
	catch (const std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << "\n";
	}
}
