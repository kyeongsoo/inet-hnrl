//
// Copyright (C) 2016 Kyeong Soo (Joseph) Kim
// Copyright (C) 2006 Levente Meszaros
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

#include "EtherMAC4.h"

Define_Module(EtherMAC4);



void EtherMAC4::initialize()
{
    EtherMAC2::initialize();

    // Output Queue
    queue = check_and_cast<DRRVLANQueue4 *>(
            getParentModule()->getSubmodule("queue"));
}

void EtherMAC4::frameReceptionComplete(EtherFrame *frame)
{
    int pauseUnits;
    EtherPauseFrame *pauseFrame;
    EtherControlFrame *controlFrame;

    if ((pauseFrame=dynamic_cast<EtherPauseFrame*>(frame))!=NULL)
    {
        pauseUnits = pauseFrame->getPauseTime();
        delete frame;
        numPauseFramesRcvd++;
        numPauseFramesRcvdVector.record(numPauseFramesRcvd);
        processPauseCommand(pauseUnits);
    }
    else if ((controlFrame=dynamic_cast<EtherControlFrame*>(frame))!=NULL)
    {
        if (controlFrame->getOpcode() == 0x0101)
        {   // PFC frame
            PFCPriorityEnableVector pev;
            PFCTimeVector tv;

            pev = controlFrame->getPev();
            tv = controlFrame->getTv();
            queue->processPFCCommand(pev, tv);
        }
        else
        {   // TODO: implement handling other control frames
            delete frame;
        }
    }
    else
    {
        // processReceivedDataFrame((EtherFrame *)frame);
        // modified by Kyeong Soo (Joseph) Kim
        processReceivedDataFrame(check_and_cast<EtherFrame *>(frame));
    }
}
