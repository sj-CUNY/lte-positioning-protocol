
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/gnuplot.h"
#include "ns3/lpp-helper.h"
#include <ns3/mobility-building-info.h>
#include <ns3/buildings-propagation-loss-model.h>
#include <ns3/building.h>
#include <ns3/buildings-helper.h>
#include <ns3/buildings-module.h>
#include <fstream>
#include <iostream>
#include <cstring>
//#include "ns3/gtk-config-store.h"

using namespace ns3;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */
bool stopSim = false;

int numberOfNodes = 20;
int numEnb = 1;
std::string p_model = "ns3::FriisPropagationLossModel";
double simTime = 10.0;
std::string fname = "first";
bool lpp = false;
bool threeGpp = false;
bool fading = false;
bool udp = false;
uint32_t udp_sent = 0;
uint32_t udp_recvd = 0;
uint32_t udp_txsize = 0;
uint32_t udp_rxsize = 0;


uint32_t *delays;
uint32_t *responseTime;
uint32_t *web_size;
uint32_t *httpDelay;
uint32_t *num_httpDelay;
double *throughput;
uint32_t *num_pages;
uint32_t *num_delays;
std::ofstream lpp_out;
std::ofstream http_out;

double distance = 60.0;
NS_LOG_COMPONENT_DEFINE ("LppSim");

uint32_t rx_3gpp = 0, tx_3gpp = 0;

