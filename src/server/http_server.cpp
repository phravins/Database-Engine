#include "server/http_server.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <cstring>

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#endif

namespace mydb {

HttpServer::HttpServer(Executor* executor, CatalogManager* catalog, int port, const std::string& api_key)
    : executor_(executor), catalog_(catalog), port_(port), api_key_(api_key), server_sock_(-1), running_(false) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
    }
#endif
}

HttpServer::~HttpServer() {
#ifdef _WIN32
    if (server_sock_ != INVALID_SOCKET) closesocket(server_sock_);
    WSACleanup();
#else
    if (server_sock_ != -1) close(server_sock_);
#endif
}

void HttpServer::Start() {
#ifdef _WIN32
    server_sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock_ == INVALID_SOCKET) {
        std::cerr << "Error creating socket." << std::endl;
        return;
    }
#else
    server_sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock_ < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return;
    }
#endif

    // Allow port reuse
    int opt = 1;
#ifdef _WIN32
    setsockopt(server_sock_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_sock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(port_);

    if (bind(server_sock_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed on port " << port_ << std::endl;
        return;
    }

    if (listen(server_sock_, 10) < 0) {
        std::cerr << "Listen failed." << std::endl;
        return;
    }

    std::cout << "\033[1;32m[v2vdb-server] Listening on 0.0.0.0:" << port_ << "\033[0m" << std::endl;
    if (!api_key_.empty()) {
        std::cout << "[v2vdb-server] API Key authentication required." << std::endl;
    }
    
    running_ = true;
    while (running_) {
        sockaddr_in client_addr;
#ifdef _WIN32
        int client_len = sizeof(client_addr);
        SOCKET client_sock = accept(server_sock_, (sockaddr*)&client_addr, &client_len);
        if (client_sock == INVALID_SOCKET) continue;
#else
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock_, (sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) continue;
#endif

        // Simple blocking handle for now (single thread DB engine)
        HandleClient(client_sock);
    }
}

std::string HttpServer::ReadRequest(int client_sock) {
    char buffer[4096];
    std::string request;
    
    // Read headers
    while (true) {
        int bytes = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        request += buffer;
        
        size_t header_end = request.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            // Find content length
            size_t cl_pos = request.find("Content-Length: ");
            if (cl_pos != std::string::npos) {
                cl_pos += 16;
                size_t cl_end = request.find("\r\n", cl_pos);
                int content_length = std::stoi(request.substr(cl_pos, cl_end - cl_pos));
                
                int header_size = header_end + 4;
                int current_body_size = request.length() - header_size;
                
                // Read remaining body
                while (current_body_size < content_length) {
                    bytes = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
                    if (bytes <= 0) break;
                    buffer[bytes] = '\0';
                    request += buffer;
                    current_body_size += bytes;
                }
            }
            break;
        }
    }
    return request;
}

void HttpServer::SendResponse(int client_sock, const std::string& status, const std::string& content_type, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Connection: close\r\n\r\n";
    response << body;
    
    std::string res_str = response.str();
    send(client_sock, res_str.c_str(), res_str.length(), 0);
    
#ifdef _WIN32
    closesocket(client_sock);
#else
    close(client_sock);
#endif
}

void HttpServer::HandleClient(int client_sock) {
    std::string request = ReadRequest(client_sock);
    if (request.empty()) {
#ifdef _WIN32
        closesocket(client_sock);
#else
        close(client_sock);
#endif
        return;
    }
    
    // Very basic parsing
    std::istringstream req_stream(request);
    std::string method, path, version;
    req_stream >> method >> path >> version;
    
    // Auth Check
    if (!api_key_.empty() && method != "OPTIONS") {
        size_t api_key_pos = request.find("X-Api-Key: ");
        bool authorized = false;
        if (api_key_pos != std::string::npos) {
            size_t key_end = request.find("\r\n", api_key_pos);
            std::string provided_key = request.substr(api_key_pos + 11, key_end - (api_key_pos + 11));
            // Trim spaces
            provided_key.erase(0, provided_key.find_first_not_of(" \t\r\n"));
            provided_key.erase(provided_key.find_last_not_of(" \t\r\n") + 1);
            if (provided_key == api_key_) authorized = true;
        }
        
        if (!authorized) {
            SendResponse(client_sock, "401 Unauthorized", "application/json", "{\"error\":\"Unauthorized. Missing or invalid X-Api-Key.\"}");
            return;
        }
    }
    
    // CORS Preflight
    if (method == "OPTIONS") {
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response << "Access-Control-Allow-Headers: Content-Type, X-Api-Key\r\n";
        response << "Connection: close\r\n\r\n";
        std::string res_str = response.str();
        send(client_sock, res_str.c_str(), res_str.length(), 0);
#ifdef _WIN32
        closesocket(client_sock);
#else
        close(client_sock);
#endif
        return;
    }

    // Routing
    try {
        if (method == "GET" && path == "/health") {
            HandleHealth(client_sock);
        } else if (method == "GET" && path == "/tables") {
            HandleTables(client_sock);
        } else if (method == "POST" && path == "/query") {
            size_t body_start = request.find("\r\n\r\n");
            std::string body = (body_start != std::string::npos) ? request.substr(body_start + 4) : "";
            HandleQuery(client_sock, body);
        } else {
            SendResponse(client_sock, "404 Not Found", "application/json", "{\"error\":\"Not Found\"}");
        }
    } catch (const std::exception& e) {
        SendResponse(client_sock, "500 Internal Server Error", "application/json", 
                     "{\"error\":\"" + EscapeJsonString(e.what()) + "\"}");
    }
}

