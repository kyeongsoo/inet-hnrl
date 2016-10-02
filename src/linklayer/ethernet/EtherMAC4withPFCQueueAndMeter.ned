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



package inet.linklayer.ethernet;

import inet.linklayer.INetworkInterface;
import inet.networklayer.queue.OutputQueue;
import inet.networklayer.queue.IQoSMeter;


//
// EtherMAC4 with output queue supporting priority-based flow control (PFC)
// and meter (e.g., token bucket) for QoS and RED; to enable PFC on a port,
// queueType must be set to PFC-enabled queue (e.g., DRRVLANQueue4).
//
//
// @see EtherMAC4, DropTailQueue, DropTailTBFQueue, REDQueue, DRRVLANQueue4
//
module EtherMAC4withPFCQueueAndMeter
{
    parameters:
        bool promiscuous = default(false);     // if true, all packets are received, otherwise only the
                                // ones with matching destination MAC address
        string address = default("auto");       // MAC address as hex string (12 hex digits), or
                                // "auto". "auto" values will be replaced by
                                // a generated MAC address in init stage 0.
        int mtu = default(1500);
        int numFlows = default(0);	// number of flows used for per-flow queues and meters
        string queueType = default("DropTailQueue");
        string meterType = default("BasicTokenBucketMeter");
        @display("i=block/ifcard;bgb=243,263");
    gates:
        input upperLayerIn @labels(EtherFrame);    // to EtherLLC or MACRelayUnitPP
        output upperLayerOut @labels(EtherFrame);  // to EtherLLC or MACRelayUnitPP
        inout phys @labels(EtherFrame);            // to physical layer or the network
    submodules:
        queue: <queueType> like OutputQueue {
            parameters:
                @display("p=60,85;q=l2queue");
        }
        meter[numFlows]: <meterType> like IQoSMeter {
            parameters:
                @display("p=172,85");
        }
        mac: EtherMAC4 {
            parameters:
                promiscuous = promiscuous;
                address = address;
                mtu = mtu;
                queueModule = "queue";
                txQueueLimit = 1; // queue sends one packet at a time
                @display("p=88,165");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> queue.in;
        queue.out --> mac.upperLayerIn;
        upperLayerOut <-- { @display("m=n"); } <-- mac.upperLayerOut;
        mac.phys <--> { @display("m=s"); } <--> phys;
}
