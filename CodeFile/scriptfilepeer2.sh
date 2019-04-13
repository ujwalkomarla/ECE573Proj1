#!/bin/bash
if [ $# != 1]
	echo "IP address of server is taken as local host."
	echo "If that is not correct."
	echo "Usage : ./scriptfilepeer2.sh IPaddrOfRS"
	cd ./SrcFiles/
	gcc -pthread -o peer PeerCode.c
	cd ..
	mkdir -p Peer2
	cp ./SrcFiles/peer ./Peer2/peer
	cd ../Peer2; gnome-terminal -e "./peer"
else
	cd ./SrcFiles/
	gcc -pthread -o peer PeerCode.c
	cd ..
	mkdir -p Peer2
	cp ./SrcFiles/peer ./Peer2/peer
	cd ../Peer2; gnome-terminal -e "./peer $1"
fi
