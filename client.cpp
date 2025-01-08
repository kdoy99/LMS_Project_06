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
    write(sock, name, strlen(name)); // 접속한 닉네임 정보 보내기
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
    cout << "DY톡에 오신걸 환영합니다 " << strtok(name, ">>") << "님!" << endl;
    cout << "명령어 확인 : /help" << endl;
    while (1)
    {
        fgets(msg, BUF_SIZE, stdin);
        if (!strncmp(msg, "/help", 5))
        {
            cout << "/quit : 채팅방을 종료합니다." << endl;
            cout << "/user : 현재 접속한 유저 정보를 확인합니다." << endl;
            cout << "/direct <username> <message> : 접속한 유저를 선택하여 다이렉트 메시지를 보낼 수 있습니다." << endl;
            cout << "/group <username>, ~ : 접속한 유저를 초대하여 단체 채팅방을 만들 수 있습니다." << endl;
        }
        else if (!strncmp(msg, "/quit", 5)) // 종료
        {
            write(sock, name, strlen(name));
            close(sock);
            exit(0);
        }
        else if (!strncmp(msg, "/user", 5)) // 조건 3. 유저 목록 확인
        {
            write(sock, msg, strlen(msg));
        }
        else if (!strncmp(msg, "/direct ", 8)) // 조건 4. 다이렉트 메시지
        {
            char * dm = msg + 8;
            strtok(dm, " ");
            string dm_content = strtok(NULL, "\n");
            sprintf(name_msg, "(DM)%s%s", name, dm_content.c_str());
            write(sock, name_msg, strlen(name_msg));
        }
        else if (!strncmp(msg, "/group ", 7)) // 조건 5. 단체 채팅방
        {
            
        }
        else if (!strcmp(msg, "shit\n")) // 조건 6. 비속어 검열 (임시)
        {
            strcpy(msg, "sxxt\n");
            sprintf(name_msg, "%s%s", name, msg);
            write(sock, name_msg, strlen(name_msg));
        }
        else
        {
            sprintf(name_msg, "%s%s", name, msg);
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