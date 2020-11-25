#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "server.h"

using namespace std;

#define SERVERPORT 9000
#define BUFSIZE    512000


#define MIN(a,b) a>b?b:a

Server_Player server_player;

bool Client[MAX_Client] = { false }; //클라이언트가 들어오는 대로 처리한다.
void send_Player(SOCKET, Server_Player);
DWORD WINAPI ProcessClient(LPVOID);
//Client_Player recv_Player(SOCKET sock);

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;
    int sum = 0;
    float size = len;

    while (left > 0) {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0) {
            cout << "전송 완료\n";
            break;
        }
        left -= received;
        ptr += received;
        sum += received;
    }

    return (len - left);
}


int no = 0;
int val[10] = { 0, };


DWORD WINAPI ProcessClient(LPVOID arg) {

    char* buf = new char[BUFSIZE + 1];

    int m_no=0;
    int threadId = GetCurrentThreadId();
    printf("스레드 생성 : %d\n", threadId);

    SOCKET client_sock = (SOCKET)arg;
    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen;

    // 클라이언트 정보
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

    for (int i = 0; i < MAX_Client; ++i) {
        if (Client[i] == false) {
            Client[i] = true;
            m_no = i;
            printf("%d번째 클라이언트 입니다", i+1);
            break;
        }
    }

    // 인원체크
    if (m_no == 2) {
        closesocket(client_sock);
        printf("클라이언트 종료: IP 주소=%s, 포트 번호=%d [인원 초과]\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        return 0;
    }

    while (1) {

        int retval;
        int buf;
        int GetSize;

        retval = recvn(client_sock, (char*)&buf, sizeof(int), 0); // 데이터 받기(고정 길이)
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
        }
        else if (retval == 0) {

        }

        char Buffer[BUFSIZE];
        Client_Player* player;
        GetSize = recv(client_sock, Buffer, buf, 0);
        if (GetSize == SOCKET_ERROR) {
            MessageBox(NULL, "error", "연결이 끊어졌습니다", 0);
            exit(1);
        }

        Buffer[GetSize] = '\0'; // 마지막 버퍼 비워줌
        player = (Client_Player*)Buffer;


        server_player.player[m_no].posX = player->player_socket.posX;
        server_player.player[m_no].posY = player->player_socket.posY;
        server_player.player[m_no].hp = player->player_socket.hp;
        server_player.player[m_no].isAttack = player->player_socket.isAttack;
        server_player.player[m_no].live = player->player_socket.live;
      

        //printf( "Xrotate : %f\n", server_data.player[0].camxrotate );

        //int retval;
        // 데이터 보내기( 구조체 크기를 먼저 보낸다. )

        send_Player(client_sock, server_player);

    }

    // closesocket()
    closesocket(client_sock);
    printf("스레드 종료 : %d\n[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n\n", threadId,
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
    delete[] buf;
    return 0;
}

int main(int argc, char* argv[])
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");


    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    HANDLE hThread;

    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // 접속한 클라이언트 정보 출력
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        hThread = CreateThread(NULL, 0, ProcessClient,
            (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) { closesocket(client_sock); }
        else { CloseHandle(hThread); }
    }

    // closesocket()
    closesocket(listen_sock);

    // 윈속 종료
    WSACleanup();

    return 0;
}

void send_Player(SOCKET sock, Server_Player player) {
    int retval;

    // 데이터 보내기( 구조체 크기를 먼저 보낸다. )
    int buf = sizeof(player);
    retval = send(sock, (char*)&buf, sizeof(int), 0);
    if (retval == SOCKET_ERROR) {
        err_display("send()");
        exit(1);
    }

    // 데이터 보내기( 구조체 데이터를 보낸다. )
    retval = send(sock, (char*)&player, sizeof(Server_Player), 0);
    if (retval == SOCKET_ERROR) {
        err_display("send()");
        exit(1);
    }
}

/*
Client_Player recv_Player(SOCKET sock) {
    int retval;
    int buf;
    int GetSize;

    retval = recvn(sock, (char*)&buf, sizeof(int), 0); // 데이터 받기(고정 길이)
    if (retval == SOCKET_ERROR) {
        err_display("recv()");
    }
    else if (retval == 0) {

    }

    char Buffer[BUFSIZE];
    Client_Player* player;
    GetSize = recv(sock, Buffer, buf, 0);
    if (GetSize == SOCKET_ERROR) {
        MessageBox(NULL, "error", "연결이 끊어졌습니다", 0);
        exit(1);
    }

    Buffer[GetSize] = '\0'; // 마지막 버퍼 비워줌
    player = (Client_Player*)Buffer;

    return *player;
}
*/



/*
전역변수로 스레드 생성시 변수 증가, 그 변수를 인덱스로 배열에 플레이어 구조체 저장, 
저장된 구조체 교환해서 전송 


차후 충돌체크?

*/