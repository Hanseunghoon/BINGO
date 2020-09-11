//클라

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <pthread.h>
#include <ctype.h>
#include <stdio_ext.h>

#define BUF_SIZE 100 
#define NAME_SIZE 20
#define BOARD_SIZE 4

void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];
int wrong_c1[BUF_SIZE];
int wrong_c2[BUF_SIZE];
int dump[BUF_SIZE];

int temp_q_num1;   // 선택한 문제의 번호를 임시 저장
int temp_q_num2;   // 선택한 문제의 번호를 임시 저장
int board[BOARD_SIZE][BOARD_SIZE];      // 게임 보드
int temp_cnt = 0;   // 문제 재입력 시 제어해주는 변수

int finish = 0;   // 가위바위보 종료 확인 변수
int init5 = 0;   // 가위바위보 승자 변수
int init4 = 0;   // 가위바위보 승자가 항목과 난이도를 선택하면 1로 변화함
int init3 = 0;   // 가위바위보 결과 제어 변수
int init2 = 0;   // 가위바위보 수를 내기위해 제어해주는 변수
int init = 0;   // 시작 할 때만 순서를 제어하기 위한 변수
int turn = 0;   // 0과 1을 통해 공격권 메시지 출력을 제어
int get_item = 0; // 아이템 보유 여부 확인 
int use_item = 0; // 아이템 사용 여부 확인

char temp[BUF_SIZE], resCategory[BUF_SIZE];
char change[BUF_SIZE];
int myRSP, yourRSP, resRSP;
int select_res = 0;

int wrong_num_c1 = 0;   // c1의 틀린 번호 저장을 위한 변수
int wrong_num_c2 = 0;   // c2의 틀린 번호 저장을 위한 변수
int c1_i, c2_i = 0;   // 틀린 문제 출력을 위한 변수
int item;            // 아이템 값을 저장하기 위한 변수

typedef struct problem {
   int num;   // 문제 번호
   char* question;   // 문제
   char* bogi[4];   // 보기
   int answer;      // 답
}P;

