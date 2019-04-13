#!/bin/bash
if [ $# != 1]
	echo "IP address of server is taken as local host."
	echo "If that is not correct."
	echo "Usage : ./scriptfilepeer1.sh IPaddrOfRS"
	cd ./SrcFiles/
	gcc -pthread -o peer PeerCode.c
	cd ..
	mkdir -p Peer1
	cp ./SrcFiles/peer ./Peer1/peer
	cd ../Peer1; gnome-terminal -e "./peer"
else
	cd ./SrcFiles/
	gcc -pthread -o peer PeerCode.c
	cd ..
	mkdir -p Peer1
	cp ./SrcFiles/peer ./Peer1/peer
	cd ../Peer1; gnome-terminal -e "./peer $1"
fi
