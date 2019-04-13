#!/bin/bash
cd ./SrcFiles/
gcc -pthread -o server ServerCode.c
cd ..
mkdir -p Server
cp ./SrcFiles/server ./Server/server 
cd ./Server; gnome-terminal -e "./server"
