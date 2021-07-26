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
#include <fcntl.h>

#define MAX_USERID_LEN 256
#define MAX_PASS_LEN 256
#define MAX_LEN 256
#define MAX_LINES_IN_MS 5

#define CREDIT 10
#define DEBIT 11

#define ADMIN 1
#define USER 2
#define JOINT 3
#define UNAUTH_USER -1

#define RESPONSE_BYTES 512
#define REQUEST_BYTES 512
struct Transaction {

    double amount;
    char type[10];
    char time[50];
};
struct NAccount {
    char status[10];
    char type[3];        //JA: Joint Account, NU: Normal User
    char userName[50];
    unsigned int accNo;
	  char contactNo[20];
    double availableAmount;

    struct Transaction transactions[1000];
    int transactionsCount;
};
struct JAccount {
    char status[10];
    char type[3];
    char userName1[40];
		char userName2[40];
    unsigned int accNo;
    char contactNo[20];
    double availableAmount;

    struct Transaction transactions[1000];
    int transactionsCount;
};
struct userInfo{
	char status[10];
  char type[4];
	char userId[50];
	char pass[50];
};
char timeofTransaction[50];
void generateTimeString(){
    char temp[40];
    memcpy(timeofTransaction, temp, sizeof(temp));
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char day[3], mon[3], year[5];
    sprintf(day, "%d", timeinfo->tm_mday);
    sprintf(mon, "%d", timeinfo->tm_mon + 1);
    sprintf(year, "%d", timeinfo->tm_year + 1900);

    memcpy(timeofTransaction, day, sizeof(day));
    strcat(timeofTransaction, mon);
    strcat(timeofTransaction, year);

    return;
}
void error(char *str) {
	perror(str);
	exit(EXIT_FAILURE);
}

void msg(char *str) {
	printf("%s", str);
}
void sendMsgToClient(int clientFD, char *str) {
	int numPacketsToSend = (strlen(str)-1)/RESPONSE_BYTES + 1;
	int n = write(clientFD, &numPacketsToSend, sizeof(int));
	char *msgToSend = (char*)malloc(numPacketsToSend*RESPONSE_BYTES);
	strcpy(msgToSend, str);
	int i;
	for(i = 0; i < numPacketsToSend; ++i) {
		int n = write(clientFD, msgToSend, RESPONSE_BYTES);
		msgToSend += RESPONSE_BYTES;
	}
}

char* receiveMsgFromClient(int clientFD) {
	int numPacketsToReceive = 0;
	int n = read(clientFD, &numPacketsToReceive, sizeof(int));
	if(n <= 0) {
		shutdown(clientFD, SHUT_WR);
		return NULL;
	}

	char *str = (char*)malloc(numPacketsToReceive*REQUEST_BYTES);
	memset(str, 0, numPacketsToReceive*REQUEST_BYTES);
	char *str_p = str;
	int i;
	for(i = 0; i < numPacketsToReceive; ++i) {
		int n = read(clientFD, str, REQUEST_BYTES);
		str = str+REQUEST_BYTES;
	}
	return str_p;
}

struct userInfo getUserInfo(int clientFD,int logintype) {
	int n;
	if(logintype==1){
   char *type="\033c\nWelcome to Admin Login Portal\n\nEnter Username:";
	 sendMsgToClient(clientFD,type);

	}else if(logintype==2){
		char *type="\033c\nWelcome to Normal Account Login Portal\n\nEnter Username:";
		sendMsgToClient(clientFD,type);

	}else if(logintype==3){
		char *type="\033c\nWelcome to Joint Account Login Portal\n\nEnter Username:";
		sendMsgToClient(clientFD,type);
  }
	//char *username = "Enter Username:";
	char *password = "Enter Password:";
	char *buffU;
	char *buffP;

	//asking for username
	//sendMsgToClient(clientFD, username);
	buffU = receiveMsgFromClient(clientFD);

	//asking for password
	sendMsgToClient(clientFD, password);
	buffP = receiveMsgFromClient(clientFD);

	struct userInfo uInfo;
	memset(&uInfo, 0, sizeof(uInfo));
	//copy username and password with triming to uInfo
	if(logintype == 1) memcpy(uInfo.type, "AD", 2);
	else if(logintype == 2) memcpy(uInfo.type, "NA", 2);
	else if(logintype == 3) memcpy(uInfo.type, "JA", 2);

	int i;
	for(i = 0; i < MAX_USERID_LEN; ++i) {
		if(buffU[i] != '\n' && buffU[i] != '\0') {
			uInfo.userId[i] = buffU[i];
		} else {
			break;
		}
	}
	uInfo.userId[i] = '\0';

	for(i = 0; i < MAX_PASS_LEN; ++i) {
		if(buffP[i] != '\n' && buffP[i] != '\0') {
			uInfo.pass[i] = buffP[i];
		} else {
			break;
		}
	}
	uInfo.pass[i] = '\0';
	if(buffU != NULL)
		free(buffU);
	buffU = NULL;
	if(buffP != NULL)
		free(buffP);
	buffP = NULL;
	return uInfo;
}


int authorizeUser(struct userInfo uInfo,int logintype,char db[40]) {
	msg("Authorizing the Following User: \n");
	msg(uInfo.userId);
	msg("\n");
	msg(uInfo.pass);
	msg("\n");

	int uBytes=sizeof(uInfo.userId);
	int pBytes=sizeof(uInfo.pass);
	int fd = open(db, O_RDONLY);
	struct userInfo record;

	struct flock lock;
	lock.l_type = F_RDLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_pid = getpid();

	fcntl(fd, F_SETLKW, &lock);
	int currBytes = read(fd, &record, sizeof(record));
	while(currBytes != 0){

			if(strncmp(record.userId, uInfo.userId,uBytes-1) == 0 && strncmp(record.pass,uInfo.pass,pBytes-1) == 0){
           if(strncmp(record.status,"true",4)==0)
				   	 { printf("Logged-in successfully: ");
               return logintype;
             }
            else{
              	return UNAUTH_USER;
            }
				}
			currBytes = read(fd, &record, sizeof(record));
      close(fd);
	}
	return UNAUTH_USER;
}


void closeWithMsg(char *str, int clientFD) {
	sendMsgToClient(clientFD, str);
	shutdown(clientFD, SHUT_RDWR);
}

