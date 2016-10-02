//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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


cplusplus {{
#include <array>
#include <stack>
#include "Ethernet.h"
#include "MACAddress.h"
#include "Ieee802Ctrl_m.h" // for enums
#include "VLAN.h"
typedef std::stack<VLANTag> VLANTagStack;
typedef std::array<bool, 8> PFCPriorityEnableVector;
typedef std::array<uint16_t, 8> PFCTimeVector;
}}


class noncobject MACAddress;

class noncobject MessageId;

class noncobject VLANTagStack;

class noncobject PFCPriorityEnableVector;

class noncobject PFCTimeVector;

enum EtherType;

enum SAPCode;

//
// Represents jam on the Ethernet.
//
packet EtherJam
{
}

//
// Common base class for classes representing Ethernet II and 802.3 frame types,
// containing their common header fields.
//
// This class should never be instantiated by the models, only specific
// subclasses: EthernetIIFrame, EtherFrameWithLLC and EtherFrameWithSNAP.
//
// Source and destination MAC address are stored in data members.
// Other fields include:
//  - preamble: not stored (only contributes length)
//  - length: stored in cMessage length, not stored explicitly
//  - payload: stored a encapsulated packet (cMessage::encapsulate())
//  - crc: represented by cMessage::hasBitError()
//
packet EtherFrame
{
    MACAddress dest;
    MACAddress src;
        //MessageId id; -- uncomment for tracing frames
}


//
// Ethernet II headers contain a 16-bit EtherType to identify
// the encapsulated protocol.
//
// Header length: src(6)+dest(6)+etherType(2) = 14 bytes
//
packet EthernetIIFrame extends EtherFrame
{
    int etherType;
}


//
// Ethernet II frame with VLAN (IEEE 802.1Q) tag.
//
// Header length: src(6)+dest(6)+802.1Q(=tpid+pcp+cfi+vid; 4)+etherType/length(2) = 18 bytes
//
//packet EthernetIIFrameWithVLAN extends EthernetIIFrame
//{
//    uint16_t tpid = 0x8100; // tag protocol identifier (16 bits; set to 0x8100)
//    uint8_t pcp; // priority code point for IEEE 802.1p class of service (3 bits; 0 (lowest) to 7 (highest))
//    bool dei; // drop eligible indicator (1 bit)
//    uint16_t vid; // VLAN identifier (12 bits; 0x000 and 0xFFF are reserved, which allows up to 4094 VLANs)
//}


//
// Ethernet II frame with VLAN (IEEE 802.1Q) tag(s).
//
// Header length: src(6)+dest(6)+802.1Q(=tpid+pcp+cfi+vid; 4)*n+etherType/length(2) = 14 + 4*n bytes
//
packet EthernetIIFrameWithVLAN extends EthernetIIFrame
{
    uint16_t tpid; // tag protocol identifier (16 bits; set to 0x8100 for C-TAG & 0x88A8 for S-TAG)
    uint8_t pcp; // priority code point for IEEE 802.1p class of service (3 bits; 0 (lowest) to 7 (highest))
    bool dei; // drop eligible indicator (1 bit)
    uint16_t vid; // VLAN identifier (12 bits; 0x000 and 0xFFF are reserved, which allows up to 4094 VLANs)
    
    // optional; IEEE 802.1Q-in-Q stacked VLANs.
	VLANTagStack innerTags; // based on std::stack
}


//
// Ethernet frame with 802.3 LLC header.
//
// Header length: src(6)+dest(6)+length(2)+ssap(1)+dsap(1)+control(1) = 17 bytes
//
packet EtherFrameWithLLC extends EtherFrame
{
    int ssap;
    int dsap;
    int control;
}


//
// Ethernet frame with 802.3 LLC and SNAP headers.
//
// The fields ssap, dsap and control are not included in the class because
// they are always set to fixed values: 0xAA, 0xAA, 0x03.
//
// Header length: src(6)+dest(6)+length(2)+ssap(1)+dsap(1)+control(1)+
// orgCode(3)+localCode(2) = 22 bytes
//
packet EtherFrameWithSNAP extends EtherFrame
{
    long orgCode;  // SNAP organization code, 0 for IP and ARP
    int localcode; // SNAP local code (stores EtherType for IP and ARP)
}


//
// Ethernet frame used by the PAUSE protocol
//
packet EtherPauseFrame extends EtherFrame
{
    int pauseTime; // in 512 bit-time units
}


//
// Ethernet control frame (currently tailored for PFC frame)
//
packet EtherControlFrame extends EthernetIIFrame
{
    uint16_t opcode; // 2-byte MAC control opcode (e.g., '00-01' for PAUSE, '01-01' for priority-based flow control (PFC)
    
    // below are MAC control parameters; currently only for PFC
    PFCPriorityEnableVector pev; // each element indicating if the corresponding element of the timeVector paramter is valid
    PFCTimeVector tv; // a list of eight 2-octet fields indicating pause times in 512 bit-time units
}


//
// Ethernet model components (EtherMAC, EtherHub and EtherBus) send this
// message at the beginning of the simulation, so that "auto"-valued "txrate"
// and "duplex" parameters of EtherMAC can be initialized. The purpose is
// similar to Ethernet Auto-Negotiation; however this is NOT meant to be
// the model of that Ethernet feature. (For example, EtherBus also sends
// an EtherAutoconfig message in the model, which obviously does not happen in
// a real Ethernet.)
//
// - txrate: EtherMAC with non-auto txrate send configured txrate value;
//   EtherBus sends 10Mbps (0 means not set). Lowest value will be chosen by
//   all EtherMACs, or if no txrate was advertised (all were 0), 100Mbps is chosen.
// - halfDuplex: EtherHub, EtherBus, plus EtherMAC with duplexEnabled=false setting
//   send true. Duplex operation only chosen by a MAC if nobody vetoed it by sending
//   halfDuplex=true.
//
packet EtherAutoconfig
{
    double txrate = 0;
    bool halfDuplex = false;
}


