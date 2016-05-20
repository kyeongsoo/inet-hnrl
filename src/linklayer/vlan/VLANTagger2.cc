///
/// @file   VLANTagger2.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   July/6/2016
///
/// @brief  Implements 'VLANTagger2' class for stacked VLAN tagging based on IEEE 802.1Q-in-Q.
///
/// @remarks Copyright (C) 2016 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///

#include "VLANTagger2.h"

// Register modules.
Define_Module(VLANTagger2);

//VLANTagger2::VLANTagger2()
//{
//}
//
//VLANTagger2::~VLANTagger2()
//{
//}

void
VLANTagger2::initialize()
{
    VLANTagger::initialize();

//    tagged = (bool) par("tagged");
//    dynamicTagging = (bool) par("dynamicTagging");
//    minVid = (int) par("minVid");
//    maxVid = (int) par("maxVid");
//
//    // assign the range of VID values into 'vidSet'
//    // FIXME allow arbitrary set of VID values later
//    vidSet.clear();
//    for (int i = minVid; i <= maxVid; i++)
//    {
//        vidSet.push_back((VID)i);
//    }
//
//    if (tagged == false)
//    {
//        if (dynamicTagging == false)
//        { // untagged port with static tagging
//            if (vidSet.size() == 1)
//            {
//                pvid = vidSet[0];
//            }
//            else
//            {
//                error("No or multiple PVIDs for untagged port with static tagging.");
//            }
//        }
//        else
//        { // get an access to the relay module for untagged port with dynamic tagging
//            cModule *relayModule = getParentModule()->getSubmodule("relayUnit");
//            relay = check_and_cast<MACRelayUnitNPWithVLAN *>(relayModule);
//        }
//    }
    stackedVlans = (bool) par("stackedVlans");
}

void VLANTagger2::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        error("No self message in VLANTagger2.");
    }
    
    if (msg->getArrivalGateId() == findGate("macg$i"))
    {   // ingress rule checking
        if (tagged)
        {
            if (dynamic_cast<EthernetIIFrameWithVLAN *>(msg) != NULL)
            {
                VID vid = dynamic_cast<EthernetIIFrameWithVLAN *>(msg)->getVid();
                if ((vid == 0x000) || (vid == 0xFFF))
                {
                    EV << "VID of 0x000 (null) or 0xFFF (reserved) is not allowed.\n";
                    delete msg;
                }
                else
                {
                    // processTaggedFrame(msg);
                    send(msg, "relayg$o");
                }
            }
            else
            {
                EV << "Tagged port cannot receive a frame without a VLAN tag. Drop.\n";
                delete msg;
            }
        }
        else
        {
            if (dynamic_cast<EthernetIIFrameWithVLAN *>(msg) == NULL)
            {   // Ethernet frame without VLAN tag
                VLANFrameVector vlanFrames;
                tagFrame(check_and_cast<EthernetIIFrame *>(msg), vlanFrames);
                VLANFrameVector::iterator it;
                for (it = vlanFrames.begin(); it < vlanFrames.end(); it++)
                {
                    send(*it, "relayg$o");
                }
            }
            else
            {   // Ethernet frame with VLAN tag
                if (stackedVlans)
                {
                    if (((EthernetIIFrameWithVLAN *) msg)->getTpid() == 0x8100)
                    {
                        VLANFrameVector vlanFrames;
                        tagPushFrame(check_and_cast<EthernetIIFrameWithVLAN *>(msg), vlanFrames);
                        VLANFrameVector::iterator it;
                        for (it = vlanFrames.begin(); it < vlanFrames.end(); it++)
                        {
                            send(*it, "relayg$o");
                        }
                    }
                    else
                    {
                        EV << "Untagged port with 'stackedVlans' enabled cannot receive a frame with stacked VLAN tags. Drop.\n";
                        delete msg;
                    }
                }
                else
                {
                    EV << "Untagged port cannot receive a frame with a VLAN tag. Drop.\n";
                    delete msg;
                }
            }
        }
    }
    else if (msg->getArrivalGateId() == findGate("relayg$i"))
    {   // egress rule checking
        if (tagged)
        {
            // processTaggedFrame(msg);
            send(msg, "macg$o");
        }
        else
        {
            if (dynamic_cast<EthernetIIFrameWithVLAN *>(msg) != NULL)
            {
//                untagFrame(check_and_cast<EthernetIIFrameWithVLAN *>(msg));
//                // processTaggedFrame(msg);
//                send(msg, "macg$o");
                EthernetIIFrame *frame = untagFrame(check_and_cast<EthernetIIFrameWithVLAN *>(msg));
                if (frame != NULL)
//                    send(untagFrame(check_and_cast<EthernetIIFrameWithVLAN *>(msg)), "macg$o");
                    send(frame, "macg$o");
            }
            else
            {
                error("Unknown frame. Modify VLANTagger2 to make it allowed or filtered.");
                delete msg;
            }
        }            
    }
    else
    {
        error("Unknown arrival gate");            
    }
}