int UserDeposit(int clientFD,struct userInfo uInfo){
  char *buff = NULL;
  sendMsgToClient(clientFD,"Enter the amount you want to deposit:");

  double size,amt;
  read(clientFD, &size, sizeof(size));
  if(size<=0){
    sendMsgToClient(clientFD,"Invalid!! Entered amount is less than or equal to zero");
    return 0;
  }
  else{
    amt=size;
  }
    struct NAccount account;
    long count;
    int fd = open("./db/user_account_DB", O_RDWR);

    int currBytes = read(fd, &account, sizeof(account));
    count = 0;
    while(currBytes != 0 && strncmp(account.userName, uInfo.userId, strlen(uInfo.userId)) != 0){
        currBytes = read(fd, &account, sizeof(account));
        count++;
    }

   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   account.availableAmount=account.availableAmount+amt;
   ////
   account.transactionsCount = account.transactionsCount+1;
   int aacount=account.transactionsCount;

 	struct Transaction transaction;
 	transaction.amount = account.availableAmount;
 	memcpy(transaction.type, "Deposit", 7);

   generateTimeString();
   memcpy(transaction.time, timeofTransaction, sizeof(timeofTransaction));
   account.transactions[aacount - 1] = transaction;
   ///
   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   write(fd, &account, sizeof(account));
   close(fd);
}

int UserWithdraw(int clientFD,struct userInfo uInfo){
  char *buff = NULL;
  sendMsgToClient(clientFD,"Enter the amount you want to withdraw:");

  double size,amt;
  read(clientFD, &size, sizeof(size));
  if(size<=0){
    sendMsgToClient(clientFD,"Invalid!! Entered amount is less than or equal to zero");
    return 0 ;
  }
  else{
    amt=size;
  }
    struct NAccount account;
    long count;
    int fd = open("./db/user_account_DB", O_RDWR);

    int currBytes = read(fd, &account, sizeof(account));
    count = 0;
    while(currBytes != 0 && strncmp(account.userName, uInfo.userId, strlen(uInfo.userId)) != 0){
        currBytes = read(fd, &account, sizeof(account));
        count++;
    }

   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   account.availableAmount=account.availableAmount-amt;
   ////
   account.transactionsCount = account.transactionsCount+1;
   int aacount=account.transactionsCount;

   struct Transaction transaction;
   transaction.amount = account.availableAmount;
   memcpy(transaction.type, "Withdraw", 7);

   generateTimeString();
   memcpy(transaction.time, timeofTransaction, sizeof(timeofTransaction));
   account.transactions[aacount - 1] = transaction;
   ///
   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   write(fd, &account, sizeof(account));
   close(fd);
}
void UserBalEnq(int clientFD,struct userInfo uInfo){
  struct NAccount account;
  long count = 0;                 //To keep track of account in file

  int fd = open("./db/user_account_DB", O_RDONLY);
  int bytes = read(fd, &account, sizeof(account));
  while(bytes != 0 && strncmp(account.userName, uInfo.userId, strlen(uInfo.userId)) != 0){
      bytes = read(fd, &account, sizeof(account));
      count++;
  }

  struct flock lock;
  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = ((count) * sizeof(account));
  lock.l_len = sizeof(account);
  lock.l_pid = getpid();

  fcntl(fd, F_SETLKW, &lock);
  lseek(fd, ((count) * sizeof(account)), SEEK_SET);
  read(fd, &account, sizeof(account));

  char *tag="Normal Accounts Balance Details";
  sendMsgToClient(clientFD, tag);
  double val;
  val=account.availableAmount;
  write(clientFD, &val, sizeof(val));
  lock.l_type = F_UNLCK;
  fcntl(fd, F_SETLK, &lock);
  close(fd);
}
void UserPassword(int clientFD,struct userInfo uInfo){
  char *buff = NULL;
  sendMsgToClient(clientFD,"Enter your new password:");
  if(buff != NULL)
    free(buff);
    buff = receiveMsgFromClient(clientFD);
    struct userInfo account;
    long count;
    int fd = open("./db/user_login_DB", O_RDWR);

    int currBytes = read(fd, &account, sizeof(account));
    count = 0;
    while(currBytes != 0 && strncmp(account.userId, uInfo.userId, strlen(uInfo.userId)) != 0){
        currBytes = read(fd, &account, sizeof(account));
        count++;
    }
   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   memcpy(account.pass,buff, sizeof(buff));
   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   write(fd, &account, sizeof(account));
   close(fd);
}

void processUserRequests(int clientFD,struct userInfo uInfo) {
	char *buff = NULL;
	sendMsgToClient(clientFD, "\n1).Deposit\n2).Withdraw\n3).Balance Enquiry\n4).Password Change\n5).Exit\nSelect option to contine with: ");
	while(1) {
		if(buff != NULL)
			free(buff);
		buff = receiveMsgFromClient(clientFD);
		  if(strcmp(buff, "1") == 0) {
        UserDeposit(clientFD,uInfo);
        break;
		} else if(strcmp(buff, "2") == 0) {
      UserWithdraw(clientFD,uInfo);
			break;
		}else if(strcmp(buff, "3") == 0) {
      UserBalEnq(clientFD,uInfo);
			break;
		}else if(strcmp(buff, "4") == 0) {
      UserPassword(clientFD,uInfo);
			break;
		}else if(strcmp(buff, "5") == 0) {
			break;
		} else {
			sendMsgToClient(clientFD, "Unknown Query");
		}
	}
}

int JointDeposit(int clientFD,struct userInfo uInfo){
  char *buff = NULL;
  sendMsgToClient(clientFD,"Enter the amount you want to deposit:");

  double size,amt;
  read(clientFD, &size, sizeof(size));
  if(size<=0){
    sendMsgToClient(clientFD,"Invalid!! Entered amount is less than or equal to zero");
    return 0;
  }
  else{
    amt=size;
  }
    struct JAccount account;
    long count;
    int fd = open("./db/joint_account_DB", O_RDWR);

    int currBytes = read(fd, &account, sizeof(account));
    count = 0;
    while(currBytes != 0 && strncmp(account.userName1, uInfo.userId, strlen(uInfo.userId)) != 0 || strncmp(account.userName2, uInfo.userId, strlen(uInfo.userId)) != 0){
        currBytes = read(fd, &account, sizeof(account));
        count++;
    }

   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   account.availableAmount=account.availableAmount+amt;
   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   write(fd, &account, sizeof(account));
   close(fd);
}

