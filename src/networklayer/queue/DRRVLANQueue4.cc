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


#include "DRRVLANQueue4.h"

Define_Module(DRRVLANQueue4);

void DRRVLANQueue4::initialize()
{
    // initialize states

}

cMessage *DRRVLANQueue4::dequeue()
{
    cMessage *msg = (cMessage *)NULL;

    // check first the list of queues with non-zero conformed bytes
    while (!conformedList.empty()) {
        int flowIndex = conformedList.front();
        conformedList.pop_front();
        // DEBUG
        ASSERT(!voq[flowIndex]->isEmpty());
        // DEBUG
        int pktByteLength = PK(voq[flowIndex]->front())->getByteLength();
        if (conformedCounters[flowIndex] >= pktByteLength)
        {   // serve the packet
            msg = (cMessage *)voq[flowIndex]->pop();
            voqCurrentSize[flowIndex] -= pktByteLength;
            conformedCounters[flowIndex] -= pktByteLength;
#ifndef NDEBUG
            conformedCounterVector[flowIndex]->record(conformedCounters[flowIndex]);
#endif
            // update statistics
            numConformedBitsSent[flowIndex] += 8*pktByteLength;
            numConformedPktsSent[flowIndex]++;

            // check whether the conformed counter value is enough for the HOL packet
            if (!voq[flowIndex]->isEmpty() && (conformedCounters[flowIndex] >= PK(voq[flowIndex]->front())->getByteLength()))
            {
                conformedList.push_back(flowIndex);
            }
            return msg;
        }
    }   // end of while ()

    // check then the list of queues with non-zero non-conformed bytes and do regular DRR scheduling
    if (nonconformedList.empty())
    {
        continuation = false;
        return (msg);
    }
    else
    {
        while (!nonconformedList.empty())
        {
            int flowIndex = nonconformedList.front();
            nonconformedList.pop_front();
            // DEBUG
            ASSERT(!voq[flowIndex]->isEmpty());
            // DEBUG
            deficitCounters[flowIndex] += continuation ? 0 : quanta[flowIndex];
            continuation = false;   // reset the flag

            int pktByteLength = PK(voq[flowIndex]->front())->getByteLength();
            if ((deficitCounters[flowIndex] >= pktByteLength) && (nonconformedCounters[flowIndex] >= pktByteLength))
            {   // serve the packet
                msg = (cMessage *)voq[flowIndex]->pop();
                voqCurrentSize[flowIndex] -= pktByteLength;
                deficitCounters[flowIndex] -= pktByteLength;
                nonconformedCounters[flowIndex] -= pktByteLength;
#ifndef NDEBUG
                nonconformedCounterVector[flowIndex]->record(nonconformedCounters[flowIndex]);
#endif
                // update statistics
                numNonconformedBitsSent[flowIndex] += 8*pktByteLength;
                numNonconformedPktsSent[flowIndex]++;
            }

            // check whether the deficit and non-conformed counter values are enough for the HOL packet
            if (!voq[flowIndex]->isEmpty())
            {
                pktByteLength = PK(voq[flowIndex]->front())->getByteLength();                
                if (nonconformedCounters[flowIndex] >= pktByteLength)
                {
                    if (deficitCounters[flowIndex] >= pktByteLength)
                    {   // set the flag and put the index back to the front of the list
                        continuation = true;
                        nonconformedList.push_front(flowIndex);
                    }
                    else
                    {
                        nonconformedList.push_back(flowIndex);
                    }
                }
                else
                {
                    deficitCounters[flowIndex] = 0;
                }
            }
            else
            {
                deficitCounters[flowIndex] = 0;
            }

            if (msg != NULL)
            {
                break;  // from the while loop
            }
        } // end of while()
    }

    return (msg);   // just in case
}

void DRRVLANQueue4::processPFCCommand(PFCPriorityEnableVector pev, PFCTimeVector tv)
{

}
