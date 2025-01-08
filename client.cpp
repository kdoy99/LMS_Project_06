#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20
#define MAX_CLNT 256

using namespace std;

void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(string msg);

char name[NAME_SIZE]="[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_adr;
    pthread_t snd_thread, rcv_thread;
    void * thread_return;
    if (argc!=4)
    {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]); // argv[1] = <IP>, argv[2] = <port>, argv[3] = <name>
        exit(1);
    }
    
    sprintf(name, "[%s]>>", argv[3]);
    sock=socket(PF_INET, SOCK_STREAM, 0);
    
    // 디폴트 설정
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_adr.sin_port=htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
    {
        error_handling("connect() error");
    }
    
    // 가변 설정
    write(sock, argv[3], strlen(argv[3])); // 접속한 닉네임 정보 보내기
    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);
    return 0;
}

void * send_msg(void * arg)
{
    int sock=*((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    
    while (1)
    {
        fgets(msg, BUF_SIZE, stdin);
        if (!strcmp(msg, "/quit\n"))
        {
            sprintf(name_msg, "%s %s", name, msg);
            write(sock, name_msg, strlen(name_msg));
            sleep(3); // 바로 종료 x
            close(sock);
            exit(0);
        }
        else if (!strcmp(msg, "shit\n"))
        {
            strcpy(msg, "sxxt\n");
            sprintf(name_msg, "%s %s", name, msg);
            write(sock, name_msg, strlen(name_msg));
        }
        else
        {
            sprintf(name_msg, "%s %s", name, msg);
            write(sock, name_msg, strlen(name_msg));
        }
    }
    return NULL;
}

void * recv_msg(void * arg)
{
    int sock=*((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    int str_len;
    while (1)
    {
        str_len = read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
        if (str_len==-1)
        {
            return (void*)-1;
        }
        name_msg[str_len]=0;
        fputs(name_msg, stdout);
    }
    return NULL;
}

void error_handling(string msg)
{
    cout << msg << endl;
	exit(1);
}