void VLANTagger2::tagPushFrame(EthernetIIFrameWithVLAN *frame, VLANFrameVector& vlanFrames)
{
    VIDVector vids;

    if (dynamicTagging == true)
    {   // dynamic tagging based on the VLAN address table
        int vid = relay->getVIDForMACAddress(frame->getDest());
        if (vid < 0)
        {   // either there is no matching VID for the MAC address or the frame is for broadcasting
            vids = vidSet;
        }
        else
        {
            vids.push_back(vid);
        }
    }
    else
    {   // static tagging based on PVID
        vids.push_back(pvid);
    }

    EthernetIIFrameWithVLAN *vlanFrame = frame->dup();
    VLANTag vlanTag = {frame->getTpid(), frame->getPcp(), frame->getDei(), frame->getVid()};
    vlanFrame->getInnerTags().push(vlanTag);
    vlanFrame->setTpid(0x88A8); // for S-TAG
    vlanFrame->setByteLength(vlanFrame->getByteLength() + ETHER_VLAN_TAG_LENGTH); // double tags
//    cPacket *pkt = frame->decapsulate();
//    if (pkt != NULL)
//        vlanFrame->encapsulate(pkt);
    delete frame;

    VIDVector::iterator it;
    for (it = vids.begin(); it < vids.end(); it++)
    {
        EthernetIIFrameWithVLAN *vf = vlanFrame->dup();
        vf->setVid(*it);
        vlanFrames.push_back(vf);
    }
    delete vlanFrame;
}


EthernetIIFrame *VLANTagger2::untagFrame(EthernetIIFrameWithVLAN *vlanFrame)
{
    uint16_t tpid = vlanFrame->getTpid();

    if (!stackedVlans)
    {
        if (tpid == 0x8100)
        {   // standard IEEE 802.1Q tagged frame without stacked tag
            EthernetIIFrame *frame = new EthernetIIFrame;
            frame->setDest(vlanFrame->getDest());
            frame->setSrc(vlanFrame->getSrc());
            frame->setEtherType(vlanFrame->getEtherType());
            frame->setByteLength(ETHER_MAC_FRAME_BYTES);
            cPacket *pkt = vlanFrame->decapsulate();
            if (pkt != NULL)
                frame->encapsulate(pkt);
            delete vlanFrame;
            return frame;
        }
        if (tpid == 0x88A8)
        {   // stacked-tag frame for a port without stacked-VLAN support as a result of broadcasting
            delete vlanFrame; // ignore and just delete it
            return NULL;
        }
    }

    if (stackedVlans)
    {
        if (tpid == 0x88A8)
        {   // stacked tag(s)
            VLANTag vtag = vlanFrame->getInnerTags().top();
            vlanFrame->getInnerTags().pop();
            vlanFrame->setTpid(0x8100);
            vlanFrame->setVid(vtag.vid);
            return check_and_cast<EthernetIIFrame *>(vlanFrame);
        }
        if (tpid == 0x8100)
        {   // single-tagged frame for a stacked-VLAN port as a result of broadcasting
            delete vlanFrame; // ignore and just delete it
            return NULL;
        }
    }

    EV << getFullPath() << endl;
    error("Unknown TPID value");
    return NULL; // to suppress compiler warning messages
}
