#pragma once

#include "executor/executor.h"
#include "catalog/catalog_manager.h"
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

namespace mydb {

class HttpServer {
public:
    HttpServer(Executor* executor, CatalogManager* catalog, int port, const std::string& api_key);
    ~HttpServer();

    void Start();

private:
    void HandleClient(int client_sock);
    std::string ReadRequest(int client_sock);
    void SendResponse(int client_sock, const std::string& status, const std::string& content_type, const std::string& body);
    
    // Endpoints
    void HandleHealth(int client_sock);
    void HandleTables(int client_sock);
    void HandleQuery(int client_sock, const std::string& body);
    
    // Helpers
    std::string ExecuteToString(const std::string& sql);
    std::string EscapeJsonString(const std::string& input);

    Executor* executor_;
    CatalogManager* catalog_;
    int port_;
    std::string api_key_;
    
#ifdef _WIN32
    SOCKET server_sock_;
#else
    int server_sock_;
#endif
    bool running_;
};

} // namespace mydb
