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
#include<limits.h>  //HOST_NAME_MAX
#include<unistd.h>
#include<time.h>


//#defines
#ifdef LOCALNETWORK
#define REGISTRATIONSERVERLISTENPORT 65423
#else
#define REGISTRATIONSERVERLISTENPORT 65443
#endif
#define TRUE 1
#define FALSE 0
#define CLIENTQUEUELIMIT 5
#define MAXRECVCHAR 500

#define MAXLINE 100


struct peerDBInfo{
struct peerList *firstNode;
struct peerList *lastNode;
int count;
}peerListInfo = {NULL,NULL,0};


struct peerList{
		unsigned long int hostName; // save in network value<- or in dotted notation?
		unsigned int cookie;
		unsigned short int flag;
		unsigned int timeToLive;
		unsigned int peerRfcServerPort;
		time_t lastAccessTime; // Stored as seconds elapsed since epoch?
		unsigned int countOfActiveSessions;
		struct peerList *nextNode;
};

struct dataToClientHandleThread{
		unsigned long int hostName;
		unsigned int sockId;
};

void DieWithError(char *msg){
perror(msg);
exit -1;
}

void AddPeer(unsigned long int, unsigned int, unsigned int);
int RenewPeer(int );
int Leave(int );
void SendRegisterResponse(int , unsigned long int , unsigned long int , char *);
void SendLeaveResponse(int );
void SendPQueryResponse(int , char *);
void SendKeepAliveResponse(int );
void WritePeerList(FILE *);




void HandleCtrlC(int signum);
void gen_random(char *s, const int len);
void* ClientHandleFunction(void*);
void parseRequestFile(FILE *);
void registration_parse(FILE *);
void leave_parse(FILE *);
void pquery_parse(FILE *);
void keepalive_parse(FILE *);





int main(int argc, char *argv[]){
	
	
	signal(SIGINT, HandleCtrlC);
	
	//Local Variables
	struct sockaddr_in serverListenSocket; 
	
	int serverListenSockId;
	int connectedCliServSockId;	
	struct sockaddr_in serverClientConn;
	
	unsigned int clntLen = sizeof(struct sockaddr_in);
	pid_t pid;
	//Create socket
	serverListenSockId = socket(AF_INET, SOCK_STREAM, 0);
	if(serverListenSockId == -1) DieWithError("Error Creating serverListenSockId");
        //SO_REUSEADDR -> So that we can use a socket that is in a wait state
	
	//Settings for server listen socket
	serverListenSocket.sin_family = AF_INET;
	serverListenSocket.sin_port = htons(REGISTRATIONSERVERLISTENPORT);
	serverListenSocket.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//Bind
	if (bind(serverListenSockId, (struct sockaddr *) &serverListenSocket, sizeof(serverListenSocket)) == -1) DieWithError("Bind() Error");
	//Listen
	if(listen(serverListenSockId, CLIENTQUEUELIMIT) == -1) DieWithError("listen serverListenSockId error");
#ifdef DEBUG	
	printf("I am listening\n");
#endif
	while(TRUE){
		
		connectedCliServSockId = accept(serverListenSockId, (struct sockaddr *) &serverClientConn, &clntLen);
#ifdef DEBUG		
		printf("connectedCliServSockId:%d\n",connectedCliServSockId);
#endif
		if(connectedCliServSockId == -1) perror("accept connectedCliServSockId error");
		else{
			struct dataToClientHandleThread *tVal = malloc(sizeof(struct dataToClientHandleThread));
			tVal->hostName = serverClientConn.sin_addr.s_addr;
			tVal->sockId = connectedCliServSockId;
			pthread_t *clientHandleThreads = malloc(sizeof(pthread_t));
			//int *arg = malloc(sizeof(int *));
			if(clientHandleThreads == NULL || tVal == NULL){
				DieWithError("Out of memory");
			}else{
				//*arg = connectedCliServSockId;
				if(pthread_create(clientHandleThreads, NULL, ClientHandleFunction,tVal)!=0){
					DieWithError("Couldn't create thread");
					//Send an error msg to peer
					close(connectedCliServSockId);
					continue;
				}
			}
		}

			
	}
			
		



		//Never reach
	if(close(serverListenSockId) == -1) DieWithError("close serverListenSockId");
	return 0;
}


void* ClientHandleFunction(void *msg){
			//tConnectedCliServSockId =*((int *) msg);
			unsigned int hostAddress = ((struct dataToClientHandleThread *)msg)->hostName;
			
			int tConnectedCliServSockId = ((struct dataToClientHandleThread *)msg)->sockId;
#ifdef DEBUG
			printf("%ul %d\r\n",hostAddress, tConnectedCliServSockId);
#endif
			//printf("servicing the client %d",tConnectedCliServSockId);
			fflush(stdout);
			char recvMsg[MAXRECVCHAR];
        	int recvdMsgSize;
			//int fileReadSize;
			char str[10];
			char method[10];
			long unsigned int peerRFCServerPortNo;
			long unsigned int tcookie;
			char time[80];
			//printf("Could be");
			gen_random(str, (10-1));
			FILE *fp = fopen(str,"w+");
			while((recvdMsgSize = recv(tConnectedCliServSockId, recvMsg, MAXRECVCHAR, 0)) > 0){
			//while((fileReadSize =fread(recvMsg,sizeof(char),MAXRECVCHAR,fd))>0){
				/*if(recvdMsgSize<0) {
					perror("receive connectedCliServSockId error");
					break;
				}*/
#ifdef DEBUG
				printf("Recvd Msg Size:%d\n",recvdMsgSize);
#endif
				
				fwrite(recvMsg, sizeof(char), recvdMsgSize, fp);
fwrite(recvMsg, sizeof(char), recvdMsgSize, stdout);
				//Save the file
				//printf("here\r\n");
				//fflush(stdout);		
				if(recvdMsgSize < MAXRECVCHAR) break;
			}
			//printf("\r\n\r\n");
			fflush(fp);
			rewind(fp);
			//Parse it
			parseRequestFile(fp);
			fscanf(fp,"%s",method);
#ifdef DEBUG
			printf("%s",method);
#endif			
			if(strcmp(method,"Register")==0){
			
				fscanf(fp, "%lu", &peerRFCServerPortNo);
				fscanf(fp, "%lu", &tcookie);
				if(tcookie == 0){
					tcookie = (rand() % 8999) + 1000;
					AddPeer( hostAddress, tcookie, peerRFCServerPortNo);
#ifdef DEBUG					
					printf("AddPeer");
#endif					
				}else{
					if(RenewPeer(tcookie)==-1){//Peer with cookie number not found
					//tcookie = (rand() % 8999) + 1000;
					//AddPeer(hostAddress,tcookie, peerRFCServerPortNo);
					//BAD REQUEST
#ifdef DEBUG					
					printf("Renewed Peer");
#endif					
					}
				}
				fclose(fp);
				SendRegisterResponse(tConnectedCliServSockId,hostAddress,tcookie,str);	
			}else if(strcmp(method,"Leave")==0){
				fscanf(fp, "%lu", &tcookie);
				Leave(tcookie);
				fclose(fp);
				SendLeaveResponse(tConnectedCliServSockId);
			}else if(strcmp(method,"KeepAlive")==0){
				fscanf(fp, "%lu", &tcookie);
				if(RenewPeer(tcookie)==-1){//Peer with cookie number not found
					//tcookie = (rand() % 8999) + 1000;
					//AddPeer(hostAddress,tcookie, peerRFCServerPortNo);
					//BAD REQUEST
					}
					fclose(fp);
					SendKeepAliveResponse(tConnectedCliServSockId);
			}else if(strcmp(method,"PQuery")==0){
				fclose(fp);
				SendPQueryResponse(tConnectedCliServSockId,str);
			}else if(strcmp(method,"BadRequest")==0){
			fclose(fp);
#ifdef DEBUG			
			printf("BadREQUEST");
#endif			
			}
			
			
			
			
			
			
			//fscanf(fp,"%s %ld %ld %s",method, &peerRFCServerPortNo, &tcookie, time);
			
			//fwrite("h",sizeof(char),1,fwr);
			//printf("%s %lu %lu %s\r\n",method, peerRFCServerPortNo, tcookie, time);
			//write(tConnectedCliServSockId,recvMsg,recvdMsgSize);
			//fclose(fd);
			//fclose(fwr);
#ifdef DEBUG			
			printf("serviced the client %d", tConnectedCliServSockId);
#endif			
			fflush(stdout);
#ifndef DEBUG
			//FILE* fDel =fopen(str,"w");
			remove(str);
#endif
			close(tConnectedCliServSockId);
			return NULL;
} 

void parseRequestFile(FILE *fp){
	char stringToParse[1024];
	if((fgets(stringToParse,1024, fp))!=NULL){
		if(strstr(stringToParse,"Register")){
		#ifdef DEBUG
			printf("%s\n",strstr(stringToParse,"Register"));
		#endif	
			registration_parse(fp);
			}
		else if(strstr(stringToParse,"Leave")){
		#ifdef DEBUG
			printf("%s\n",strstr(stringToParse,"Leave"));
			#endif
			leave_parse(fp);
			}
		else if(strstr(stringToParse,"PQuery")){
		#ifdef DEBUG
			printf("%s\n",strstr(stringToParse,"PQuery"));
			#endif
			pquery_parse(fp);
			}
		else if(strstr(stringToParse,"KeepAlive")){
		#ifdef DEBUG
			printf("%s\n",strstr(stringToParse,"KeepAlive"));
#endif			
			keepalive_parse(fp);
			}
		else {
		rewind(fp);
		fprintf(fp,"BadRequest\r\n"); //check
		fflush(fp);
		rewind(fp);
		#ifdef DEBUG
		perror("Wrong Request");
		#endif
		}
	}else DieWithError("Error With File Read for Parsing");
} 

void HandleCtrlC(int signum){
	signal(SIGINT, HandleCtrlC);
#ifdef DEBUG
	printf("Handler\n");
#endif
	//if(close(serverListenSockId) == -1) 
		//perror("Close() error in HandleCtrlC"); // Close open ports on error
	exit(3);
}

