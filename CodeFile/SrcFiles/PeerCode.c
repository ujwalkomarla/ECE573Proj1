//#define DEBUG
#define HEADER
#define LOCALNETWORK
//#includes
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<dirent.h>
#include<pthread.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<unistd.h>

#ifdef LOCALNETWORK
	#define SERVERLISTENPORT 65423
#else
	#define SERVERLISTENPORT 65443
#endif

#define TRUE 1
#define FALSE 0
#define CLIENTQUEUELIMIT 5
#define MAXRECVCHAR 500
#define MAXSENDCHAR 500
#define HOSTNAMEMAXSIZE 256 //255+1

struct peerRfcDatabase{
	char fileName[100];
	struct peerRfcDatabase *nextNode;
};

struct infoPeerListDB{
struct peerListDatabase *firstNode;
struct peerListDatabase *lastNode;
int count;
};

struct peerListDatabase{
unsigned long int hostname;
unsigned int peerRfcServerPort;
struct peerRfcDatabase *myRfcListFirstNode;
struct peerRfcDatabase *myRfcListLastNode;
struct peerListDatabase *nextNode;
};
void DieWithError(char *msg){
perror(msg);
exit -1;
}

//Function declarations
void HandleCtrlC(int);

int CreatePeerServerListenSock(unsigned int);
int ScanLocalDirectoryForCookieRFC();
struct peerListDatabase* SearchPeerListDatabaseForRfc(struct peerListDatabase *, int);
int SearchParticularPeerForRfc(struct peerRfcDatabase *,int );
long unsigned int RequestToRegister(long unsigned int);
int GetFromPeer(long unsigned int ,unsigned int , unsigned int);

void gen_random(char *, const int );
void* HandleUiFunction(void *);
void* ServerFunction(void *);
void* StayAliveFunction(void *);
void *PeerPeerHandleFunction(void *);
void parseRequestFile(FILE *);
void rfcquery_parse(FILE *);
void getrfc_parse(FILE *);
void RequestToPQuery(void);
void RequestToKeepAlive(long unsigned int);
void RequestToLeave(long unsigned int);
//Global Variables
int peerServerListenSockId;
long unsigned int cookie; 
struct peerListDatabase LocalPeerListInfo;
struct sockaddr_in PeerRSconn;

struct infoPeerListDB infoPeerListDatabase;
	
int main(int argc, char *argv[]){
//Local variables
	
	cookie = 0;
	peerServerListenSockId = -1;
	
	pthread_t stayAliveThread, serverThread, handleUiThread;
	int iReturnStayAlive, iServer, iHandleUi;
	
	//Declare Peer RS Connection settings
#ifdef LOCALNETWORK	
	//PeerRSconn = {AF_INET, htons(SERVERLISTENPORT),  inet_addr("127.0.0.1")}; 
	PeerRSconn.sin_family = AF_INET;
	PeerRSconn.sin_port = htons(SERVERLISTENPORT);
	if(argc !=2)
		PeerRSconn.sin_addr.s_addr = inet_addr("127.0.0.1");
	else
		PeerRSconn.sin_addr.s_addr = inet_addr(argv[1]);
#else
	
	struct hostent *hptr = gethostbyname2("registrationserver.thetwistedtransistors.com",AF_INET);
	PeerRSconn.sin_family = AF_INET;
	PeerRSconn.sin_port = htons(SERVERLISTENPORT);
	
	if(hptr == NULL){
	PeerRSconn.sin_addr.s_addr	= inet_addr("174.97.168.75");
	}else{
	PeerRSconn.sin_addr.s_addr	=  (hptr->h_addr);
	}
#endif
	//struct peerListDatabase LocalPeerListInfo;
	//strcpy(LocalPeerListInfo.hostname, "\0"); //CORRECT WAY TO DECLARE AN ARRAY?
	LocalPeerListInfo.hostname = 0;
	LocalPeerListInfo.myRfcListFirstNode = NULL;
	LocalPeerListInfo.myRfcListLastNode = NULL;
	LocalPeerListInfo.nextNode = NULL;
	LocalPeerListInfo.peerRfcServerPort = ((rand()%100) + 65400);
	
	infoPeerListDatabase.firstNode = &LocalPeerListInfo;
	infoPeerListDatabase.lastNode = &LocalPeerListInfo;
	infoPeerListDatabase.count = 1;
#ifdef DEBUG
	//PrintPeerListDatabase();
	
#endif	
	struct infoPeerListDB infoPeerListDatabase;
	infoPeerListDatabase.firstNode = &LocalPeerListInfo;
	infoPeerListDatabase.lastNode = &LocalPeerListInfo;
	infoPeerListDatabase.count = 1;
#ifdef DEBUG
	//PrintInfoPeerListDatabase();
	
#endif	
	signal(SIGINT, HandleCtrlC);
	
	//Create Peer Server Socket
	//Parameter = random socket number between 65400 - 65500
	#ifdef DEBUG
	printf("%ul",LocalPeerListInfo.peerRfcServerPort);
	#endif
	while((peerServerListenSockId = CreatePeerServerListenSock(LocalPeerListInfo.peerRfcServerPort = ((rand()%100) + 65400))) < 0);
	/* if((peerServerListenSockId = CreatePeerServerListenSock(LocalPeerListInfo.peerRfcServerPort)) < 0)
		if((peerServerListenSockId = CreatePeerServerListenSock((LocalPeerListInfo.peerRfcServerPort = ((rand()%103) + 65401)))) < 0){
		#ifdef DEBUG
	printf("%ul",LocalPeerListInfo.peerRfcServerPort);
	#endif
			perror("CreatePeerServerListenSock Error : Use $\? to get error no. ");
			exit(peerServerListenSockId);
			} */

	


#ifdef DEBUG
	printf("peerServerListenSockId : %d (in main)\r\n ",peerServerListenSockId);
#endif		


	
	//Get info about peer's cookie and RFCs it holds
	
	if((cookie = ScanLocalDirectoryForCookieRFC(LocalPeerListInfo)) < 0 ){
		perror("ScanLocalDirectoryForCookieRFC Error : Use $\? to get error no. ");
		exit(cookie);
		}
		
	
		
		
		
#ifdef DEBUG
	printf("Cookie : %lu (in main)\r\n ",cookie);
	printf("%p %p %s %s %p\r\n",&LocalPeerListInfo, LocalPeerListInfo.myRfcListFirstNode, LocalPeerListInfo.myRfcListFirstNode ->fileName ,LocalPeerListInfo.myRfcListLastNode ->fileName, LocalPeerListInfo.myRfcListLastNode );
#endif			
	/*	
-> Send register w or w/o Cookie		
	*/	
	while((cookie=RequestToRegister(cookie) )< 0);
		
	//Create Threads
	iReturnStayAlive = pthread_create( &stayAliveThread, NULL, StayAliveFunction, NULL);
    if(iReturnStayAlive)
    {
		#ifdef DEBUG
		fprintf(stderr,"Error - pthread_create() return code: %d\n",iReturnStayAlive);
		#endif
		exit(EXIT_FAILURE);
	}

	iServer = pthread_create( &serverThread, NULL, ServerFunction, NULL);
    if(iServer)
    {
		#ifdef DEBUG
		 fprintf(stderr,"Error - pthread_create() return code: %d\n",iServer);
		#endif
		 exit(EXIT_FAILURE);
	}

	
	
	iHandleUi = pthread_create( &handleUiThread, NULL, HandleUiFunction, NULL);
    if(iHandleUi)
    {
		#ifdef DEBUG
		 fprintf(stderr,"Error - pthread_create() return code: %d\n",iHandleUi);
		 #endif
		 exit(EXIT_FAILURE);
	}
	

	
	//Wait for join
	pthread_join( stayAliveThread, NULL);
	pthread_join( serverThread, NULL);
	pthread_join( handleUiThread, NULL);
	
	return 0;
}


