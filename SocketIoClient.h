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

#if CORE_DEBUG_LEVEL >= 4
#define SOCKETIOCLIENT_DEBUG(...) Serial.printf(__VA_ARGS__);
#else
   #define SOCKETIOCLIENT_DEBUG(...)
#endif

#define PING_INTERVAL 10000 //TODO: use socket.io server response

#define DEFAULT_URL "/socket.io/?transport=websocket"
#define DEFAULT_FINGERPRINT ""


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
    void beginSSL(const char* host, const int port = 443, const char* url = DEFAULT_URL, const char* fingerprint = DEFAULT_FINGERPRINT);
	void begin(const char* host, const int port = 80, const char* url = DEFAULT_URL);
	void loop();
	void on(const char* event, std::function<void (const char * payload, size_t length)>);
	void emit(const char* event, const char * payload = NULL);
	void remove(const char* event);
	void disconnect();
	void setAuthorization(const char * user, const char * password);
};

#endif
