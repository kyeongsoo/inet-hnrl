//
// copyright (C) 2012 Kyeong Soo (Joseph) Kim
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#ifndef __INET_UDPBURSTAPP_H
#define __INET_UDPBURSTAPP_H

#include "UDPBasicApp.h"


const int UDP_MAX_PAYLOAD = 1472; // = 1500-byte MTU of Ethernet - 8-byte UDP header - 20-byte IP header


/**
 * UDP application for burst packet generation. See NED for more info.
 */
class INET_API UDPBurstApp : public UDPBasicApp
{
protected:
    cMessage *burstTimer, *messageTimer;
    int messageLength;
    double lineRate;
    
    virtual cPacket *createPacket(int payloadLength);
    virtual void sendPacket();
//    virtual void sendToUDPDelayed(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort, double delay);

public:
  UDPBurstApp();
  virtual ~UDPBurstApp();

protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif
