#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "Server.h"


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
