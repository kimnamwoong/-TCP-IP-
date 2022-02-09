# -TCP-IP-
C언어로 구현한 TCP/IP 통신 프로그램 



1. 프로젝트명: 회사 회계 프로그램


2. 프로젝트 기간 :
2020.11.16. ~ 2020.11.22


3. 개발환경
- 서버 : Linux Visual Studio Code/ C
- 클라이언트 : Linux Visual Studio Code/ C


4. 프로그램 소개
회사 회계프로그램을 모티브로 하여 다수 클라이언트가 접속하여 채팅,송금,월급 지급 등이 가능한 프로그램

5. 맡은 파트
- 서버 클라이언트 통신 연결 및 client thread 구현


6. 주요 기능

1)  채팅기능
- 다중 클라이언트 접속 및 채팅
2)  송금 기능
- 1번 : 입력한 상대에게 송금기능 
- 2번 : 각 클라이언트 잔고 확인
- 3번 : 모든 클라이언트에게 명목비를 걷기.(모든 클라이언트에게 지출 발생)


구조도

![image](https://user-images.githubusercontent.com/94125986/153133405-5221da4c-3b4b-4cd9-9e9c-5b65993ce81b.png)



서버 : 클라이언트 접속화면

![image](https://user-images.githubusercontent.com/94125986/153133471-685d0001-f4a9-4503-9af8-f27a5a8392ad.png)


입금 대상 및 금액 입력

![image](https://user-images.githubusercontent.com/94125986/153133525-d66718b2-2cca-4e45-8c83-468dd73edf1d.png)

모든 클라이언트에게 지출 발생

![image](https://user-images.githubusercontent.com/94125986/153133575-8838696d-83f4-4153-9f8e-63e0fd3c027e.png)



10% 세율을 계산해서 모든 클라이언트에게 월급지급


![image](https://user-images.githubusercontent.com/94125986/153133611-bc2d9132-553b-417b-812a-3915c4acdaa9.png)



