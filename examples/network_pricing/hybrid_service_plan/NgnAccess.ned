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
import inet.nodes.access.NgnAccessNode2;
import inet.nodes.access.NgnAccessSplitter;
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
        @display("bgb=745,357");
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=52,42");
        }
        server: StandardServerWithEthernetInterface2 {
            parameters:
                @display("p=659,116;i=device/server_l");
            gates:
                ethg[1];
        }
        server_c: StandardServerWithEthernetInterface2 {
            parameters:
                @display("p=659,246;i=device/server_l");
            gates:
                ethg[1];
        }
        olt: NgnAccessNode2 {
            parameters:
                @display("p=396,116");
            gates:
                ethg[3];
        }
        olt_c: NgnAccessNode2 {
            parameters:
                @display("p=396,245");
            gates:
                ethg[2];
        }
        accessSwitch: NgnAccessSplitter {
            parameters:
                @display("p=277,116");
            gates:
                ethg[N+2];
        }
        accessSwitch_c: NgnAccessSplitter {
            parameters:
                @display("p=277,245");
            gates:
                ethg[Nh+1];
        }
        onu[N]: NgnAccessModem {
            parameters:
                @display("p=139,116");
                pvid = index+10;
            gates:
                ethg[n+1];
        }
        onu_c[Nh]: NgnAccessModem {
            parameters:
                @display("p=139,245");
                pvid = index+10;
            gates:
                ethg[n+1];
        }
        host[N*n]: StandardHost3WithEthernetInterface2WithDLPerformanceMeter {
            parameters:
                @display("i=device/laptop;p=54,116");
            gates:
                ethg[1];
        }
        host_c[Nh*n]: StandardHost3WithEthernetInterface2WithDLPerformanceMeter {
            parameters:
                @display("i=device/laptop;p=52,246");
            gates:
                ethg[1];
        }
        router: RouterWithEthernetInterface2 {
            @display("p=515,117");
        }

        router_c: RouterWithEthernetInterface2 {
            @display("p=515,246");
        }
    connections:
        for i=0..N-1, for j=0..n-1 {
            host[i*n+j].ethg[0] <--> { delay = uni_delay; datarate = uni_rate; } <--> onu[i].ethg[j];
        }
        for i=0..Nh-1, for j=0..n-1 {
            host_c[i*n+j].ethg[0] <--> { delay = uni_delay; datarate = uni_rate; } <--> onu_c[i].ethg[j];
        }
        for i=0..N-1 {
            onu[i].ethg[n] <--> { delay = distribution_delay; datarate = distribution_rate; } <--> accessSwitch.ethg[i];
        }
        for i=0..Nh-1 {
            onu_c[i].ethg[n] <--> { delay = distribution_delay; datarate = distribution_rate; } <--> accessSwitch_c.ethg[i];
        }
        accessSwitch_c.ethg[Nh] <--> { datarate = distribution_rate; } <--> accessSwitch.ethg[N]; // no delay
        accessSwitch.ethg[N+1] <--> { delay = distribution_delay; datarate = distribution_rate; } <--> olt.ethg[0];
        olt.ethg[1] <--> { delay = backbone_delay1; datarate = backbone_rate; } <--> router.ethg++;
        olt.ethg[2] <--> { datarate = backbone_rate; } <--> olt_c.ethg[0]; // no delay
        olt_c.ethg[1] <--> { delay = backbone_delay1; datarate = backbone_rate; } <--> router_c.ethg++;
        router.ethg++ <--> { delay = backbone_delay2; datarate = backbone_rate; } <--> server.ethg++;
        router_c.ethg++ <--> { delay = backbone_delay2; datarate = backbone_rate; } <--> server_c.ethg++;
}