int JointWithdraw(int clientFD,struct userInfo uInfo){
  char *buff = NULL;
  sendMsgToClient(clientFD,"Enter the amount you want to withdraw:");

  double size,amt;
  read(clientFD, &size, sizeof(size));
  if(size<=0){
    sendMsgToClient(clientFD,"Invalid!! Entered amount is less than or equal to zero");
    return 0 ;
  }
  else{
    amt=size;
  }
    struct JAccount account;
    long count;
    int fd = open("./db/joint_account_DB", O_RDWR);

    int currBytes = read(fd, &account, sizeof(account));
    count = 0;
    while(currBytes != 0 && strncmp(account.userName1, uInfo.userId, strlen(uInfo.userId)) != 0 || strncmp(account.userName2, uInfo.userId, strlen(uInfo.userId)) != 0){
        currBytes = read(fd, &account, sizeof(account));
        count++;
    }

   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   account.availableAmount=account.availableAmount-amt;
   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   write(fd, &account, sizeof(account));
   close(fd);
}
void JointBalEnq(int clientFD,struct userInfo uInfo){
  struct JAccount account;
  long count = 0;                 //To keep track of account in file

  int fd = open("./db/joint_account_DB", O_RDONLY);
  int bytes = read(fd, &account, sizeof(account));
  while(bytes != 0 && strncmp(account.userName1, uInfo.userId, strlen(uInfo.userId)) != 0 || strncmp(account.userName2, uInfo.userId, strlen(uInfo.userId)) != 0){
      bytes = read(fd, &account, sizeof(account));
      count++;
  }

  struct flock lock;
  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = ((count) * sizeof(account));
  lock.l_len = sizeof(account);
  lock.l_pid = getpid();

  fcntl(fd, F_SETLKW, &lock);
  lseek(fd, ((count) * sizeof(account)), SEEK_SET);
  read(fd, &account, sizeof(account));

  char *tag="Normal Accounts Balance Details";
  sendMsgToClient(clientFD, tag);
  double val;
  val=account.availableAmount;
  write(clientFD, &val, sizeof(val));
  lock.l_type = F_UNLCK;
  fcntl(fd, F_SETLK, &lock);
  close(fd);
}
void JointPassword(int clientFD,struct userInfo uInfo){
  char *buff = NULL;
  sendMsgToClient(clientFD,"Enter your new password:");
  if(buff != NULL)
    free(buff);
    buff = receiveMsgFromClient(clientFD);
    struct userInfo account;
    long count;
    int fd = open("./db/joint_login_DB", O_RDWR);

    int currBytes = read(fd, &account, sizeof(account));
    count = 0;
    while(currBytes != 0 && strncmp(account.userId, uInfo.userId, strlen(uInfo.userId)) != 0){
        currBytes = read(fd, &account, sizeof(account));
        count++;
    }
   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   memcpy(account.pass,buff, sizeof(buff));
   lseek(fd, ((count) * sizeof(account)), SEEK_SET);
   write(fd, &account, sizeof(account));
   close(fd);
}

void processJointRequests(int clientFD, struct userInfo uInfo) {
  char *buff = NULL;
	sendMsgToClient(clientFD, "\n1.Deposit\n2.Withdraw\n3.Balance Enquiry\n4.Password Change\n5.Exit\nSelect option to contine with: ");
	while(1) {
		if(buff != NULL)
			free(buff);
		buff = receiveMsgFromClient(clientFD);
		  if(strcmp(buff, "1") == 0) {
        JointDeposit(clientFD,uInfo);
        break;
		} else if(strcmp(buff, "2") == 0) {
      JointWithdraw(clientFD,uInfo);
			break;
		}else if(strcmp(buff, "3") == 0) {
      JointBalEnq(clientFD,uInfo);
			break;
		}else if(strcmp(buff, "4") == 0) {
      JointPassword(clientFD,uInfo);
			break;
		}else if(strcmp(buff, "5") == 0) {
			break;
		} else {
			sendMsgToClient(clientFD, "Unknown Query");
		}
	}
}

