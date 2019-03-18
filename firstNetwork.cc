#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ExampleForFirstScript");

int main (int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);
 
    Time::SetResolution (Time::NS);
	LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

	NodeContainer nodes;
	nodes.Create (2); // Create function from NodeContainer class creates 2 nodes.

	PointToPointHelper pointToPoint; //The PointToPointHelper creates a link between two nodes.
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue("5Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue("2ms"));

	NetDeviceContainer devices;
	devices = pointToPoint.Install(nodes);
	
	InternetStackHelper stackNet;
	stackNet.Install (nodes);

	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0"); //10.1.1.0 is set as the domain of the network.

	Ipv4InterfaceContainer interfaces = address.Assign(devices); //We needed to interface the nodes(using the IP address, ofcourse).

	UdpEchoServerHelper echoServer(3000);//We need to run an application from A to B or B to A, so we install an application on one of the node.
									  //3000 is the port number.

	ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));//Get is a function from NodeContainer. Get(0) would have been correct too.
	serverApps.Start(Seconds (1.0)); //Its echoServer, so it means, whatever we send from client to server, we will get that back.
	serverApps.Stop(Seconds(10.0));

	UdpEchoClientHelper echoClient (interfaces.GetAddress(1), 3000);//GetAddress gets the address of second device ie is device 1
	echoClient.SetAttribute("MaxPackets", UintegerValue(1)); //
	echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
	echoClient.SetAttribute("PacketSize", UintegerValue(1024)); //1024 bytes

	ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
	clientApps.Start(Seconds(2.0));
	clientApps.Stop(Seconds (10.0));

	Simulator::Run();
	Simulator::Destroy();

	return 0;	

}
