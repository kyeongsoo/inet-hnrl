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
                    pev[1] = true;  // enable PCF for non-conformed frames
                                    // 1 is the lowest priority per IEEE 802.1Q 2014 Appendix I.4
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