/*void
plot(std::string fname, std::string configfname, NodeContainer ueNodes, int num);
*/
void
setMobility1(NodeContainer ueNodes, NodeContainer enbNodes)
{

  MobilityHelper mobility;
  // Install Mobility Model
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  double  x = 60, y = 15, z = 2; 
  mobility.Install(ueNodes);
  mobility.Install(enbNodes);

  Ptr<MobilityModel> mm0;
 
 Ptr<UniformRandomVariable> pos = CreateObject<UniformRandomVariable> ();
  for (uint32_t i = 0; i < ueNodes.GetN(); ++i)
    {
       mm0 = ueNodes.Get (i)->GetObject<ConstantPositionMobilityModel> ();
       mm0->SetPosition (Vector(pos->GetValue(0,x) , pos->GetValue(0,y), pos->GetValue(1,z)));
    }
  
   mm0 = enbNodes.Get (0)->GetObject<ConstantPositionMobilityModel> ();
   mm0->SetPosition (Vector(x/2 , y/2, z));
 

}
static void
PlotHttpParams(std::string fname, NodeContainer ueNodes)
{
  GnuplotCollection gnuplots (fname + "http.plt");
  Gnuplot plot1, plot2,plot3, plot4;
  Gnuplot2dDataset dataset1, dataset2, dataset3, dataset4;
  plot1.SetTitle ("Http throughput per node pages_size/responseTime");
  plot1.AppendExtra ("set xlabel 'Node Id'");
  plot1.AppendExtra ("set ylabel 'Throughput'");
  plot1.AppendExtra ("set key top right");


  plot2.SetTitle ("Http response time per node responseTime/number of pages");
  plot2.AppendExtra ("set xlabel 'Node Id'");
  plot2.AppendExtra ("set ylabel 'Response time (ms)'");
  plot2.AppendExtra ("set key top right");

  plot3.SetTitle ("Udp number of packets sent and received");
  plot3.AppendExtra ("set xlabel 'packets sent'");
  plot3.AppendExtra ("set ylabel  'packets received'");
  plot3.AppendExtra ("set key top right");

  plot4.SetTitle ("Http total throughput total data/response time and overall responseTime (response time acros all us/number of pages across all");
  plot4.AppendExtra ("set xlabel 'throughput'");
  plot4.AppendExtra ("set ylabel  'response time'");
  plot4.AppendExtra ("set key top right");



  dataset1.SetStyle (Gnuplot2dDataset::LINES_POINTS);
  dataset2.SetStyle (Gnuplot2dDataset::DOTS);
  dataset3.SetStyle (Gnuplot2dDataset::POINTS);
  dataset4.SetStyle (Gnuplot2dDataset::POINTS);
  stopSim = true;
  dataset3.Add(udp_sent,udp_recvd);
  uint32_t total_size = 0;
  uint32_t total_time = 0;
  uint32_t pages = 0;
  for(uint32_t i = 0; i <ueNodes.GetN(); ++i)
  {
        int index = ueNodes.Get(i)->GetId();

	if (num_pages[index] == 0)
       {
	dataset1.Add (index, 0);
        dataset2.Add(index, 0);
	   continue;
	}
	double throughput = web_size[index]/double(responseTime[index]);
        total_size += web_size[index];
        total_time += responseTime[index];
        pages += num_pages[index];
	dataset1.Add (index, throughput);
        dataset2.Add(index, responseTime[index]/num_pages[index]);

  }
  dataset4.Add(total_size/double(total_time), total_time/double(pages));
  plot1.AddDataset (dataset1);
  plot2.AddDataset (dataset2);
  plot3.AddDataset (dataset3);
  plot4.AddDataset (dataset4);
  gnuplots.AddPlot (plot1);
  gnuplots.AddPlot (plot2);
  gnuplots.AddPlot (plot3);
  gnuplots.AddPlot (plot4);
  gnuplots.GenerateOutput (http_out);
}
static void
PlotLppDelay(std::string fname, NodeContainer ueNodes)
{
  GnuplotCollection gnuplots (fname + "lpp.plt");
  Gnuplot plot1, plot2;
  Gnuplot2dDataset dataset, dataset2;
  plot1.SetTitle ("Average Delay per node to obtain LPP Certificate");
  plot1.AppendExtra ("set xlabel 'Node Id'");
  plot1.AppendExtra ("set ylabel 'Delay (ms)'");
  plot1.AppendExtra ("set key top right");
  plot2.SetTitle ("Average Delay over all UEs to obtain LPP Certificate");
  plot2.AppendExtra ("set xlabel 'number of measurements'");
  plot2.AppendExtra ("set ylabel 'Delay (ms)'");
  plot2.AppendExtra ("set key top right");
 
  dataset.SetStyle (Gnuplot2dDataset::LINES);
  dataset2.SetStyle (Gnuplot2dDataset::DOTS);
  uint32_t total_delay=0;
  uint32_t count = 0;
  stopSim = true;
  for(uint32_t i = 0; i < ueNodes.GetN(); ++i)
  {
        int index = ueNodes.Get(i)->GetId();

	if (num_delays[index] == 0)
	   continue;
	double Delay = delays[index]/num_delays[index];
        total_delay += delays[index];
        count += num_delays[index]; 
	dataset.Add (index, Delay);
  }
  dataset2.Add(count, total_delay/(double)count);
  plot1.AddDataset (dataset);
  plot2.AddDataset (dataset2);
  gnuplots.AddPlot (plot1);
  gnuplots.AddPlot (plot2);
  gnuplots.GenerateOutput (lpp_out);
}

void 
ConfigParams(int argc, char *argv[])
{
	CommandLine cmd;
	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults ();
	// parse again so you can override default values from the command line
	cmd.Parse (argc, argv);
}
void
setScheduler(Ptr<LteHelper> lteHelper)
{
    lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");  // Proportionately fair scheduler
/*
    lteHelper->SetSchedulerType ("ns3::TdTbfqFfMacScheduler");  // TD-TBFQ scheduler
    lteHelper->SetSchedulerAttribute("DebtLimit", IntegerValue(-625000)); // default value -625000 bytes (-5Mb)
    lteHelper->SetSchedulerAttribute("CreditLimit", UintegerValue(625000)); // default value 625000 bytes (5Mb)
    lteHelper->SetSchedulerAttribute("TokenPoolSize", UintegerValue(1)); // default value 1 byte
    lteHelper->SetSchedulerAttribute("CreditableThreshold", UintegerValue(0)); // default value 0
 */
} 

