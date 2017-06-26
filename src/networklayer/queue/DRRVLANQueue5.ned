//
// Copyright (C) 2017 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


package inet.networklayer.queue;

import inet.networklayer.queue.BasicTokenBucketMeter;

//
// Model for an ISP traffic controller enabling proportional allocation of
// excess bandwidth allocation.
// An incoming packet is first classfied by a VLAN classifier and then metered
// by token buckets. The metered packet is put into per-subscriber queues served
// by a DRR scheduler with conformance information, which enables to maintain two
// separate lists of active queues with conformed and non-conformed packets.
// Conformed and non-conformed bits/packets sent are separately measured.
// Major difference comapred to DRRVLANQueue4 is marking of non-conformed packets
// using IEEE 802.1Q drop eligible indicator (DEI) field.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2017-06-26
//
simple DRRVLANQueue5 like OutputQueue
{
    parameters:
        @labels(node,queue-node);
        @display("i=block/subqueue;bgb=130,90");
        int numFlows = default(1);	// number of flows (i.e., subscribers)
		int fifoSize @unit(B) = default(15220000B);	// FIFO queue size; for compatibility with DRRVLANQueue module, and not used
        int voqSize @unit(B) = default(15220000B);	// per-flow virtual output queue (VOQ) size; default value corresponds to 10,000 maximum-sized VLAN frames.
        string quanta = default("1522");	// DRR quantum sizes in byte
        string classifierClass = default("BasicVLANClassifier");  // class that inherits from IQoSClassifier        
		string vids = default("");	// VIDs used for initialization of index table
    gates:
        input in;
        output out;
}
