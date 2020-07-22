#!/bin/bash

echo -e "Compiling MUSSE"
scons

# if test -f "NFNU_single"; then
#     echo -e "\e[1;32mNFNU Compiled Successfully\e[0m"
#     echo -e "Testing Single Machine NFNU"
#     ./NFNU_single
# fi
if test -f "MUSSE_server" && test -f "NFNU_client"; then
   echo -e "Testing Client-Server NFNU"
   echo -e "\e[1;93mStarting Server\e[0m"
   ./MUSSE_server &
   echo -e "\e[1;93mStarting Client\e[0m"
    ./NFNU_client
    echo -e "\e[1;93mShutting Down Server\e[0m"
    pkill MUSSE_server
fi

