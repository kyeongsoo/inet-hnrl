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


package inet.networklayer.queue;

//
// Drop-tail queue with VLAN classifier,  token bucket filter (TBF) traffic
// shaper based on external token bucket meter, and round-robin (RR) scheduler
// to be used in network interfaces.
//
// Compared to DropTailRRVLANTBFQueue2, a major new feature is the capability
// of back pressure flow control: When a VOQ passes a threshold, it can send an
// Ethernet pause frame to a port, from which a flow corresponding to the VOQ
// comes. This is essential to the implementation of hybrid ISP traffic control.
// Note that this is enabled only for flows with stacked-VLANs.
//
// Conforms to the OutputQueue interface.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2016-07-16
//
simple DropTailRRVLANTBFQueue3 like OutputQueue
{
    parameters:
        int numFlows = default(1); // number of flows (i.e., per-VLAN queues)
        int voqSize @unit(B) = default(15220000B); // per-flow virtual output queue (VOQ) size; default value corresponds to 10,000 maximum-sized VLAN frames.
		int voqThreshold @unit(B) = default(int(0.25*this.voqSize)*1B); // VOQ threshold to detect link congestion
		string classifierClass = default("BasicVLANClassifier");  // class that inherits from IQoSClassifier
		string vids = default("");	// VIDs used for initialization of index table
        @display("i=block/queue");
    gates:
        input in;
        output out;
}