P e[4][16] =
{
{
   {1,"banana",{"바나나","사과","딸기","포도"},1},
   {2,"bean",{"맥주","빵","고기","콩"},4},
   {3,"coffee",{"국","커피","차","밥"},2},
   {4,"ground",{"땅","집","하늘","바다"},1},
   {5,"milk",{"점심","잡지","우유","지갑"},3},
   {6,"pig",{"소","돼지","닭","생선"},2},
   {7,"receipt",{"레시피","이력서","영수증","사장"},3},
   {8,"vegetable",{"채소","잠","고기","소스"},1},
   {9,"water",{"맥주","와인","소주","물"},4},
   {10,"salt",{"설탕","소금","후추","계피"},2},
   {11,"dinner",{"저녁","점심","아침","야식"},1},
   {12,"chocolate",{"치즈","초코렛","사탕","계란"},2},
   {13,"bill",{"병","술","계산서","그릇"},3},
   {14,"movie",{"음악","골프","여행","영화"},4},
   {15,"soccer",{"테니스","축구","농구","야구"},2},
   {16,"south",{"동쪽","서쪽","남쪽","북쪽"},3}
},
{
   {1,"appraisal",{"평가","문제","깨달음","도착"},1},
   {2,"veracity",{"감정","인내","역량","정직함"},4},
   {3,"gripe",{"포도","괴롭힘","질책","보복"},2},
   {4,"reprimand",{"비난","소비","명예","수리"},1},
   {5,"sanction",{"장점","잡지","제재","몰두"},3},
   {6,"pinnacle",{"회전","정점","사치","삭제"},2},
   {7,"connoisseur",{"범죄자","이력서","전문가","응대"},3},
   {8,"embezzlement",{"횡령","재화","불안","유혹"},1},
   {9,"transaction",{"공간","범위","전환","거래"},4},
   {10,"privision",{"수치","조항","예상","미래"},2},
   {11,"pore",{"가공","폭음","원고","변화"},1},
   {12,"haze",{"비","안개","원시","애정"},2},
   {13,"slur",{"병","추구","결점","완벽"},3},
   {14,"plank",{"처분","감정","운동","널판지"},4},
   {15,"lethality",{"권위","치사율","법률","제재"},2},
   {16,"debacle",{"가공","부상","붕괴","전개"},3}
} ,
{
   {1,"고구려의 대한 설명으로 옳은 것은?",{"지방에 22담로를 두고 왕족을 파견하였다","골품제라는 엄격한 신분 제도를 운영하였다","김수로가 김해 지역을 중심으로 건국하였다","빈민을 구제하기 위하여 진대법을 실시하였다"},4},
   {2,"진흥왕이 세운 비석으로 옳지 않은 것은?",{"단양 적성비","창녕 척경비","사택지적비","북한산 순수비"},3},
   {3,"지역에 따라 있는 문화유산으로 옳은 것은?",{"서울 - 미륵사지 석탑","공주 - 무령왕릉","부여 - 석촌동 고분","익산 - 정림사지 오층 석탑"},2},
   {4,"발해에 대한 설명으로 옳은 것은?",{"무천이라는 제천 행사가 있었다","전국에 9주 5소경을 설치하였다","대조영이 동모산에서 건국하였다","화백 회의에서 중요한 일을 결정하였다"},3},
   {5,"장보고에 대한 설명으로 틀린 것은?",{"완도에 청해진을 설치하였다","전성 여왕에게 개혁안을 올렸다","당으로 건너가 군인으로 활약했다","중국 산둥 반도에 법화원을 건립했다"},2},
   {6,"광종에 대한 설명으로 틀린 것은?",{"국자감 설립","과거제 시행","노비안검법 실시","광덕,준풍 연호 사용"},1},
   {7,"조선통신사의 설명으로 옳은 것은?",{"학문과 기술을 전해주었다","매년 정기적으로 파견되었다","스에키 토기 제작에 영향을 주었다","처음으로 일본에 불교를 전파하였다"},1},
   {8,"고려의 대외 항쟁이 아닌 것은?",{"강감찬의 귀주 대첩","최무선의 진포 대첩","김윤후의 처인성 전투","권율의 행주 대첩"},4},
   {9,"청의 발달된 문물과 기술을 소개하고, 이를 적극적으로 받아들이자고 주장한 박지원의 서적은?",{"북학의","목민심서","반계수록","열하일기"},4},
   {10,"임진왜란의 설명 중 틀린 것은?",{"을지문덕의 살수대첩","곽재우의 의병활동","김시민의 진주성전투","신립의 탄금대전투"},1},
   {11,"다음 중 시대가 다른 업적은?",{"측우기 제작","농사직설 보급","수원 화성 축조","삼강행실도 편찬"},3},
   {12,"정조가 설립한 왕실 도서관 중 인재 학문 기구는?",{"교정청","규장각","성균관","의금부"},2},
   {13,"한성순보를 발행한 기관은?",{"광혜원","기기창","박문국","전환국"},3},
   {14,"천도교 소년회를 조직하여 소년 운동을 전개한 인물은?",{"방정환","신채호","안창호","윤동주"},1},
   {15,"1990년대 대한민국에서 금융실명제와 외환 위기 사건 사이에 있는 적합한 사건은?",{"호주 제도 폐지","6월 민주 항쟁","지방 자치 제도 전면 시행","서울 올림픽 개최"},3},
   {16,"상하이, 충칭에서 활동하고 김구, 이승만, 이동녕이 중심 인물 이였던 단체의 활동 중 틀린 것은?",{"만민 공동회 개최","한국 광복군 창설","국채 보상 운동 주도","동학 농민 운동 주도"},2}
} ,
{
   {1,"다른 부족의 영역을 침범하면 노비나 소, 말로 변생하는 책화라는 풍습이 있었고 특산물로는 단궁, 과하마, 반어피 등 있던 나라의 설명으로 옳은 것은?",{"신성 지역인 소도가 존재하였다.","혼인 풍습으로 서옥제가 있었다.","여러 가들이 별도로 사출도를 다스렸다.","매년 10월에 무천이라는 제천 행사를 열었다."},4},
   {2,"대승기신론소, 십문화쟁론을 저술한 인물의 설명으로 옳은 것은?",{"황룡사 구층 목탑의 건립을 건의했다.","무애가를 만들어 불교의 대중화에 힘썼다.","국청사를 중심으로 해동 천태종을 창시하였다.","유불 일치설을 주장하여 유교와 불교의 조화를 도모하였다."},2},
   {3,"국선도, 풍월도라고 불리는 단체에 대한 설명으로 옳은 것은?",{"경당에서 글과 활쏘기를 배웠다","진흥왕 때 국가적인 조직으로 정비되었다.","박사와 조교를 두어 유교 경전을 가르쳤다","정사암에 모여 국가의 중대사를 결정하였다."},2},
   {4,"최승로의 시무 28조를 받은 '왕'의 업적으로 옳은 것은?",{"관학의 재정 기반을 마련하고자 양현고를 두었다","빈민을 구제하기 위하여 흑창을 처음 설치하였다","쌍기의 건의를 받아들여 과거 제도를 실시하였다","전국의 주요 지역에 12목을 설치하고 지방관을 파견하였다."},4},
   {5,"'이 건물'은 국보 제18호로 경상북도 영주시에 있다. 지붕의 형태는 팔작 지붕이며, 처마를 받치기 위한 공포를 기둥 위에만 올린 주심포 양식이다. 기둥은 배흘림 기법으로 세워졌으며, 건물 내부에는 소조여래 좌상이 있다.",{"수덕사 대웅전","봉정사 극락전","부석사 무량수전","쌍계사 대웅전"},3},
   {6,"계유정난을 통해 정권을 장학한 조선 7대 '왕'의 재위 시기에 있었던 사실로 옳은 것은?",{"초계문신제를 실시하여 관리를 재교육하였다","집현전을 폐지하여 왕권을 강화하고자 하였다","현량과를 실시하여 신진 사림을 등용하고자 하였다","균역법을 시행하여 군역의 부담을 줄여주고자 하였다"},2},
   {7,"선현제향과 학문 연구를 위하여 설립된 조선 시대의 사설 교육 기관으로 향촌 사림의 모임 장소로 시정을 비판하고 공론을 형성하는 기관에 대한 설명으로 옳은 것은?",{"좌수와 별감을 선발하여 운영되었다","중앙에서 파견된 교수나 훈도가 지도하였다","국왕으로부터 편액과 함께 서적 등을 받기도 하였다","전국의 부.목.군.현에 하나씩 설립되었다"},3},
   {8,"경세유표가 저술된 시기에 있었던 사실로 옳은 것은?",{"비변사를 중심으로 소수의 가문이 권력을 행사하였다","공신과 왕족의 사병이 혁파되고 군사권을 강화되었다","왕권을 강화하기 위하여 6조 직계제가 실시되었다","외척 간의 대립으로 을사사화가 발생하였다"},1},
   {9,"요세가 백련 결사를 제창했던 백련사와 고려 시대 청자를 만들어 공급한 도요지와 동일한 지역은?",{"정약용이 유배 생활을 했던 다산 초당","율곡 이이가 태어난 오죽헌","퇴계 이황의 학덕을 기리는 도산 서원","대가야의 위용을 보여주는 지산동 고분군"},2},
   {10,"어재연 장군 수자기와 초지진 덕진진 광성보전투 이후 일어난 사실은?",{"종로와 전국 각지에 척화비가 세워졌다","오페르트가 남연군 묘를 도굴했다","평양 관민들에 의해 제너럴 셔먼호가 불탔다","외규장각 건물이 불타고 의궤가 약탈당했다"},1},
   {11,"무위영과 장어영에 소속된 군인들이 포도청과 경기 감영을 습격하였고, 여기에 군중까지 가세하여 일본 공사관을 공격한 '사건'에 옳은 것은?",{"훈련대가 해산되는 원일을 제공하였다","포접제가 활용되어 조직적으로 전개되었다","청의 내정 간섭이 본격화되는 결과를 가져왔다","청,일 양국의 군대가 조선에게 철수하는 계기가 되었다"},3},
   {12,"평안도 강서 출생, 독립 협회 가입, 대한인 국민회 중앙 총회 조직, 흥사단 조직, 대한민국 임시 정부 내무총장 겸 국무총리 대리를 맞은 '인물'의 활동으로 옳은 것은?",{"가갸날을 제정하고 기관지인 한글을 발행하였다","인재를 양성하기 위하여 대성 학교를 설립하였다","조선학 운동을 주도하여 여유당전서를 간행하였다","서유견문을 집필하여 서양 근대 문명을 소개하였다"},2},
   {13,"중국 대전자령 전투에 참여하고 지청천이 이끈 독립군 부대에 대한 설명으로 옳은 것은?",{"대한민국 임시 정부의 직할 부대로 창설되었다","중국 관내에서 결성된 최초의 한인 무장 부대였다","조선 혁명 간부 학교를 세워 군사력을 강화하였다","중국 호로군과 연합 작전을 통해 항일 전쟁을 전개하였다"},4},
   {14,"여운형이 위원장, 안재홍이 부위원장이고 완전한 독립 국가 건설을 기하는 단체에 대한 설명으로 옳은 것은?",{"조선 건국 동맹 세력으로 바탕으로 조직되었다","모스크바 3국 외상 회의의 결정을 반대하였다","미 군정의 후원을 받아 좌우 합작 운동을 전개하였다","농지개혁법 제정을 주장하였다"},1},
   {15,"낮은 단계의 연방제 안이 서로 공통성이 있다고 인정하는 남북 공동 선언을 발표한 정부 시기에 사실로 옳은 것은?",{"한-일 협정을 체결하였다","중국, 소련 등과 수교하였다","금강산 관광 사업을 시작하였다","경제 협력 개발 기구(OECD)에 가입하였다"},3},
   {16,"금융 실명제를 실시 했을 때 정부는?",{"김영삼","노태우","노무현","김대중"},1}
}
};