void HttpServer::HandleHealth(int client_sock) {
    SendResponse(client_sock, "200 OK", "application/json", "{\"status\":\"ok\",\"version\":\"1.0.8\"}");
}

void HttpServer::HandleTables(int client_sock) {
    std::string result = ExecuteToString("SHOW TABLES");
    // In executor, SHOW TABLES prints output. ExecuteToString will capture it.
    // For a real API, we would parse that string into JSON, or modify Executor to return struct.
    // For now, return the raw stdout capture inside a JSON message.
    
    std::ostringstream json;
    json << "{\"message\":\"" << EscapeJsonString(result) << "\"}";
    SendResponse(client_sock, "200 OK", "application/json", json.str());
}

void HttpServer::HandleQuery(int client_sock, const std::string& body) {
    // Extract "sql" from minimal JSON body: {"sql": "..."}
    std::string sql;
    size_t sql_pos = body.find("\"sql\"");
    if (sql_pos != std::string::npos) {
        size_t start = body.find("\"", sql_pos + 5);
        if (start != std::string::npos) {
            start++;
            // Find matching double quote, taking care of escapes (simplified)
            size_t end = start;
            while (end < body.length()) {
                if (body[end] == '"' && body[end-1] != '\\') break;
                end++;
            }
            if (end < body.length()) {
                sql = body.substr(start, end - start);
            }
        }
    }
    
    if (sql.empty()) {
        SendResponse(client_sock, "400 Bad Request", "application/json", "{\"error\":\"Invalid JSON or empty 'sql' field.\"}");
        return;
    }
    
    std::cout << "[v2vdb-server] Query: " << sql << std::endl;
    
    // Execute and capture
    std::string result = ExecuteToString(sql);
    
    // Must save catalog if a schema changed
    if (sql.find("CREATE") != std::string::npos || sql.find("make") != std::string::npos ||
        sql.find("DROP") != std::string::npos) {
        catalog_->SaveCatalog();
    }
    
    std::ostringstream json;
    json << "{\"result\":\"" << EscapeJsonString(result) << "\"}";
    SendResponse(client_sock, "200 OK", "application/json", json.str());
}

std::string HttpServer::ExecuteToString(const std::string& sql) {
    // Redirect std::cout to stringstream
    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());
    
    try {
        executor_->Execute(sql);
    } catch (const std::exception& e) {
        buffer << "Error: " << e.what();
    }
    
    // Restore
    std::cout.rdbuf(old_cout);
    
    // Clean escape codes (ANSI colors used by terminal)
    std::string raw = buffer.str();
    std::string clean;
    bool in_escape = false;
    for (char c : raw) {
        if (c == '\033') {
            in_escape = true;
        } else if (in_escape) {
            if (c == 'm') in_escape = false;
        } else {
            clean += c;
        }
    }
    return clean;
}

std::string HttpServer::EscapeJsonString(const std::string& input) {
    std::ostringstream ss;
    for (char c : input) {
        if (c == '"') ss << "\\\"";
        else if (c == '\\') ss << "\\\\";
        else if (c == '\n') ss << "\\n";
        else if (c == '\r') ss << "\\r";
        else if (c == '\t') ss << "\\t";
        else ss << c;
    }
    return ss.str();
}

} // namespace mydb
