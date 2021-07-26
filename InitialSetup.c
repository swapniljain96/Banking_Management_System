#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
struct userInfo{
	char status[10];
  char type[4];
	char userID[50];
	char pass[50];
};
int main(){

    printf("Initializing DB...\n\n");
    int fd = open("./db/admin_Login_DB", O_CREAT|O_WRONLY|O_TRUNC, 0644);
    int fdAccountNumber = open("./db/accountNumber_DB", O_CREAT|O_WRONLY|O_TRUNC, 0644);

    struct userInfo adminLogin;
    unsigned int accountNumber = 0;

		memcpy(adminLogin.status, "true", 4);
    memcpy(adminLogin.type, "AD", 2);
    memcpy(adminLogin.userID, "admin", 5);
    memcpy(adminLogin.pass, "root", 4);

    write(fd, &adminLogin, sizeof(adminLogin));
    write(fdAccountNumber, &accountNumber, sizeof(accountNumber));

    close(fd);
    close(fdAccountNumber);

    printf("Checking the initialization of DB...\n");
    struct userInfo test;
    long accountNumberTest;
    fd = open("./db/admin_Login_DB", O_RDONLY);
    fdAccountNumber = open("./db/accountNumber_DB", O_RDONLY);

    read(fd, &test, sizeof(test));
    read(fdAccountNumber, &accountNumberTest, sizeof(accountNumberTest));

    printf("\nStatus: %s\nType: %s\nUserName: %s\nPassword: %s\n",test.status,test.type, test.userID, test.pass);
    printf("Account number: %ld\n", accountNumberTest);

    close(fd);
    close(fdAccountNumber);

    return 0;
}
