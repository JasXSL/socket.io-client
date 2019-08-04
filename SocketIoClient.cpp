/*
    Socket.io
    
    A fork of https://github.com/timum-viw/socket.io-client
    Requires arduinoWebSockets, https://github.com/Kadah/arduinoWebSockets
*/

#include <SocketIoClient.h>

static const char* TAG = "SIoC";

const String getEventName(const String msg) {
    return msg.substring(4, msg.indexOf("\"",4));
}

const String getEventPayload(const String msg) {
    String result = msg.substring(msg.indexOf("\"",4)+2,msg.length()-1);
    if(result.startsWith("\"")) {
        result.remove(0,1);
    }
    if(result.endsWith("\"")) {
        result.remove(result.length()-1);
    }
    return result;
}

static void hexdump(const uint32_t* src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        Serial.printf("%08x ", *src);
        ++src;
        if ((i + 1) % 4 == 0) {
            Serial.printf("\n");
        }
    }
}

void SocketIoClient::webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    String msg;
    switch(type) {
        case WStype_DISCONNECTED:
            ESP_LOGI(TAG, "disconnected!");
            trigger("disconnect", NULL, 0);
            break;
        case WStype_CONNECTED:
            ESP_LOGI(TAG, "connected to url: %s",  payload);
            break;
        case WStype_TEXT:
            msg = String((char*)payload);
            if(msg.startsWith("42")) {
                trigger(getEventName(msg).c_str(), getEventPayload(msg).c_str(), length);
            } else if(msg.startsWith("2")) { // PING
                _webSocket.sendTXT("3"); // PONG
            } else if(msg.startsWith("40")) {
                trigger("connect", NULL, 0);
            } else if(msg.startsWith("41")) {
                trigger("disconnect", NULL, 0);
            }
            break;
#if CORE_DEBUG_LEVEL >= 4
        case WStype_BIN:
            ESP_LOGD(TAG, "get binary length: %u", length);
            hexdump((uint32_t*) payload, length);
#endif
        break;
    }
}

void SocketIoClient::beginSSL(const char* host, const int port, const char* url, const char* CA_cert) {
    ESP_LOGI(TAG, "starting SSL socket: %s:%i, %s", host, port, url);
    _webSocket.beginSslWithCA(host, port, url, CA_cert);

    initialize();
}

void SocketIoClient::begin(const char* host, const int port, const char* url) {
    ESP_LOGI(TAG, "starting socket: %s:%i, %s", host, port, url);
    _webSocket.begin(host, port, url);
    initialize();
}

void SocketIoClient::initialize() {
    _webSocket.onEvent(std::bind(&SocketIoClient::webSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _lastPing = millis();
}

void SocketIoClient::loop() {
    _webSocket.loop();
    for(auto packet=_packets.begin(); packet != _packets.end();) {
        if(_webSocket.sendTXT(*packet)) {
            ESP_LOGD(TAG, "packet \"%s\" emitted", packet->c_str());
            packet = _packets.erase(packet);
        } else {
            ++packet;
        }
    }

    if(millis() - _lastPing > PING_INTERVAL) {
        _webSocket.sendTXT("2");
        _lastPing = millis();
    }
}

void SocketIoClient::on(const char* event, std::function<void (const char * payload, size_t length)> func) {
    _events[event] = func;
}

void SocketIoClient::emit(const char* event, const char * payload) {
    String msg = String("42[\"");
    msg += event;
    msg += "\"";
    if(payload) {
        msg += ",";
        msg += payload;
    }
    msg += "]";
    ESP_LOGD(TAG, "add packet %s", msg.c_str());
    _packets.push_back(msg);
}

void SocketIoClient::remove(const char* event) {
    auto e = _events.find(event);
    if(e != _events.end()) {
        _events.erase(e);
    } else {
        ESP_LOGD(TAG, "event %s not found, can not be removed", event);
    }
}

void SocketIoClient::trigger(const char* event, const char * payload, size_t length) {
    auto e = _events.find(event);
    if(e != _events.end()) {
        ESP_LOGD(TAG, "trigger event %s", event);
        e->second(payload, length);
    } else {
        ESP_LOGD(TAG, "event %s not found. %d events available", event, _events.size());
    }
}

void SocketIoClient::disconnect()
{
    _webSocket.disconnect();
}

void SocketIoClient::setAuthorization(const char * user, const char * password) {
    _webSocket.setAuthorization(user, password);
}
