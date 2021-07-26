 #include <stdio.h>
 #include <stdlib.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <string.h>
 #include <time.h>
 #include <unistd.h>
 #include <arpa/inet.h>
 #include <ctype.h>

#define MAX_MSG_LEN 257
#define RESPONSE_BYTES 512
#define REQUEST_BYTES 512

int sockFD, portNO;

void error(char *str) {
	perror(str);
	exit(EXIT_FAILURE);
}

void msg(char *str) {
	printf("%s", str);
}

char* receiveMsgFromServer(int sockFD) {
	int numPacketsToReceive = 0;
	int n = read(sockFD, &numPacketsToReceive, sizeof(int));
	if(n <= 0) {
		shutdown(sockFD, SHUT_WR);
		return NULL;
	}
	char *str = (char*)malloc(numPacketsToReceive*RESPONSE_BYTES);
	memset(str, 0, numPacketsToReceive*RESPONSE_BYTES);
	char *str_p = str;
	int i;
	for(i = 0; i < numPacketsToReceive; ++i) {
		int n = read(sockFD, str, RESPONSE_BYTES);
		str = str+RESPONSE_BYTES;
	}
	return str_p;
}

void sendMsgToServer(int sockFD, char *str) {
	int numPacketsToSend = (strlen(str)-1)/REQUEST_BYTES + 1;
	int n = write(sockFD, &numPacketsToSend, sizeof(int));
	char *msgToSend = (char*)malloc(numPacketsToSend*REQUEST_BYTES);
	strcpy(msgToSend, str);
	int i;
	for(i = 0; i < numPacketsToSend; ++i) {
		int n = write(sockFD, msgToSend, REQUEST_BYTES);
		msgToSend += REQUEST_BYTES;
	}
}
void AddJointAcc(){
  char *msgFromServer;
  char msgToServer[MAX_MSG_LEN];
  int i=5;
  while(i>0)
  {
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
  free(msgFromServer);

  memset(msgToServer, 0, sizeof(msgToServer));
  scanf("%s", msgToServer);
  sendMsgToServer(sockFD, msgToServer);
  i--;
  }
  //printf("Hiiii Londe\n");
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
  free(msgFromServer);

  double val,size;
  scanf("%lf", &size);
  write(sockFD, &size, sizeof(size));

  //read(sockFD, &val, sizeof(val));
  //printf("%lf\n",val);
  //msgFromServer = receiveMsgFromServer(sockFD);
  //msg(msgFromServer);
  //free(msgFromServer);
}
void AddNormalAcc(){
  char *msgFromServer;
  char msgToServer[MAX_MSG_LEN];
  int i=3;
  while(i>0)
  {
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
  free(msgFromServer);

  memset(msgToServer, 0, sizeof(msgToServer));
  scanf("%s", msgToServer);
  sendMsgToServer(sockFD, msgToServer);
  i--;
  }
  //printf("Hiiii Bhidu\n");
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
  free(msgFromServer);

  double val,size;
  scanf("%lf", &size);
  write(sockFD, &size, sizeof(size));

  //read(sockFD, &val, sizeof(val));
  //printf("%lf\n",val);
  //msgFromServer = receiveMsgFromServer(sockFD);
  //msg(msgFromServer);
  //free(msgFromServer);

}
void jointaccountdetails(){
  char *msgFromServer;
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
  printf("\nAccount Name1: ");
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
  printf("\nAccount Name2: ");
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
  printf("\nAccount Type: ");
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
  printf("\nContact No: ");
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
  printf("\nAvailable Balance: ");
  double val;
  read(sockFD, &val, sizeof(val));
  printf("%0.2lf\n",val);
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);


}
void normalaccountdetails(){
   char *msgFromServer;
   msgFromServer = receiveMsgFromServer(sockFD);
   msg(msgFromServer);

   printf("\nAccount Name: ");
   msgFromServer = receiveMsgFromServer(sockFD);
   msg(msgFromServer);

   printf("\nAccount Type: ");
   msgFromServer = receiveMsgFromServer(sockFD);
   msg(msgFromServer);
   printf("\nContact No: ");

   msgFromServer = receiveMsgFromServer(sockFD);
   msg(msgFromServer);
   printf("\nAvailable Balance: ");
   double val;
   read(sockFD, &val, sizeof(val));
   printf("%0.2lf\n",val);
   msgFromServer = receiveMsgFromServer(sockFD);
   msg(msgFromServer);
}
void ModifyNormalAcc(){
  char msgToServer[MAX_MSG_LEN];
  char *msgFromServer;
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);

  printf("\nAccount Name: ");
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);

  printf("\nAccount Type: ");
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
  printf("\nContact No: ");

  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
  printf("\nAvailable Balance: ");
  double val;
  read(sockFD, &val, sizeof(val));
  printf("%0.2lf\n",val);
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);

  //printf("Enter the modified value in the respected field or Enter -1 for no change\n");
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);

  printf("UserName: ");
  memset(msgToServer, 0, sizeof(msgToServer));
  scanf("%[^\n]", msgToServer);
  sendMsgToServer(sockFD, msgToServer);

  printf("ContactNo: ");
  memset(msgToServer, 0, sizeof(msgToServer));
  scanf("%[^\n]", msgToServer);
  sendMsgToServer(sockFD, msgToServer);

  double size;
  printf("Available amount: ");
  scanf(" %lf", &size);
  write(sockFD, &size, sizeof(size));
}
void ModifyJointAcc(){
  char msgToServer[MAX_MSG_LEN];
  printf("Enter the modified value in the respected field or Enter -1 for no change\n");

  printf("UserName1: ");
  memset(msgToServer, 0, sizeof(msgToServer));
  scanf("%[^\n]", msgToServer);
  sendMsgToServer(sockFD, msgToServer);

  printf("UserName2: ");
  memset(msgToServer, 0, sizeof(msgToServer));
  scanf("%[^\n]", msgToServer);
  sendMsgToServer(sockFD, msgToServer);

  printf("ContactNo: ");
  memset(msgToServer, 0, sizeof(msgToServer));
  scanf("%[^\n]", msgToServer);
  sendMsgToServer(sockFD, msgToServer);

  double size;
  printf("Available amount: ");
  scanf(" %lf", &size);
  write(sockFD, &size, sizeof(size));
}
void UserDeposit(char* msgFromServer)
{  msg(msgFromServer);
  double size,val;
  scanf("%lf", &size);
  write(sockFD, &size, sizeof(size));
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);
}
void UserWithdraw(char* msgFromServer)
{ msg(msgFromServer);
  double size,val;
  scanf("%lf", &size);
  write(sockFD, &size, sizeof(size));
  msgFromServer = receiveMsgFromServer(sockFD);
  msg(msgFromServer);

}
void UserViewDetail(char* msgFromServer)
{ msg(msgFromServer);
  printf("\n    Type | Amount |  Date & Time \n");
  int last,start;
  double amt;
  read(sockFD, &last, sizeof(last));
  read(sockFD, &start, sizeof(start));
  for(int i = start; i <= last; i++){
    msgFromServer = receiveMsgFromServer(sockFD);
    msg(msgFromServer);
    read(sockFD, &amt, sizeof(amt));
    msgFromServer = receiveMsgFromServer(sockFD);
    msg(msgFromServer);
  }

  //msgFromServer = receiveMsgFromServer(sockFD);
  //msg(msgFromServer);
}
int main(int argc, char **argv) {
	struct sockaddr_in serv_addr;
	char *msgFromServer;
	char msgToServer[MAX_MSG_LEN];

	if(argc < 3) {
		fprintf(stderr, "Usage: %s host_addr port_number\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	portNO = atoi(argv[2]);
	if((sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error in opening socket.\n");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	//setting sockaddr_in serv_addr
	serv_addr.sin_family = AF_INET;			//setting DOMAIN
	serv_addr.sin_port = htons(portNO);		//setting port numbet
	if((inet_aton(argv[1], &serv_addr.sin_addr)) == 0) {
		error("Error Invalid Host Name");
	}

	if(connect(sockFD, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		error("Error in connecting.\n");
	}

	msg("Connecting to Server.....\n");
  //sleep(2);
  printf("\033c");
  msg("--------------------------------------------------------\n");
  msg("=*=*=*=*=*=-*-*-*-*-*- LOGIN PAGE -*-*-*-*-*-=*=*=*=*=*=\n");
  msg("--------------------------------------------------------\n");

	while(1) {
		msgFromServer = receiveMsgFromServer(sockFD);
		if(msgFromServer == NULL)
			break;
		if(strncmp(msgFromServer, "unauth", 6) == 0) {
			msg("Unautherized User.\n");
			shutdown(sockFD, SHUT_WR);
			break;
    }
      else if(strncmp(msgFromServer,"AddNormalUser", 13) == 0) {
        		  AddNormalAcc();
              goto next;
          		}
      else if(strncmp(msgFromServer,"AddJointUser", 12) == 0) {
        		  AddJointAcc();
              goto next;
              }
      else if(strncmp(msgFromServer,"Normal Accounts Details",23) == 0) {
      		  normalaccountdetails();
            goto next;
        		}
      else if(strncmp(msgFromServer,"Joint Accounts Details", 16) == 0) {
          		  jointaccountdetails();
                goto next;
            	}
      else if(strncmp(msgFromServer,"Joint Account Modify", 20) == 0) {
                ModifyJointAcc();
                goto next;
              }
      else if(strncmp(msgFromServer,"Normal Account Detial and Modify", 32) == 0) {
                  ModifyNormalAcc();
                goto next;
              }
      else if(strncmp(msgFromServer,"Do you want to delete account?(Y/N)", 35) == 0) {
              memset(msgToServer, 0, sizeof(msgToServer));
              scanf("%s", msgToServer);
              sendMsgToServer(sockFD, msgToServer);
              msgFromServer = receiveMsgFromServer(sockFD);
          		msg(msgFromServer);
              goto next;
              }
     else if(strncmp(msgFromServer,"Enter the amount you want to deposit:", 37) == 0) {
              UserDeposit(msgFromServer);
              goto next;
            }
     else if(strncmp(msgFromServer,"Enter the amount you want to withdraw:", 38) == 0) {
              UserWithdraw(msgFromServer);
              goto next;
             }
     else if(strncmp(msgFromServer,"Normal Accounts Balance Details", 31) == 0) {
              double val;
              read(sockFD, &val, sizeof(val));
              printf("Your avaliable bal is: %0.2lf\n",val);
              goto next;
           }
      else if(strncmp(msgFromServer,"Enter Password:", 15) == 0) {
              char pass[50];
              memset(msgToServer, 0, sizeof(msgToServer));
              strcpy(pass, getpass("Enter Password:\n"));
              sendMsgToServer(sockFD,pass);
              goto next;
             }
      else if(strncmp(msgFromServer,"Last 3 Transaction Details:", 27) == 0) {
              UserViewDetail(msgFromServer);
              goto next;
             }

		msg(msgFromServer);
		msg("\n");
		free(msgFromServer);

    memset(msgToServer, 0, sizeof(msgToServer));
		scanf("%s", msgToServer);
		sendMsgToServer(sockFD, msgToServer);
    next:
		if(strncmp(msgToServer, "6", 1) == 0) {
			shutdown(sockFD, SHUT_WR);
			break;
		}
	}

	msg("Receiving Pending Messages from server.\n");

	while(1) {
		msgFromServer = receiveMsgFromServer(sockFD);
		if(msgFromServer == NULL)
			break;
		msg(msgFromServer);
		msg("\n");
		free(msgFromServer);
	}
	msg("Write end closed by the server.\n");
	shutdown(sockFD, SHUT_RD);
	msg("Connection closed gracefully.\n");
	return 0;
}
	/*else if(strncmp(msgFromServer, "Enter the account opening amount:", 33) == 0) {
  msg(msgFromServer);
  int val;
  scanf("%d", &val);
  val = htonl(val);
  write(sockFD, &val, sizeof(val));
  goto next;
}*/
