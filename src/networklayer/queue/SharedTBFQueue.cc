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


#include "SharedTBFQueue.h"
#include <simtime.h>

Define_Module(SharedTBFQueue);

SharedTBFQueue::SharedTBFQueue()
{
//    queues = NULL;
//    numQueues = NULL;
}

SharedTBFQueue::~SharedTBFQueue()
{
    for (int i=0; i<numQueues; i++)
    {
        delete queues[i];
        cancelAndDelete(conformityTimer[i]);
		cancelAndDelete(conformityTimer[i+numQueues]);
    }
}

void SharedTBFQueue::initialize()
{	
    PassiveQueueBase::initialize();

	// configuration
	useSharing = par("useSharing");
	useSharedBucket = par("useSharedBucket");
	frameCapacity = par("frameCapacity");
	numQueues = par("numQueues");
	mtu = par("mtu").longValue()*8; // in bit
	peakRate = par("peakRate"); // in bps
	lengthPerBPS = par("lengthPerBPS"); // for bucket size
	threshValue = 0.95;
	donationValue = 0.90;
//	earliestThreshTime = SimTime::getMaxTime();
//	secondEarliestThreshTime = SimTime::getMaxTime();

    // state
    const char *classifierClass = par("classifierClass");
    classifier = check_and_cast<IQoSClassifier *>(createOne(classifierClass));
    classifier->setMaxNumQueues(numQueues);
	currentEarliestQueue = 0;

    outGate = gate("out");

    // set subqueues
    queues.assign(numQueues, (cQueue *)NULL);
    meanBucketLength.assign(numQueues, 0.0); // was bucketSize
    peakBucketLength.assign(numQueues, mtu);
    lastTime.assign(numQueues, simTime());
	threshTime.assign(numQueues, simTime());
    conformityFlag.assign(numQueues, false);
    conformityTimer.assign(numQueues*2, (cMessage *)NULL);
	isActive.assign(numQueues, true);
	meanRate.assign(numQueues, 0.0);
	modRate.assign(numQueues, 0.0);
	bucketSize.assign(numQueues, 0.0);
	contribution.assign(numQueues, 0.0);
	threshTime.assign(numQueues, simTime());
	lastDequeueTime.assign(numQueues, 0);
	
    for (int i=0; i<numQueues; i++)
    {
        char buf[32];
        sprintf(buf, "queue-%d", i);
        queues[i] = new cQueue(buf);
        conformityTimer[i] = new cMessage("Conformity Timer", i);   // message kind carries a queue index
		conformityTimer[i+numQueues] = new cMessage("Threshold Timer", i+numQueues);
    }

    // state: RR scheduler
    currentQueueIndex = 0;

    // statistic
    warmupFinished = false;
    numQueueReceived.assign(numQueues, 0);
    numQueueDropped.assign(numQueues, 0);
//	numQueueShaped.assign(numQueues, 0);
    numQueueUnshaped.assign(numQueues, 0);
    numQueueSent.assign(numQueues, 0);
	
	meanRateTotal = 0.0;
	
	for (int i=0; i<numQueues; i++)
	{
		std::stringstream meanRateParam;
		meanRateParam << "meanRate" << i;
		meanRate[i] = par((meanRateParam.str()).c_str()); // in bps
	//	bucketSize[i] = par("bucketSize").longValue()*8LL; // in bit
		bucketSize[i] = max(mtu, meanRate[i] * lengthPerBPS); // in bit
		meanRateTotal += meanRate[i];
	}
	
	// get contribution values (between 0 and 1)
	for (int i=0; i<numQueues; i++)
	{
		contribution[i] = meanRate[i] / meanRateTotal;
	}
	
	updateAll();
}



// =============
//  Q U E U E S
// =============

