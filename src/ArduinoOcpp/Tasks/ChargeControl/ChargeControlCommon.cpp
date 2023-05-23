// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2023
// MIT License

#include <string>

#include <ArduinoOcpp/Tasks/ChargeControl/ChargeControlCommon.h>
#include <ArduinoOcpp/Core/Context.h>
#include <ArduinoOcpp/Core/Configuration.h>
#include <ArduinoOcpp/MessagesV16/ChangeAvailability.h>
#include <ArduinoOcpp/MessagesV16/ChangeConfiguration.h>
#include <ArduinoOcpp/MessagesV16/ClearCache.h>
#include <ArduinoOcpp/MessagesV16/GetConfiguration.h>
#include <ArduinoOcpp/MessagesV16/RemoteStartTransaction.h>
#include <ArduinoOcpp/MessagesV16/RemoteStopTransaction.h>
#include <ArduinoOcpp/MessagesV16/Reset.h>
#include <ArduinoOcpp/MessagesV16/TriggerMessage.h>
#include <ArduinoOcpp/MessagesV16/UnlockConnector.h>

#include <ArduinoOcpp/MessagesV16/Authorize.h>
#include <ArduinoOcpp/MessagesV16/StartTransaction.h>
#include <ArduinoOcpp/MessagesV16/StatusNotification.h>
#include <ArduinoOcpp/MessagesV16/StopTransaction.h>

#include <ArduinoOcpp/Debug.h>

using namespace ArduinoOcpp;

ChargeControlCommon::ChargeControlCommon(Context& context, unsigned int numConn, std::shared_ptr<FilesystemAdapter> filesystem) :
        context(context) {
    
    std::shared_ptr<Configuration<int>> numberOfConnectors =
            declareConfiguration<int>("NumberOfConnectors", numConn >= 1 ? numConn - 1 : 0, CONFIGURATION_VOLATILE, false, true, false, false);

    const char *fpId = "Core,RemoteTrigger";
    const char *fpIdCore = "Core";
    const char *fpIdRTrigger = "RemoteTrigger";
    auto fProfile = declareConfiguration<const char*>("SupportedFeatureProfiles",fpId, CONFIGURATION_VOLATILE, false, true, true, false);
    if (!strstr(*fProfile, fpIdCore)) {
        auto fProfilePlus = std::string(*fProfile);
        if (!fProfilePlus.empty() && fProfilePlus.back() != ',')
            fProfilePlus += ",";
        fProfilePlus += fpIdCore;
        fProfile->setValue(fProfilePlus.c_str(), fProfilePlus.length() + 1);
    }
    if (!strstr(*fProfile, fpIdRTrigger)) {
        auto fProfilePlus = std::string(*fProfile);
        if (!fProfilePlus.empty() && fProfilePlus.back() != ',')
            fProfilePlus += ",";
        fProfilePlus += fpIdRTrigger;
        fProfile->setValue(fProfilePlus.c_str(), fProfilePlus.length() + 1);
    }
    
    /*
     * Further configuration keys which correspond to the Core profile
     */
    declareConfiguration<bool>("AuthorizeRemoteTxRequests",false,CONFIGURATION_VOLATILE,false,true,false,false);
    declareConfiguration<int>("GetConfigurationMaxKeys",30,CONFIGURATION_VOLATILE,false,true,false,false);
    
    context.getOperationRegistry().registerOperation("ChangeAvailability", [&context] () {
        return new Ocpp16::ChangeAvailability(context.getModel());});
    context.getOperationRegistry().registerOperation("ChangeConfiguration", [] () {
        return new Ocpp16::ChangeConfiguration();});
    context.getOperationRegistry().registerOperation("ClearCache", [filesystem] () {
        return new Ocpp16::ClearCache(filesystem);});
    context.getOperationRegistry().registerOperation("GetConfiguration", [] () {
        return new Ocpp16::GetConfiguration();});
    context.getOperationRegistry().registerOperation("RemoteStartTransaction", [&context] () {
        return new Ocpp16::RemoteStartTransaction(context.getModel());});
    context.getOperationRegistry().registerOperation("RemoteStopTransaction", [&context] () {
        return new Ocpp16::RemoteStopTransaction(context.getModel());});
    context.getOperationRegistry().registerOperation("Reset", [&context] () {
        return new Ocpp16::Reset(context.getModel());});
    context.getOperationRegistry().registerOperation("TriggerMessage", [&context] () {
        return new Ocpp16::TriggerMessage(context.getModel());});
    context.getOperationRegistry().registerOperation("UnlockConnector", [&context] () {
        return new Ocpp16::UnlockConnector(context.getModel());});

    /*
     * Register further message handlers to support echo mode: when this library
     * is connected with a WebSocket echo server, let it reply to its own requests.
     * Mocking an OCPP Server on the same device makes running (unit) tests easier.
     */
    context.getOperationRegistry().registerOperation("Authorize", [&context] () {
        return new Ocpp16::Authorize(context.getModel(), nullptr);});
    context.getOperationRegistry().registerOperation("StartTransaction", [&context] () {
        return new Ocpp16::StartTransaction(context.getModel(), nullptr);});
    context.getOperationRegistry().registerOperation("StatusNotification", [&context] () {
        return new Ocpp16::StatusNotification(-1, OcppEvseState::NOT_SET, Timestamp());});
    context.getOperationRegistry().registerOperation("StopTransaction", [&context] () {
        return new Ocpp16::StopTransaction(context.getModel(), nullptr);});
}

void ChargeControlCommon::loop() {
    //do nothing
}
