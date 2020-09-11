#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <pthread.h>
#include <ctype.h>
#include <stdio_ext.h>

#define BUF_SIZE 100	// 버퍼 사이즈
#define NAME_SIZE 20	// 클라이언트 접속 이름 사이즈

void * handle_clnt(void * arg);		// 클라이언트 동기화 함수
void send_msg(char * msg, int len);	// 메시지 전달
void error_handling(char * msg);	// 에러 처리

int clnt_cnt = 0;					// 접속된 클라이언트 수 
int clnt_socks[2] = {0,0};			// 접속된 클라이언트 주소 정보 배열

pthread_mutex_t mutx;

// 해당 문자열의 숫자를 분리해서 저장하여 정수로 반환하는 함수
int search_answer(char temp[])
{
	int return_answer = 0;
	char str1[100];
	char str2[100];
	char* pstr1 = str1;
	char* pstr2 = str2;

	strcpy(str1, temp);
	memset(str2, 0, sizeof str2);

	// memset(void* dest, int c, size_t count) dest는 버퍼의 주소 c는 초기화 할 값.
	// count는 초기화할 버퍼의 크기
	while (*pstr1)
	{
		if (isdigit(*pstr1))  //isdigit(int c) c의 값이 숫자이면 참값, 거짓이면 0
			* pstr2++ = *pstr1;
		pstr1++;
	}

	return_answer = atoi(str2);
	return return_answer;
}

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;

	pthread_t t_id;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	pthread_mutex_init(&mutx, NULL);	// 동기화 시작

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");

	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	while (1)
	{
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);

		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}

	close(serv_sock);
	return 0;
}

void * handle_clnt(void * arg) {
	int clnt_sock = *((int*)arg);
	int str_len = 0;
	char msg[BUF_SIZE];

	while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0) 
		send_msg(msg, str_len);

	pthread_mutex_lock(&mutx);

	for (int i = 0; i < clnt_cnt; i++) // remove disconnected client
	{
		if (clnt_sock == clnt_socks[i])
		{
			while (i++ < clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i + 1];
			break;
		}
	}
	clnt_cnt--;

	pthread_mutex_unlock(&mutx);
	close(clnt_sock);

	return NULL;
}
void send_msg(char * msg, int len) // send to all 
{
	int j = 0;
	pthread_mutex_lock(&mutx);

	for (int i = 0; i < clnt_cnt; i++) {
		write(clnt_socks[i], msg, len);
	}

	pthread_mutex_unlock(&mutx);
}
	
void error_handling(char * msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
