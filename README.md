# 2016_ch_bonding_ns3
***
# How to use
> How to install
> 1. clone this project
> 2. Download NS3 version 3.26 [link](https://www.nsnam.org/releases/ns-allinone-3.26.tar.bz2) and unzip
> 3. override this project folder to unziped file
> ***
> How to comfile
> 1. go to ./ns-allinone-3.26/ns-3.26
> 2. command ./waf configure
> 3. command ./waf
> ***
> How to simulate
> 1. go to ./ns-allinone-3.26/ns-3.26
> 2. make input files to ./input
> 3. make director ./output
> 4. command ./waf --run scratch/my_test --command-template="%s --link_type=(link option) --test_number=(input file path)"
link option: True(downlink), False(uplink)
