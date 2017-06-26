//
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


#include "DropTailRRVLANTBFQueue5.h"

Define_Module(DropTailRRVLANTBFQueue5);

bool DropTailRRVLANTBFQueue5::enqueue(cMessage *msg)
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
        // congestion control: drop marked packets if above threshold
        if (dynamic_cast<EthernetIIFrameWithVLAN *>(msg) != NULL)
        {
            EthernetIIFrameWithVLAN *vlanFrame = dynamic_cast<EthernetIIFrameWithVLAN *>(msg);
            uint16_t tpid = vlanFrame->getTpid();

            if (tpid == 0x88A8)
            {   // stacked-VLAN frame
                if ((voqCurrentSize[flowIndex] + pktByteLength > voqThreshold) && vlanFrame->getDei())
                {
                    EV << "VOQ[" << flowIndex << "] exceeds threshold, dropping marked packet.\n";
                    delete vlanFrame;
                    return true;
                }
            }
        }

        voq[flowIndex]->insert(msg);
        voqCurrentSize[flowIndex] += pktByteLength;

        return false;
    }
}
