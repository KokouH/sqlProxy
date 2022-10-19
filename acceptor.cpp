#include "header.hpp"

tcp_proxy::bridge::acceptor::acceptor(boost::asio::io_context& io_context,
	const std::string& local_host, unsigned short local_port,
	const std::string& upstream_host, unsigned short upstream_port,
	std::ofstream& file)
		: io_context_(io_context),
		localhost_address(boost::asio::ip::address_v4::from_string(local_host)),
		acceptor_(io_context_,ip::tcp::endpoint(localhost_address,local_port)),
		upstream_port_(upstream_port),
		upstream_host_(upstream_host),
		file_(file)
	{};

bool tcp_proxy::bridge::acceptor::accept_connections()
{
	try
	{
		session_ = boost::shared_ptr<bridge>(new bridge(io_context_, file_));

		acceptor_.async_accept(session_->downstream_socket(),
			boost::bind(&acceptor::handle_accept,
				this,
				boost::asio::placeholders::error));
	}
	catch(std::exception& e)
	{
		std::cerr << "Acceptor exception: " << e.what() << std::endl;
		return (false);
	}

	return (true);
}

void tcp_proxy::bridge::acceptor::handle_accept(const boost::system::error_code& error)
{
	if (!error)
	{
		std::cout << "Accept created" << std::endl;
		session_->start(upstream_host_, upstream_port_);

		if (!accept_connections())
			std::cerr << "Failure during call to accept." << std::endl;
	}
	else
		std::cerr << "Error: " << error.message() << std::endl;
}
