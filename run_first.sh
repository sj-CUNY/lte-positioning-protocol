#!/bin/sh
for r in 2 3 4 5 6 7 8 9 10
do
for u in  10 20 30 40 50 60
do
 ./waf  --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=80 --time=200 --output=first_lpp_3gpp_{$u}_{$r} --lpp=true --udp=false --threeGpp=true --fading=false --RngRun=$r --interval=60"
 ./waf --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=160 --time=200 --output=first_lpp_udp_{$u}_{$r} --lpp=true --udp=true --threeGpp=false --fading=false --RngRun=$r --interval=60" 
 ./waf --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=160 --time=200 --output=first_lpp_{$u}_{$r} --lpp=true --udp=false --threeGpp=false --fading=false --RngRun=$r --interval=60" 
 ./waf --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=160 --time=200 --output=first_3gpp_{$u}_{$r} --lpp=false --udp=false --threeGpp=true --fading=false --RngRun=$r --interval=60" 
 ./waf --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=160 --time=200 --output=first_udp_{$u}_{$r} --lpp=false --udp=true --threeGpp=false --fading=false --RngRun=$r --interval=60" 
done
done
