/*
    Socket.io Client
    
    A fork of https://github.com/timum-viw/socket.io-client
*/

#ifndef __SOCKET_IO_CLIENT_H__
#define __SOCKET_IO_CLIENT_H__

#include <Arduino.h>
#include <map>
#include <vector>
#include <WebSocketsClient.h>

#define PING_INTERVAL 10000

//#define SOCKETIOCLIENT_USE_SSL
#ifdef SOCKETIOCLIENT_USE_SSL
    #define SIO_PORT_DEFAULT 443
#else
    #define SIO_PORT_DEFAULT 80
#endif
#define DEFAULT_URL "/socket.io/?transport=websocket"


class SocketIoClient {
    private:
        std::vector<String> _packets;
        WebSocketsClient _webSocket;
        int _lastPing;
        std::map<String, std::function<void (const char * payload, size_t length)>> _events;

        void trigger(const char* event, const char * payload, size_t length);
        void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
        void initialize();
    public:
        void beginSSL(const char* host, const int port = SIO_PORT_DEFAULT, const char* url = DEFAULT_URL, const char* CA_cert = NULL);
        void begin(const char* host, const int port = SIO_PORT_DEFAULT, const char* url = DEFAULT_URL);
        void loop();
        void on(const char* event, std::function<void (const char * payload, size_t length)>);
        void emit(const char* event, const char * payload = NULL);
        void remove(const char* event);
        void disconnect();
        void setAuthorization(const char * user, const char * password);
};

#endif
