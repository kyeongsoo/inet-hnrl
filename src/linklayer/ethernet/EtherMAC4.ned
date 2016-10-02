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

//
// Implementation of EtherMAC2 with priority-based flow control (PFC).
// For details other than PFC implementation, refer to EtherMAC2.
//
// <b>PFC</b>
//
// In addition to a PAUSE frame, EtherMAC4 can handle a PFC frame as well,
// which is an extension of a PAUSE frame with priorities.
// In the current implementation, we assume that actual handling of a PFC
// frame is to be done by an output queue module (* currently supporting
// "DRRVLANQueue4" only *) by directly calling
// the ProcessPFCCommand function of the output queue module.
//
// @see EtherMAC2
//
simple EtherMAC4
{
    parameters:
        bool promiscuous = default(false);     // if true, all packets are received, otherwise only the
                                // ones with matching destination MAC address
        string address = default("auto");       // MAC address as hex string (12 hex digits), or
                                // "auto". "auto" values will be replaced by
                                // a generated MAC address in init stage 0.
        int txQueueLimit = default(1000); // maximum number of frames queued up for transmission;
                                // additional frames are dropped. Only used if queueModule==""
        string queueModule = default("");    // name of optional external queue module
        int mtu = default(1500);
        @display("i=block/queue");
    gates:
        input upperLayerIn @labels(EtherFrame);    // to EtherLLC or MACRelayUnitPP
        output upperLayerOut @labels(EtherFrame);  // to EtherLLC or MACRelayUnitPP
        inout phys @labels(EtherFrame);            // to physical layer or the network
}
