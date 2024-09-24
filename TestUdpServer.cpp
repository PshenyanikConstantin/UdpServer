#include "UdpServer.h"
#include <fstream>

int main() {
    try {
        std::ifstream jsonFileStream("ServerSettings.json");
        nlohmann::json jsonData = nlohmann::json::parse(jsonFileStream);
        ServerSettings ServerSettings;
        ServerSettings.Parse(jsonData);

        boost::asio::io_context io_context;
        UdpServer server(io_context, ServerSettings);
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}