void HandleCtrlC(int signum){
	signal(SIGINT, HandleCtrlC);
	RequestToLeave(cookie);
#ifdef DEBUG
	printf("\r\nYou can't cut me off, so easily\r\n");
#endif	
	//Any Processing?
	
	
	exit(1);
}


int CreatePeerServerListenSock(unsigned int listenPort){
	struct sockaddr_in serverListenSocket; 
	int serverListenSockId;
	if((serverListenSockId = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;
	/*
->  //SO_REUSEADDR -> So that we can use a socket that is in a wait state
	*/
	//Settings for server listen socket
	serverListenSocket.sin_family = AF_INET;
	serverListenSocket.sin_port = htons(listenPort);
	serverListenSocket.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//Bind
	if (bind(serverListenSockId, (struct sockaddr *) &serverListenSocket, sizeof(serverListenSocket)) == -1) return -2;
	
	//Listen
	if(listen(serverListenSockId, CLIENTQUEUELIMIT) == -1) return -3;
#ifdef DEBUG
	printf("serverListenPort : %d \r\n", listenPort);
	printf("serverListenSockId : %d \r\n", serverListenSockId);
#endif
	return serverListenSockId;
}


int ScanLocalDirectoryForCookieRFC(/*struct peerListDatabase LocalPeerListInfo*/){
	//extern long unsigned int cookie; //in main(), return value from this function.
	int tcookie = 0;
	DIR *localDirectoryPointer;
	struct dirent *nextFileInPresentDirectory;
	localDirectoryPointer = opendir ("./");
	if(localDirectoryPointer != NULL){
		char *cookieString = "~Cookie";
		char *fStr, *fStr1, *fStr2;
		int ptrDiff;
		while (nextFileInPresentDirectory = readdir (localDirectoryPointer)){
		/*
		Search for a setting file with a cookie available, load it into cookie variable
		search for any RFC files and load on to the linked list database
		*/
		
		fStr =strstr(nextFileInPresentDirectory->d_name,cookieString);
		if( fStr != NULL){
			tcookie = strtol( (fStr + strlen(cookieString)), NULL, 10);
			
			}
		else{
		/*
->		//"." ".." Temporary Files and Hidden files handling
		//Executables, Directories, and more!!
		//Just accept files terminating with .'something'
		*/
				fStr1 = strstr(nextFileInPresentDirectory->d_name,".");
				//printf ("\n%s found at %d\n",nextFileInPresentDirectory->d_name,fStr1-(nextFileInPresentDirectory->d_name) + 1);
				ptrDiff = fStr1-(nextFileInPresentDirectory->d_name) + 1;
				/*if(fStr1 !=NULL)
					ptrDiff = fStr1-(nextFileInPresentDirectory->d_name) + 1;
				//else ptrDiff = 2; //TO INCLUDE FILES WITHOUT EXTENSION i.e./ Executables, directories are all included. ?DUH!
				*/
				fStr2 = strstr(nextFileInPresentDirectory->d_name,"~");
				if((ptrDiff > 1 /*&& ptrDiff <255 */&& fStr2 == NULL )){
				struct peerRfcDatabase *newEntry = (struct peerRfcDatabase*)malloc(sizeof(struct peerRfcDatabase));
	#ifdef DEBUG
				printf("First Node: %p Last Node: %p File Name: %s Next Node: %p\r\n",LocalPeerListInfo.myRfcListFirstNode,LocalPeerListInfo.myRfcListLastNode,newEntry->fileName,newEntry->nextNode);
				if(LocalPeerListInfo.myRfcListLastNode != NULL) printf("Prev RFC node's last node : %p\r\n",(LocalPeerListInfo.myRfcListLastNode)->nextNode);
	#endif			
				
				if(LocalPeerListInfo.myRfcListFirstNode == NULL) LocalPeerListInfo.myRfcListFirstNode = newEntry;
				if(LocalPeerListInfo.myRfcListLastNode != NULL) (LocalPeerListInfo.myRfcListLastNode)->nextNode = newEntry;
	#ifdef DEBUG			
				if(LocalPeerListInfo.myRfcListLastNode != NULL) printf("Prev RFC node's last node : %p\r\n",(LocalPeerListInfo.myRfcListLastNode)->nextNode);
	#endif
				LocalPeerListInfo.myRfcListLastNode = newEntry;
				
				strcpy(newEntry->fileName, nextFileInPresentDirectory->d_name);
				newEntry->nextNode = NULL;
	#ifdef DEBUG
				printf("First Node: %p Last Node: %p File Name: %s Next Node: %p\r\n\r\n",LocalPeerListInfo.myRfcListFirstNode,LocalPeerListInfo.myRfcListLastNode,newEntry->fileName,newEntry->nextNode);
	#endif
				}
			}
		}
		(void) closedir (localDirectoryPointer);
	}else{
	return -1;
	}
#ifdef DEBUG
	printf("Cookie : %d \r\n", tcookie);
	printf("Peer self address : %p\r\n", &LocalPeerListInfo);
	/*
->	//PrintPeerListDatabase();
	*/
#endif	
	return tcookie;
}


int SearchParticularPeerForRfc( struct peerRfcDatabase *startWithNode,int iSearchForRfc){
	char sSearchForRfc[10];
	struct peerRfcDatabase *peerRfcDatabasePresentNode = startWithNode;
	sprintf(sSearchForRfc, "%d.txt", iSearchForRfc);
	#ifdef DEBUG
	printf("%s\r\n",sSearchForRfc);
	#endif

	//itoa(iSearchForRfc,sSearchForRfc,10);
	
	fflush(stdout);
	#ifdef DEBUG
	printf("%p\r\n ", peerRfcDatabasePresentNode);
	//printf("%p",peerRfcDatabasePresentNode->nextNode);
	#endif
	while(peerRfcDatabasePresentNode != NULL){
		//printf("abc");
		#ifdef DEBUG
		printf("File Name : %s\r\n",peerRfcDatabasePresentNode->fileName);
			#endif
		if(strcmp(peerRfcDatabasePresentNode->fileName,sSearchForRfc) == 0) return TRUE;
		else {
		peerRfcDatabasePresentNode = peerRfcDatabasePresentNode->nextNode;
		}
		#ifdef DEBUG
		printf("%p \r\n",peerRfcDatabasePresentNode);
		#endif
	}
	
	return FALSE;
}


struct peerListDatabase* SearchPeerListDatabaseForRfc( struct peerListDatabase *startWithNode, int searchForRfc){ 
	struct peerListDatabase *databasePresentNode = startWithNode;
while(databasePresentNode != NULL ){
#ifdef DEBUG
	printf("Searching Peer Database present node : %p \r\n",databasePresentNode);
#endif
	if(SearchParticularPeerForRfc(databasePresentNode->myRfcListFirstNode, searchForRfc ) == TRUE)
	{
#ifdef DEBUG
		printf("Peer which has the file : %p \r\n",databasePresentNode);
#endif
	return databasePresentNode;
	}
	else{
	databasePresentNode = databasePresentNode->nextNode;
	}
}
return NULL;
}


void* StayAliveFunction(void *msg){
	while(TRUE){

	sleep(7200);
	RequestToKeepAlive(cookie);
#ifdef DEBUG
	printf("Sent stay alive\r\n");
#endif
	}
}


void* ServerFunction(void *msg){
	int connectedPeerCliServSockId;
	unsigned int clntLen = sizeof(struct sockaddr_in);
	struct sockaddr_in serverClientConn;
	while(connectedPeerCliServSockId = accept(peerServerListenSockId, (struct sockaddr *) &serverClientConn, &clntLen)){
	if(connectedPeerCliServSockId == -1) DieWithError("Connection error Peer(Client) Server ");
	else{
		int *arg = malloc(sizeof(*arg));
		*arg = connectedPeerCliServSockId;
		pthread_t *peerPeerHandleThreads = malloc(sizeof(pthread_t));
			//int *arg = malloc(sizeof(int *));
			if(peerPeerHandleThreads == NULL || arg == NULL){
				DieWithError("Out of memory");
			}else{
				//*arg = connectedCliServSockId;
				if(pthread_create(peerPeerHandleThreads, NULL, PeerPeerHandleFunction,arg)!=0){
					DieWithError("Couldn't create thread");
					//Send an error msg to peer
					close(connectedPeerCliServSockId);
					continue;
				}
			}
		
	    }
	}
}


void* HandleUiFunction(void *msg){
	//WTS
	//printf("Query RFCQuery P2P-DI/1.0\r\n\r\n");
	printf("Type in the RFC number to fetch\n");
	unsigned int fetchRfcNo;
	while(TRUE){
	printf(">");
	scanf("%d", &fetchRfcNo);
	//WTS
	//printf("OK RFCQuery P2P-DI/1.0\r\n\r\n");
#ifdef DEBUG

	//printf("Cookie : %lu (in main)\r\n ",cookie);
	printf("%p %p %s %s %p\r\n",&LocalPeerListInfo, LocalPeerListInfo.myRfcListFirstNode, LocalPeerListInfo.myRfcListFirstNode ->fileName ,LocalPeerListInfo.myRfcListLastNode ->fileName, LocalPeerListInfo.myRfcListLastNode );
#endif
	
	struct peerListDatabase *nodeWhichHas ;
		//printf("VALUE : %p", SearchPeerListDatabaseForRfc(&LocalPeerListInfo, fetchRfcNo));
	if((nodeWhichHas = SearchPeerListDatabaseForRfc(&LocalPeerListInfo, fetchRfcNo)) !=NULL ){
	//Get RFC from 'nodeWhichHas'
#ifdef DEBUG	
		printf("NodeWhichHas:%p = \n",nodeWhichHas);
		printf("LocalPeerListInfo:%p\r\n",&LocalPeerListInfo);
		printf("fetchRFCNo:%d\n",fetchRfcNo);
#endif
		fflush(stdout);
			if(nodeWhichHas == &LocalPeerListInfo) {
				
//OPEN IN A NEW PROCESS	
/*			
				char ptr[30];
				sprintf(ptr, "gedit %d.txt",fetchRfcNo);
				pid_t pid=fork();
				if(pid>=0){
					if(pid==0)
					system(ptr);
					//exit(1);
*/					
				printf("File Present Locally\r\n");
				
#ifdef DEBUG
				//printf("BYE");
#endif
			}else{
				//Contact the peer which has, request, download
				//if(GetFromPeer(nodeWhichHas->hostname,nodeWhichHas->peerRfcServerPort,fetchRfcNo)==-1)
			}
			
		}
	else{
		
		//GetListOfRFC
	//query RS for new database
	//update database
	//Contact the peer which has, request, download
		//unsigned long int tHostName = (LocalPeerListInfo.nextNode) ->hostname;
		#ifdef DEBUG
		printf("Before sending requests to peer\r\n");
		#endif
		struct peerListDatabase *tVariable = LocalPeerListInfo.nextNode;
		//printf("%p",LocalPeerListInfo.nextNode);
		
		while(tVariable != NULL){
				#ifdef DEBUG
				printf("%p",LocalPeerListInfo.nextNode);
				#endif
				fflush(stdout);
				if(GetFromPeer(tVariable -> hostname,tVariable->peerRfcServerPort,fetchRfcNo)==-1){
				tVariable = tVariable -> nextNode;
				}
				else {
				#ifdef DEBUG
				printf("break");
				#endif
				struct peerRfcDatabase *newEntry = (struct peerRfcDatabase*)malloc(sizeof(struct peerRfcDatabase));
				if(LocalPeerListInfo.myRfcListFirstNode == NULL) LocalPeerListInfo.myRfcListFirstNode = newEntry;
				if(LocalPeerListInfo.myRfcListLastNode != NULL) (LocalPeerListInfo.myRfcListLastNode)->nextNode = newEntry;
				LocalPeerListInfo.myRfcListLastNode = newEntry;
				char rfcNoDotTxt[10]; 
				sprintf(rfcNoDotTxt, "%d.txt", fetchRfcNo);
				strcpy(newEntry->fileName, rfcNoDotTxt);
				newEntry->nextNode = NULL;
				
				printf("The RFC No. %d was successful and placed in the present directory\r\n",fetchRfcNo);
				break;
				
				}
			}
			if(tVariable == NULL) {
				RequestToPQuery();
				#ifdef DEBUG
		printf("Before sending requests to peer : SECOND Time\r\n");
		#endif
		struct peerListDatabase *tVariable = LocalPeerListInfo.nextNode;
		//printf("%p",LocalPeerListInfo.nextNode);
		
		while(tVariable != NULL){
				#ifdef DEBUG
				printf("%p",LocalPeerListInfo.nextNode);
				#endif
				fflush(stdout);
				if(GetFromPeer(tVariable -> hostname,tVariable->peerRfcServerPort,fetchRfcNo)==-1){
				tVariable = tVariable -> nextNode;
				}
				else {
				#ifdef DEBUG
				printf("break");
				#endif
				struct peerRfcDatabase *newEntry = (struct peerRfcDatabase*)malloc(sizeof(struct peerRfcDatabase));
				if(LocalPeerListInfo.myRfcListFirstNode == NULL) LocalPeerListInfo.myRfcListFirstNode = newEntry;
				if(LocalPeerListInfo.myRfcListLastNode != NULL) (LocalPeerListInfo.myRfcListLastNode)->nextNode = newEntry;
				LocalPeerListInfo.myRfcListLastNode = newEntry;
				char rfcNoDotTxt[10]; 
				sprintf(rfcNoDotTxt, "%d.txt", fetchRfcNo);
				strcpy(newEntry->fileName, rfcNoDotTxt);
				newEntry->nextNode = NULL;
				
				printf("The RFC No. %d was successful and placed in the present directory\r\n",fetchRfcNo);
				break;
				
				}
			}
			if(tVariable == NULL) printf("No Peer has the RFC No. %d\r\n", fetchRfcNo);
			}
		}
	} 
}


long unsigned int RequestToRegister(long unsigned int tcookie){
	
	int fileReadSize=0;
	int connectedPeerRSSockId;
	char line[MAXSENDCHAR];
	char recvMsg[MAXRECVCHAR];
	int recvdMsgSize;
	//create a socket
	connectedPeerRSSockId = socket(AF_INET, SOCK_STREAM, 0);
	if(connectedPeerRSSockId == -1) DieWithError("Socket() Error");
	if(connect(connectedPeerRSSockId, (struct sockaddr *) &PeerRSconn, sizeof(PeerRSconn))<0) {DieWithError("Connect() Error");return -1;}
	//FILE *fd = fdopen()
	FILE *FileOpen = fopen("RScommFile","w+");
	if (FileOpen == NULL){
	DieWithError( "Can't open file in directory, RScommFile!\n");
	return -1;
	}
	char str1[80], str2[80], str3[80],str4[80];
	char method[] = "Registration";
	char message[] = "Register";
	char version[] = "P2P-DI/1.0";
	time_t rawtime;
	struct tm *info;
	char buffer[80];

  	time( &rawtime );

   	info = localtime( &rawtime );
	strftime(buffer,80,"%x - %I:%M%p", info);
	sprintf(str1, "%s %s %s \r\n",method, message, version);
	sprintf(str2, "PeerRFCServerListenPort:\t%d\r\n", LocalPeerListInfo.peerRfcServerPort);
	sprintf(str3, "Cookie:\t%lu\r\n", tcookie);	
	sprintf(str4, "Date&Time:\t%s\r\n\r\n", buffer);

//WTS
	//printf(str1, "%s %s %s \r\n",method, message, version);
	//printf(str2, "PeerRFCServerListenPort:\t%d\r\n", LocalPeerListInfo.peerRfcServerPort);
	//printf(str3, "Cookie:\t%lu\r\n", tcookie);	
	//printf(str4, "Date&Time:\t%s\r\n\r\n", buffer);

	fprintf(FileOpen, "%s%s%s%s", str1, str2, str3,str4);
		fprintf(stdout, "%s%s%s%s", str1, str2, str3,str4);
	fflush(FileOpen);
	
	//fclose(FileOpen);
	rewind(FileOpen);
	//FileOpen = fopen("RScommFile","r");
	while((fileReadSize = fread (line,sizeof(char),MAXRECVCHAR,FileOpen)) > 0){
		/*if(fileReadSize<0){
			perror("File Read Error");
			break;
		}*/
		if(write(connectedPeerRSSockId, line, fileReadSize)!=fileReadSize){
			perror("Connection trouble"); 
			return -1;
		}
		#ifdef DEBUG
		fwrite(line, sizeof(char), fileReadSize, stdout);
		#endif
		if(recvdMsgSize < MAXRECVCHAR) break;
	}
	fclose(FileOpen); 
	//if(send(connectedPeerRSSockId,str4,strlen(str4),0)!=strlen(str4))perror("Connection trouble");
	//Receive
#ifdef DEBUG
	printf("here\r\n");
#endif
	FileOpen = fopen("RScommFile","w+");
	if (FileOpen == NULL){
	DieWithError( "Can't open file in directory, RScommFile!\n");
	return -1;
	}
	while((recvdMsgSize = recv(connectedPeerRSSockId, recvMsg, MAXRECVCHAR, 0))>0){
#ifdef DEBUG
			printf("here recv1\r\n");
			printf("RecvdMsgSize:%d\n",recvdMsgSize);
#endif
			fwrite(recvMsg, sizeof(char), recvdMsgSize, FileOpen);
			}
	close(connectedPeerRSSockId);		
	fflush(FileOpen);
	rewind(FileOpen);
	if(recvdMsgSize<0) {
		error("receive connectedCliServSockId error");
		return;
	}
	//PARSE
	
	//PARSE
	char str[1024];
	int count =0;
	long int buf[4];
	char *FirstLine;
	while((fgets(str,1024, FileOpen))!=NULL){
	    count++;
	 
	    if(count==1){
	    FirstLine = str;
		#ifdef DEBUG
	    printf("%s\n", FirstLine); //check  
		#endif
		}
	 
	    if(count>1 && count<4){
	    char *token;
	    char *ptr;
	    long val;
	    token = strtok(str, "\t");
	    token = strtok(NULL, "\t");
	//  printf("%s\n", token);
	    val = strtol(token, &ptr, 10);
	//  printf("%ld\n", val);
	    buf[count] = val;
		}
	 
	    }
	
 
	#ifdef HEADER
	printf("%s\n%ld\n%ld\n", FirstLine, buf[2], buf[3]); //check
	#endif
	LocalPeerListInfo.hostname = buf[2];
	tcookie = buf[3];
	char str20[12];
	sprintf(str20,"~Cookie%lu",tcookie);
	FILE *ftemp = fopen(str20,"w");
	fclose(ftemp);
	return tcookie;
}



int GetFromPeer(long unsigned int hostAddr,unsigned int otherPeerServerPort, unsigned int rfcNo){
	
	struct sockaddr_in PeerPeerconn;
	int fileReadSize=0;
	int connectedPeerPeerSockId;
	char line[MAXSENDCHAR];
	char recvMsg[MAXRECVCHAR];
	int recvdMsgSize;
	char str[10];
	//create a socket
	#ifdef DEBUG
	printf("Inside GetFromPeer");
	fflush(stdout);
	#endif
	PeerPeerconn.sin_family = AF_INET;
	PeerPeerconn.sin_port = htons(otherPeerServerPort);
	PeerPeerconn.sin_addr.s_addr = hostAddr;
	
	connectedPeerPeerSockId = socket(AF_INET, SOCK_STREAM, 0);
	if(connectedPeerPeerSockId == -1) DieWithError("Socket() Error");
	if(connect(connectedPeerPeerSockId, (struct sockaddr *) &PeerPeerconn, sizeof(PeerPeerconn))<0) {DieWithError("Connect() Error");}
	gen_random(str, (10-1));
	FILE *FileOpen = fopen(str,"w+");
	if (FileOpen == NULL){
	DieWithError( "Can't open file in directory, RScommFile!\n");
	
	}
	
	char str1[80],str2[80];
    char method[] = "Get";
    char message[] = "GetRFC";
    char version[] = "P2P-DI/1.0";
    sprintf(str1, "%s %s %s\r\n",method, message, version);
    sprintf(str2, "RFC Number:\t%d\r\n\r\n", rfcNo);

//WTS
	//printf(str1, "%s %s %s\r\n",method, message, version);
      //  printf(str2, "RFC Number:\t%d\r\n\r\n", rfcNo);

    fprintf(FileOpen, "%s%s", str1, str2);
	fprintf(stdout,"%s%s",str1,str2);
	fflush(FileOpen);
	rewind(FileOpen);
	while((fileReadSize = fread (line,sizeof(char),MAXRECVCHAR,FileOpen)) > 0){
		if(write(connectedPeerPeerSockId, line, fileReadSize)!=fileReadSize){
			perror("Connection trouble"); 
			
		}
		#ifdef DEBUG
		printf("Getting RFC from Peer\n");
		fwrite(line, sizeof(char), fileReadSize, stdout);
		#endif
		if(recvdMsgSize < MAXRECVCHAR) break;
	}
	fclose(FileOpen); 
#ifndef DEBUG
	//FILE *fDel = fopen(str,"w");
	remove(str);
#endif
	sprintf(str,"%d.txt",rfcNo );
	FileOpen = fopen(str,"w+");
	if (FileOpen == NULL){
	DieWithError( "Can't open file in directory, RScommFile!\n");
	
	}
	/* if((recvdMsgSize = recv(connectedPeerPeerSockId, recvMsg, MAXRECVCHAR, 0))<0) perror("receive connectedCliServSockId error");
	char str2[10];
	gen_random(str2, (10-1));
	FILE *tempFile = fopen(str2,"w");
	fwrite(recvMsg, sizeof(char), recvdMsgSize, tempFile); */
	
	//if()
	recvdMsgSize = 0;
	while((recvdMsgSize = recv(connectedPeerPeerSockId, recvMsg, MAXRECVCHAR, 0))>0){
#ifdef DEBUG
			printf("here recv2\r\n");
			printf("RecvdMsgSize%d\n",recvdMsgSize);

			fwrite(recvMsg, sizeof(char), recvdMsgSize, stdout);
#endif			
			fwrite(recvMsg, sizeof(char), recvdMsgSize, FileOpen);
			if(recvdMsgSize<MAXRECVCHAR) break;
			}
	
	close(connectedPeerPeerSockId);		
	fflush(FileOpen);
	//rewind(FileOpen);
	if(0==recvdMsgSize){
		remove(str);
		return -1;
	}
	if(recvdMsgSize<0) {
		error("receive connectedCliServSockId error");
		return;
	}
	
    fclose(FileOpen);
	return 0;
}


void gen_random(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
		int i;

    for ( i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}


void RequestToPQuery(){
	
	int fileReadSize=0;
	int connectedPeerRSSockId;
	char line[MAXSENDCHAR];
	char recvMsg[MAXRECVCHAR];
	int recvdMsgSize;
	//create a socket
	connectedPeerRSSockId = socket(AF_INET, SOCK_STREAM, 0);
	if(connectedPeerRSSockId == -1) DieWithError("Socket() Error");
	if(connect(connectedPeerRSSockId, (struct sockaddr *) &PeerRSconn, sizeof(PeerRSconn))<0) {DieWithError("Connect() Error");return;}
	//FILE *fd = fdopen()
	FILE *FileOpen = fopen("RScommFile","w+");
	if (FileOpen == NULL){
	DieWithError( "Can't open file in directory, RScommFile!\n");
	return;
	}
	char str1[80];
	char method[] = "Query";
	char message[] = "PQuery";
	char version[] = "P2P-DI/1.0";
	sprintf(str1, "%s %s %s \r\n\r\n", method, message, version);

	//WTS
	//printf(str1, "%s %s %s \r\n\r\n", method, message, version);
	
	fprintf(FileOpen, "%s", str1);
	fprintf(stdout, "%s", str1);
	fflush(FileOpen);
	
	//fclose(FileOpen);
	rewind(FileOpen);
	//FileOpen = fopen("RScommFile","r");
	while((fileReadSize = fread (line,sizeof(char),MAXRECVCHAR,FileOpen)) > 0){
		/*if(fileReadSize<0){
			perror("File Read Error");
			break;
		}*/
		if(write(connectedPeerRSSockId, line, fileReadSize)!=fileReadSize){
			perror("Connection trouble"); 
			return;
		}
		#ifdef DEBUG
		fwrite(line, sizeof(char), fileReadSize, stdout);
		#endif
		if(fileReadSize < MAXRECVCHAR) break;
	}
	fclose(FileOpen); 
	//if(send(connectedPeerRSSockId,str4,strlen(str4),0)!=strlen(str4))perror("Connection trouble");
	//Receive
#ifdef DEBUG
	printf("Getting PQuery\r\n");
#endif
	FileOpen = fopen("RScommFile","w+");
	if (FileOpen == NULL){
	DieWithError( "Can't open file in directory, RScommFile!\n");
	return;
	}
	while((recvdMsgSize = recv(connectedPeerRSSockId, recvMsg, MAXRECVCHAR, 0))>0){
#ifdef DEBUG
			printf("here recv3\r\n");
			printf("RecvdMsgSize:%d\n",recvdMsgSize);
#endif
			fwrite(recvMsg, sizeof(char), recvdMsgSize, FileOpen);
			}
	close(connectedPeerRSSockId);		
	fflush(FileOpen);
	rewind(FileOpen);
	if(recvdMsgSize<0) {
		error("receive connectedCliServSockId error");
		return;
	}
	//PARSE
	
	//PARSE	
	char str[1024];
	//int count =0;
	long int buf[4];
	char *FirstLine;
	unsigned long int tValue1;
	unsigned int tValue2;
	//while((fgets(str,1024, FileOpen))!=NULL){
	if((fgets(str,1024, FileOpen))!=NULL){
	//count++;

	//if(count==1){
	FirstLine = str;

	

	#ifdef HEADER
	printf("%s\n", FirstLine); //check	
	#endif
	/*}*/
	//fflush(FileOpen);
	//fseek(FileOpen,position_in_file, SEEK_SET);
	//ACTION->write the file with the peer index database->send text file 
	}
	#ifdef DEBUG
	printf("%s",str);
	printf(" PQuery Not going inside\r\n");
	#endif
	while(fscanf(FileOpen,"%lu %ul",&tValue1,&tValue2)==2){
	if(tValue1 == LocalPeerListInfo.hostname && tValue2 == LocalPeerListInfo.peerRfcServerPort){continue;}
		struct peerListDatabase *newEntry = (struct peerListDatabase*)malloc(sizeof(struct peerListDatabase));
		newEntry->hostname = tValue1;
		newEntry->peerRfcServerPort = tValue2;
		newEntry->nextNode = NULL;
		#ifdef DEBUG
		printf("%p PQuery1\r\n",(infoPeerListDatabase.lastNode)->nextNode);
		#endif
		(infoPeerListDatabase.lastNode)->nextNode = newEntry;
		#ifdef DEBUG
		printf("%p PQuery2\r\n ",(infoPeerListDatabase.lastNode)->nextNode);
		#endif
		infoPeerListDatabase.lastNode = newEntry;
	}
	#ifndef DEBUG
	remove("RScommFile");
	#endif
	return;
}


void RequestToLeave(long unsigned int tcookie){
	
	int fileReadSize=0;
	int connectedPeerRSSockId;
	char line[MAXSENDCHAR];
	char recvMsg[MAXRECVCHAR];
	int recvdMsgSize;
	//create a socket
	connectedPeerRSSockId = socket(AF_INET, SOCK_STREAM, 0);
	if(connectedPeerRSSockId == -1) DieWithError("Socket() Error");
	if(connect(connectedPeerRSSockId, (struct sockaddr *) &PeerRSconn, sizeof(PeerRSconn))<0) {DieWithError("Connect() Error");return;}
	//FILE *fd = fdopen()
	FILE *FileOpen = fopen("RScommFile","w+");
	if (FileOpen == NULL){
	DieWithError( "Can't open file in directory, RScommFile!\n");
	return;
	}
	char str1[80], str2[80];
	char method[] = "Exit";
	char message[] = "Leave";
	char version[] = "P2P-DI/1.0";
	sprintf(str1, "%s %s %s \r\n", method, message, version);
	sprintf(str2, "Cookie:\t%lu\r\n", tcookie);

//WTS
	//printf(str1, "%s %s %s \r\n", method, message, version);
	//printf(str2, "Cookie:\t%lu\r\n", tcookie);

	fprintf(FileOpen, "%s%s", str1, str2);
	fprintf(stdout, "%s%s", str1, str2);
	fflush(FileOpen);
	
	//fclose(FileOpen);
	rewind(FileOpen);
	//FileOpen = fopen("RScommFile","r");
	while((fileReadSize = fread (line,sizeof(char),MAXRECVCHAR,FileOpen)) > 0){
		/*if(fileReadSize<0){
			perror("File Read Error");
			break;
		}*/
		if(write(connectedPeerRSSockId, line, fileReadSize)!=fileReadSize){
			perror("Connection trouble"); 
			return;
		}
		#ifdef DEBUG
		fwrite(line, sizeof(char), fileReadSize, stdout);
		#endif
		if(recvdMsgSize < MAXRECVCHAR) break;
	}
	fclose(FileOpen); 
	//if(send(connectedPeerRSSockId,str4,strlen(str4),0)!=strlen(str4))perror("Connection trouble");
	//Receive
#ifdef DEBUG
	printf("Requesting To Leave\r\n");
#endif
	FileOpen = fopen("RScommFile","w+");
	if (FileOpen == NULL){
	DieWithError( "Can't open file in directory, RScommFile!\n");
	return;
	}
	while((recvdMsgSize = recv(connectedPeerRSSockId, recvMsg, MAXRECVCHAR, 0))>0){
#ifdef DEBUG
			printf("here recv4\r\n");
			printf("RecvdMsgSize:%d\n",recvdMsgSize);
#endif
			fwrite(recvMsg, sizeof(char), recvdMsgSize, FileOpen);
			}
	close(connectedPeerRSSockId);		
	fflush(FileOpen);
	rewind(FileOpen);
	if(recvdMsgSize<0) {
		error("receive connectedCliServSockId error");
		return;
	}
	//PARSE
	
	//PARSE
	char str[1024];
	int count =0;
	long int buf[4];
	char *FirstLine;
	while((fgets(str,1024, FileOpen))!=NULL){
	count++;

	if(count==1){
	FirstLine = str;

	

	#ifdef HEADER
	printf("%s\n", FirstLine); //check	
	#endif
	}
	}
	return;
}


void RequestToKeepAlive(long unsigned int tcookie){
	
	int fileReadSize=0;
	int connectedPeerRSSockId;
	char line[MAXSENDCHAR];
	char recvMsg[MAXRECVCHAR];
	int recvdMsgSize;
	//create a socket
	connectedPeerRSSockId = socket(AF_INET, SOCK_STREAM, 0);
	if(connectedPeerRSSockId == -1) DieWithError("Socket() Error");
	if(connect(connectedPeerRSSockId, (struct sockaddr *) &PeerRSconn, sizeof(PeerRSconn))<0) {DieWithError("Connect() Error");return;}
	//FILE *fd = fdopen()
	FILE *FileOpen = fopen("RScommFile","w+");
	if (FileOpen == NULL){
	DieWithError( "Can't open file in directory, RScommFile!\n");
	return;
	}
	char str1[80],str2[80];
	char method[] = "Poll";
	char message[] = "KeepAlive";
	char version[] = "P2P-DI/1.0";
	sprintf(str1, "%s %s %s \r\n",method, message, version);
	sprintf(str2, "Cookie\t%lu\r\n", tcookie);

//WTS
	//printf(str1, "%s %s %s \r\n",method, message, version);
	//printf(str2, "Cookie\t%lu\r\n", tcookie);

	fprintf(FileOpen, "%s%s", str1, str2);
	fprintf(stdout, "%s%s", str1, str2);
	fflush(FileOpen);
	
	//fclose(FileOpen);
	rewind(FileOpen);
	//FileOpen = fopen("RScommFile","r");
	while((fileReadSize = fread (line,sizeof(char),MAXRECVCHAR,FileOpen)) > 0){
		/*if(fileReadSize<0){
			perror("File Read Error");
			break;
		}*/
		if(write(connectedPeerRSSockId, line, fileReadSize)!=fileReadSize){
			perror("Connection trouble"); 
			return;
		}
		#ifdef DEBUG
		fwrite(line, sizeof(char), fileReadSize, stdout);
		#endif
		if(recvdMsgSize < MAXRECVCHAR) break;
	}
	fclose(FileOpen); 
	//if(send(connectedPeerRSSockId,str4,strlen(str4),0)!=strlen(str4))perror("Connection trouble");
	//Receive
#ifdef DEBUG
	printf("Requesting To Keep Me Alive\r\n");
#endif
	FileOpen = fopen("RScommFile","w+");
	if (FileOpen == NULL){
	DieWithError( "Can't open file in directory, RScommFile!\n");
	return;
	}
	while((recvdMsgSize = recv(connectedPeerRSSockId, recvMsg, MAXRECVCHAR, 0))>0){
#ifdef DEBUG
			printf("here recv5\r\n");
			printf("recvdMsgSize:%d\n",recvdMsgSize);
#endif
			fwrite(recvMsg, sizeof(char), recvdMsgSize, FileOpen);
			}
	close(connectedPeerRSSockId);		
	fflush(FileOpen);
	rewind(FileOpen);
	if(recvdMsgSize<0) {
		error("receive connectedCliServSockId error");
		return;
	}
	//PARSE
	
	//PARSE
	char str[1024];
	int count =0;
	long int buf[4];
	char *FirstLine;
	while((fgets(str,1024, FileOpen))!=NULL){
	count++;

	if(count==1){
	FirstLine = str;



	#ifdef DEBUG
	printf("%s\n", FirstLine); //check	
	#endif
	}

	//ACTION->Browse the peer in peer index using cookie_num ->Set the peer active in peer index
	}
 
	#ifdef HEADER
	printf("%s\n", FirstLine); //check
	#endif
return;
	
}


void *PeerPeerHandleFunction(void *msg){
		int p2pSockId = *((int *) msg);
		char recvMsg[MAXRECVCHAR];
		int recvdMsgSize;
		char str[10];
		char method[10];
		int fetchRfcNo;
		
		gen_random(str, (10-1));
		FILE *fp = fopen(str,"w+");
		while((recvdMsgSize = recv(p2pSockId, recvMsg, MAXRECVCHAR, 0)) > 0){
#ifdef DEBUG
				printf("InPeerPeerHandle Recvdmsgsize:%d\n",recvdMsgSize);
#endif
				
				fwrite(recvMsg, sizeof(char), recvdMsgSize, fp);	
				if(recvdMsgSize < MAXRECVCHAR) break;
		}
		fflush(fp);
		rewind(fp);
		parseRequestFile(fp);
		fscanf(fp,"%s",method);
		#ifdef HEADER
		printf("%s",method);
		#endif
		if(strcmp("GetRFC",method)==0){
			char line[MAXRECVCHAR];
			int fileReadSize;
			char strTemp[80];
			fscanf(fp,"%d",&fetchRfcNo);
			#ifdef DEBUG
			printf("Almost Der\r\n");
			#endif
			if(SearchParticularPeerForRfc( LocalPeerListInfo.myRfcListFirstNode ,fetchRfcNo) == TRUE){
			
			sprintf(strTemp,"%d.txt",fetchRfcNo);
			FILE *fetchfp=fopen(strTemp, "r");
				while((fileReadSize = fread (line,sizeof(char),MAXRECVCHAR,fetchfp)) > 0){
				/*if(fileReadSize<0){
				perror("File Read Error");
				break;
				}*/
						if(write(p2pSockId, line, fileReadSize)!=fileReadSize){
						perror("Connection trouble"); 
					
					}
					#ifdef DEBUG
				fwrite(line, sizeof(char), fileReadSize, stdout);
				#endif
				}
			}
			remove(str);
		}else if (strcmp("RFCQuery", method)==0){
			
		}else if(strcmp(method,"BadRequest")==0){
			
			#ifdef DEBUG
			printf("BadREQUEST");
			#endif
	}
	//getrfc? check for local files
	//pquery? send peerdatabase
	fclose(fp);
	fflush(stdout);
	close(p2pSockId);
}


void parseRequestFile(FILE *fp){
	char str[1024];
	if((fgets(str,1024, fp))!=NULL){
		
		if(strstr(str,"RFCQuery")){
		#ifdef DEBUG
		printf("%s\n",strstr(str,"RFCQuery"));
		#endif
		rfcquery_parse(fp);
		}
		else if(strstr(str,"GetRFC")){
			#ifdef DEBUG
			printf("%s\n",strstr(str,"GetRFC"));
			#endif
			getrfc_parse(fp);
		}else{
			rewind(fp);
			fprintf(fp,"BadRequest\r\n"); //check
			fflush(fp);
			rewind(fp);
			perror("Wrong Request");
		}
	}else DieWithError("Error With File Read for Parsing");
}


void rfcquery_parse(FILE *fp){
rewind(fp);
unsigned int count = 0;
	char str[1024];
	char *FirstLine;
	unsigned long int buf[4];
while((fgets(str,1024, fp))!=NULL){
	count++;

	if(count==1){
	FirstLine = str;


	#ifdef HEADER
	printf("%s\n", FirstLine); //check	
	#endif
	}
	//fflush(FileOpen);
	//fseek(FileOpen,position_in_file, SEEK_SET);
//ACTION->write the file with the rfc database->send text file 
	}
}


void getrfc_parse(FILE *fp){
unsigned long int buf[4];
rewind(fp);
char str[1024];
int count;
	char *FirstLine;
while((fgets(str,1024, fp))!=NULL){
	count++;

	if(count==1){
	FirstLine = str;
	#ifdef DEBUG
	printf("%s\n", FirstLine); //check	
	#endif
	}

	if(count>1 && count<3){
	char *token;
	char *ptr;
	long val;
	token = strtok(str, "\t");
	token = strtok(NULL, "\t");
//	printf("%s\n", token);
	val = strtol(token, &ptr, 10);
//	printf("%ld\n", val);
	buf[count] = val;
	}
//ACTION->Browse the rfc in rfc index using rfc_num ->Send the rfc document to the peer
	}
	
//long int rfc_num = buf[2];
rewind(fp);

fprintf(fp,"GetRFC\r\n%ld\r\n", buf[2]); //check
fprintf(stdout,"GetRFC %ld\r\n", buf[2]);
fflush(fp);
	rewind(fp);
}