void AddNormalAcc(int clientFD)
{ struct NAccount account;
	struct userInfo login;
  //Status and type
  memcpy(account.status,"true",4);
	memcpy(account.type, "NU", 2);
  memcpy(login.status,"true",4);
	memcpy(login.type, "NU", 2);
  char *asking = "AddNormalUser";
  sendMsgToClient(clientFD,asking);
  //Username
  asking = "\nName (This will also be your account login userName): ";
  sendMsgToClient(clientFD, asking);
	char *buff=NULL;
  if(buff != NULL)
		free(buff);
	buff = receiveMsgFromClient(clientFD);
  memcpy(account.userName, buff, sizeof(buff));
	memcpy(login.userId, buff, sizeof(buff));

  //password in userinfo
  if(asking != NULL)
		free(buff);
	sendMsgToClient(clientFD, "\nPassword:");
	if(buff != NULL)
		free(buff);
	buff = receiveMsgFromClient(clientFD);
	memcpy(login.pass,buff, sizeof(buff));
 //contactNo
	if(asking != NULL)
		free(buff);
	asking ="\nContact Number: ";
	sendMsgToClient(clientFD, asking);
	if(buff != NULL)
		free(buff);
	buff = receiveMsgFromClient(clientFD);
  memcpy(account.contactNo,buff, sizeof(buff));
  //account balance
	asking ="Enter the account opening amount:";
	sendMsgToClient(clientFD, asking);

  double amtval,size;
  read(clientFD, &size, sizeof(size));
  amtval=size;
  account.availableAmount=amtval;
	account.transactionsCount = 1;
  int count=account.transactionsCount;

	struct Transaction transaction;
	transaction.amount = account.availableAmount;
	memcpy(transaction.type, "Deposit", 7);

  generateTimeString();
  memcpy(transaction.time, timeofTransaction, sizeof(timeofTransaction));
  // memcpy(account.transactions[count-1].timeofTransaction, timeofTransaction, sizeof(timeofTransaction));
  account.transactions[count - 1] = transaction;

	int userLoginFd	 = open("./db/user_login_DB", O_CREAT|O_RDWR, 0644);
	int accountFd = open("./db/user_account_DB", O_CREAT|O_RDWR, 0644);
	int accountNumberFd = open("./db/accountNumber_DB", O_RDWR);
  struct flock lock;
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;
  lock.l_pid = getpid();

  //Lock accountNumber_DB
  fcntl(accountNumberFd, F_SETLKW, &lock);
	lseek(accountNumberFd, 0, SEEK_SET);

	unsigned int accountNumber;
	read(accountNumberFd, &accountNumber, sizeof(accountNumber));
  accountNumber=accountNumber+1;
  account.accNo = accountNumber;
	lseek(accountNumberFd, 0, SEEK_SET);
  write(accountNumberFd, &accountNumber, sizeof(accountNumber));
	lock.l_type = F_UNLCK;
	fcntl(accountNumberFd, F_SETLK, &lock);

	//Lock user_login_DB
	lock.l_whence = SEEK_END;
	lock.l_len = sizeof(login);
	fcntl(userLoginFd, F_SETLKW, &lock);

	lseek(userLoginFd, 0, SEEK_END);
	write(userLoginFd, &login, sizeof(login));
	lock.l_type = F_UNLCK;
	fcntl(userLoginFd, F_SETLK, &lock);

	//Lock account_DB
	lock.l_type = F_WRLCK;
	lock.l_len = sizeof(account);
	fcntl(accountFd, F_SETLKW, &lock);

	lseek(accountFd,0, SEEK_END);
	write(accountFd, &account, sizeof(account));
	lock.l_type = F_UNLCK;
	fcntl(accountFd, F_SETLK, &lock);



}
void AddJointAcc(int clientFD){
  struct JAccount account;
  	struct userInfo login1,login2;
    //Status and type
    memcpy(account.status,"true",4);
  	memcpy(account.type, "JA", 2);
    memcpy(login1.status,"true",4);
  	memcpy(login1.type, "JA", 2);
    memcpy(login2.status,"true",4);
  	memcpy(login2.type, "JA", 2);
    char *asking = "AddJointUser";
    sendMsgToClient(clientFD,asking);
    //Username 1
    asking = "\nUserName 1:(This will also be your account login userName): ";
    sendMsgToClient(clientFD, asking);
  	char *buff=NULL;
    if(buff != NULL)
  		free(buff);
  	buff = receiveMsgFromClient(clientFD);
    memcpy(account.userName1, buff, sizeof(buff));
  	memcpy(login1.userId, buff, sizeof(buff));

    //password in userinfo 1
    if(asking != NULL)
  		free(buff);
  	sendMsgToClient(clientFD, "\nPassword:");
  	if(buff != NULL)
  		free(buff);
  	buff = receiveMsgFromClient(clientFD);
  	memcpy(login1.pass,buff, sizeof(buff));

    //Username 2
    asking = "\nUserName 2:(This will also be your account login userName): ";
    sendMsgToClient(clientFD, asking);
    buff=NULL;
    if(buff != NULL)
      free(buff);
    buff = receiveMsgFromClient(clientFD);
    memcpy(account.userName2, buff, sizeof(buff));
    memcpy(login2.userId, buff, sizeof(buff));

    //password in userinfo 2
    if(asking != NULL)
      free(buff);
    sendMsgToClient(clientFD, "\nPassword:");
    if(buff != NULL)
      free(buff);
    buff = receiveMsgFromClient(clientFD);
    memcpy(login2.pass,buff, sizeof(buff));
   //contactNo
  	if(asking != NULL)
  		free(buff);
  	asking ="\nContact Number: ";
  	sendMsgToClient(clientFD, asking);
  	if(buff != NULL)
  		free(buff);
  	buff = receiveMsgFromClient(clientFD);
    memcpy(account.contactNo,buff, sizeof(buff));
    //account balance
  	asking ="Enter the account opening amount:";
  	sendMsgToClient(clientFD, asking);

    double amtval,size;
    read(clientFD, &size, sizeof(size));
    amtval=size;
    account.availableAmount=amtval;
  	account.transactionsCount = 1;


  	struct Transaction transaction;
  	transaction.amount = account.availableAmount;
  	memcpy(transaction.type, "Deposit", 7);


  	account.transactions[account.transactionsCount - 1] = transaction;

  	int userLoginFd	 = open("./db/joint_login_DB", O_CREAT|O_RDWR, 0644);
  	int accountFd = open("./db/joint_account_DB", O_CREAT|O_RDWR, 0644);
  	int accountNumberFd = open("./db/accountNumber_DB", O_RDWR);
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    //Lock accountNumber_DB
    fcntl(accountNumberFd, F_SETLKW, &lock);
  	lseek(accountNumberFd, 0, SEEK_SET);

  	unsigned int accountNumber;
  	read(accountNumberFd, &accountNumber, sizeof(accountNumber));
    accountNumber=accountNumber+1;
    account.accNo = accountNumber;
  	lseek(accountNumberFd, 0, SEEK_SET);
    write(accountNumberFd, &accountNumber, sizeof(accountNumber));
  	lock.l_type = F_UNLCK;
  	fcntl(accountNumberFd, F_SETLK, &lock);

  	//Lock user_login_DB  1
  	lock.l_whence = SEEK_END;
  	lock.l_len = sizeof(login1);
  	fcntl(userLoginFd, F_SETLKW, &lock);

  	lseek(userLoginFd, 0, SEEK_END);
  	write(userLoginFd, &login1, sizeof(login1));
  	lock.l_type = F_UNLCK;
  	fcntl(userLoginFd, F_SETLK, &lock);

    //Lock user_login_DB  2
  	lock.l_whence = SEEK_END;
  	lock.l_len = sizeof(login2);
  	fcntl(userLoginFd, F_SETLKW, &lock);

  	lseek(userLoginFd, 0, SEEK_END);
  	write(userLoginFd, &login2, sizeof(login2));
  	lock.l_type = F_UNLCK;
  	fcntl(userLoginFd, F_SETLK, &lock);

  	//Lock account_DB
  	lock.l_type = F_WRLCK;
  	lock.l_len = sizeof(account);
  	fcntl(accountFd, F_SETLKW, &lock);

  	lseek(accountFd,0, SEEK_END);
  	write(accountFd, &account, sizeof(account));
  	lock.l_type = F_UNLCK;
  	fcntl(accountFd, F_SETLK, &lock);

}
void AddUser(int clientFD)
{
    char *asking = "\nAdd User for:-\n1).Normal Account\n2).Joint Account";
  sendMsgToClient(clientFD, asking);
    char *buff=NULL;
		if(buff != NULL)
			free(buff);
		buff = receiveMsgFromClient(clientFD);
		if(strcmp(buff, "1") == 0) {
			AddNormalAcc(clientFD);
		} else if(strcmp(buff, "2") == 0) {
			AddJointAcc(clientFD);
		}else {
			sendMsgToClient(clientFD, "Unknown Request !!!!!");
		}

}
void SearchNormalAcc(int clientFD){
  char *accountName=NULL;
  char *asking = "\nEnter the UserNames that you want to Search: ";
  char *buff=NULL;
  sendMsgToClient(clientFD, asking);
//  if(buff != NULL)
  //  free(buff);
  if(accountName != NULL)
      free(accountName);
  accountName= receiveMsgFromClient(clientFD);

  struct NAccount account;
  long count = 0;                 //To keep track of account in file

  int fd = open("./db/user_account_DB", O_RDONLY);
  if(fd == -1){
    char *asking = "No accounts are present";
    sendMsgToClient(clientFD, asking); }

  int bytes = read(fd, &account, sizeof(account));
  while(bytes != 0 && strncmp(account.userName,accountName,sizeof(accountName)) != 0){
      bytes = read(fd, &account, sizeof(account));
      count++;
  }

  if(bytes == 0){
    char *asking = "Entered account number not present.";
    sendMsgToClient(clientFD, asking);
    close(fd);
  }

  struct flock lock;
  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = ((count) * sizeof(account));
  lock.l_len = sizeof(account);
  lock.l_pid = getpid();

  fcntl(fd, F_SETLKW, &lock);
  lseek(fd, ((count) * sizeof(account)), SEEK_SET);
  read(fd, &account, sizeof(account));

  char *tag="Normal Accounts Details";
  sendMsgToClient(clientFD, tag);
  asking ="\nCurrent Details of the selected account:";
  sendMsgToClient(clientFD, asking);
  sendMsgToClient(clientFD, account.userName);
  sendMsgToClient(clientFD, account.type);
  sendMsgToClient(clientFD, account.contactNo);
    double val;
    val=account.availableAmount;
  write(clientFD, &val, sizeof(val));

  /*printf(\nAvailable Amount: %lld\n\n",
              account.accountNumber,
              account.Name, account.SecondaryAccountHolderName,
              account.type, account.contactNo, account.availableAmount);
  */
  lock.l_type = F_UNLCK;
  fcntl(fd, F_SETLK, &lock);
  close(fd);

}
void SearchJointAcc(int clientFD)
{
  char *accountName=NULL;
  char *asking = "\nEnter the UserNames that you want to Search: ";
  char *buff=NULL;
  sendMsgToClient(clientFD, asking);
//  if(buff != NULL)
  //  free(buff);
  if(accountName != NULL)
      free(accountName);
  accountName= receiveMsgFromClient(clientFD);

  struct JAccount account;
  long count = 0;                 //To keep track of account in file

  int fd = open("./db/joint_account_DB", O_RDONLY);
  if(fd == -1){
    char *asking = "No accounts are present";
    sendMsgToClient(clientFD, asking); }

  int bytes = read(fd, &account, sizeof(account));
  while(bytes != 0 && strncmp(account.userName1,accountName,sizeof(accountName)) != 0 && strncmp(account.userName2,accountName,sizeof(accountName)) != 0){
      bytes = read(fd, &account, sizeof(account));
      count++;
  }

  if(bytes == 0){
    char *asking = "Entered account number not present.";
    sendMsgToClient(clientFD, asking);
    close(fd);
  }

  struct flock lock;
  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = ((count) * sizeof(account));
  lock.l_len = sizeof(account);
  lock.l_pid = getpid();

  fcntl(fd, F_SETLKW, &lock);
  lseek(fd, ((count) * sizeof(account)), SEEK_SET);
  read(fd, &account, sizeof(account));

  char *tag="Joint Accounts Details:";
  sendMsgToClient(clientFD, tag);
  asking ="\nCurrent Details of the selected account:";
  sendMsgToClient(clientFD, asking);
  sendMsgToClient(clientFD, account.userName1);
  sendMsgToClient(clientFD, account.userName2);
  sendMsgToClient(clientFD, account.type);
  sendMsgToClient(clientFD, account.contactNo);
    double val;
    val=account.availableAmount;
  write(clientFD, &val, sizeof(val));

  lock.l_type = F_UNLCK;
  fcntl(fd, F_SETLK, &lock);
  close(fd);
}
void SearchUser(int clientFD){
  char *asking = "\nSearch User for:-\n1).Normal Account\n2).Joint Account";
      char *buff=NULL;
      sendMsgToClient(clientFD, asking);
      if(buff != NULL)
  			free(buff);
  		buff = receiveMsgFromClient(clientFD);
  		if(strcmp(buff, "1") == 0) {
  			SearchNormalAcc(clientFD);
  		} else if(strcmp(buff, "2") == 0) {
  			SearchJointAcc(clientFD);
  		} else {
  			sendMsgToClient(clientFD, "Unknown Request !!!!!");
  		}
      if(buff != NULL)
      free(buff);
      buff = NULL;
}
void ModifyNormalAcc(int clientFD){
  char *accountName=NULL;
  char *asking = "\nEnter the UserName that you want to Modify: ";
  char *buff=NULL;
  sendMsgToClient(clientFD, asking);
  //  if(buff != NULL)
  //  free(buff);
  if(accountName != NULL)
      free(accountName);
  accountName= receiveMsgFromClient(clientFD);

  struct NAccount account;
  long count = 0;                 //To keep track of account in file

  int fd = open("./db/user_account_DB", O_RDONLY);
  if(fd == -1){
    char *asking = "No accounts are present";
    sendMsgToClient(clientFD, asking); }

  int bytes = read(fd, &account, sizeof(account));
  while(bytes != 0 && strncmp(account.userName,accountName,sizeof(accountName)) != 0 ){
      bytes = read(fd, &account, sizeof(account));
      count++;
  }

  if(bytes == 0){
    char *asking = "Entered account number not present.";
    sendMsgToClient(clientFD, asking);
    close(fd);
  }
  close(fd);

  fd = open("./db/user_account_DB", O_RDWR);
  struct flock lock;
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = ((count) * sizeof(account));
  lock.l_len = sizeof(account);
  lock.l_pid = getpid();

  fcntl(fd, F_SETLKW, &lock);
  lseek(fd, ((count) * sizeof(account)), SEEK_SET);
  read(fd, &account, sizeof(account));

  char oldName[50];
  strcpy(oldName, account.userName);

  char *tag="Normal Account Detial and Modify";
  sendMsgToClient(clientFD, tag);
  asking ="\nCurrent Details of the selected account:";
  sendMsgToClient(clientFD, asking);
  sendMsgToClient(clientFD, account.userName);
  sendMsgToClient(clientFD, account.type);
  sendMsgToClient(clientFD, account.contactNo);
  double val;
  val=account.availableAmount;
  write(clientFD, &val, sizeof(val));

  //tag="Normal Account Modify";
  //sendMsgToClient(clientFD, tag);
  tag="Enter the modified value in the respected field or Enter -1 for no change";
  sendMsgToClient(clientFD, tag);
  if(buff != NULL)
  free(buff);
  buff = receiveMsgFromClient(clientFD);
  if((strncmp("-1",buff, 2)) != 0){
      memcpy(account.userName, buff, sizeof(buff));
  }
  if(buff != NULL)
  free(buff);
  buff = receiveMsgFromClient(clientFD);
  if((strncmp("-1",buff, 2)) != 0){
      memcpy(account.contactNo, buff, sizeof(buff));
  }

  double amtval,size;
  read(clientFD, &size, sizeof(size));
  amtval=size;
  account.availableAmount=amtval;

  lseek(fd, ((count) * sizeof(account)), SEEK_SET);
  write(fd, &account, sizeof(account));
  lock.l_type = F_UNLCK;
  fcntl(fd, F_SETLK, &lock);
  close(fd);

  fd = open("./db/user_login_DB", O_RDWR);
  struct userInfo oldLogin;

  int currBytes = read(fd, &oldLogin, sizeof(oldLogin));
  count = 0;
  while(currBytes != 0 && strncmp(oldName, oldLogin.userId, strlen(oldName)) != 0){
      currBytes = read(fd, &oldLogin, sizeof(oldLogin));
      count++;
  }

  lseek(fd, ((count) * sizeof(oldLogin)), SEEK_SET);
  memcpy(oldLogin.userId, &account.userName, sizeof(account.userName));

  lseek(fd, ((count) * sizeof(oldLogin)), SEEK_SET);
  write(fd, &oldLogin, sizeof(oldLogin));
  close(fd);

}
void ModifyJointAcc(int clientFD){
  char *accountName=NULL;
  char *asking = "\nEnter the UserName that you want to Modify: ";
  char *buff=NULL;
  sendMsgToClient(clientFD, asking);
  //  if(buff != NULL)
  //  free(buff);
  if(accountName != NULL)
      free(accountName);
  accountName= receiveMsgFromClient(clientFD);

  struct JAccount account;
  long count = 0;                 //To keep track of account in file

  int fd = open("./db/joint_account_DB", O_RDONLY);
  if(fd == -1){
    char *asking = "No accounts are present";
    sendMsgToClient(clientFD, asking); }

  int bytes = read(fd, &account, sizeof(account));
  while(bytes != 0 && strncmp(account.userName1,accountName,sizeof(accountName)) != 0 && strncmp(account.userName2,accountName,sizeof(accountName)) != 0){
      bytes = read(fd, &account, sizeof(account));
      count++;
  }

  if(bytes == 0){
    char *asking = "Entered account number not present.";
    sendMsgToClient(clientFD, asking);
    close(fd);
  }

  struct flock lock;
  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = ((count) * sizeof(account));
  lock.l_len = sizeof(account);
  lock.l_pid = getpid();

  fcntl(fd, F_SETLKW, &lock);
  lseek(fd, ((count) * sizeof(account)), SEEK_SET);
  read(fd, &account, sizeof(account));

  char *tag="Joint Accounts Details";
  sendMsgToClient(clientFD, tag);
  asking ="\nCurrent Details of the selected account:";
  sendMsgToClient(clientFD, asking);
  sendMsgToClient(clientFD, account.userName1);
  sendMsgToClient(clientFD, account.userName2);
  sendMsgToClient(clientFD, account.type);
  sendMsgToClient(clientFD, account.contactNo);
  double val;
  val=account.availableAmount;
  write(clientFD, &val, sizeof(val));

  lock.l_type = F_UNLCK;
  fcntl(fd, F_SETLK, &lock);
  tag="Joint Account Modify";
  close(fd);

  char oldName1[50],oldName2[50];
  strcpy(oldName1, account.userName1);
  strcpy(oldName2, account.userName2);
  sendMsgToClient(clientFD, tag);

  if(buff != NULL)
  free(buff);
  buff = receiveMsgFromClient(clientFD);
  if((strncmp("-1",buff, 2)) != 0){
      memcpy(account.userName1, buff, sizeof(buff));
  }

  if(buff != NULL)
  free(buff);
  buff = receiveMsgFromClient(clientFD);
  if((strncmp("-1",buff, 2)) != 0){
      memcpy(account.userName2, buff, sizeof(buff));
  }
  if(buff != NULL)
  free(buff);
  buff = receiveMsgFromClient(clientFD);
  if((strncmp("-1",buff, 2)) != 0){
      memcpy(account.contactNo, buff, sizeof(buff));
  }
  double amtval,size;
  read(clientFD, &size, sizeof(size));
  amtval=size;
  account.availableAmount=amtval;

  lseek(fd, ((count) * sizeof(account)), SEEK_SET);
  write(fd, &account, sizeof(account));
  lock.l_type = F_UNLCK;
  fcntl(fd, F_SETLK, &lock);
  close(fd);

  fd = open("./db/joint_login_DB", O_RDWR);
  struct JAccount oldLogin;

  int currBytes = read(fd, &oldLogin, sizeof(oldLogin));
  count = 0;
  while(currBytes != 0 && strncmp(oldName1, oldLogin.userName1, strlen(oldName1)) != 0 && strncmp(oldName2, oldLogin.userName2, strlen(oldName2)) != 0){
      currBytes = read(fd, &oldLogin, sizeof(oldLogin));
      count++;
  }

  lseek(fd, ((count) * sizeof(oldLogin)), SEEK_SET);
  memcpy(oldLogin.userName1, &account.userName1, sizeof(account.userName1));
  memcpy(oldLogin.userName2, &account.userName2, sizeof(account.userName2));

  lseek(fd, ((count) * sizeof(oldLogin)), SEEK_SET);
  write(fd, &oldLogin, sizeof(oldLogin));
  close(fd);
}
void ModifyUser(int clientFD){
      char *asking = "\nSearch User for:-\n1).Normal Account\n2).Joint Account";
      char *buff=NULL;
      sendMsgToClient(clientFD, asking);
      if(buff != NULL)
  			free(buff);
  		buff = receiveMsgFromClient(clientFD);
  		if(strcmp(buff, "1") == 0) {
  			ModifyNormalAcc(clientFD);
  		} else if(strcmp(buff, "2") == 0) {
  			ModifyJointAcc(clientFD);
  		} else {
  			sendMsgToClient(clientFD, "Unknown Request !!!!!");
  		}
      if(buff != NULL)
      free(buff);
      buff = NULL;
}

