#include <stdio.h>
#include <stdlib.h>    //atoi를 사용하려면 있어야함
#include <string.h>    // memset 등
#include <unistd.h>    //sockaddr_in, read, write 등
#include <arpa/inet.h> //htnol, htons, INADDR_ANY, sockaddr_in 등
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFSIZE 1024            //서버에 표시될 메세지 크기
#define NAMESIZE 100            //이름 문자열 크기 설정

//클라이언트와 서버간 데이터 송수신할때 저장할 구조체(클라이언트 이름,돈이 모두 저장되어있음)
typedef struct                  
{
    char name[NAMESIZE];        //이름
    int money;                  //돈
} BANK;

typedef struct                  //BANK구조체의 맴버를 저장해줄 구조체!! 클라이언트 송수신 구조체 변수 선언(포장지 개념)
{
    BANK Mm[4];
    char name_message[NAMESIZE+BUFSIZE];
} Mem;                      

Mem data;           
int c_num = 0;              //현재 접속중인 클라이언트 수
int c_sockets[10];          //접속한 클라이언트 소켓 저장(메모리 저장 공간)
pthread_mutex_t mutex;      //뮤텍스 구조체 선언

void *c_connection(void *arg);             //새로운 클라이언트를 연결 처리해주는 함수
void send_message(int len);                 //서버에 연결되어 있는 모든 소켓을 통해 메세지를 전송해주는 함수
void error_handling(char *message);        //error 발생시 에러 부분 메세지 출력 함수


int main(int argc, char *argv[])    //argc=명령어라인의 문자열 개수, argv=명령어라인의 문자열
{                                   //실행방법 : ./client 127.0.0.1 9190(실행파일명, ip주소, 포트번호)
    int s_socket;                   //서버 소켓
    int c_socket;                   //클라이언트 소켓
    void *thread_result;

    //data 구조체 데이터 값 초기화
    data.Mm[0].money = 10000;
    data.Mm[1].money = 20000;
    data.Mm[2].money = 30000;
    data.Mm[3].money = 40000;
    strcpy(data.Mm[0].name, "남웅");
    strcpy(data.Mm[1].name, "민영");
    strcpy(data.Mm[2].name, "혜나");
    strcpy(data.Mm[3].name, "재현");  
    
    //sockaddr_in 구조체는 소켓 주소와 틀을 형성해주는 구조체로, AF_INET일 경우 사용
    struct sockaddr_in s_addr;      //서버 주소
    struct sockaddr_in c_addr;      //클라이언트 주소

    pthread_t send_thread, Human;   //생성된 쓰레드의 ID를 저장할 변수를 포인터 인자로 전달

    int c_addr_size;                //클라이언트 주소 사이즈

    if (argc != 2)                  //명령어라인의 문자열 개수(0=기본, 1=PORT번호)
    {
        printf("How to use : %s [port]\n", argv[0]);
        return -1;
    }

    if (pthread_mutex_init(&mutex, NULL))       //뮤텍스 초기화
    {
        error_handling("mutex() error");
    }

    //TCP 연결지향형이고 ipv4 도메인을 위한 소켓생성
    s_socket = socket(PF_INET, SOCK_STREAM, 0);         //(PR_INET=ipv4도메인, SOCK_STREAM=TCP 프로토콜 전송방식, 0=프로토콜)

    //주소를 초기화한 후 IP주소와 포트지정(서버의 소켓 내부 초기화)
    memset(&s_addr, 0, sizeof(s_addr));             //연결 요청을 수신할 주소 설정
    s_addr.sin_family = AF_INET;                    //타입 : ipv4
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);     //ip주소
    s_addr.sin_port = htons(atoi(argv[1]));         //PORT번호

    //서버소켓과 서버주소를 바인딩(소켓에 서버소켓에 필요한 정보를 할당하고 커널에 등록함)
    if (bind(s_socket, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1)   //소켓을 포트에 연결(서버소켓, 서버주소 포인터, 서버주소 크기)
    {
        error_handling("binding() error");  //성공=0, 실패=-1
    }

    //커널에 개통 요청
    if (listen(s_socket, 5) == -1) //연결 대기(서버소켓, 연결 대기열 5개 생성)
    {
        error_handling("listening() error");    //성공=0, 실패=-1
    }

    while (1) //ip의 길이가 다르므로 사이즈를 정함(ipv4=32비트, ipv6=64비트)
    {
        c_addr_size = sizeof(c_addr);                                          //클라이언트 연결 요청 수신(클라이언트 주소 크기)
        c_socket = accept(s_socket, (struct sockaddr *)&c_addr, &c_addr_size); //크라이언트 소켓=accept(서버소켓, 클라이언트 주소 포인터, 클라이언트 주소 크기)
                                                                               
        write(c_socket, &data, sizeof(Mem));                                    //클라이언트에게 서버에 저장된 클라이언트 정보 모두 보내주기
        pthread_mutex_lock(&mutex);                                              //닫기(임계영역에 해당하기 때문에 뮤텍스 동기화)
        c_sockets[c_num++] = c_socket;                                           //클라이언트 소켓(클라이언트 수++), 생성된 소켓의 정보를 저장하기 위해 전역으로 선언된 변수와 배열에 접근
        pthread_mutex_unlock(&mutex);                                            //열기
        pthread_create(&send_thread, NULL, c_connection, (void *)c_socket);      //새로 생성된 소켓의 입출력을 담당할 쓰레드 생성
        printf("새로운 연결, 클라이언트 IP : %s\n", inet_ntoa(c_addr.sin_addr)); //접속된 클라이언트 ip주소 출력
    }

    return 0;
}

//클라이언트 연결 함수(소켓을 들고 클라이언트와 통신하는 함수)
void *c_connection(void *arg) 
{
    int c_socket = (int)arg;        //클라이언트 소켓(쓰레드가 통신할 클라이언트)
    int str_len = 0;
    int i;

    while ((str_len = read(c_socket, &data, sizeof(Mem))) != 0) //받은 데이터를 클라이언트 소켓을 통해 다시 돌려줌
    {
        send_message(str_len);                          //읽어들인 데이터 전송(메세지, 문자열 길이)
    }

    pthread_mutex_lock(&mutex);     //소켓이 종료되었을 때 전체소켓의 정보에서 종료된 소켓의 정보를 제외시키는 기능을 하는 영역

    for (i = 0; i < c_num; i++)
    {
        if (c_socket == c_sockets[i]) //종료된 클라이언트 파일 디스크립터를 빼는 작업
        {
            for (; i < c_num - 1; i++)
            {
                c_sockets[i] = c_sockets[i + 1];
                break;
            }
        }
    }
    c_num--;
    pthread_mutex_unlock(&mutex);
    close(c_socket);
    return 0;
}

void send_message(int len)
{
    int i;
    pthread_mutex_lock(&mutex);         //다른 쓰레드가 접근하지 못하도록 lock을 걸어줌

    for (i = 0; i < c_num; i++)         //클라이언트 수만큼 같은 메세지 전달
    {
        write(c_sockets[i], &data, len);
    }

    pthread_mutex_unlock(&mutex);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1); //에러 발생시 종료
}