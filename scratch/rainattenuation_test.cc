/* Explanation:


*/

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/netanim-module.h"

//Network Topology        
                          

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("WifiSpectrum");


int main (int argc,char *argv[]){

  bool verbose =true;
  uint32_t nWifi = 1;  //stations per access point
  uint32_t nAP = 1; 
  double stop_time = 5;

  CommandLine cmd;
  cmd.AddValue ("nAP", "Number of AP nodes/devices", nAP);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_ALL);  //Level of logging information for Clients
      LogComponentEnable ("UdpEchoServerApplication", LOG_ALL);  //Level of logging information for Servers
    }
  // create  Access Point
  NodeContainer APoints;
  APoints.Create(nAP); 

  //Wifi-Stations for AP
  NodeContainer wifiNodes;
  wifiNodes.Create (nWifi);


  // Config::SetDefault ("ns3::WifiPhy::CcaMode1Threshold", DoubleValue (-62.0));

  SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();

  Ptr<MultiModelSpectrumChannel> spectrumChannel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<FriisPropagationLossModel> lossModel = CreateObject<FriisPropagationLossModel> ();
  Ptr<RainAttenuationLossModel> rainLossModel = CreateObject<RainAttenuationLossModel> (52.52,13.4049,6,99.99);

  lossModel->SetFrequency (5.180e9);
  rainLossModel->SetFrequency (60e9);

  spectrumChannel->AddPropagationLossModel (lossModel);
  spectrumChannel->AddPropagationLossModel(rainLossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  spectrumChannel->SetPropagationDelayModel (delayModel);
  spectrumPhy.SetChannel (spectrumChannel);
  std::string errorModelType = "ns3::NistErrorRateModel";
  spectrumPhy.SetErrorRateModel (errorModelType);
  spectrumPhy.Set ("Frequency", UintegerValue (5180));

  //WifiHelper::EnableLogComponents();												//Helper object for Wifi_0
  WifiHelper wifi;	
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  std::string phyMode ("HtMcs0");
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  

  WifiMacHelper mac;												           		 //helper object for MAC in Wifi_0
                                            
  Ssid ssid = Ssid ("ns-3-ssid");	
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),   	
               "ActiveProbing", BooleanValue (false));
  NetDeviceContainer wifiDevices;
  wifiDevices = wifi.Install(spectrumPhy,mac,wifiNodes);

  mac.SetType ("ns3::ApWifiMac",	
                "Ssid", SsidValue (ssid));

  NetDeviceContainer APDevice;
  APDevice = wifi.Install(spectrumPhy,mac,APoints.Get(0));

  


  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (0.0, 100.0, 0.0));


  mobility.SetPositionAllocator (positionAlloc);

  mobility.Install (APoints.Get(0));
  mobility.Install (wifiNodes);

  //Protocol stacks installation
  InternetStackHelper stack;												

  stack.Install(APoints);												
  stack.Install(wifiNodes);
  							


  //IP Allocation
  Ipv4AddressHelper address; 												

  Ipv4InterfaceContainer BSS_AP;
  Ipv4InterfaceContainer BSS_C;


  address.SetBase("10.1.1.0","255.255.255.0");
  BSS_AP = address.Assign(APDevice);
  BSS_C = address.Assign(wifiDevices);



  //create Applications and Client/Servers
  //testing BSS_0
  OnOffHelper onoff(
  "ns3::UdpSocketFactory",
  InetSocketAddress("10.1.1.1", 9));
  onoff.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=10]"));
  onoff.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute("DataRate", StringValue("15024Kbps"));
  //onoff.SetAttribute("MaxBytes", StringValue("550"));
  onoff.SetAttribute("PacketSize", StringValue("512"));
  ApplicationContainer clientApps = onoff.Install(wifiNodes.Get(0));

  PacketSinkHelper sink(
                        "ns3::UdpSocketFactory",
                         InetSocketAddress("10.1.1.1", 9));
                         sink.Install(APoints.Get(0)
                        );


  spectrumPhy.EnablePcap ("scratch/WifiSpectrum0", APDevice);


  Simulator::Stop (Seconds (stop_time));
  // double time_left = stop_time*1000;  //time left in milliseconds

  // while(time_left>0){
  //   Simulator::Schedule(MilliSeconds(tdma_time),   &GiveSlot_0(wifiNodes_0.Get(0), wifiNodes_1.Get(0), wifiNodes_2.Get(0)));
  //   time_left-=tdma_time;
  // }


  //Config::ConnectWithoutContext("/NodeList/0/DeviceList/0/$ns3::WifiNetDevice/Phy/$ns3::SpectrumWifiPhy/SignalArrival", MakeCallback(&SignalArrivalCallback));
  //Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::SpectrumWifiPhy/PhyRxDrop", MakeCallback(&PacketDropCallback));

  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

  Simulator::Run ();
  Simulator::Destroy ();

  flowMonitor->SerializeToXmlFile("scratch/flowminitoring.xml", true, true);
  return 0;
}