void registration_parse(FILE *fp){
	rewind(fp);
	char *FirstLine;
	unsigned long int buf[4];
	unsigned int count = 0;
	char *date_time;
	char str[1024];
	while((fgets(str,1024, fp))!=NULL){
		count++;

		if(count==1){
		FirstLine = str;
		#ifdef HEADER
		printf("%s\n", FirstLine); //check	
		#endif
		}

		if(count>1 && count<4){
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

		if(count==4){
		char *token;
		token = strtok(str,"\t");
		token = strtok(NULL, "\t");
	//	printf("%s\n", token);
		date_time = token;
			}
		} 

	//long int rfcserverlistenport_num = buf[2];
	//long int cookie_num = buf[3];
	rewind(fp);
	fprintf(fp,"Register\r\n%lu\r\n%lu\r\n",  buf[2], buf[3]); //check
	fflush(fp);
	rewind(fp);
}
void leave_parse(FILE *fp){
rewind(fp);
char *FirstLine;
unsigned long int buf[4];
unsigned int count = 0;
	char str[1024];
while((fgets(str,1024, fp))!=NULL){
	count++;

	if(count==1){
	FirstLine = str;
	#ifdef HEADER
	printf("%s\n", FirstLine); //check	
	#endif
	}

	if(count>1 && count<4){
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
	
	}

	
//long int cookie_num = buf[2];
//long int timetolive = buf[3];
//printf("Leave\n%ld\n%ld\n", cookie_num, timetolive); //check
	rewind(fp);
	fprintf(fp,"Leave\r\n%lu\r\n",  buf[2]); //check
	fflush(fp);
	rewind(fp);
}


void pquery_parse(FILE *fp){
rewind(fp);
char *FirstLine;
unsigned int count = 0;
	char str[1024];
	//unsigned long int buf[4];
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
//ACTION->write the file with the peer index database->send text file -- Not in this function
	}
	rewind(fp);
	
	fprintf(fp,"PQuery\r\n"); //check
	fflush(fp);
	rewind(fp);
}


void keepalive_parse(FILE *fp){
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

	if(count>1 && count<4){
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
//ACTION->Browse the peer in peer index using cookie_num ->Set the peer active in peer index - Not in this function
	}

//long int cookie_num = buf[2];
//long int timetolive = buf[3];
//printf("KeepAlive\n%ld\n", cookie_num); //check
	rewind(fp);

	fprintf(fp,"KeepAlive\r\n%lu\r\n",  buf[2]); //check
	fflush(fp);
	rewind(fp);
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



void AddPeer(unsigned long int hostName, unsigned int cookie, unsigned int peerRfcServerPort){
	struct peerList *newEntry;
	newEntry = (struct peerList*)malloc(sizeof(struct peerList));
	if(0==peerListInfo.count){
	#ifdef DEBUG
	printf("Updating PeerList Info??");
	#endif
	//Update peerListInfo
	peerListInfo.firstNode = newEntry;
	}//Update last peer in database with new entry's address
	if(peerListInfo.lastNode!=NULL) (peerListInfo.lastNode)->nextNode = newEntry; 
	#ifdef DEBUG
	printf("Updating Last Peer??");
		#endif
		//Update the peer list info with new entry node
	peerListInfo.lastNode = newEntry;
	(peerListInfo.count)++;
	
	#ifdef DEBUG
	printf("Updating Peer List Info with new entry node????");
	#endif
	//Fill in new entry fields
	newEntry->hostName = hostName;
	newEntry->cookie = cookie;
	newEntry->peerRfcServerPort = peerRfcServerPort;
	newEntry->flag = TRUE;
	newEntry->timeToLive = 7200;
	time(&(newEntry->lastAccessTime));
	#ifdef DEBUG
	printf("TIME : %f\r\n",(float)newEntry->lastAccessTime);
	#endif
	newEntry->countOfActiveSessions = 1;
	newEntry->nextNode = NULL;
};

int RenewPeer(int cookieId){
struct peerList *presentNode = peerListInfo.firstNode;
while(presentNode !=NULL){
		if(presentNode->cookie == cookieId) {
			presentNode->flag = TRUE;
			time(&(presentNode->lastAccessTime));
			#ifdef DEBUG
				printf("TIME : %f\r\n",(float)presentNode->lastAccessTime);
			#endif	
			(presentNode->countOfActiveSessions)++;
			return 0;
		}else presentNode = presentNode->nextNode;
	}
//->
	return -1;// If NULL then a new peer?
}

int Leave(int cookieId){
struct peerList *presentNode = peerListInfo.firstNode;
while(presentNode !=NULL){
	if(presentNode->cookie == cookieId) {
		presentNode->flag = FALSE;
		
		return 0;
		}
	presentNode = presentNode->nextNode;
	}
//->
	return -1;// error peer not found
}

void SendLeaveResponse(int tSockId){
	char str1[80];
    
    char status[] = "OK";
    char phrase[] = "Leave";
    char version[] = "P2P-DI/1.0";
    sprintf(str1, "%s %s %s \r\n", status, phrase, version);

//WTS
	printf(str1, "%s %s %s \r\n", status, phrase, version);
    
	
	
	if(write(tSockId, str1, sizeof(str1))!=sizeof(str1)){
		perror("Connection trouble"); 
	}
}

void SendKeepAliveResponse(int tSockId){
	char str1[80];
    char status[] = "OK";
    char phrase[] = "KeepAlive";
    char version[] = "P2P-DI/1.0";
    sprintf(str1, "%s %s %s \r\n", status, phrase, version);
    
//WTS
	printf(str1, "%s %s %s \r\n", status, phrase, version);
	
	if(write(tSockId, str1, sizeof(str1))!=sizeof(str1)){
		perror("Connection trouble"); 
	}
}
void SendRegisterResponse(int tSockId, unsigned long int tHostName, unsigned long int tCookieNumber, char *fileName)
{
    char status[] = "OK";
    char phrase[] = "Register";
    char version[] = "P2P-DI/1.0";
	FILE *fp = fopen(fileName, "w+");
	char line[MAXRECVCHAR];
	int fileReadSize;
    char str1[80], str2[80],str3[80];
    sprintf(str1,"%s %s %s\r\n", phrase, status, version);
    sprintf(str2,"Hostname:\t%lu\r\n",tHostName);
    sprintf(str3, "Cookie:\t%lu \r\n\r\n", tCookieNumber);
		fopen(fileName,"w+");
		fprintf(fp,"%s%s%s",str1,str2,str3);
		

	//WTS
	printf(str1,"%s %s %s\r\n", phrase, status, version);
        printf(str2,"Hostname\t%lu\r\n",tHostName);
        printf(str3, "Cookie\t%lu \r\n\r\n", tCookieNumber);
	fflush(fp);
	//fclose(FileOpen);
	rewind(fp);
	//FileOpen = fopen("4202.txt","r");
	while((fileReadSize = fread (line,sizeof(char),MAXRECVCHAR,fp)) > 0){
		/*if(fileReadSize<0){
			perror("File Read Error");
			break;
		}*/
		if(write(tSockId, line, fileReadSize)!=fileReadSize){
			perror("Connection trouble"); 
			
		}
		#ifdef DEBUG
		fwrite(line, sizeof(char), fileReadSize, stdout);
		#endif
		}
	fclose(fp); 

}
 
 
 
 void SendPQueryResponse(int tSockId, char *fileName){
	char status[] = "OK";
    char phrase[] = "PQuery";
    char version[] = "P2P-DI/1.0";
	FILE *fp = fopen(fileName, "w+");
	char line[MAXRECVCHAR];
	int fileReadSize;
    char str1[80], str2[80],str3[80];
    sprintf(str1,"%s %s %s\r\n\r\n", phrase, status, version);

//WTS
	//printf(str1,"%s %s %s\r\n\r\n", phrase, status, version);

		fopen(fileName,"w+");
		fprintf(fp,"%s",str1);
		WritePeerList(fp);
		fflush(fp);
		rewind(fp);
	//FileOpen = fopen("4202.txt","r");
	while((fileReadSize = fread (line,sizeof(char),MAXRECVCHAR,fp)) > 0){
		/*if(fileReadSize<0){
			perror("File Read Error");
			break;
		}*/
		if(write(tSockId, line, fileReadSize)!=fileReadSize){
			perror("Connection trouble"); 
			
		}
		#ifdef DEBUG
		fwrite(line, sizeof(char), fileReadSize, stdout);
		#endif
		}
	fclose(fp); 
 }
 
 
void updatePeerFlag(void){
#ifdef DEBUG
printf("updatePeerFlag\r\n");
#endif
time_t currentTime;
time(&currentTime);
 
struct peerList *presentNode = peerListInfo.firstNode;
while(presentNode !=NULL){
#ifdef DEBUG
printf("NOT nUll");
#endif
	if(difftime(currentTime,presentNode->lastAccessTime)>7200){
	presentNode->flag = FALSE;
	#ifdef DEBUG
	printf("one inactive inside update peer flag\r\n");
	#endif
	}
	presentNode = presentNode->nextNode;
	}
}
 
 
 
 
 void WritePeerList(FILE *fp){
 #ifdef DEBUG
	printf("WritePeerList\r\n");
#endif
	struct peerList *presentNode = peerListInfo.firstNode;
	updatePeerFlag();
	while(presentNode !=NULL){
	if(presentNode->flag !=FALSE){
		fprintf(fp,"%lu %ul\r\n",presentNode->hostName, presentNode->peerRfcServerPort);
		fprintf(stdout,"%lu %ul\r\n",presentNode->hostName, presentNode->peerRfcServerPort);
	}else {
	#ifdef DEBUG
	printf("One inactive\r\n");	
	#endif
	}
	presentNode = presentNode->nextNode;
	}
 }
