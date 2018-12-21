#include "scratch/lppsim-headers.h"

class Location
{
public:
      Location()
	{
	}
         void	
	CertificateRequested(Ptr<Node> n)
	{
//  		uint32_t startTime = Simulator::Now().GetMilliSeconds(); 
  // 		std::cout << "Certificate Requested from node "  << n->GetId() << " at time " << startTime <<  std::endl;
	}
    	void	
	CertificateReceived(Ptr<Packet> p,uint32_t delay, Ptr<Node> node)
	{
                LppHeader header;
		p->PeekHeader(header);
		delays[node->GetId()] += delay;
		num_delays[node->GetId()]++;
   //		std::cout << "Certificate Received by node "  << node->GetId() << " at " << Simulator::Now().GetMilliSeconds() << " with delay " << delay  << " packet size " << p->GetSize() <<  std::endl;

	}
};
