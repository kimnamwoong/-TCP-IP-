#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_size 1024                       //메세지 버퍼 크기 설정
#define NAMESIZE 100                        //이름 문자열 크기 설정
#define TAX 0.1

typedef struct Info                             //클라이언트 이름과 돈을 저장할 구조체
{
    char name[NAMESIZE];                    //이름
    int money;                              //돈
} BANK;

typedef struct Save
{
    BANK Mm[4];
    char name_message[BUF_size+NAMESIZE];
} Mem;                                      //서버로부터 받은 정보 저장 및 클라이언트에서 변경된 값을 보내줄 구조체

Mem data;                                   //서버로 보내줄 구조체 변수data 선언(포장지)
char name[NAMESIZE]="[Default]";            //클라이언트 이름을 저장할 변수
char message[BUF_size];                     //메세지를 입력할 변수

void* recv_message(void *arg);              //client가 받을 메세지(금액도 포함), 쓰레드 메인함수
void* send_message(void *arg);              //client에 보낼 메세지(금액도 포함), 쓰레드 메인함수
void error_hadling(char *message);          //오류 메세지 출력하는 함수

int main(int argc, char *argv[])
{
    int c_sock;                             //클라이언트 소켓
    struct sockaddr_in serv_addr;           //서버주소 정보 구조체
    pthread_t thread_1, thread_2;           //thread_1 :send thread, thread2 :recv thread
    void *thread_result;                    //쓰레드 리턴값
    
    //data구조체 변수 0으로 초기화
    memset(&data, 0, sizeof(Mem));

    if (argc != 4)                          //컴파일 할때 입력->0:기본 1,아이피 2.포트 3.이름
    { 
        printf("사용방법 : %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    sprintf(name, "%s" , argv[3]);                  //입력한 이름을 name배열에 저장하기!!
    c_sock = socket(AF_INET, SOCK_STREAM, 0);       //클라이언트 소켓 생성, 성공:파일디스크립터 실패:-1
    if (c_sock == -1)
        error_hadling("소켓 에러");
    
    memset(&serv_addr, 0, sizeof(serv_addr));       //주소 정보 0으로 초기화
    serv_addr.sin_family = AF_INET;                 //IPv4인 경우
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); //ip주소
    serv_addr.sin_port = htons(atoi(argv[2]));      //port번호
    
    //클라이언트 소켓부분에 서버를 연결!
    if (connect(c_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_hadling("연결 오류!!");
    }

    fflush(stdout);                                 //버퍼 비우기
    read(c_sock, &data, sizeof(Mem));               //서버로부터 클라이언트 정보 받기
    for (int i = 0; i < 4; i++) 
    {                                               //받은 정보에서 해당 클라이언트의 이름을 비교해서 정보 찾기
        if (!strcmp(data.Mm[i].name, name))         //리턴값: 두개의 문자열이 같으면 0, 다르면 1 or -1
        {
            printf("[이름]: %s ,[금액]: %d \n", data.Mm[i].name, data.Mm[i].money); 
        }
    }

    //각각 쓰레드를 생성, 각 쓰레드가 send_money, recv_money 함수를 실행
    pthread_create(&thread_1, NULL, send_message,(void*)c_sock);       
    pthread_create(&thread_2, NULL, recv_message,(void*)c_sock);       
    
    //각각의 쓰레드가 끝날때까지 기다려준다
    pthread_join(thread_1, &thread_result);
    pthread_join(thread_2, &thread_result);

    //통신 후 소켓 클로우즈 종료
    close(c_sock);
    return 0;
}

void* send_message(void* arg)
{
    int sock = (int)arg;                            //소켓
    int input_money=0;                              //입력할 금액
    char target[NAMESIZE] = "[Default]";            //돈을 보낼 대상
    
    printf("선택메뉴->[q]종료,[1]입금,[2]잔고 확인,[3]명목비,[4]월급주기\n");
    while(1)
    {
        fgets(message,BUF_size,stdin);              //전역 변수 message를 입력
        
        //1번 입력시 입금메뉴
        if (strcmp(message, "1\n") == 0)
        {
            printf("보내는 [사람]과 [금액]을 차례대로 입력하세요: \n");
            scanf("%s %d", target, &input_money);
            for (int i = 0; i < 4; i++)
            {
                if (!strcmp(data.Mm[i].name, name))
                {
                    data.Mm[i].money = data.Mm[i].money - input_money;
                    printf("입금 후 잔액: %d \n", data.Mm[i].money);
                    for (int i = 0; i < 4; i++)
                    {
                        if (!strcmp(data.Mm[i].name, target))           //입금할 대상을 찾기 위한 조건문
                        {
                            data.Mm[i].money = data.Mm[i].money + input_money;
                            printf("[보낸사람]: %s [금액]: %d \n",data.Mm[i].name,data.Mm[i].money); 
                            break;                                  //입금 대상을 찾으면 반복문을 빠져나간다.
                        }
                    }
                }
            }
        }
        //2번 잔액 확인하는 메뉴
        else if (strcmp(message, "2\n") == 0) 
        {
            for (int i = 0; i < 4; i++)
            {
                if (!strcmp(data.Mm[i].name, name))
                {
                    printf("[이름]: %s [돈]: %d\n", data.Mm[i].name, data.Mm[i].money);
                    break; 
                }
            }
        }
        //돈 뺏기 메뉴
        else if (strcmp(message, "3\n") == 0)
        {
            printf("[명목비]을 걷습니다!! 금액을 입력하세요: ");
            scanf("%d", &input_money);
            for (int i = 0; i < 4; i++)
            {
                if (strcmp(data.Mm[i].name, name) != 0) //돈을 뺏을 대상 찾기 위한 조건문
                {
                    data.Mm[i].money = data.Mm[i].money - input_money;
                }
            }
            printf("[총 금액]: %d\n", input_money * 3);
        }
        // 월급 주기 메뉴
        else if (strcmp(message, "4\n") == 0)
        {
            printf("월급을 입력하세요. : ");
            scanf("%d", &input_money);
            for (int i = 0; i < 4; i++)
            {
                if (strcmp(data.Mm[i].name, name) != 0)
                {
                    data.Mm[i].money = data.Mm[i].money + input_money * 0.1;
                }
            }
        }
        else if(!strcmp(message,"q\n"))      //q를 누르면 종료 
        {
            close(sock);
            exit(0);
        }
        //메세지를 입력했을 경우
        else
        {
            sprintf(data.name_message, "%s %s",name,message);
            write(sock, &data, sizeof(Mem));
        }
    }
}

void* recv_message(void *arg)
{
    int sock = (int)arg;    //소켓
    int str_len;            //문자 길이 

    while(1)
    {
        str_len = read(sock, &data, sizeof(Mem));   //서버로부터 받은 데이터를 구조체에 저장
        if(str_len == -1) 
        {
            return 1;
        }
        data.name_message[str_len]=0;
        fputs(data.name_message,stdout);            //데이터를 담은 구조체 맴버중 name_message만 출력
    }
}

void error_hadling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}