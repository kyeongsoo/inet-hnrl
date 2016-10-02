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


#ifndef __INET_DRRVLANQUEUE4_H
#define __INET_DRRVLANQUEUE4_H

#include <array>
#include "EtherFrame_m.h"
#include "DRRVLANQueue3.h"

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
 * See NED for more info.
 */
class INET_API DRRVLANQueue4 : public DRRVLANQueue3
{
    protected:
        // states
        std::array<bool, N_PFC_PRIORITIES> priorityPaused;

        // self messages
        std::array<cMessage*, N_PFC_PRIORITIES> endPFCMsg;

    public:
        DRRVLANQueue4();
        virtual ~DRRVLANQueue4();

    protected:
        /**
         * Redefined from DRRVLANQueue.
         */
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        virtual cMessage *dequeue();

        /**
         * Newly defined.
         */
        virtual void scheduleEndPFCPeriod(int priority, int pauseUnits, simtime_t bitTime);

    public:
        /**
         * Newly defined.
         */
        virtual void processPFCCommand(PFCPriorityEnableVector pev, PFCTimeVector tv, simtime_t bitTime);
};

#endif
