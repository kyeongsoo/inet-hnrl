//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


package inet.users;

//
// Interface for user behavior models controlling underlying applications (e.g., tcpapp, udpapp) in host modules.
//
// The user status (e.g., active, inactive) is broadcasted to all relevant applications via \NotificationBoard.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2013-05-01
//
simple OnoffUser like IUser
{
	parameters:
        volatile double onPeriod @unit(s);	// should usually be a random value, e.g. exponential(1)
        volatile double offPeriod @unit(s);	// should usually be a random value, e.g. exponential(1)
    	@display("i=abstract/person");
}

