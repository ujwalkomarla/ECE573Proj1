#!/bin/bash
cd ./SrcFiles/
gcc -pthread -o server ServerCode.c
gcc -pthread -o peer PeerCode.c
cd ..
mkdir -p Peer1
mkdir -p Peer2
mkdir -p Server
cp ./SrcFiles/peer ./Peer1/peer
cp ./SrcFiles/peer ./Peer2/peer
cp ./SrcFiles/server ./Server/server 
cp ./SrcFiles/*.txt ./Peer2/
#cp ./SrcFiles/6666.txt ./Peer2/6666.txt
cd ./Server; gnome-terminal -e "./server"
sleep 1
cd ../Peer1; gnome-terminal -e "./peer"
sleep 1
cd ../Peer2; gnome-terminal -e "./peer"
