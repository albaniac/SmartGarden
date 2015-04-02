/*
        Remote Station Protocol support for Sprinklers system.

  This module is intended to be used with my multi-station environment monitoring and sprinklers control system (SmartGarden),
  as well as modified version of the OpenSprinkler code and with modified sprinkler control program sprinklers_pi.


This module handles remote communication protocol. Initial version operates over Xbee link in API mode, but protocol itself is
generic and can operate over multiple transports.

 Creative Commons Attribution-ShareAlike 3.0 license
 Copyright 2014 tony-osp (http://tony-osp.dreamwidth.org/)


*/

#ifndef _SGRPROTOCOL_S_h
#define _SGRPROTOCOL_S_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Time.h>
#include "port.h"
#include <inttypes.h>

#include "SGRProtocol.h"        // wire protocol definitions

typedef bool (*PTransportCallback)(uint8_t nStation, void *msg, size_t mSize);
typedef bool (*PARPCallback)(uint8_t nStation, uint8_t *pNetAddress);

class RProtocolSlave {

public:
				RProtocolSlave();

                bool            begin(void);
                void            ProcessNewFrame(uint8_t *ptr, int len, uint8_t *pNetAddress);

                void            RegisterTransport(PTransportCallback ptr);

                bool            SendZonesReport(uint8_t transactionID, uint8_t toUnitID, uint8_t firstZone, uint8_t numZones);
                bool            SendSensorsReport(uint8_t transactionID, uint8_t toUnitID, uint8_t firstSensor, uint8_t numSensors);
                bool            SendSystemRegisters(uint8_t transactionID, uint8_t toUnitID, uint8_t firstRegister, uint8_t numRegisters);
                bool            SendEvtMasterReport(uint8_t transactionID, uint8_t toUnitID);

                bool            SendPingReply(uint8_t transactionID, uint8_t toUnitID, uint32_t cookie);
                bool            SendOKResponse(uint8_t transactionID, uint8_t toUnitID, uint8_t FCode);
                bool            SendErrorResponse(uint8_t transactionID, uint8_t toUnitID, uint8_t fCode, uint8_t errorCode);

                uint8_t         myUnitID;
                uint16_t        PANId;

                // transport callback, will be populated by the caller beforehand.
                PTransportCallback _SendMessage;
// ARP address update
				PARPCallback		_ARPAddressUpdate;

private:

};

// holding registers area size
//
// Currently we support up to 8 zones (can be changed if necessary)
#define         MODBUSMAP_SYSTEM_MAX    (MREGISTER_ZONE_COUNTDOWN + 8)

extern RProtocolSlave rprotocol;

#endif  //_SGRPROTOCOL_S_h
