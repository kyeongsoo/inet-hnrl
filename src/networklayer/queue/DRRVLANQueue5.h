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


#ifndef __INET_DRRVLANQUEUE5_H
#define __INET_DRRVLANQUEUE5_H

#include <array>
#include "EtherFrame_m.h"
#include "DRRVLANQueue4.h"

/**
 * Priority queueing (PQ) over FIFO queue and per-flow queues with
 * deficit round-robin (DRR) scheduler.
 * Incoming packets are classified by an external VLAN classifier and
 * metered by an external token bucket meters before being put into
 * a FIFO or per-flow queues.
 * The numbers of conformed and non-conformed bits/packets sent are now
 * separately measured for billing (i.e., network pricing study).
 * IEEE 802.1Q priority-based flow control (PFC) is implemented for
 * conformed (i.e., higher priority) and non-conformed flows (i.e.,
 * lower priority).
 * Non-conformed packets are now marked by setting IEEE 802.1Q
 * drop eligible indicator (DEI) field to 1.
 * See NED for more info.
 */
class INET_API DRRVLANQueue5 : public DRRVLANQueue4
{
    protected:
        /**
         * Redefined from DRRVLANQueue.
         */
        virtual void handleMessage(cMessage *msg);
        virtual cMessage *dequeue();
};

#endif