// 문자열 끝에 개행문자 제거하여 깔끔한 출력을 해주기 위해 작성한 함수
void RemoveEnd(char* buf)
{
   int i = 0;
   while (buf[i])      // buf[i]가 참(널문자가 아님)이면 반복하여라.
      i++;

   //현재 i는 널문자가 있는 위치, i-1은 마지막 문자 위치
   buf[i - 1] = '\0';
}

// 가위바위보 결과를 저장하는 함수
void RSP() {

   if (myRSP == yourRSP) {
      resRSP = 3;
   }
   else if (myRSP == 1)
   {
      if (yourRSP == 2)
      {
         resRSP = 1;
      }
      if (yourRSP == 3)
      {
         resRSP = 2;
      }
   }
   else if (myRSP == 2)
   {
      if (yourRSP == 1)
      {
         resRSP = 2;
      }
      if (yourRSP == 3)
      {
         resRSP = 1;
      }
   }
   else if (myRSP == 3)
   {
      if (yourRSP == 1)
      {
         resRSP = 1;
      }
      if (yourRSP == 2)
      {
         resRSP = 2;
      }
   }
}

// 틀린 문제를 저장한 배열을 번호 순서대로 정렬
void insertion_sort(int list[], int n);

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


// 클라이언트의 보드판을 랜덤하게 생성한다. 
void makeBoard()
{
   int check_number[BOARD_SIZE * BOARD_SIZE] = { 0 };
   int array_len;

   srand(time(NULL) + rand() % 25); // 서버 보드판과 다르게 하기위해 100을 추가

   item = (rand() % 16) + 1;   //아이템 값을 1 ~ 16까지 난수로 생성
   for (int i = 0; i < BOARD_SIZE; i++)
   {
      for (int j = 0; j < BOARD_SIZE; j++)
      {
         while (1)
         {
            int temp = rand() % 16; // 0 ~ 15 난수 발생

            if (check_number[temp] == 0) // 중복 제거 알고리즘
            {
               check_number[temp] = 1;
               board[i][j] = temp + 1;
               break;
            }
         }
      }
   }
}

