///
/// @file   VLAN.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jan/5/2012
///
/// @brief  Define various types and constants for VLAN support based on IEEE 802.1Q.
///
/// @remarks Copyright (C) 2012, Kyeong Soo (Joseph) Kim.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __INET_VLAN_H
#define __INET_VLAN_H

// 32-bit fields of IEEE 802.1Q header
//typedef unsigned int TPID;  ///< tag protocol identifier (16 bits)
//// the following are subfields of 16-bit tag control information (TCI):
//typedef unsigned short PCP; ///< priority code point (3 bits)
//typedef unsigned short DEI; ///< drop eligible indicator (1 bit)

typedef uint16_t VID; ///< VLAN identifier type

//typedef struct
//{
//    unsigned short pcp; ///< priority code point (3 bits)
//    bool dei; ///< drop eligible indicator (1 bit)
//    VID vid;    ///< VLAN identifier (12 bits)
//} TCI; ///< tag control information (TCI) type

typedef struct {
    uint16_t tpid = 0x8100; // tag protocol identifier (16 bits; set to 0x8100)
    uint8_t pcp; // priority code point for IEEE 802.1p class of service (3 bits; 0 (lowest) to 7 (highest))
    bool dei; // drop eligible indicator (1 bit)
    uint16_t vid; // VLAN identifier (12 bits; 0x000 and 0xFFF are reserved, which allows up to 4094 VLANs)
//    unsigned int tpid; ///< tag protocol identifier (16 bits)
//    TCI tci; ///< 16-bit tag control information (TCI)
} VLANTag;

//#define ETHER_VLAN_TAG_LENGTH   (4) // tpid(2)+tci(2)
//const int   ETHER_1Q_TAG_LENGTH = 4;    // VID(12bit)+DEI(1bit)+PCP(3bit)+TPID(2B)
//#define ETHER_1AH_ITAG_LENGTH       (6) /* I-Tag(6)+B-Tag(4)+B-src(6)+B-dest(6) */
//#define ETHER_MAC_2ND_FRAME_BYTES   (12)  /* src(6)+dest(6)    Does not have Length/Type and FCS*/
//#define ETHER_II_DISPLAY_STRING     "b=,,,#656665"
//#define ETHER_1Q_DISPLAY_STRING     "b=,,,#659965"
//#define ETHER_1AD_DISPLAY_STRING    "b=,,,#65cc65"
//#define ETHER_1AH_DISPLAY_STRING    "b=,,,#65ff65"

#endif  // _INET_VLAN_H