void deletenormalrecord(char *accname){
  long count;
  int fd = open("./db/user_login_DB", O_RDWR);
  struct userInfo oldLogin;

  int currBytes = read(fd, &oldLogin, sizeof(oldLogin));
  count = 0;
  while(currBytes != 0 && strncmp(accname, oldLogin.userId, strlen(accname)) != 0){
      currBytes = read(fd, &oldLogin, sizeof(oldLogin));
      count++;
  }

  lseek(fd, ((count) * sizeof(oldLogin)), SEEK_SET);
  memcpy(oldLogin.status,"false", 5);
  lseek(fd, ((count) * sizeof(oldLogin)), SEEK_SET);
  write(fd, &oldLogin, sizeof(oldLogin));
  close(fd);
}
void DeleteNormalAcc(int clientFD){
  char *accountName=NULL;
  char *asking = "\nEnter the UserName that you want to Delete: ";
  char *buff=NULL;
  sendMsgToClient(clientFD, asking);
  //  if(buff != NULL)
  //  free(buff);
  if(accountName != NULL)
      free(accountName);
  accountName= receiveMsgFromClient(clientFD);
  int fd = open("./db/user_login_DB", O_RDONLY);
  //int fd = open(db, O_RDONLY);
  struct userInfo record;

	int currBytes = read(fd, &record, sizeof(record));
	while(currBytes != 0){

			if(strncmp(record.userId,accountName,sizeof(accountName)) == 0){
           if(strncmp(record.status,"true",4)==0)
				   	 { //printf("Logged-in successfully: ");
             char *ask = "\nDo you want to delete account?(Y/N)";
             sendMsgToClient(clientFD, ask);
             char *buffask=NULL;
             if(buffask != NULL)
                 free(buffask);
              buffask= receiveMsgFromClient(clientFD);
              if(strncmp(buffask,"Y",1)==0)
              {
                 deletenormalrecord(accountName);
                 char *ask = "\nDeleted successfully";
                 sendMsgToClient(clientFD, ask);
              }
              if(strncmp(buffask,"N",1)==0)
              { break;  }

             }
          }
			currBytes = read(fd, &record, sizeof(record));
      close(fd);
	}


}