void
setFadingModel(Ptr<LteHelper> lteHelper)
{
   lteHelper->SetFadingModel("ns3::TraceFadingLossModel");
   lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
   lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
   lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
   lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
   lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));
}

void
setPathLossModel(Ptr<LteHelper> lteHelper, std::string pathloss)
{
   //lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::OhBuildingsPropagationLossModel")); 
   lteHelper->SetAttribute ("PathlossModel", StringValue (pathloss)); 
//   lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (100));
  // lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (18100));
}

void
setGridBuildings(Ptr<GridBuildingAllocator> gridBuildingAllocator ,double height, int n)
{
	gridBuildingAllocator->SetAttribute ("GridWidth", UintegerValue (3));
	gridBuildingAllocator->SetAttribute ("LengthX", DoubleValue (7));
	gridBuildingAllocator->SetAttribute ("LengthY", DoubleValue (13));
	gridBuildingAllocator->SetAttribute ("DeltaX", DoubleValue (3));
	gridBuildingAllocator->SetAttribute ("DeltaY", DoubleValue (3));
	gridBuildingAllocator->SetAttribute ("Height", DoubleValue (height));
	gridBuildingAllocator->SetBuildingAttribute ("NRoomsX", UintegerValue (2));
	gridBuildingAllocator->SetBuildingAttribute ("NRoomsY", UintegerValue (4));
	gridBuildingAllocator->SetBuildingAttribute ("NFloors", UintegerValue (height/3));
	gridBuildingAllocator->SetAttribute ("MinX", DoubleValue (0));
	gridBuildingAllocator->SetAttribute ("MinY", DoubleValue (0));
	gridBuildingAllocator->Create (n);
}
void
setBuildings(Ptr<Building> b)
{
   double x_min = 0.0;
   double x_max = 10.0;
   double y_min = 0.0;
   double y_max = 20.0;
   double z_min = 1.5;
   double z_max = 10.0;
   b->SetBoundaries (Box (x_min, x_max, y_min, y_max, z_min, z_max));
   b->SetBuildingType (Building::Residential);
   b->SetExtWallsType (Building::ConcreteWithWindows);
   b->SetNFloors (3);
   b->SetNRoomsX (3);
   b->SetNRoomsY (2);
}
void
listPosition(Ptr<ListPositionAllocator> positionAlloc, Ptr<UniformRandomVariable> random, int n, double maxX, double maxY, double maxZ)
{

  // Install Mobility Model
  for (int i = 0; i < n ; i++)
    {
      positionAlloc->Add (Vector(random->GetValue()*maxX, random->GetValue()*maxY, random->GetValue()*maxZ));
    }

}
void
gridPosition(MobilityHelper mobility)
{
    mobility.SetPositionAllocator("ns3::GridPositionAllocator", 
	"MinX", DoubleValue(0.0), 
	"MinY", DoubleValue(0.0), 
	"Z", DoubleValue(1.5), 
	"DeltaX", DoubleValue(10.0), 
	"DeltaY", DoubleValue(10.0), 
	"GridWidth", UintegerValue(1), 
         "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
 
}

void
generateREM()
{
Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();
remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
remHelper->SetAttribute ("OutputFile", StringValue ("rem.out"));
remHelper->SetAttribute ("XMin", DoubleValue (-400.0));
remHelper->SetAttribute ("XMax", DoubleValue (400.0));
remHelper->SetAttribute ("XRes", UintegerValue (100));
remHelper->SetAttribute ("YMin", DoubleValue (-300.0));
remHelper->SetAttribute ("YMax", DoubleValue (300.0));
remHelper->SetAttribute ("YRes", UintegerValue (75));
remHelper->SetAttribute ("Z", DoubleValue (0.0));
remHelper->SetAttribute ("UseDataChannel", BooleanValue (true));
remHelper->SetAttribute ("RbId", IntegerValue (10));
remHelper->Install ();


}
