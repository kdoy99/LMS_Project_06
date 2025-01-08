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
char * user_list[MAX_CLNT]; // 접속한 유저 명단
pthread_mutex_t mutx;

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
    char name[100]; // 접속한 유저 이름 받아올 곳
    int name_len = 0; // 접속한 유저 이름 길이

    // 유저 이름 읽어오기
    name_len = read(clnt_sock, name, sizeof(name)); // 유저 이름 받아옴
    name[name_len] = '\0';

    if (clnt_cnt > 0) // 클라이언트가 최소 하나라도 접속 중일때
    {
        user_list[clnt_cnt - 1] = strtok(name, ">>"); // 받아온 유저 이름 리스트에 집어넣기
        pthread_mutex_lock(&mutx);
        user_list[clnt_cnt - 1] = new char[strlen(name) + 1];
        strcpy(user_list[clnt_cnt - 1], name);
        pthread_mutex_unlock(&mutx);
    }
    else // 접속중인 클라이언트 X
    {
        cout << "현재 접속중인 클라이언트 없음" << endl;
    }

    while ((str_len=read(clnt_sock, msg, sizeof(msg)))!=0)
    {
        msg[str_len] = '\0';
        // /quit 명령어, 채팅창 종료 및 모두에게 종료 메시지 전송
        if (strstr(msg, "/quit") != NULL)
        {
            char quit_message[BUF_SIZE] = {0};
            // 접속 종료한 유저의 정보 출력
            pthread_mutex_lock(&mutx);
            char* quit_user = strtok(msg, ">>"); // 접속 종료한 유저 이름 따오기
            snprintf(quit_message, sizeof(quit_message), "%s 님이 퇴장하셨습니다.\n", quit_user);
            pthread_mutex_unlock(&mutx);

            // 접속 종료한 유저의 소켓 압수 및 재정렬
            pthread_mutex_lock(&mutx);
            for (i = 0; i < clnt_cnt; i++)
            {
                if (clnt_socks[i] == clnt_sock) // 소켓 리스트에서 소켓 찾기
                {
                    while (i < clnt_cnt)
                    {
                        clnt_socks[i] = clnt_socks[i+1];
                        user_list[i] = user_list[i+1];
                        i++;
                    }
                    break;
                }
            }
            clnt_cnt--;
            pthread_mutex_unlock(&mutx);
            send_msg(quit_message, strlen(quit_message));
        }
        // /user 라는 메세지 받으면 유저 명단수 보내주는 조건문
        else if (strstr(msg, "/user") != NULL)
        {
            string name_list = "[접속한 유저]\n";
            pthread_mutex_lock(&mutx);
            for (i = 0; i < clnt_cnt; i++) // 클라이언트 수에 따라 반복하고, string에 char*형 배열에 담긴걸 합쳐서 보냄
            {
                name_list += user_list[i];
                name_list += "\n";
            }
            pthread_mutex_unlock(&mutx);
            write(clnt_sock, name_list.c_str(), name_list.length());
        }
        else
        {
            send_msg(msg, str_len);
        }
    }
    delete[] user_list[clnt_cnt];
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