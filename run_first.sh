#!/bin/sh
for u in  10 20 30 40 50 60
do
for r in 1 2 3 4 5 6 7 8 9 10
do
 ./waf  --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=160 --time=7200 --output=first_lpp_udp_3gpp_{$u}_{$r} --lpp=true --udp=true --threeGpp=true --fading=false --RngRun=$r"
 ./waf --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=160 --time=7200 --output=first_lpp_3gpp_{$u}_{$r} --lpp=true --udp=false --threeGpp=true --fading=false --RngRun=$r" 
 ./waf --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=160 --time=7200 --output=first_lpp_udp_{$u}_{$r} --lpp=true --udp=true --threeGpp=false --fading=false --RngRun=$r" 
 ./waf --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=160 --time=7200 --output=first_lpp_{$u}_{$r} --lpp=true --udp=false --threeGpp=false --fading=false --RngRun=$r" 
 ./waf --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=160 --time=7200 --output=first_udp_3gpp_{$u}_{$r} --lpp=false --udp=true --threeGpp=true --fading=false --RngRun=$r" 
 ./waf --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=160 --time=7200 --output=first_udp_{$u}_{$r} --lpp=false --udp=true --threeGpp=false --fading=false --RngRun=$r" 
 ./waf --run scratch/lpp_first --command-template="%s --numUe=$u --numEnb=1 --pmodel=ns3::FriisPropagationLossModel --ns3::LteEnbRrc::SrsPeriodicity=160 --time=7200 --output=first_3gpp_{$u}_{$r} --lpp=false --udp=false --threeGpp=true --fading=false --RngRun=$r" 
done
