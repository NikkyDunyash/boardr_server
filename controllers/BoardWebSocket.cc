#include "BoardWebSocket.h"
#include "utils.h"

void BoardWebSocket::handleNewMessage(const WebSocketConnectionPtr &wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    LOG_DEBUG << endl
              << wsConnPtr->peerAddr().toIp() << ": " << message << endl;
    for (auto client : clients)
    {
        client.second->send("comments updated");
    }
}

void BoardWebSocket::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr &wsConnPtr)
{

    LOG_DEBUG << endl
              << "IP: " << wsConnPtr->peerAddr().toIp() << endl
              << wsConnPtr->peerAddr().ipNetEndian() << endl;

    clients.insert({wsConnPtr->peerAddr().ipNetEndian(), wsConnPtr});
}

void BoardWebSocket::handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr)
{
    LOG_DEBUG << endl
              << "IP: " << wsConnPtr->peerAddr().toIp() << endl;

    clients.erase(wsConnPtr->peerAddr().ipNetEndian());
}
