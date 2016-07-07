///
/// @file   VLANTagger2.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   July/6/2016
///
/// @brief  Declares 'VLANTagger2' class for stacked VLAN tagging based on IEEE 802.1Q-in-Q.
///
/// @remarks Copyright (C) 2016 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __INET_VLAN_TAGGER2_H
#define __INET_VLAN_TAGGER2_H

#include "VLANTagger.h"

///
/// @class VLANTagger2
/// @brief Implements VLAN tagger module based on IEEE 802.1Q with stacked VLANs (Q-in-Q).
/// @ingroup vlan
///
class INET_API VLANTagger2 : public VLANTagger
{
    protected:
        bool stackedVlans; ///< (experimental) capable of stacking of the 2nd VLAN tag (S-TAG) in addition to the 1st one (C-TAG)

    public:
//        VLANTagger2();
//        ~VLANTagger2();
        bool isStacktedVlans() const {return stackedVlans;}

    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);

        typedef std::vector<EthernetIIFrameWithVLAN *> VLANFrameVector;

        // create stacked-VLAN Ethernet frame(s) for a given VLAN-tagged Ethernet frame
        virtual void tagPushFrame(EthernetIIFrameWithVLAN *vlanFrame, VLANFrameVector& vlanFrames);

        // extract VLAN tag from an Ethernet frame
        virtual EthernetIIFrame *untagFrame(EthernetIIFrameWithVLAN *msg);
};

#endif // __INET_VLAN_TAGGER2_H
