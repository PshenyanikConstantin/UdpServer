#include "UdpServer.h"
#include <random>
#include <algorithm>

UdpServer::UdpServer(boost::asio::io_context& io_context, ServerSettings settings)
    : socket_(io_context, udp::endpoint(udp::v4(), settings.port))
    , protocol_version_(settings.protocol_version) {
    receive_message();
}

UdpServer::~UdpServer() {
    socket_.close();
}

void UdpServer::receive_message() {
    socket_.async_receive_from(
        boost::asio::buffer(recieve_buffer_), endpoint_,
        [this](boost::system::error_code error_code, std::size_t bytes_received) {
            if (!error_code && bytes_received > 0) {
                std::string message(recieve_buffer_.data(), bytes_received);
                parse_request(message);
            }
            else {
                receive_message();
            }
        });
}

void UdpServer::parse_request(const std::string& message) {
    try {
        nlohmann::json handshake_msg = nlohmann::json::parse(message);
        if (handshake_msg["type"].get<std::string>() == "HANDSHAKE") {
            std::string message;
            int protocol_version = handshake_msg["protocol_version"].get<unsigned int>();
            if (protocol_version != protocol_version_) {
                message = std::string("Error different protocol versions, sever protocol version - " + std::to_string(protocol_version_) + ", client protocol version" + std::to_string(protocol_version));
            }
            send_handshake(message);
        }
        else if (handshake_msg["type"] == "REQUEST") {
            send_response(handshake_msg["element_amount"].get<unsigned int>(), handshake_msg["value"].get<double>());
        }
        else {
            send_handshake("Invalide request type");
        }
    }
    catch (nlohmann::json::parse_error& e) {
        send_handshake("Invalid message format.");
    }
}

void UdpServer::send_handshake(const std::string& message) {
    nlohmann::json json_message = {
        {"type", "HANDSHAKE"},
        {"error_message", message}
    };

    handshake_message_ = json_message.dump();

    socket_.async_send_to(
        boost::asio::buffer(handshake_message_), endpoint_,
        [this](boost::system::error_code errror_code, std::size_t bytes_received) {
            receive_message();
        });
}

void UdpServer::send_response(const unsigned int element_amount, const double value) {
    for (std::size_t i = 0; i < element_amount; i += chunk_size_) {
        buffer_size_ = std::min(chunk_size_, element_amount - i);

        response_message_.fill(0.0);
        for (int j = 0; j < buffer_size_; j++) {
            double random_value = get_random_value(value);
            response_message_[j] = random_value;
        }

        socket_.async_send_to(
            boost::asio::buffer(response_message_, sizeof(double) * buffer_size_), endpoint_,
            [this](boost::system::error_code ec, std::size_t length) {
                receive_message();
            });
    }
}

double UdpServer::get_random_value(const double value) {
    std::random_device random_device;
    std::mt19937 random_generator(random_device());

    std::uniform_real_distribution<> disdistribution(-value, value);

    return disdistribution(random_generator);
}