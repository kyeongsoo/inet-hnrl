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

#ifndef __INET_ETHER_MAC4_H
#define __INET_ETHER_MAC4_H

#include "DRRVLANQueue4.h"
#include "EtherMAC2.h"

/**
 * Implementation of EtherMAC2 with priority-based flow control (PFC).
 */
class INET_API EtherMAC4: public EtherMAC2
{
protected:
    // output queue
    DRRVLANQueue4 *queue;

    virtual void initialize();

    // event handlers
    virtual void frameReceptionComplete(EtherFrame *frame);
};

#endif

