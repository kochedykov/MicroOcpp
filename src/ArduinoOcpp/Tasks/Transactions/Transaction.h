// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2023
// MIT License

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <ArduinoOcpp/Core/Time.h>
#include <ArduinoOcpp/MessagesV16/CiStrings.h>
#include <memory>
#include <ArduinoJson.h>

namespace ArduinoOcpp {

/*
 * A transaction is initiated by the client (charging station) and processed by the server (central system).
 * The client side of a transaction is all data that is generated or collected at the charging station. The
 * server side is all transaction data that is assigned by the central system.
 * 
 * "ClientTransaction" is short for the client-side data, and the same goes for "ServerTranaction". The rest
 * of the terminology is documented in OCPP 1.6 Specification - Edition 2, sections 3.6, 4.8, 4.10 and 5.11. 
 */

class ConnectorTransactionStore;

class RpcSync {
private:
    friend class Transaction;
    
    bool requested = false;
    bool confirmed = false;

    bool serializeSessionState(JsonObject out);
    bool deserializeSessionState(JsonObject in);
public:
    void setRequested() {this->requested = true;}
    bool isRequested() {return requested;}
    void confirm() {confirmed = true;}
    bool isConfirmed() {return confirmed;}
    bool isCompleted() {return isRequested() && isConfirmed();}
};

class ClientTransactionStart {
private:
    friend class Transaction;

    Timestamp timestamp = MIN_TIME;      //timestamp of StartTx; can be set before actually initiating
    int32_t meter = -1;           //meterStart of StartTx
    int reservationId = -1;
};

class ServerTransactionStart {
private:
    friend class Transaction;

    int transactionId = -1; //only valid if confirmed = true
};

class TransactionStart {
private:
    friend class Transaction;
    
    RpcSync rpc;
    ClientTransactionStart client;
    ServerTransactionStart server;
};

class ClientTransactionStop {
private:
    friend class Transaction;

    char idTag [IDTAG_LEN_MAX + 1] = {'\0'};
    Timestamp timestamp = MIN_TIME;
    int32_t meter = -1;
    char reason [REASON_LEN_MAX + 1] = {'\0'};
};

class ServerTransactionStop {
//no data at the moment
};

class TransactionStop {
private:
    friend class Transaction;

    RpcSync rpc;
    ClientTransactionStop client;
    ServerTransactionStop server;
};

class ChargingSession {
private:
    friend class Transaction;
    
    char idTag [IDTAG_LEN_MAX + 1] = {'\0'};
    bool authorized = false;    //if the given idTag was authorized
    bool deauthorized = false;  //if the server revoked a local authorization
    Timestamp timestamp = MIN_TIME;
    int txProfileId = -1;

    bool active = true;         //true: ignore
                                //false before StartTx init: abort 
                                //false between StartTx init and StopTx init: end
                                //false after StopTx init: ignore
};

class Transaction {
private:
    ConnectorTransactionStore& context;

    ChargingSession session;
    TransactionStart start;
    TransactionStop stop;

    unsigned int connectorId = 0;
    unsigned int txNr = 0;

    bool silent = false; //silent Tx: process tx locally, without reporting to the server
public:
    Transaction(ConnectorTransactionStore& context, unsigned int connectorId, unsigned int txNr, bool silent = false) : 
                context(context),
                connectorId(connectorId), 
                txNr(txNr),
                silent(silent) {}

    bool serializeSessionState(DynamicJsonDocument& out);
    bool deserializeSessionState(JsonObject in);

    unsigned int getConnectorId() {return connectorId;}
    void setConnectorId(unsigned int connectorId) {this->connectorId = connectorId;}
    unsigned int getTxNr() {return txNr;} //only valid if getConnectorId() >= 0
    void setTxNr(unsigned int txNr) {this->txNr = txNr;}

    RpcSync& getStartRpcSync() {return start.rpc;}

    RpcSync& getStopRpcSync() {return stop.rpc;}

    bool isAborted() {return !start.rpc.requested && !session.active;}
    bool isCompleted() {return stop.rpc.isConfirmed();}
    bool isPreparing() {return session.active && !start.rpc.isRequested();}
    bool isRunning() {return start.rpc.isRequested() && !stop.rpc.isRequested();}
    bool isActive() {return session.active;}

    const char *getIdTag() {return session.idTag;}
    void setIdTag(const char *idTag) {snprintf(session.idTag, IDTAG_LEN_MAX + 1, "%s", idTag);}
    Timestamp& getSessionTimestamp() {return session.timestamp;}
    void setSessionTimestamp(Timestamp timestamp) {session.timestamp = timestamp;}

    const char *getStopReason() {return stop.client.reason;}
    void setStopReason(const char *reason) {snprintf(stop.client.reason, REASON_LEN_MAX + 1, "%s", reason);}
    void endSession() {session.active = false;}

    void setAuthorized() {session.authorized = true;}
    bool isAuthorized() {return session.authorized;}

    void setIdTagDeauthorized() {session.deauthorized = true;}
    bool isIdTagDeauthorized() {return session.deauthorized;}

    int getTransactionId() {return start.server.transactionId;}
    void setTransactionId(int transactionId) {start.server.transactionId = transactionId;}

    void setMeterStart(int32_t meter) {start.client.meter = meter;}
    bool isMeterStartDefined() {return start.client.meter >= 0;}
    int32_t getMeterStart() {return start.client.meter;}

    void setReservationId(int reservationId) {start.client.reservationId = reservationId;}
    int getReservationId() {return start.client.reservationId;}

    void setStartTimestamp(Timestamp timestamp) {start.client.timestamp = timestamp;}
    Timestamp& getStartTimestamp() {return start.client.timestamp;}

    void setMeterStop(int32_t meter) {stop.client.meter = meter;}
    bool isMeterStopDefined() {return stop.client.meter >= 0;}
    int32_t getMeterStop() {return stop.client.meter;}

    void setStopTimestamp(Timestamp timestamp) {stop.client.timestamp = timestamp;}
    Timestamp& getStopTimestamp() {return stop.client.timestamp;}

    const char *getStopIdTag() {return stop.client.idTag;}
    void setStopIdTag(const char *idTag) {snprintf(stop.client.idTag, IDTAG_LEN_MAX + 1, "%s", idTag);}

    bool commit();
    bool isSilent() {return silent;} //no data will be sent to server and server will not assign transactionId
};

}

#endif