void deletejointrecord1(char *accname1){
  long count;
  int fd = open("./db/joint_login_DB", O_RDWR);
  struct userInfo oldLogin;

  int currBytes = read(fd, &oldLogin, sizeof(oldLogin));
  count = 0;
  while(currBytes != 0 && strncmp(accname1, oldLogin.userId, strlen(accname1)) != 0){
      currBytes = read(fd, &oldLogin, sizeof(oldLogin));
      count++;
  }

  lseek(fd, ((count) * sizeof(oldLogin)), SEEK_SET);
  memcpy(oldLogin.status,"false", 5);
  lseek(fd, ((count) * sizeof(oldLogin)), SEEK_SET);
  write(fd, &oldLogin, sizeof(oldLogin));
  close(fd);
}
void deletejointrecord(char *accname1,char *accname2){
  deletejointrecord1(accname1);
  deletejointrecord1(accname2);
}

char* getaccname1(char *name){

  long count=0;
  struct JAccount account;
  int fd = open("./db/joint_account_DB", O_RDONLY);
  int bytes = read(fd, &account, sizeof(account));
  while(bytes != 0 && strncmp(account.userName1,name,sizeof(name)) != 0 || strncmp(account.userName2,name,sizeof(name)) != 0){
   bytes = read(fd, &account, sizeof(account));
   count++;
  }

  lseek(fd, ((count) * sizeof(account)), SEEK_SET);
  read(fd, &account, sizeof(account));
  name=account.userName1;
  close(fd);
  return name;

}
char* getaccname2(char *name){
  long count=0;
  struct JAccount account;
  int fd2 = open("./db/joint_account_DB", O_RDONLY);
  int bytes = read(fd2, &account, sizeof(account));
  while(bytes != 0 && strncmp(account.userName1,name,sizeof(name)) != 0 || strncmp(account.userName2,name,sizeof(name)) != 0){
   bytes = read(fd2, &account, sizeof(account));
   count++;
  }

  lseek(fd2, ((count) * sizeof(account)), SEEK_SET);
  read(fd2, &account, sizeof(account));
  name=account.userName2;
  close(fd2);
  return name;
}
void DeleteJointAcc(int clientFD){
  char *accountName=NULL;
  char *acc1;
  char *acc2;
  char *asking = "\nEnter the UserName that you want to Delete: ";
  char *buff=NULL;
  sendMsgToClient(clientFD, asking);
  //  if(buff != NULL)
  //  free(buff);
  if(accountName != NULL)
      free(accountName);
  accountName= receiveMsgFromClient(clientFD);
  int fd = open("./db/joint_login_DB", O_RDONLY);
  //int fd = open(db, O_RDONLY);
  struct userInfo record;

	int currBytes = read(fd, &record, sizeof(record));
	while(currBytes != 0){

			if(strncmp(record.userId,accountName,sizeof(accountName)) == 0){
           if(strncmp(record.status,"true",4)==0)
				   	 { //printf("Logged-in successfully: ");
             char *ask = "\nDo you want to delete account?(Y/N)";
             sendMsgToClient(clientFD, ask);
             char *buffask=NULL;
             if(buffask != NULL)
              free(buffask);
              buffask= receiveMsgFromClient(clientFD);

              if(strncmp(buffask,"Y",1)==0)  //////////////////////////////
               {

              acc1=getaccname1(accountName);
              acc2=getaccname2(accountName);
              deletejointrecord(acc1,acc2);
              char *ask = "Deleted successfully";
              sendMsgToClient(clientFD, ask);
              }
              if(strncmp(buffask,"N",1)==0)
              { break;  }

             }
          }
			currBytes = read(fd, &record, sizeof(record));
      close(fd);
	}
}
void DeleteUser(int clientFD){
  char *asking = "\nSearch User for:-\n1).Normal Account\n2).Joint Account";
  char *buff=NULL;
  sendMsgToClient(clientFD, asking);
  if(buff != NULL)
    free(buff);
  buff = receiveMsgFromClient(clientFD);
  if(strcmp(buff, "1") == 0) {
    DeleteNormalAcc(clientFD);
  } else if(strcmp(buff, "2") == 0) {
    DeleteJointAcc(clientFD);
  } else {
    sendMsgToClient(clientFD, "Unknown Request !!!!!");
  }
  if(buff != NULL)
  free(buff);
  buff = NULL;


}
void processAdminRequests(int clientFD) {
	char *askfromClient ="\n------------------------Welcome Admin------------------------\n\n1).Add\n2).Delete\n3).Modify\n4).Search\n5).Exit\nEnter the operation that you want to perform: ";
  sendMsgToClient(clientFD,askfromClient);

  char *buff=NULL;;
	//buff = receiveMsgFromClient(clientFD);
	while(1) {
		if(buff != NULL)
			free(buff);
		buff = receiveMsgFromClient(clientFD);
		if(strcmp(buff, "1") == 0) {
			AddUser(clientFD);
		} else if(strcmp(buff, "2") == 0) {
       DeleteUser(clientFD);
		}else if(strcmp(buff, "3") == 0) {
       ModifyUser(clientFD);
		}else if(strcmp(buff, "4") == 0) {
      SearchUser(clientFD);
		} else if(strcmp(buff, "5") == 0) {
			break;
		} else {
			sendMsgToClient(clientFD, "Unknown Request !!!!!");
		}
	}
	if(buff != NULL)
		free(buff);
	buff = NULL;
}

