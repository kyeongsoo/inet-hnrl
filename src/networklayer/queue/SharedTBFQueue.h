//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
// Copyright (C) 2005 Andras Varga
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


#ifndef __INET_SHAREDTBFQUEUE_H
#define __INET_SHAREDTBFQUEUE_H

#include <omnetpp.h>
#include <sstream>
#include <vector>
#include "PassiveQueueBase.h"
#include "IQoSClassifier.h"

/**
 * Returns the maximum of a and b.
 */
inline double max(const double a, const double b) { return (a > b) ? a : b; }

/**
 * Drop-tail queue with VLAN classifier, token bucket filter (TBF) traffic shaper,
 * and round-robin (RR) scheduler.
 * See NED for more info.
 */
class SharedTBFQueue : public PassiveQueueBase
{
    // type definitions for member variables
    typedef std::vector<bool> BoolVector;
    typedef std::vector<int> IntVector;
    typedef std::vector<long long> LongLongVector;
	typedef std::vector<double> DoubleVector;
    typedef std::vector<cMessage *> MsgVector;
    typedef std::vector<cQueue *> QueueVector;
    typedef std::vector<simtime_t> TimeVector;

  protected:
	// configuration
	bool useSharing;
	bool useSharedBucket;
	int frameCapacity;
	int numQueues;
	double meanRateTotal;
	int mtu;   // in bit; note that the corresponding parameter in NED/INI is in byte.
	double peakRate;
	double threshValue;
	double donationValue;
	int lengthPerBPS;
	simtime_t earliestThreshTime;
	simtime_t secondEarliestThreshTime;

    // state
	IQoSClassifier *classifier;
	QueueVector queues;
	TimeVector lastTime; // vector of the last time the TBF used
	BoolVector conformityFlag;  // vector of flag to indicate whether the HOL frame conforms to TBF
	int currentEarliestQueue;
	
	// components of each queue
	LongLongVector meanBucketLength;  // vector of the number of tokens (bits) in the bucket for mean rate/burst control
	LongLongVector modBucketLength;
	IntVector peakBucketLength;  // vector of the number of tokens (bits) in the bucket for peak rate/MTU control
	BoolVector isActive;
	DoubleVector meanRate;
	DoubleVector modRate;
	DoubleVector contribution;
	LongLongVector bucketSize;    // in bit; note that the corresponding parameter in NED/INI is in byte.
	TimeVector threshTime;
	TimeVector lastDequeueTime;
	
	// controller components
	double sharedRate;
	long long sharedBucketSize;
	long long sharedBucketLength;

    // state: RR scheduler
    int currentQueueIndex;  // index of a queue whose HOL frame is scheduled for TX during the last RR scheduling

    // statistics
    bool warmupFinished;        ///< if true, start statistics gathering
    IntVector numQueueReceived; // redefined from PassiveQueueBase with 'name hiding'
    IntVector numQueueDropped;  // redefined from PassiveQueueBase with 'name hiding'
    IntVector numQueueUnshaped;
//    IntVector numQueueShaped;
    IntVector numQueueSent;

    // timer
    MsgVector conformityTimer;  // vector of timer indicating that enough tokens will be available for the transmission of the HOL frame

    cGate *outGate;

  public:
    SharedTBFQueue();
    virtual ~SharedTBFQueue();

  protected:
    virtual void initialize();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void handleMessage(cMessage *msg);

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void finish();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual bool enqueue(cMessage *msg);

    /**
     * Redefined from PassiveQueueBase to implement round-robin (RR) scheduling.
     */
    virtual cMessage *dequeue();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void sendOut(cMessage *msg);

    /**
     * The queue should send a packet whenever this method is invoked.
     * If the queue is currently empty, it should send a packet when
     * when one becomes available.
     */
    virtual void requestPacket();

    /**
     * Newly defined.
     */
    virtual bool isConformed(int queueIndex, int pktLength);

    virtual void triggerConformityTimer(int queueIndex, int pktLength);

    virtual void dumpTbfStatus(int queueIndex);
	
	/**
	 * New for SharedTBFQueue.
	 */
	 
	 virtual void updateAll(); // update the whole system
	 
	 virtual void updateState(int queueIndex); // update the burst length of one queue
	 
	 virtual void updateOneQueue(int queueIndex); // like updateState, but is called after a packet is sent i.e. the threshold time has changed
	 
	 virtual simtime_t getThreshTime(int queueIndex);
};

#endif


