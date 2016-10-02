//
// Copyright (C) 2016 Kyeong Soo (Joseph) Kim
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


#include "DropTailRRVLANTBFQueue4.h"

Define_Module(DropTailRRVLANTBFQueue4);

void DropTailRRVLANTBFQueue4::initialize(int stage)
{
    DropTailRRVLANTBFQueue2::initialize(stage);

    if (stage == 0)
    {
        // Relay Unit
        relay = check_and_cast<MACRelayUnitNPWithVLAN *>(getParentModule()->getParentModule()->getSubmodule("relayUnit"));

        // Per-subscriber VOQs
        voqThreshold = par("voqThreshold").longValue();
    }

    if (stage == 1)
    {   // the following should be initialized in the 2nd stage (stage==1)
        // because token bucket meters are not initialized in the 1st stage yet.

        // 1 pause unit is 512 bit times; we assume 10Gb MAC here.
        // TODO: set it properly with the actual TX rate of a MAC
        pauseUnits.assign(numFlows, 0);
        pauseInterval.assign(numFlows, 0.0);
        pauseLastSent = 0.0;
        for (int i=0; i<numFlows; i++)
        {
            pauseInterval[i] = (voqThreshold * 8)
                / tbm[i]->getMeanRate(); // in second
            pauseUnits[i] = int(pauseInterval[i] * 10e9 / PAUSE_BITTIME);
            pauseUnits[i] = pauseUnits[i] > 65535 ? 65535 : pauseUnits[i];
        }
    }
}

void DropTailRRVLANTBFQueue4::handleMessage(cMessage *msg)
{
    if (warmupFinished == false)
    {   // start statistics gathering once the warm-up period has passed.
        if (simTime() >= simulation.getWarmupPeriod()) {
            warmupFinished = true;
            for (int i = 0; i < numFlows; i++)
            {
                numPktsReceived[i] = voq[i]->getLength();   // take into account the frames/packets already in VOQs
            }
        }
    }

    if (msg->isSelfMessage())
    {   // Conformity Timer expires
        int flowIndex = msg->getKind();    // message kind carries a flow index
        conformityFlag[flowIndex] = true;

        // update TBF status
        cPacket *frontPkt = check_and_cast<cPacket *>(voq[flowIndex]->front());
        bool conformance = (tbm[flowIndex]->meterPacket(frontPkt) == 0) ? true : false;   // result of metering; 0 for conformed and 1 for non-conformed packet
// DEBUG
        ASSERT(conformance == true);
// DEBUG
        if (packetRequested > 0)
        {
            cPacket *pkt = check_and_cast<cPacket *>(dequeue());
            if (pkt != NULL)
            {
                packetRequested--;
                if (warmupFinished == true)
                {
                    numBitsSent[currentFlowIndex] += pkt->getBitLength();
                    numPktsSent[currentFlowIndex]++;
                }
                sendOut(pkt);
            }
            else
            {
                error("%s::handleMessage: Error in round-robin scheduling", getFullPath().c_str());
            }
        }
    }
    else
    {   // a frame arrives
        int flowIndex = classifier->classifyPacket(msg);
        cQueue *queue = voq[flowIndex];
        if (warmupFinished == true)
        {
            numPktsReceived[flowIndex]++;
        }
        int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG
        if (queue->isEmpty())
        {
            if (tbm[flowIndex]->meterPacket(msg) == 0)
            {   // packet is conformed
                if (warmupFinished == true)
                {
                    numPktsUnshaped[flowIndex]++;
                }
                if (packetRequested > 0)
                {
                    packetRequested--;
                    if (warmupFinished == true)
                    {
                        numBitsSent[flowIndex] += pktLength;
                        numPktsSent[flowIndex]++;
                    }
                    currentFlowIndex = flowIndex;
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
                    }
                    else
                    {
                        conformityFlag[flowIndex] = true;
                    }
                }
            }
            else
            {   // packet is not conformed
                bool dropped = enqueue(msg);
                if (dropped)
                {
                    if (warmupFinished == true)
                    {
                        numPktsDropped[flowIndex]++;
                    }
                }
                else
                {
                    triggerConformityTimer(flowIndex, pktLength);
                    conformityFlag[flowIndex] = false;
                }
            }
        }
        else
        {   // queue is not empty
            bool dropped = enqueue(msg);
            if (dropped)
            {
                if (warmupFinished == true)
                {
                    numPktsDropped[flowIndex]++;
                }
            }
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numPktsReceived[flowIndex], numPktsDropped[flowIndex]);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
}

bool DropTailRRVLANTBFQueue4::enqueue(cMessage *msg)
{
    int flowIndex = classifier->classifyPacket(msg);
    int pktByteLength = PK(msg)->getByteLength();

    if (voqCurrentSize[flowIndex] + pktByteLength > voqSize)
    {
        EV << "VOQ[" << flowIndex << "] full, dropping packet.\n";
        delete msg;
        return true;
    }
    else
    {
        // priority-based flow control: send PFC frame if above threshold
        // TODO: Implement it for non-stacked VLANs as well
        if (dynamic_cast<EthernetIIFrameWithVLAN *>(msg) != NULL)
        {
            EthernetIIFrameWithVLAN *vlanFrame = dynamic_cast<EthernetIIFrameWithVLAN *>(msg);
            uint16_t tpid = vlanFrame->getTpid();

            if (tpid == 0x88A8)
            {   // stacked-VLAN frame
                if ((voqCurrentSize[flowIndex] + pktByteLength > voqThreshold)
                        && (SIMTIME_DBL(simTime()) - pauseLastSent > pauseInterval[flowIndex]))
                {
                    PFCPriorityEnableVector pev;
                    pev.fill(false);
                    pev[0] = true; // enable PCF for the lowest priority flow (i.e., non-conformed frames)
                    PFCTimeVector tv;
                    tv.fill(pauseUnits[flowIndex]);
                    relay->sendPFCFrameWithVLANAddress(vlanFrame->getSrc(),
                            vlanFrame->getVid(), pev, tv);
                    pauseLastSent = SIMTIME_DBL(simTime());
                }
            }
        }

        voq[flowIndex]->insert(msg);
        voqCurrentSize[flowIndex] += pktByteLength;

        return false;
    }
}
