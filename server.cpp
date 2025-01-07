#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

using namespace std;

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(string msg);

int clnt_cnt = 0; // 접속한 클라이언트 개수
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

// class List
// {
//     private:
//         int i=0;
//         char name_list[10000];
//         char* total_list[1];
//     public:
//         char* userList(char* list[]);
// };

// char* List::userList(char* list[])
// {
//     for (i = 0; i <= clnt_cnt; i++)
//     {
//         strcat(name_list, list[i]);
//         strcat(name_list, "\n");
//     }
//     total_list[0] = name_list ;
//     return (char*)total_list[0];
// }


int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    pthread_t t_id;
    
    if (argc!=2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    
    pthread_mutex_init(&mutx, NULL);
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);

    // 디폴트 설정
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
    {
        error_handling("bind() error");
    }
    if (listen(serv_sock, 5)==-1)
    {
        error_handling("listen() error");
    }
    
    // 가변 설정
    while (1)
    {
        clnt_adr_sz=sizeof(clnt_adr);
        clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++]=clnt_sock;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);
        printf("Coneected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
    }
    close(serv_sock);
    return 0;
}

void * handle_clnt(void * arg)
{
    int clnt_sock=*((int*)arg);
    int str_len=0, i;
    char msg[BUF_SIZE];
    char* list[MAX_CLNT]; // 접속한 유저 명단
    char name[100]; // 접속한 유저 이름 받아올 곳
    int name_len = 0; // 접속한 유저 이름 길이
    // List user; // 유저 관련 클래스
    string name_list = "[접속한 유저]\n";

    name_len = read(clnt_sock, name, sizeof(name)); // 유저 이름 받아옴
    list[clnt_cnt] = name; // 받아온 유저 이름 리스트에 집어넣기
    
    for (int j = 0; j <= clnt_cnt; j++)
    {
        name_list = name_list + list[j] + "\n";
    }
    

    cout << name_list << endl;

    while ((str_len=read(clnt_sock, msg, sizeof(msg)))!=0)
    {
        if (!strcmp(msg, "/user"))
        {
            // send_msg(user.userList(list), strlen(user.userList(list)));
        }
        else
        {
            send_msg(msg, str_len);
        }
    }
    pthread_mutex_lock(&mutx); // 접속 끊었을 때 동작
    for (i = 0; i < clnt_cnt; i++) // 접속 끊은 클라이언트가 있으면 그 자리 다음 순번이 대체하는 로직
    {
        if (clnt_sock==clnt_socks[i])
        {
            while (i++<clnt_cnt-1) // 클라 2개 이상일 때만 작동
            {
                clnt_socks[i]=clnt_socks[i+1]; // 빈 자리에 들어감
                list[i] = list[i+1]; // 접속 유저 이름도 옮겨줌
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}

void send_msg(char * msg, int len)
{
    int i;
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++)
    {
        write(clnt_socks[i], msg, len);
    }
    pthread_mutex_unlock(&mutx);
}

void error_handling(string msg)
{
    cout << msg << endl;
	exit(1);
}