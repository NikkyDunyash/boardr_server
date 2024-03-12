#pragma once

#include <drogon/WebSocketController.h>
#include <map>

using namespace drogon;

class BoardWebSocket : public drogon::WebSocketController<BoardWebSocket>
{
public:
  std::map<uint32_t, WebSocketConnectionPtr> clients;
  void handleNewMessage(const WebSocketConnectionPtr &,
                        std::string &&,
                        const WebSocketMessageType &) override;
  void handleNewConnection(const HttpRequestPtr &,
                           const WebSocketConnectionPtr &) override;
  void handleConnectionClosed(const WebSocketConnectionPtr &) override;
  WS_PATH_LIST_BEGIN
  // list path definitions here;
  // WS_PATH_ADD("/path", "filter1", "filter2", ...);
  WS_PATH_ADD("/comments", "LoginFilter");
  WS_PATH_LIST_END
};
