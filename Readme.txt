Scenario I : If server and peers are run on same system


1. In *nix shell command line, do a change directory(cd) to CodeFile folder. Type "cd CodeFile", without the quotes
2. Type "chmod 755 scriptfile.sh"
3. Type "./scriptfile.sh" 
4. In the command line with Peer1 : Type the RFC no. that you wish to get.(4202, 6666)
5. The peer sends a leave request when you exit the program by hitting ctrl+c(Do this in Peer1).
6. Now when you request for any other RFC(i.e 7301 in Peer2), you will get a message saying no other peer has that particular RFC.
(This indicates that though the Peer2 has the file, but has left the system. Hence, the response)













Scenario II : If server and peers are run on different systems.


1. In *nix shell command line, do a change directory(cd) to CodeFile folder. Type "cd CodeFile", without the quotes
2.a) Type "chmod 755 scriptfileserver.sh" on the system where you wish the server should run.
2.b) On same system type "./scriptfileserver.sh"
3.a) Type "chmod 755 scriptfilepeer1.sh" on the system where you wish the peer should run.
3.b) On same system type "./scriptfilepeer1.sh IP" (Where, IP address is that of the Registration server)
4.a) Type "chmod 755 scriptfilepeer2.sh" on the system where you wish the peer should run.
4.b) On same system type "./scriptfilepeer2.sh IP" (Where, IP address is that of the Registration server)
5. In the command line with Peer1 : Type the RFC no. that you wish to get.(4202, 6666)
6. The peer sends a leave request when you exit the program by hitting ctrl+c(Do this in Peer1).
7. Now when you request for any other RFC(i.e 7301 in Peer2), you will get a message saying no other peer has that particular RFC.





NOTE : For simplyfing the test scenario, I have copied 4202.txt, 6666.txt and 7301.txt RFC in Peer2 folder.
You can copy any other RFC with the format "NNNN.txt" and place it inside the Peer2 folder, before running the script file.
