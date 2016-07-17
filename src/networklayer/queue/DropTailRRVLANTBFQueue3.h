//
// Copyright (C) 2016 Kyeong Soo (Joseph) Kim
// Copyright (C) 2005 Andras Varga
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


#ifndef __INET_DROPTAILVLANTBFQUEUE3_H
#define __INET_DROPTAILVLANTBFQUEUE3_H

#include "DropTailRRVLANTBFQueue2.h"
#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "MACRelayUnitNPWithVLAN.h"

/**
 * Drop-tail queue with VLAN classifier, token bucket filter (TBF) traffic
 * shaper based on external token bucket meter, round-robin (RR) scheduler,
 * and back pressure flow control.
 * See NED for more info.
 */
class INET_API DropTailRRVLANTBFQueue3 : public DropTailRRVLANTBFQueue2
{
  protected:
    // relay unit
    MACRelayUnitNPWithVLAN *relay;

    // flow control
    IntVector pauseUnits; ///< pause time in 512 bit times
    DoubleVector pauseInterval; ///< pause time in second
    double pauseLastSent; ///< last time a pause frame was sent

    // RR scheduler
    int voqThreshold; ///< VOQ threshold in byte to detect link congestion


//    // timer
//    MsgVector conformityTimer;  ///< vector of timer indicating that enough tokens will be available for the transmission of the HOL frame

  public:

  protected:
    virtual void initialize(int stage);

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void handleMessage(cMessage *msg);

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual bool enqueue(cMessage *msg);
};

#endif