// 빙고 판 출력
void game_print(int number)
{
   int i, j;

   system("clear"); // 동적 효과를 위한 화면 초기화
   printf("%c[1;33m", 27); // 터미널 글자색을 노랑색으로 변경

   printf("@---- B I N G O ----@");

   if (select_res == 1)
      printf("   <ENG - EASY>\n");
   else if (select_res == 2)
      printf("   <ENG - HARD>\n");
   else if (select_res == 3)
      printf("   <HIS - EASY>\n");
   else if (select_res == 4)
      printf("   <HIS - HARD>\n");

   printf("+----+----+----+----+\n");

   for (i = 0; i < 4; i++)
   {
      for (j = 0; j < 4; j++)
      {
         if (board[i][j] == number)
            board[i][j] = 0; // X표 처리

         if (board[i][j] == item && use_item == 0) {
            printf("| ");
            printf("%c[1;31m", 27);
            printf("%2d ", board[i][j]);
            printf("%c[1;33m", 27);
         }
         else if (board[i][j] == 0)
         {
            printf("| ");
            printf("%c[1;31m", 27);
            printf("%2c ", 88);
            printf("%c[1;33m", 27);
         }
         else
            printf("| %2d ", board[i][j]);
      }
      printf("|\n");
      printf("+----+----+----+----+\n");
   }

   printf("IF YOU WANNA EXIT : ENTER 'Q' OR 'q' \n");
   printf("%c[0m", 27); // 터미널 글자색을 원래색으로 변경
}

// 빙고가 3줄이 완성되면 서버에게 /win이라는 문자열을 보낸다.
int bingo_check(int board[4][4]) {

   int count = 0;

   for (int i = 0; i < BOARD_SIZE; i++) // 가로
   {
      if (board[i][0] == 0 && board[i][1] == 0 && board[i][2] == 0 && board[i][3] == 0) // 가로
         count++;
      if (board[0][i] == 0 && board[1][i] == 0 && board[2][i] == 0 && board[3][i] == 0) // 세로
         count++;
   }
   // 대각선 빙고 확인
   if (board[0][0] == 0 && board[1][1] == 0 && board[2][2] == 0 && board[3][3] == 0)
      count++;
   if (board[0][3] == 0 && board[1][2] == 0 && board[2][1] == 0 && board[3][0] == 0)
      count++;

   return count;
}

