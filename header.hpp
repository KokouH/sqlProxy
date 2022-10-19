#ifndef HEADER_HPP
# define HEADER_HPP

#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>
#include <fstream>
#include <deque>

#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>

namespace tcp_proxy
{
	# define max_data_length 8192
	namespace ip = boost::asio::ip;

	class bridge : public boost::enable_shared_from_this<bridge>
	{
	public:
		typedef ip::tcp::socket socket_type;
		typedef boost::shared_ptr<bridge> ptr_type;

	private:
		void handle_upstream_read(const boost::system::error_code& error,
			const size_t& bytes_transferred);
		void handle_downstream_write(const boost::system::error_code& error);

		void handle_downstream_read(const boost::system::error_code& error,
			const size_t& bytes_transferred);
		void handle_upstream_write(const boost::system::error_code& error);

		void close();

		socket_type downstream_socket_;
		socket_type upstream_socket_;

		unsigned char downstream_data_[max_data_length];
		unsigned char upstream_data_[max_data_length];

		boost::mutex mutex_;

		std::ofstream& file_;

	public:
		bridge(boost::asio::io_context &ios,
			std::ofstream& file);

		socket_type& downstream_socket();
		socket_type& upstream_socket();

		void start(const std::string& upstream_host, unsigned short upstream_port);

		void handle_upstream_connect(const boost::system::error_code& error);

		class acceptor
		{
		public:
			acceptor(boost::asio::io_context& io_context,
				const std::string& local_host, unsigned short local_port,
				const std::string& upstream_host, unsigned short upstream_port,
				std::ofstream& file);

			bool accept_connections();

		private:
			void handle_accept(const boost::system::error_code& error);

			boost::asio::io_context& io_context_;
			ip::address_v4 localhost_address;
			ip::tcp::acceptor acceptor_;
			ptr_type session_;
			unsigned short upstream_port_;
			std::string upstream_host_;
			std::ofstream& file_;
		};

	};
}


#endif