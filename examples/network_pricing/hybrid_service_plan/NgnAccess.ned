//
// Copyright (C) 2014 Kyeong Soo (Joseph) Kim
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


package inet.examples.network_pricing.hybrid_service_plan;

import inet.examples.ethernet.lans.ethernetline;
import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.access.NgnAccessModem;
import inet.nodes.access.NgnAccessModem2;
import inet.nodes.access.NgnAccessNode2;
import inet.nodes.access.NgnAccessNode3;
import inet.nodes.access.NgnAccessNode4;
import inet.nodes.access.NgnAccessSplitter;
import inet.nodes.access.NgnAccessSplitter2;
import inet.nodes.inet.RouterWithEthernetInterface2;
import inet.nodes.inet.StandardHost3WithEthernetInterface2;
import inet.nodes.inet.StandardHost3WithEthernetInterface2WithDLPerformanceMeter;
import inet.nodes.inet.StandardServerWithEthernetInterface2;


//
// 'NgnAccess' network with hosts with DLPerformanceMeter.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2013-07-15
//
network NgnAccess
{
    parameters:
        int N = default(1);	// number of ONUs (subscribers)
        int n = default(1);	// number of hosts (users) per ONU (subscriber)
        double uni_rate @unit(bps) = default(100Mbps);	// transmission rate of ONU Ethernet UNIs
        double uni_delay @unit(s) = default(0s);	// no delay at UNIs
        double distribution_rate @unit(bps) = default(10Gbps);	// transmission rate of distribution fiber (between Onu and OLT)
        double distribution_delay @unit(s) = default(150us);	// default delay of 150 us corresponds to 30 km fiber  (60 km is the max. logical reach of GPON)
        double backbone_rate @unit(bps) = default(100Gbps);	// transmission rate of backbone link (between Olt and server)
        double backbone_delay @unit(s) = default(4.7ms);	// default delay of 4.7 ms gives total RTT of 10 ms including delays in access
        double backbone_delay1 @unit(s) = default(2.0ms);	// together with backbone_delay2 (i.e. 4.7ms), it gives total RTT of 10 ms including delays in access
        double backbone_delay2 @unit(s) = default(2.7ms);
        @display("bgb=709,233");
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=52,42");
        }
        server: StandardServerWithEthernetInterface2 {
            parameters:
                @display("p=651,119;i=device/server_l");
            gates:
                ethg[1];
        }
        olt: NgnAccessNode2 {
            parameters:
                @display("p=388,119");
            gates:
                ethg[2];
        }
        accessSwitch: NgnAccessSplitter {
            parameters:
                @display("p=269,119");
            gates:
                ethg[N+1];
        }
        onu[N]: NgnAccessModem {
            parameters:
                @display("p=131,119");
                pvid = index+10;
            gates:
                ethg[n+1];
        }
        host[N*n]: StandardHost3WithEthernetInterface2WithDLPerformanceMeter {
            parameters:
                @display("i=device/laptop;p=46,119");
            gates:
                ethg[1];
        }
        router: RouterWithEthernetInterface2 {
            @display("p=507,120");
        }
    connections:
        for i=0..N-1, for j=0..n-1 {
            host[i*n+j].ethg[0] <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu[i].ethg[j];
        }
        for i=0..N-1 {
            onu[i].ethg[n] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> accessSwitch.ethg[i];
        }
        accessSwitch.ethg[N] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> olt.ethg[0];
        olt.ethg[1] <--> {  delay = backbone_delay1; datarate = backbone_rate; } <--> router.ethg++;
        router.ethg++ <--> {  delay = backbone_delay2; datarate = backbone_rate; } <--> server.ethg++;
}


