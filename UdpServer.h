#pragma once

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

using boost::asio::ip::udp;

struct ServerSettings {
    unsigned int port = 0;
    unsigned int protocol_version = 0;

    void Parse(nlohmann::json JsonData) {
        port = JsonData["port"].get<unsigned int>();
        protocol_version = JsonData["protocol_version"].get<unsigned int>();
    }
};

class UdpServer {
public:
    UdpServer(boost::asio::io_context& io_context, ServerSettings settings);
    ~UdpServer();

private:
    void receive_message();
    void parse_request(const std::string& message);
    void send_handshake(const std::string& message);
    void send_response(const unsigned int element_amount, const double value);
    double get_random_value(const double value);

private:
    const std::size_t chunk_size_ = 1024;
    udp::socket socket_;
    udp::endpoint endpoint_;
    std::array<char, 1024> recieve_buffer_;
    std::array<double, 1024> response_message_;
    std::string handshake_message_;

    unsigned int protocol_version_;
    std::size_t buffer_size_ = 0;
};