void processRequests(int uType, int clientFD, struct userInfo uInfo) {
	if(uType == UNAUTH_USER) {
		msg("Unautherized user.\n");
		closeWithMsg("unauth", clientFD);
	} else if(uType == USER) {
		msg("USER.\n");
		processUserRequests(clientFD, uInfo);
		closeWithMsg("Thanks User!", clientFD);
	} else if(uType == ADMIN) {
		msg("ADMIN.\n");
		processAdminRequests(clientFD);
		closeWithMsg("Thanks Admin!", clientFD);
	} else if(uType == JOINT) {
		msg("JOINT.\n");
		processJointRequests(clientFD, uInfo);
		closeWithMsg("Thanks U!", clientFD);
	}
}
int getlogintype(int clientFD) {
	char *asking = "\nPlease enter the type of login:\n1.Administrator Login\n2.Normal Account Login\n3.Joint Account Login\nSelec option to continue with";
	char *buff=NULL;

	//asking for logintype
	sendMsgToClient(clientFD, asking);
	//buff = receiveMsgFromClient(clientFD);
	int toRet = -1;
	int retry = 1;
	while(1) {
		if(buff!=NULL)
			free(buff);
		buff = receiveMsgFromClient(clientFD);
		if(strcmp(buff, "1") == 0) {
			toRet=1;
			break;
		} else if(strcmp(buff, "2") == 0) {
			toRet=2;
			break;
		} else if(strcmp(buff, "3") == 0) {
			toRet=3;
			break;
		} else {
			  if(retry == 3){
				printf("Terminating the connection after 3 unsuccessful retries\n");
				return -1;
	     	}
			sendMsgToClient(clientFD, "Invalid selection Please Enter valid number:");
		retry++;
	}
	}
	if(buff!=NULL)
		free(buff);
	buff=NULL;
	return toRet;
}

