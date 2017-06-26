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


#include "DRRVLANQueue5.h"

Define_Module(DRRVLANQueue5);

void DRRVLANQueue5::handleMessage(cMessage *msg)
{
    if (warmupFinished == false)
    {   // start statistics gathering once the warm-up period has passed.
        if (simTime() >= simulation.getWarmupPeriod()) {
            warmupFinished = true;
            for (int i = 0; i < numFlows; i++)
            {
                numPktsReceived[i] = voq[i]->getLength();   // take into account the packets already in VOQs
            }
        }
    }

    if (!msg->isSelfMessage())
    {   // a packet arrives
        int flowIndex = classifier->classifyPacket(msg);
        int pktLength = PK(msg)->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG
        int pktByteLength = PK(msg)->getByteLength();
        int color = tbm[flowIndex]->meterPacket(msg);   // result of metering; 0 for conformed and 1 for non-conformed packet
        if (warmupFinished == true)
        {
            numPktsReceived[flowIndex]++;
            if (color == 0)
            {   // packet is conformed
                numPktsConformed[flowIndex]++;
            }
        }

        if (packetRequested > 0)
        {
            packetRequested--;
#ifndef NDEBUG
            pktReqVector.record(packetRequested);
#endif
            if (warmupFinished == true)
            {
                numBitsSent[flowIndex] += pktLength;
                numPktsSent[flowIndex]++;
                if (color == 0) {
                    numConformedBitsSent[flowIndex] += pktLength;
                    numConformedPktsSent[flowIndex]++;
                } else {
                    numNonconformedBitsSent[flowIndex] += pktLength;
                    numNonconformedPktsSent[flowIndex]++;
                    if (dynamic_cast<EthernetIIFrameWithVLAN *>(msg) != NULL)
                    {   // mark a non-conformed packet if the packet is IEEE 802.1Q frame
                        ((EthernetIIFrameWithVLAN *)msg)->setDei(true);
                    }
                }
            }
            sendOut(msg);
        }
        else
        {
            bool dropped = enqueue(msg);
            if (dropped)
            {
                if (warmupFinished == true)
                {
                    numPktsDropped[flowIndex]++;
                }
                if (color == 0)
                {   // exchange of the values of counters to emulate priority queueing
                    if (nonconformedCounters[flowIndex] >= pktByteLength)
                    {
                        conformedCounters[flowIndex] += pktByteLength;
                        nonconformedCounters[flowIndex] -= pktByteLength;
#ifndef NDEBUG
                        conformedCounterVector[flowIndex]->record(conformedCounters[flowIndex]);
                        nonconformedCounterVector[flowIndex]->record(nonconformedCounters[flowIndex]);
#endif
                        // update active list for conformed packets
                        IntList::iterator iter = std::find(conformedList.begin(), conformedList.end(), flowIndex);
                        if (iter == conformedList.end())
                        {   // add flow index to active list
                            conformedList.push_back(flowIndex);
                        }

                        // update active list for non-conformed packets
                        if (nonconformedCounters[flowIndex] == 0)
                        {
                            nonconformedList.remove(flowIndex);
                        }
                    }
                }
            }
            else
            {
                if (color == 0)
                {   // packet is conformed
                    conformedCounters[flowIndex] += pktByteLength;
#ifndef NDEBUG
                    conformedCounterVector[flowIndex]->record(conformedCounters[flowIndex]);
#endif
                    IntList::iterator iter = std::find(conformedList.begin(), conformedList.end(), flowIndex);
                    if (iter == conformedList.end())
                    {   // add flow index to active list
                        conformedList.push_back(flowIndex);
                    }
                }
                else
                {   // packet is not conformed
                    nonconformedCounters[flowIndex] += pktByteLength;
#ifndef NDEBUG
                    nonconformedCounterVector[flowIndex]->record(nonconformedCounters[flowIndex]);
#endif
                    IntList::iterator iter = std::find(nonconformedList.begin(), nonconformedList.end(), flowIndex);
                    if (iter == nonconformedList.end())
                    {   // add flow index to active list
                        nonconformedList.push_back(flowIndex);
                    }
                }
            }
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numPktsReceived[flowIndex], numPktsDropped[flowIndex]);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }   // end of if () for non-self message
    else
    {   // endPFCMsg
        // DEBUG
        ASSERT(priorityPaused[msg->getKind()] == true);
        // DEBUG
        priorityPaused[msg->getKind()] = false;
    }
}

cMessage *DRRVLANQueue5::dequeue()
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

    // Check then the list of queues with non-zero non-conformed bytes and do regular
    // DRR scheduling only when priorityPaused[1] is false; we associate non-conformed
    // traffic with the priority value of 1.
    //
    // Note that, according to IEEE 802.1Q 2014 Appendix I.4, the priority value of 1
    // is associated with 'Background' traffic and has a lower priority than the default
    // value of 0 for 'Best Effort' traffic.
    if (priorityPaused[1] == false)
    {
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
                    msg = (cMessage *) voq[flowIndex]->pop();
                    if (dynamic_cast<EthernetIIFrameWithVLAN *>(msg) != NULL)
                    {   // mark a non-conformed packet if the packet is IEEE 802.1Q frame
                        ((EthernetIIFrameWithVLAN *)msg)->setDei(true);
                    }

                    voqCurrentSize[flowIndex] -= pktByteLength;
                    deficitCounters[flowIndex] -= pktByteLength;
                    nonconformedCounters[flowIndex] -= pktByteLength;
#ifndef NDEBUG
                    nonconformedCounterVector[flowIndex]->record(nonconformedCounters[flowIndex]);
#endif
                    // update statistics
                    numNonconformedBitsSent[flowIndex] += 8 * pktByteLength;
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
    } // end of if() for checking priorityPaused[1]

    return (msg);   // just in case
}
