//
// Copyright (C) 2017 Kyeong Soo (Joseph) Kim
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


#ifndef __INET_DROPTAILVLANTBFQUEUE5_H
#define __INET_DROPTAILVLANTBFQUEUE5_H

#include "DropTailRRVLANTBFQueue3.h"

/**
 * Drop-tail queue with VLAN classifier, token bucket filter (TBF) traffic
 * shaper based on external token bucket meter, round-robin (RR) scheduler,
 * and congestion control based on dropping marked packets.
 * See NED for more info.
 */
class INET_API DropTailRRVLANTBFQueue5 : public DropTailRRVLANTBFQueue3
{
  protected:
    /**
     * Redefined from PassiveQueueBase.
     */
    virtual bool enqueue(cMessage *msg);
};

#endif