//
// 'NgnAccess' network with stacked VLANs.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2016-07-06
//
network NgnAccessWithStackedVlans
{
    parameters:
        int N = default(1);	// number of ONUs (subscribers to a normal traffic control & pricing scheme)
        int Nh = default(1); // number of ONU_Cs (subscribers to a hybrid traffic control & pricing scheme)
        int n = default(1);	// number of hosts (users) per ONU (subscriber)
        double uni_rate @unit(bps) = default(100Mbps);	// transmission rate of ONU Ethernet UNIs
        double uni_delay @unit(s) = default(0s);	// no delay at UNIs
        double distribution_rate @unit(bps) = default(10Gbps);	// transmission rate of distribution fiber (between Onu and OLT)
        double distribution_delay @unit(s) = default(150us);	// default delay of 150 us corresponds to 30 km fiber  (60 km is the max. logical reach of GPON)
        double backbone_rate @unit(bps) = default(100Gbps);	// transmission rate of backbone link (between Olt and server)
        double backbone_delay @unit(s) = default(4.7ms);	// default delay of 4.7 ms gives total RTT of 10 ms including delays in access
        double backbone_delay1 @unit(s) = default(2.0ms);	// together with backbone_delay2 (i.e. 4.7ms), it gives total RTT of 10 ms including delays in access
        double backbone_delay2 @unit(s) = default(2.7ms);
        @display("bgb=834,303");
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=50,31");
        }
        server: StandardServerWithEthernetInterface2 {
            parameters:
                @display("p=776,104;i=device/server_l");
            gates:
                ethg[1];
        }
        server2: StandardServerWithEthernetInterface2 {
            parameters:
                @display("p=776,234;i=device/server_l");
            gates:
                ethg[1];
        }
        olt: NgnAccessNode3 {
            parameters:
                @display("p=513,104");
            gates:
                ethg[3];
        }
        olt_c: NgnAccessNode4 {
            parameters:
                @display("p=513,233");
            gates:
                ethg[2];
        }
        accessSwitch: NgnAccessSplitter2 {
            parameters:
                @display("p=394,104");
            gates:
                ethg[N+2];
        }
        onu[N]: NgnAccessModem2 {
            parameters:
                @display("p=246,104");
                pvid = index+10;
            gates:
                ethg[n+1];
        }
        onu2[Nh]: NgnAccessModem2 {
            parameters:
                @display("p=123,234");
                pvid = index+10;
            gates:
                ethg[n+1];
        }
        accessSwitch_c: NgnAccessSplitter {
            parameters:
                @display("p=246,234");
            gates:
                ethg[Nh+1];
        }
        host[N*n]: StandardHost3WithEthernetInterface2WithDLPerformanceMeter {
            parameters:
                @display("i=device/laptop;p=52,105");
            gates:
                ethg[1];
        }
        host2[Nh*n]: StandardHost3WithEthernetInterface2WithDLPerformanceMeter {
            parameters:
                @display("i=device/laptop;p=50,235");
            gates:
                ethg[1];
        }
        router: RouterWithEthernetInterface2 {
            @display("p=632,105");
        }

        router2: RouterWithEthernetInterface2 {
            @display("p=632,234");
        }
    connections:
        for i=0..N-1, for j=0..n-1 {
            host[i*n+j].ethg[0] <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu[i].ethg[j];
        }
        for i=0..Nh-1, for j=0..n-1 {
            host2[i*n+j].ethg[0] <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu2[i].ethg[j];
        }
        for i=0..N-1 {
            onu[i].ethg[n] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> accessSwitch.ethg[i];
        }
        for i=0..Nh-1 {
            onu2[i].ethg[n] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> accessSwitch_c.ethg[i];
        }
        accessSwitch_c.ethg[Nh] <--> {  datarate = distribution_rate; } <--> accessSwitch.ethg[N]; // no delay
        accessSwitch.ethg[N+1] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> olt.ethg[0];
        olt.ethg[1] <--> {  datarate = backbone_rate; } <--> olt_c.ethg[0]; // no delay
        olt.ethg[2] <--> {  delay = backbone_delay1; datarate = backbone_rate; } <--> router.ethg++;
        olt_c.ethg[1] <--> {  delay = backbone_delay1; datarate = backbone_rate; } <--> router2.ethg++;
        router.ethg++ <--> {  delay = backbone_delay2; datarate = backbone_rate; } <--> server.ethg++;
        router2.ethg++ <--> {  delay = backbone_delay2; datarate = backbone_rate; } <--> server2.ethg++;
}
