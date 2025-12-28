#include <boost/asio.hpp>
#include <deque>
#include <iostream>
#include <memory>


using boost::asio::ip::tcp;

struct pack
{
	char name[32];
	char msg[256];
};

class AsyncClient : public std::enable_shared_from_this<AsyncClient>
{
public:
	AsyncClient(boost::asio::io_context& io, const std::string& host, uint16_t port, std::string name)
		: m_IO(io)
		, m_Socket(io)
		, m_Endpoint(boost::asio::ip::make_address(host), port)
		, m_name(name)
	{
	}

	void Start()
	{
		DoConnect();
	}

	void Send(const std::string& msg)
	{
		// 메인스레드에서 호출해도 안전하게 write 가능하도록 strand 권장
		auto self = shared_from_this();
		boost::asio::post(m_IO,
			[this, self, msg]()
			{
				bool writing = !m_WriteQueue.empty();

				pack temp{};

				memcpy(temp.name, m_name.data(), std::min(m_name.size(), sizeof(temp.name) - 1));
				memcpy(temp.msg, msg.data(), std::min(msg.size(), sizeof(temp.msg) - 1));

				m_WriteQueue.push_back(temp);

				if (!writing)
					DoWrite();
			});
	}

private:
	// -----------------------------
	// 1. Connect
	// -----------------------------
	void DoConnect()
	{
		auto self = shared_from_this();
		m_Socket.async_connect(m_Endpoint,
			[this, self](boost::system::error_code ec)
			{
				if (!ec)
				{
					DoLoginRequest();
				}
				else
				{
					std::cout << "Connect failed: " << ec.message() << "\n";
				}
			});
	}

	void DoLoginRequest()
	{
		pack login{};
		std::string temp{ " : Login request" };
		
		memcpy(login.name, m_name.data(), std::min(m_name.size(), sizeof(login.name) - 1));
		memcpy(login.msg, temp.data(), std::min(temp.size(), sizeof(login.msg) - 1));

		auto self = shared_from_this();
		boost::asio::async_write(
			m_Socket,
			boost::asio::buffer(&login, sizeof(pack)),
			[this, self](boost::system::error_code ec,	std::size_t len)
			{
				if (!ec)
				{
					CheckLogin();
				}
				else
				{
					std::cout << "Login Request Failed" << std::endl;
				}
			});
	}

	void DoRead()
	{
		auto self = shared_from_this();
		boost::asio::async_read(m_Socket,boost::asio::buffer(&m_ReadBuf,sizeof(pack)),
			[this, self](boost::system::error_code ec, std::size_t len)
			{
				if (!ec)
				{
					std::cout << m_ReadBuf.name << " : " << m_ReadBuf.msg << "\n";

					DoRead(); // 계속 읽기
				}
				else
				{
					std::cout << "Read error: " << ec.message() << "\n";
				}
			});
	}
	
	void DoWrite()
	{
		auto self = shared_from_this();
		boost::asio::async_write(m_Socket,
			boost::asio::buffer(&m_WriteQueue.front(),sizeof(pack)),
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
					std::cout << "Write error: " << ec.message() << "\n";
				}
			});
	}

	void CheckLogin()
	{
		auto self = shared_from_this();
		boost::asio::async_read(m_Socket, boost::asio::buffer(&m_ReadBuf, sizeof(pack)),
			[this, self](boost::system::error_code ec, std::size_t len)
			{
				if (!ec && strcmp(m_ReadBuf.msg,"LOGIN_SUCCESS") == 0)
				{
					std::cout << m_ReadBuf.name << " : " << m_ReadBuf.msg << "\n";

					DoRead(); // 계속 읽기
				}
				else
				{
					std::cout << "Login Failed";
				}
			});
	}

private:
	boost::asio::io_context& m_IO;
	tcp::socket m_Socket;
	tcp::endpoint m_Endpoint;

	std::string m_name;

	pack m_ReadBuf;

	std::deque<pack> m_WriteQueue;
};

int main()
{
	std::string ip;
	std::cout << "Server IP:";
	std::getline(std::cin, ip);

	std::string name;
	std::cout << "Nick name:";
	std::getline(std::cin, name);



	boost::asio::io_context io;

	auto client = std::make_shared<AsyncClient>(io, ip, 7777, name);
	client->Start();

	// 입력을 보내는 스레드는 따로
	std::thread input_thread([client]()
		{
			while (true)
			{
				std::string line;
				std::getline(std::cin, line);
				if (line == "quit") break;

				client->Send(line);
			}
		});

	io.run();
	input_thread.join();

	return 0;
}
