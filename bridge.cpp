#include "header.hpp"

tcp_proxy::bridge::bridge(boost::asio::io_context& ios,
		std::ofstream& file)
	: downstream_socket_(ios),
	upstream_socket_(ios),
	file_(file)
{}

tcp_proxy::bridge::socket_type& tcp_proxy::bridge::downstream_socket()
{
	return downstream_socket_;
}

tcp_proxy::bridge::socket_type& tcp_proxy::bridge::upstream_socket()
{
	return upstream_socket_;
}

void tcp_proxy::bridge::close()
{
	boost::mutex::scoped_lock lock(mutex_);

	if (downstream_socket_.is_open())
		downstream_socket_.close();

	if (upstream_socket_.is_open())
		upstream_socket_.close();
}

void tcp_proxy::bridge::handle_downstream_read(const boost::system::error_code& error,
	const size_t& bytes_transferred)
{
	if (!error)
	{
		if (downstream_data_[4] == 3 || downstream_data_[4] == 22)
		{	
			for (size_t i = 7; i < bytes_transferred; i++)
			{
				file_ << (char)downstream_data_[i];
			}
			file_ << std::endl;
		}
		// file_ << downstream_data_ << std::endl;
		async_write(upstream_socket_,
			boost::asio::buffer(downstream_data_, bytes_transferred),
			boost::bind(&bridge::handle_upstream_write,
				shared_from_this(),
				boost::asio::placeholders::error));
	}
	else
		close();
}

void tcp_proxy::bridge::handle_upstream_write(const boost::system::error_code& error)
{
	if (!error)
	{
		downstream_socket_.async_read_some(
			boost::asio::buffer(downstream_data_, max_data_length),
			boost::bind(&bridge::handle_downstream_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}
	else
		close();
}

void tcp_proxy::bridge::handle_upstream_read(const boost::system::error_code& error,
	const size_t& bytes_transferred)
{
	if (!error)
	{
		async_write(downstream_socket_,
			boost::asio::buffer(upstream_data_, bytes_transferred),
			boost::bind(&bridge::handle_downstream_write,
				shared_from_this(),
				boost::asio::placeholders::error));
	}
	else
		close();
}

void tcp_proxy::bridge::handle_downstream_write(const boost::system::error_code& error)
{
	if (!error)
	{
		upstream_socket_.async_read_some(
			boost::asio::buffer(upstream_data_, max_data_length),
			boost::bind(&bridge::handle_upstream_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}
	else
		close();
}

void tcp_proxy::bridge::start(
	const std::string& upstream_host,
	unsigned short upstream_port)
{
	upstream_socket_.async_connect(
		ip::tcp::endpoint(
			ip::address::from_string(upstream_host),
			upstream_port),
		boost::bind(&bridge::handle_upstream_connect,
			shared_from_this(),
			boost::asio::placeholders::error));
}

void tcp_proxy::bridge::handle_upstream_connect(const boost::system::error_code& error)
{
	if (!error)
	{
		upstream_socket_.async_read_some(
			boost::asio::buffer(upstream_data_, max_data_length),
			boost::bind(&bridge::handle_upstream_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		downstream_socket_.async_read_some(
			boost::asio::buffer(downstream_data_, max_data_length),
			boost::bind(&bridge::handle_downstream_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}
	else
		close();
}