void talkToClient(int clientFD) {
	int logintype= getlogintype(clientFD);
	//printf("login as:%d\n",logintype);
  int uType;
	struct userInfo uInfo = getUserInfo(clientFD,logintype);
	if(logintype == 1){
		 	uType =authorizeUser(uInfo,logintype,"./db/admin_Login_DB");
	}
	else if(logintype == 2){
		  uType =	authorizeUser(uInfo,logintype,"./db/user_login_DB");
	}
	else if(logintype == 3){
			uType =authorizeUser(uInfo,logintype,"./db/joint_login_DB");
	}
	processRequests(uType, clientFD, uInfo);
}

int main(int argc, char **argv) {
	int sockFD, clientFD, portNO, cliSz;
	struct sockaddr_in serv_addr, cli_addr;

	if(argc < 2) {
		fprintf(stderr, "Usage: %s port_number\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/*
		socket(DOMAIN, TYPE, PROTOCOL) returns int
	*/
	if((sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("Error opening socket.\n");
	}

	//initializing variables
	memset((void*)&serv_addr, 0, sizeof(serv_addr));
	portNO = atoi(argv[1]);

	//setting serv_addr
	serv_addr.sin_family = AF_INET;				//setting DOMAIN
	serv_addr.sin_addr.s_addr = INADDR_ANY;		//permits any incoming IP
	/*
		Note: to permit a fixed IP:
		ret = inet_aton((char *)"a.b.c.d", &serv_addr.sin_addr);
		if(ret == 0)
			address is invalid
		else
			valid
	*/
	serv_addr.sin_port = htons(portNO);			//set the port number

	//binding the socket with the server logistics which are in sockaddr_in serv_addr
	if(bind(sockFD, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		error("Error on binding.\n");
	}

	//setting socket option to reuse the same port immmediately after closing socket
	//BUT with a caveat: Client should close first
	int reuse = 1;
	setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

	/*
		listen(SOCKETFD, BACKLOG) returns 0 on success -1 on failure
		backlog is the maximum number of connections the kernel should queue for this socket.
		The backlog argument provides an hint to the system of the number of outstanding connect
		requests that is should enqueue in behalf of the process. Once the queue is full, the
		system will reject additional connection requests.
	*/


	if(listen(sockFD, 7) < 0) {
		error("Error on listening.\n");
	}

	cliSz = sizeof(cli_addr);

	while(1) {
		//blocking call
		memset(&cli_addr, 0, sizeof(cli_addr));
		if((clientFD = accept(sockFD, (struct sockaddr*)&cli_addr, &cliSz)) < 0) {
			error("Error on accept.\n");
		}

		switch(fork()) {
			case -1:
				msg("Error in fork.\n");
				break;
			case 0: {
				close(sockFD);
				talkToClient(clientFD);
				exit(EXIT_SUCCESS);
				break;
			}
			default:
				close(clientFD);
				break;
		}
	}

	return 0;
}
