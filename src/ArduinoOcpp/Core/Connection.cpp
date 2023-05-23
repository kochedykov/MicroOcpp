// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2023
// MIT License

#include <ArduinoOcpp/Core/OcppSocket.h>
#include <ArduinoOcpp/Debug.h>

using namespace ArduinoOcpp;

void OcppEchoSocket::loop() { }

bool OcppEchoSocket::sendTXT(std::string &out) {
    if (!connected) {
        return true;
    }
    if (receiveTXT) {
        lastRecv = ao_tick_ms();
        return receiveTXT(out.c_str(), out.length());
    } else {
        return false;
    }
}

void OcppEchoSocket::setReceiveTXTcallback(ReceiveTXTcallback &receiveTXT) {
    this->receiveTXT = receiveTXT;
}

unsigned long OcppEchoSocket::getLastRecv() {
    return lastRecv;
}

#ifndef AO_CUSTOM_WS

using namespace ArduinoOcpp::EspWiFi;

OcppClientSocket::OcppClientSocket(WebSocketsClient *wsock) : wsock(wsock) {

}

void OcppClientSocket::loop() {
    wsock->loop();
}

bool OcppClientSocket::sendTXT(std::string &out) {
    return wsock->sendTXT(out.c_str(), out.length());
}

void OcppClientSocket::setReceiveTXTcallback(ReceiveTXTcallback &callback) {
    auto& captureLastRecv = lastRecv;
    wsock->onEvent([callback, &captureLastRecv](WStype_t type, uint8_t * payload, size_t length) {
        switch (type) {
            case WStype_DISCONNECTED:
                AO_DBG_INFO("Disconnected");
                break;
            case WStype_CONNECTED:
                AO_DBG_INFO("Connected to url: %s", payload);
                captureLastRecv = ao_tick_ms();
                break;
            case WStype_TEXT:
                if (callback((const char *) payload, length)) { //forward message to OcppEngine
                    captureLastRecv = ao_tick_ms();
                } else {
                    AO_DBG_WARN("Processing WebSocket input event failed");
                }
                break;
            case WStype_BIN:
                AO_DBG_WARN("Binary data stream not supported");
                break;
            case WStype_PING:
                // pong will be send automatically
                AO_DBG_TRAFFIC_IN(8, "WS ping");
                captureLastRecv = ao_tick_ms();
                break;
            case WStype_PONG:
                // answer to a ping we send
                AO_DBG_TRAFFIC_IN(8, "WS pong");
                captureLastRecv = ao_tick_ms();
                break;
            case WStype_FRAGMENT_TEXT_START: //fragments are not supported
            default:
                AO_DBG_WARN("Unsupported WebSocket event type");
                break;
        }
    });
}

unsigned long OcppClientSocket::getLastRecv() {
    return lastRecv;
}

#endif