// 메인 함수
int main(int argc, char* argv[])
{
   int sock;
   struct sockaddr_in serv_addr;

   pthread_t snd_thread, rcv_thread;
   void* thread_return;

   if (argc != 6) {
      printf("Usage : %s <IP> <port> <name> <RSP> <Category>\n", argv[0]);
      printf("\n\n<RSP> : 1. Rock / 2. Scissor / 3. Paper\n");
      printf("<Category>\n1. Eng(Easy)\n2. Eng(Hard)\n3. History(Easy)\n4. History(Hard)\n");
      exit(1);
   }

   sprintf(name, "[%s]", argv[3]);
   sprintf(temp, "/rsp %s", argv[4]);
   sprintf(resCategory, "/sel %s", argv[5]);
   select_res = search_answer(argv[5]);
   myRSP = search_answer(argv[4]);

   system("clear"); // 동적 효과를 위한 화면 초기화
   printf("%c[1;33m", 27); // 터미널 글자색을 노랑색으로 변경
   printf("----------------------------------------------------\n");
   printf("환영합니다 ! %s님 !\n", name);
   printf("----------------------------------------------------\n");
   if (myRSP == 1)
      printf("나의 가위바위보 : 가위 !\n");
   if (myRSP == 2)
      printf("나의 가위바위보 : 바위 !\n");
   else if (myRSP == 3)
      printf("나의 가위바위보 : 보 !\n");
   printf("----------------------------------------------------\n");
   if (select_res == 1)
      printf("선택한 게임 항목 : 영어 / 게임 난이도 : EASY\n");
   else if (select_res == 2)
      printf("선택한 게임 항목 : 영어 / 게임 난이도 : HARD\n");
   else if (select_res == 3)
      printf("선택한 게임 항목 : 한국사 / 게임 난이도 : EASY\n");
   else if (select_res == 4)
      printf("선택한 게임 항목 : 한국사 / 게임 난이도 : HARD\n");
   printf("----------------------------------------------------\n");
   printf("상대방의 서버 접속을 기다리는 중입니다...\n");

   sock = socket(PF_INET, SOCK_STREAM, 0);

   memset(&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
   serv_addr.sin_port = htons(atoi(argv[2]));

   if (connect(sock, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) == -1)
      error_handling("connect() error");

   pthread_create(&snd_thread, NULL, send_msg, (void*)& sock);
   pthread_create(&rcv_thread, NULL, recv_msg, (void*)& sock);

   pthread_join(snd_thread, &thread_return);
   pthread_join(rcv_thread, &thread_return);

   close(sock);
   return 0;
}

// send thread main 
void* send_msg(void* arg)
{
   int sock = *((int*)arg);
   char name_msg[NAME_SIZE + BUF_SIZE];

   while (1) {

      // 빙고가 3줄이 완성되면 승패 결과 출력 후 게임이 종료된다.
      if (bingo_check(board) == 3) {
         system("clear"); // 동적 효과를 위한 화면 초기화
         printf("%c[1;33m", 27); // 터미널 글자색을 노랑색으로 변경
         printf("게임 승리\n");
         write(sock, "/win", sizeof("/win"));
         printf("%c[0m", 27); // 터미널 글자색을 원래색으로 변경

         insertion_sort(wrong_c1, wrong_num_c1 - 1);   // 클라이언트 1이 틀린 문제를 내림차순으로 정렬

         // 클라이언트 1가 틀린 문제 출력
         for (c1_i = 0; wrong_c1[c1_i] != 0; c1_i++)
            printf("%d.%s: %s \n", e[select_res - 1][wrong_c1[c1_i]-1].num, e[select_res - 1][wrong_c1[c1_i]-1].question, e[select_res - 1][wrong_c1[c1_i]-1].bogi[e[select_res - 1][wrong_c1[c1_i]-1].answer - 1]);
         exit(1);
      }

      // ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
      // 빙고 시작 전 가위바위보하기 위한 제어
      if (init2 == 0 && init4 == 0 && init5 == 0 && yourRSP == 0 && finish == 0 && resRSP == 0) {
         printf("%c[1;33m", 27); // 터미널 글자색을 노랑색으로 변경
         write(sock, temp, sizeof(temp));
         init2 = 1;
         continue;
      }

      // 선접속 클라이언트가 가위바위보 결과를 서로에게 전달
      if (init2 == 1 && init3 == 1 && init4 == 0 && finish == 0 && yourRSP != 0) {
         RSP();
         sprintf(temp, "/res %d", resRSP);
         write(sock, temp, sizeof(temp));
         init3 = 0;
         continue;
      }

      // 선 접속 가위바위보가 끝나고 항목을 모두에게 전달한다.(여기가 문제인데?)
      if (select_res != 0 && init5 == 1 && finish == 0 && turn == 1) {
         write(sock, resCategory, sizeof(resCategory));
      }
      // ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

      fgets(msg, BUF_SIZE, stdin);

      // 문제를 입력하고자 할때
      if (strstr(msg, "/n") != NULL)
      {
         // 자신의 턴이 아닐 때 입력하면 발생하는 예외 처리
         if (turn == 1) {
            temp_q_num1 = search_answer(msg);
         }
         else {
            printf("NOT YOUR TURN!\n");
            continue;
         }

         // 문제가 출력되지 않은 상태에서 답을 입력한 경우
         if (temp_cnt == 1) {
            printf("PLEASE PRINT OUT THE QUESTIONS FIRST BY ENTERING THE CODE\n");
            continue;
         }

         // 문제 번호 초과 입력 시 예외 처리
         if (search_answer(msg) > 16) {
            printf("PLEASE ENTER TO 1 FROM 16\n");
            continue;
         }

         temp_cnt = 1;
      }
      else if (strstr(msg, "/i") != NULL)
      {
         if (get_item == 1) {
            int i, j;
            char ch[20];
            int* ptr;
            for (i = 0; i < 4; i++)
            {
               for (j = 0; j < 4; j++)
               {
                  ch[i * 4 + j] = (char)(board[i][j]);
               }
            }

            sprintf(msg, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", ch[0], ch[1], ch[2], ch[3], ch[4], ch[5], ch[6], ch[7], ch[8], ch[9], ch[10], ch[11], ch[12], ch[13], ch[14], ch[15]);

            sprintf(name_msg, "/i %s", msg);
            if (turn == 1) {
               write(sock, name_msg, strlen(name_msg));
         
            }
            continue;
         }
         else {
            printf("You don't have an item... \n");
         
            continue;
         }


      }
      // 게임을 종료하고 싶을 때 q, Q를 입력하면 연결이 종료된다.
      else if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
         close(sock);
         exit(0);
      }

      // 답을 입력 했을 때
      else if (strstr(msg, "/a") != NULL) {

         // 문제가 출력되지 않았을 때, 답을 입력하려는 행위에 대한 예외 처리
         if (temp_cnt == 0) {
            printf("PLEASE WAIT UNTIL THER PROBLEM COMES UP\n");
            continue;
         }

         // 자신의 턴의 경우의 수에 따른 문제를 풀었을 때, 답이 맞으면 X 처리, 틀리면 아무 일도 없다.
         if ((search_answer(msg) == e[select_res - 1][temp_q_num1 - 1].answer) || (search_answer(msg) == e[select_res - 1][temp_q_num2 - 1].answer)) {
            if (turn == 1) {
               if (e[select_res - 1][temp_q_num1 - 1].num == item) {
                  game_print(temp_q_num1);   // 선공 클라이언트의 문제 정답 출력
                  printf("You get item!! \n");
                  get_item = 1;
                  temp_q_num1 = 0;
               }
               else {
                  game_print(temp_q_num1);
               }
            }
            else {
               if (e[select_res - 1][temp_q_num2 - 1].num == item) {
                  game_print(temp_q_num2);   // 후공 클라이언트의 문제 정답 출력 (선공 클라이언트일 경우 0 이므로 영향없다.
                  printf("You get item!! \n");
                  get_item = 1;
                  temp_q_num2 = 0;
               }
               else {
                  game_print(temp_q_num2);
               }
            }
            printf("%c[1;34m", 27); // 터미널 글자색을 파란색으로 변경
            printf("Correct ! \n");
            printf("%c[1;37m", 27); // 터미널 글자색을 흰색으로 변경
            temp_cnt = 0;

            // 자신의 턴인지 아닌지를 표시
            if (turn == 0) {
               printf("YOUR TURN!!\n");
               turn = 1;
            }
            else {
               turn = 0;
            }
            continue;
         }
         else {
            // 틀리면 오답 메시지가 출력되고 아무일도 처리하지 않는다.
            game_print(0);
            printf("%c[0;31m", 27); // 터미널 글자색을 빨간색으로 변경
            printf("Not Correct !\n");
            printf("%c[1;37m", 27); // 터미널 글자색을 흰색으로 변경
            temp_cnt = 0;
            // 자신의 턴인지 아닌지를 표시
            if (turn == 0) {
               // 틀린 문제의 번호를 wrong 배열에 저장
               wrong_c1[wrong_num_c1] = e[select_res - 1][temp_q_num1 - 1].num;
               wrong_num_c1++;
               wrong_c2[wrong_num_c2] = e[select_res - 1][temp_q_num2 - 1].num;
               wrong_num_c2++;
               printf("YOUR TURN!!\n");
               turn = 1;
            }
            else {
               // 틀린 문제의 번호를 wrong 배열에 저장
               wrong_c1[wrong_num_c1] = e[select_res - 1][temp_q_num1 - 1].num;
               wrong_num_c1++;
               wrong_c2[wrong_num_c2] = e[select_res - 1][temp_q_num2 - 1].num;
               wrong_num_c2++;
               turn = 0;
            }
            continue;
         }
      }

      // 예외 없이 입력하면 입력된 메시지를 서버에게 보내면서 화면에 출력한다.
      sprintf(name_msg, "%s %s", name, msg);
      write(sock, name_msg, strlen(name_msg));
   }
   return NULL;
}

void* recv_msg(void* arg) // read thread main 
{
   int sock = *((int*)arg);
   char name_msg[NAME_SIZE + BUF_SIZE];
   int str_len;   // 받은 메시지의 길이

   while (1) {
      str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);
      if (strstr(name_msg, "/i") != NULL) {
         if (get_item == 1 || turn == 0) {
            char ch[20];
            int* ptr;

            for (int i = 0; i < 4; i++)
            {
               for (int j = 0; j < 4; j++)
               {
                  ch[i * 4 + j] = (char)(board[i][j]);
               }
            }
            sprintf(msg, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", ch[0], ch[1], ch[2], ch[3], ch[4], ch[5], ch[6], ch[7], ch[8], ch[9], ch[10], ch[11], ch[12], ch[13], ch[14], ch[15]);

            char* result;
            int i = 10;
            int j = 10;

            result = strtok(name_msg, " ");

            while (result != NULL)
            {
               strcpy(change, result);
               board[i][j] = atoi(change);
               result = strtok(NULL, " ");

               if (j == 3)
               {
                  j = 0;
                  i++;
               }
               else
                  j++;

               if (i == 4)
                  break;

               if (i == 10)
               {
                  i = 0;
                  j = 0;
               }

            }

            sprintf(name_msg, "Change!!\n %s", msg);
            if (turn == 1) {
               write(sock, name_msg, strlen(name_msg));
               
            }
            
            game_print(0);
            
            if (turn == 0) {
               write(sock, name_msg, strlen(name_msg));
            
            }
         }
      }

      if (strstr(name_msg, "Change!!") != NULL) {
         system("clear");
         char* result;
         int i = 10;
         int j = 10;

         result = strtok(name_msg, " ");
         while (result != NULL)
         {
            strcpy(change, result);

            board[i][j] = atoi(change);
            result = strtok(NULL, " ");

            if (j == 3)
            {
               j = 0;
               i++;
            }

            else
               j++;

            if (i == 4)
               break;

            if (i == 10)
            {
               i = 0;
               j = 0;
            }

            if (turn == 1) {
               use_item = 1;
            }
            else {
               use_item = 0;
            }
         }
      
         game_print(0);
         if (turn == 1) {
            printf("YOUR TURN!!\n");
         }
         else {

         }

      }
      if ((strstr(name_msg, "/res") != NULL) && finish == 0) {

         if (resRSP != 0 && (search_answer(name_msg) == 1 || search_answer(name_msg) == 3)) {
            system("clear"); // 동적 효과를 위한 화면 초기화
            printf("%c[1;34m", 27); // 터미널 글자색을 파란색으로 변경
            printf("-----------------------------------------------\n");
            printf("승리! 당신은 선공! (계속하려면 엔터를 누르세요)\n");
            printf("-----------------------------------------------\n");
            printf("%c[1;37m", 27); // 터미널 글자색을 흰색으로 변경
            init5 = 1;      // 승자에게 부여되는 패
            turn = 1;
            continue;
         }
         else if (resRSP == 0 && (search_answer(name_msg) == 2)) {      // init2가 0이면서, 0을 받았으니 후공 클라이언트 승리
            system("clear"); // 동적 효과를 위한 화면 초기화
            printf("%c[1;34m", 27); // 터미널 글자색을 파란색으로 변경
            printf("-----------------------------------------------\n");
            printf("승리! 당신은 선공! (계속하려면 엔터를 누르세요)\n");
            printf("-----------------------------------------------\n");
            printf("%c[1;37m", 27); // 터미널 글자색을 흰색으로 변경
            init5 = 1;
            turn = 1;
            continue;
         }
      }
      else if ((strstr(name_msg, "/sel") != NULL) && finish == 0) {
         // 항목과 난이도를 선택받고 게임을 시작한다.
         select_res = search_answer(name_msg);
         printf("%c[1;33m", 27); // 터미널 글자색을 노랑색으로 변경
         printf("----------------------------------------\n");
         if (select_res == 1)
            printf("게임 항목 : 영어\n게임 난이도 : EASY\n");
         else if (select_res == 2)
            printf("게임 항목 : 영어\n게임 난이도 : HARD\n");
         else if (select_res == 3)
            printf("게임 항목 : 한국사\n게임 난이도 : EASY\n");
         else if (select_res == 4)
            printf("게임 항목 : 한국사\n게임 난이도 : HARD\n");
         printf("%c[1;37 m", 27); //터미널 글자색을 흰색으로 변경
         finish = 1;
         printf("---------------------------------------\n");
         printf("가위바위보 게임을 종료합니다... \n잠시만 기다려주세요...\n");
         printf("---------------------------------------\n");

         if (turn == 1)
            sleep(3);
         if (turn == 0)
            sleep(2);

         makeBoard();
         game_print(0);

         if (turn == 1)
            printf("YOUR TURN!\n");

         continue;
      }
      else if ((strstr(name_msg, "/rsp") != NULL) && finish == 0) {

         if (init2 == 1) {   // 보낸자의 여유
            init2 = 0;
            continue;
         }
         else            // 먼저들어온 새기에다가 두 클라 수를 저장해서 결과 조합
         {
            system("clear"); // 동적 효과를 위한 화면 초기화
            printf("%c[1;33m", 27); // 터미널 글자색을 노랑색으로 변경
            yourRSP = search_answer(name_msg);
            printf("--------------------------------------------------------------\n");
            printf("당신 선 접속 클라이언트입니다 !\n무승부 시 당신은 선공입니다! \n");
            printf("상대의 수는 %d입니다 !\n결과를 출력하려면 엔터 키를 누르세요 !\n", yourRSP);
            printf("--------------------------------------------------------------\n");
            init2 = 1;
            init3 = 1;      // 결과를 조합하는 친구는 init3 가 1이다.
            continue;
         }
      }
      else if (str_len == -1) {
         return (void*)-1;
      }
      else if (strstr(name_msg, "/n") != NULL) {
         temp_cnt = 1;

         RemoveEnd(name_msg);

         printf("%c[1;33m", 27); // 터미널 글자색을 노랑색으로 변경
         printf("----------------------QUESTION-----------------------\n");
         printf("%d. %s\n 1.%s\n 2.%s\n 3.%s\n 4.%s\n",
            e[select_res - 1][search_answer(name_msg) - 1].num, e[select_res - 1][search_answer(name_msg) - 1].question,
            e[select_res - 1][search_answer(name_msg) - 1].bogi[0], e[select_res - 1][search_answer(name_msg) - 1].bogi[1],
            e[select_res - 1][search_answer(name_msg) - 1].bogi[2], e[select_res - 1][search_answer(name_msg) - 1].bogi[3]);
         printf("-----------------------------------------------------\n");
         printf("%c[1;37 m", 27); //터미널 글자색을 흰색으로 변경
         temp_q_num2 = search_answer(name_msg);

         // 후공 클라이언트의 답 예외처리
         if (turn == 0) {
            temp_q_num1 = temp_q_num2;
         }

      }
      else if (strstr(name_msg, "/win") != NULL)
      {
         system("clear"); // 동적 효과를 위한 화면 초기화
         printf("%c[1;33m", 27); // 터미널 글자색을 노랑색으로 변경
         printf("게임 패배\n");
         printf("%c[1;37 m", 27); //터미널 글자색을 흰색으로 변경

         insertion_sort(wrong_c2, wrong_num_c2 - 1);      // 클라이언트 2이 틀린 문제를 내림차순으로 정렬

         // 클라이언트 2가 틀린 문제 출력
         for (c2_i = 0; wrong_c2[c2_i] != 0; c2_i++) {
            printf("%d.%s: %s \n", e[select_res - 1][wrong_c2[c2_i]-1].num, e[select_res - 1][wrong_c2[c2_i]-1].question, e[select_res - 1][wrong_c2[c2_i]-1].bogi[e[select_res - 1][wrong_c2[c2_i]-1].answer - 1]);
         }
         exit(0);
      }
      else {
         name_msg[str_len] = 0;
         fputs(name_msg, stdout);
      }
   }
   return NULL;
}

void error_handling(char* msg) {
   fputs(msg, stderr);
   fputc('\n', stderr);
   exit(1);
}

//틀린 문제 정렬을 위한 함수
void insertion_sort(int list[], int n) {
   int temp = 0;
   for (int i = 0; i < n; i++)
   {
      for (int j = 0; j < n - i; j++)
      {
         if (list[j] > list[j + 1])
         {
            temp = list[j];
            list[j] = list[j + 1];
            list[j + 1] = temp;
         }
      }
   }
   printf("--------------------틀린 문제-------------------- \n");
}
