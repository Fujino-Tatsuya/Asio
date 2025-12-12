#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main()
{
	try
	{
		boost::asio::io_context io;

		// 0.0.0.0:7777 Listen
		tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 7777));
		std::cout << "[Sync Echo] Server listening on port 7777...\n";

		while (true)
		{
			tcp::socket socket(io);
			acceptor.accept(socket); // Blocking accept (client waits)
			//back log ? 
			std::cout << "Client connected.\n";

			while (true)
			{
				char buffer[1024] = {};
				boost::system::error_code ec;

				size_t len = socket.read_some(boost::asio::buffer(buffer), ec);
				if (ec)
				{
					std::cout << "Client disconnected.\n";
					break;
				}

				boost::asio::write(socket, boost::asio::buffer(buffer, len));
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << "\n";
	}
}