void SharedTBFQueue::handleMessage(cMessage *msg)
{
    if (warmupFinished == false)
    {   // start statistics gathering once the warm-up period has passed.
        if (simTime() >= simulation.getWarmupPeriod()) {
            warmupFinished = true;
            for (int i = 0; i < numQueues; i++)
            {
                numQueueReceived[i] = queues[i]->getLength();   // take into account the frames/packets already in queues
            }
        }
    }

    if (msg->isSelfMessage())
    {   // Conformity Timer expires
        int queueIndex = msg->getKind();    // message kind carries a queue index
		
		if (queueIndex >= 0 && queueIndex < numQueues) // conformity message
		{
			// update TBF status
			int pktLength = (check_and_cast<cPacket *>(queues[queueIndex]->front()))->getBitLength();
			ASSERT(isConformed(queueIndex, pktLength)); 
			conformityFlag[queueIndex] = true;
			updateOneQueue(queueIndex);
			
			if (packetRequested > 0)
			{
				cMessage *msg = dequeue();
				if (msg != NULL)
				{
					packetRequested--;
					if (warmupFinished == true)
					{
						numQueueSent[currentQueueIndex]++;
					}
					sendOut(msg);
				}
				else
				{
					error("%s::handleMessage: Error in round-robin scheduling", getFullPath().c_str());
				}
			}
		}
		else // queue threshold message
		{
			updateAll();
		}
    }
    else
    {   // a frame arrives
        int queueIndex = classifier->classifyPacket(msg);
        cQueue *queue = queues[queueIndex];
        if (warmupFinished == true)
        {
            numQueueReceived[queueIndex]++;
        }
        int pktLength = (check_and_cast<cPacket *>(msg))->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG
        if (queue->isEmpty())
        {
            if (isConformed(queueIndex, pktLength))
            {
                if (warmupFinished == true)
                {
                    numQueueUnshaped[queueIndex]++;
                }
                if (packetRequested > 0)
                {
                    packetRequested--;
                    if (warmupFinished == true)
                    {
                        numQueueSent[queueIndex]++;
                    }
					lastDequeueTime[queueIndex] = simTime();
					updateOneQueue(queueIndex);
                    currentQueueIndex = queueIndex;
                    sendOut(msg);
                }
                else
                {
                    bool dropped = enqueue(msg);
                    if (dropped)
                    {
						updateOneQueue(queueIndex);
                        if (warmupFinished == true)
                        {
                            numQueueDropped[queueIndex]++;
                        }
                    }
                    else
                    {
                        conformityFlag[queueIndex] = true;
						updateOneQueue(queueIndex);
                    }
                }
            }
            else
            {
//                numQueueShaped[queueIndex]++;
                bool dropped = enqueue(msg);
                if (dropped)
                {
                    if (warmupFinished == true)
                    {
                        numQueueDropped[queueIndex]++;
                    }
                }
                else
                {
                    triggerConformityTimer(queueIndex, pktLength);
                    conformityFlag[queueIndex] = false;
                }
            }
        }
        else
        {
            bool dropped = enqueue(msg);
            if (dropped)
            {
                if (warmupFinished == true)
                {
                    numQueueDropped[queueIndex]++;
                }
            }
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numQueueReceived[queueIndex], numQueueDropped[queueIndex]);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
}

bool SharedTBFQueue::enqueue(cMessage *msg)
{
    int queueIndex = classifier->classifyPacket(msg);
    cQueue *queue = queues[queueIndex];

    if (frameCapacity && queue->length() >= frameCapacity)
    {
        EV << "Queue " << queueIndex << " full, dropping packet.\n";
        delete msg;
        return true;
    }
    else
    {
        queue->insert(msg);
        return false;
    }
}

cMessage *SharedTBFQueue::dequeue()
{
    bool found = false;
	simtime_t earliest = SimTime::getMaxTime();
	
/*	if (useSharing)
	{ // longest wait first
		for (int i = 0; i < numQueues; i++)
		{
			if (conformityFlag[i])
			{
				found = true;
				if (lastDequeueTime[i] < earliest)
				{
					earliest = lastDequeueTime[i];
					currentQueueIndex = i;
				}
			}
		}
	}
	else */
	{ // round robin
		int startQueueIndex = (currentQueueIndex + 1) % numQueues;  // search from the next queue for a frame to transmit
		for (int i = 0; i < numQueues; i++)
		{
		   if (conformityFlag[(i+startQueueIndex)%numQueues])
		   {
			   currentQueueIndex = (i+startQueueIndex)%numQueues;
			   found = true;
			   break;
		   }
		}
	}

    if (!found)
    {
        // TO DO: further processing?
        return NULL;
    }
	
	lastDequeueTime[currentQueueIndex] = simTime();
	
	if (conformityTimer[currentQueueIndex]->isScheduled())
	{
		EV << "Error: pop while conformity timer is scheduled." << endl;
	//	cancelEvent(conformityTimer[currentQueueIndex]);
	}
    cMessage *msg = (cMessage *)queues[currentQueueIndex]->pop();

    // TO DO: update statistics

    // conformity processing for the HOL frame
    if (!queues[currentQueueIndex]->isEmpty())
    {
        int pktLength = (check_and_cast<cPacket *>(queues[currentQueueIndex]->front()))->getBitLength();
        if (isConformed(currentQueueIndex, pktLength))
        {
            conformityFlag[currentQueueIndex] = true;
			updateOneQueue(currentQueueIndex);
        }
        else
        {
            conformityFlag[currentQueueIndex] = false;
            triggerConformityTimer(currentQueueIndex, pktLength);
        }
    }
    else
    {
        conformityFlag[currentQueueIndex] = false;
    }

    // return a packet from the scheduled queue for transmission
    return (msg);
}

void SharedTBFQueue::requestPacket()
{
    Enter_Method("requestPacket()");

    cMessage *msg = dequeue();
    if (msg==NULL)
    {
        packetRequested++;
    }
    else
    {
        if (warmupFinished == true)
        {
            numQueueSent[currentQueueIndex]++;
        }
        sendOut(msg);
    }
}

void SharedTBFQueue::updateState(int i) // i = queue index
{
	simtime_t now = simTime();
	
//	unsigned long long meanTemp = meanBucketLength[i] + (unsigned long long)(((isActive[i] ? meanRate[i] : 0.0) + modRate[i])*(now - lastTime[i]).dbl() + 0.5);
	unsigned long long meanTemp = meanBucketLength[i] + (unsigned long long)ceil(((isActive[i] ? meanRate[i] : 0.0) + modRate[i])*(now - lastTime[i]).dbl());
	meanBucketLength[i] = (unsigned long long)((meanTemp > bucketSize[i]) ? bucketSize[i] : meanTemp);
//	unsigned long long peakTemp = peakBucketLength[i] + (unsigned long long)(peakRate*(now - lastTime[i]).dbl() + 0.5);
	unsigned long long peakTemp = peakBucketLength[i] + (unsigned long long)ceil(peakRate*(now - lastTime[i]).dbl());
	peakBucketLength[i] = int((peakTemp > mtu) ? mtu : peakTemp);
	
	lastTime[i] = now;
}

simtime_t SharedTBFQueue::getThreshTime(int i) // i = queue index
{
	updateState(i);                                                           // data remaining / rate
	simtime_t time2 = simTime() + 0.001 + ((bucketSize[i] * threshValue) - meanBucketLength[i]) / ((isActive[i] ? meanRate[i] : 0.0) + modRate[i]);
//	simtime_t time2 = simTime() + ((bucketSize[i] * threshValue) - meanBucketLength[i]) / ((isActive[i] ? meanRate[i] : 0.0) + modRate[i]);
	
	EV << "Threshold time calculations for queue " << i << endl;
	EV << "meanRate = " << meanRate[i] / 1e6 << " Mbps" << endl;
	EV << "bucketSize = " << bucketSize[i] / 1e6 / 8 << " MB" << endl;
	EV << "Time to reach threshold: ~" << bucketSize[i] / meanRate[i] << " seconds" << endl;
	EV << "Thresh time = " << time2 << endl;
	
	return time2;
}

bool SharedTBFQueue::isConformed(int queueIndex, int pktLength)
{
    Enter_Method("isConformed()");

// DEBUG
    EV << "Last Time = " << lastTime[queueIndex] << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Packet Length = " << pktLength << endl;
// DEBUG
	
    // update state for this one queue
	updateState(queueIndex);

    if (pktLength <= meanBucketLength[queueIndex])
    {
        if  (pktLength <= peakBucketLength[queueIndex])
        {
            meanBucketLength[queueIndex] -= pktLength;
            peakBucketLength[queueIndex] -= pktLength;
            return true;
        }
    }
    return false;
}

void SharedTBFQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

// trigger TBF conformity timer for the HOL frame in the queue,
// indicating that enough tokens will be available for its transmission
void SharedTBFQueue::triggerConformityTimer(int queueIndex, int pktLength)
{
    Enter_Method("triggerConformityCounter()");
	
	updateState(queueIndex);
	
    double meanDelay = (pktLength - meanBucketLength[queueIndex]) / ((isActive[queueIndex] ? meanRate[queueIndex] : 0.0) + modRate[queueIndex]);
    double peakDelay = (pktLength - peakBucketLength[queueIndex]) / peakRate;

// DEBUG
    EV << "Packet Length = " << pktLength << endl;
    dumpTbfStatus(queueIndex);
    EV << "Delay for Mean TBF = " << meanDelay << endl;
    EV << "Delay for Peak TBF = " << peakDelay << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Counter Expiration Time = " << simTime() + max(meanDelay, peakDelay) << endl;
// DEBUG
	
//	bool isScheduled = conformityTimer[queueIndex]->isScheduled();
	
	if (conformityTimer[queueIndex]->isScheduled())
	{
		EV << "Trying to send conformity message already scheduled" << endl;
	//	cancelEvent(conformityTimer[queueIndex]);
	}
	scheduleAt(simTime() + max(meanDelay, peakDelay), conformityTimer[queueIndex]);
}

void SharedTBFQueue::dumpTbfStatus(int queueIndex)
{
    EV << "Last Time = " << lastTime[queueIndex] << endl;
    EV << "Current Time = " << simTime() << endl;
    EV << "Token bucket for mean rate/burst control " << endl;
    EV << "- Bucket size [bit]: " << bucketSize[queueIndex] << endl;
	EV << "- Mean rate [bps]: " << meanRate[queueIndex] << endl;
    EV << "- Bucket length [bit]: " << meanBucketLength[queueIndex] << endl;
    EV << "Token bucket for peak rate/MTU control " << endl;
    EV << "- MTU [bit]: " << mtu << endl;
    EV << "- Peak rate [bps]: " << peakRate << endl;
    EV << "- Bucket length [bit]: " << peakBucketLength[queueIndex] << endl;
}

void SharedTBFQueue::finish()
{
    unsigned long sumQueueReceived = 0;
    unsigned long sumQueueDropped = 0;
    unsigned long sumQueueShaped = 0;
//    unsigned long sumQueueUnshaped = 0;

    for (int i=0; i < numQueues; i++)
    {
        std::stringstream ss_received, ss_dropped, ss_shaped, ss_sent;
        ss_received << "packets received by per-VLAN queue[" << i << "]";
        ss_dropped << "packets dropped by per-VLAN queue[" << i << "]";
        ss_shaped << "packets shaped by per-VLAN queue[" << i << "]";
        ss_sent << "packets sent by per-VLAN queue[" << i << "]";
        recordScalar((ss_received.str()).c_str(), numQueueReceived[i]);
        recordScalar((ss_dropped.str()).c_str(), numQueueDropped[i]);
        recordScalar((ss_shaped.str()).c_str(), numQueueReceived[i]-numQueueUnshaped[i]);
        recordScalar((ss_sent.str()).c_str(), numQueueSent[i]);
        sumQueueReceived += numQueueReceived[i];
        sumQueueDropped += numQueueDropped[i];
        sumQueueShaped += numQueueReceived[i] - numQueueUnshaped[i];
    }
    recordScalar("overall packet loss rate of per-VLAN queues", sumQueueDropped/double(sumQueueReceived));
    recordScalar("overall packet shaped rate of per-VLAN queues", sumQueueShaped/double(sumQueueReceived));
}



// =====================
//  C O N T R O L L E R
// =====================

void SharedTBFQueue::updateAll()
{
	double tempSharedRateUsage = 0.0;
	float tempContrib;
	float tempActiveContribTotal = 0;
	
	// update own shared bucket first
	if (useSharedBucket)
	{
		// TODO: Complete this, similar to updateState()
	}
	
	sharedRate = 0.0;
	
	// get latest state of all queues
	for (int i=0; i<numQueues; i++)
	{
		updateState(i);
	}
	
	if (useSharing){
		// gather rates from donating subscribers and cancel all conformity/threshold timers
		for (int i=0; i<numQueues; i++)
		{
			if (meanBucketLength[i] >= bucketSize[i] * threshValue)
			{
				isActive[i] = false;
			}
			else
			{
				isActive[i] = true;
				tempActiveContribTotal += contribution[i];
			}		
			if (!isActive[i])
			{
				modRate[i] = meanRate[i] * (1 - donationValue);
				sharedRate += meanRate[i] * donationValue;
			}
			cancelEvent(conformityTimer[i]);
			cancelEvent(conformityTimer[i+numQueues]);
		}
		
		// then distribute rates among active subscribers and send new conformity/threshold timers
		for (int i=0; i<numQueues; i++)
		{
			if (isActive[i])
			{
				tempContrib = contribution[i] / tempActiveContribTotal;
				modRate[i] = (tempContrib * sharedRate > meanRate[i] ? meanRate[i] : tempContrib * sharedRate);
				tempSharedRateUsage += modRate[i];
				threshTime[i] = getThreshTime(i);
				scheduleAt(threshTime[i], conformityTimer[i+numQueues]);
			}
			if (!queues[i]->isEmpty() && !conformityFlag[i])
			{
				triggerConformityTimer(i, (check_and_cast<cPacket *>(queues[i]->front()))->getBitLength());
			}
		}
	}
	
	sharedRate -= tempSharedRateUsage;
}

// called when a packet is conformed and its meanBucketLength is reduced
// must be called AFTER that queues' conformityFlag is set (this occurs outside isConformed())
void SharedTBFQueue::updateOneQueue(int queueIndex)
{
	if (meanBucketLength[queueIndex] < (bucketSize[queueIndex] * threshValue) && useSharing)
	{
		if (!isActive[queueIndex])
		{
			updateAll();
		}
		else
		{
			threshTime[queueIndex] = getThreshTime(queueIndex);
			cancelEvent(conformityTimer[queueIndex+numQueues]);
			scheduleAt(threshTime[queueIndex], conformityTimer[queueIndex+numQueues]);
		}
	}
}
