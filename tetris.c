#include <curses.h>
#include <stdlib.h>
#include <string.h>

void go();
int main(int argc, char **argv){
  char text[128];
  initscr();
  start_color();
  noecho();
  cbreak();
  curs_set(0);
  init_pair(1, COLOR_CYAN, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);
  init_pair(4, COLOR_BLUE, COLOR_BLACK);
  init_pair(5, COLOR_YELLOW, COLOR_BLACK);
  init_pair(6, COLOR_CYAN, COLOR_BLACK);
  init_pair(7, COLOR_RED, COLOR_BLACK);
  init_pair(8, COLOR_WHITE, COLOR_BLACK);
  bkgd(COLOR_PAIR(8));
  keypad(stdscr,TRUE);
  go();

  endwin();
  return 0;
}

#define PATNUM 7
#define PATSIZEW 3
#define PATSIZEH 3
#define S_SIZE_LEFT COLS/5
#define S_SIZE_RIGHT COLS*3/8
static char blockPattern[PATNUM][PATSIZEH][PATSIZEW] =
  {
    {{' ',' ',' '}, {'#','#',' '},{'#','#',' '}},
    {{' ','#',' '}, {' ','#',' '},{' ','#',' '}},
    {{' ',' ',' '}, {' ','#',' '},{'#','#','#'}},
    {{' ',' ',' '}, {' ','#','#'},{'#','#',' '}},
    {{' ',' ',' '}, {'#','#',' '},{' ','#','#'}},
    {{' ',' ',' '}, {'#',' ',' '},{'#','#','#'}},
    {{' ',' ',' '}, {' ',' ','#'},{'#','#','#'}}
  };

/*底へのあたり判定*/
int collisionBottomWall(int blocX, int blocY, int bottom, int pattern, char buf[256][256])
{
  int i, j, a=0, flag = 0;

  for(i=0;i<3;i++){
    for(j=0;j<3;j++){
      //ブロックの要素が空白でない時
      if(blockPattern[pattern][i][j] != ' '){
	//ブロックの下に既にブロックがある時、flagを1にしてループを抜ける
	if(buf[blocY+1+i][blocX+j] == '#'){
	  flag = 1;
	  break;
	}
	a=i;
      }
    }
  }

  if( blocY+a == bottom) {
    flag = 1;
  }

  return flag;
}

/*ブロック回転判定*/
int my_turn_check(int blocX, int blocY, char turn[PATNUM][PATSIZEH][PATSIZEW], int pattern, char buf[256][256])
{
  int i, j, a=0, flag=0;

  for(i=0;i<3;i++){
    for(j=0;j<3;j++){
      //ブロックの要素が空白でない時
      if(turn[pattern][i][j] != ' '){
	//回転先に文字がある場合を確認
	if(buf[blocY+i][blocX+j]=='|'||buf[blocY+i][blocX+j]=='#')
	  a++;
      }
    }
  }
  if(a==0){//回転先に文字がない場合、回転flagを立てる
    flag=1;
  }
  return flag;
}

/*ブロック消去判定*/
int my_clear_block(int blocY, char buf[256][256])
{
  int i, j, a=0, flag = 0;

  //ステージエリア内の指定された一行に空白がある場合を確認
  for(i=S_SIZE_LEFT+1;i<S_SIZE_RIGHT;i++){
    if(buf[blocY][i]==' '){
      a++;
    }
  }
  if(a==0){//ステージエリア内の指定された一行に空白がない場合、消去flagを立てる
    flag=1;
  }
  return flag;
}

int gameover(char buf[256][256])
{
  int i, flag=0;
  for(i=S_SIZE_LEFT+1;i<S_SIZE_RIGHT+2;i++){
    if(buf[2][i]=='#'){
      return 1;
      
    }
  }
  /*  
  for(i=S_SIZE_LEFT+1;i<S_SIZE_RIGHT;i++){
    if(buf[4][i]=='#' &&buf[5][i]=='#' ){
      flag=1;
      break;
    }
  }
  */
  return flag;
}

void go()
{
  int blocX, blocY;

  int ch;
  int delay=0;
  int waitCount = 20000;
  char msg[256];
  int pattern, patA, patB;
  char buffer[256][256];
  char color_buffer[256][256];
  char block_turn[PATNUM][PATSIZEH][PATSIZEW]={0};
  int i, j, k, left_flag, right_flag, x, y;
  int ii, jj;
  int score=0, sc=1, level=1;
  int pat_flag=0;
  int gameover_flag;
  FILE *fp;
  char par[100];
  char name[6][30];
  char highscore[6][256];
  int high_score;
  char Name[6][30];

  srand((unsigned) time(NULL));
  patA = rand()%7;//次のブロックパターン
  patB = rand()%7;//次の次のブロックパターン
  pattern = rand()%7;
  blocX = (S_SIZE_LEFT+S_SIZE_RIGHT)/2+1;
  blocY = 1;

  //ステージエリアの作成
  for(j=0;j<256;j++){
    for(i=0;i<256;i++){
      if((i==S_SIZE_LEFT&&j!=0)||(i==S_SIZE_RIGHT+3&&j!=0)){
	buffer[j][i]='|';
      }
      else if(i>S_SIZE_LEFT&&i<S_SIZE_RIGHT+3&&j==0){
	buffer[j][i]='=';
      }
      else if((i==S_SIZE_LEFT&&j==0)||(i==S_SIZE_RIGHT+3&&j==0)){
	buffer[j][i]='*';
      }
      else{
	buffer[j][i]=' ';
      }
      color_buffer[j][i]=0;
    }
  }

  timeout(0);

  while((ch=getch())!='Q'){
    attron(COLOR_PAIR(8));
    mvaddstr(blocY,blocX,"   ");
    mvaddstr(blocY+1,blocX,"   ");
    mvaddstr(blocY+2,blocX,"   ");

    if(delay%waitCount==0){
      blocY += 1;
    }
    if(blocX<0){
      blocX =0;
      beep();
    }
    if(blocX>=COLS/2){
      blocX =COLS/2-1;
      beep();
    }
    delay++;
    switch(ch){
    case KEY_LEFT:
      left_flag=0;x=0;
      for(i=0;i<3;i++){
	for(j=0;j<3;j++){
	  //ブロックの要素が空白でない時
	  if(blockPattern[pattern][j][i] != ' '){
	    x++;
	    if((buffer[blocY+j][blocX+i-1]=='|')||(buffer[blocY+j][blocX+i-1]=='#')){
	      left_flag++;
	    }
	  }
	}
	if(x==0){//ブロックの左の列の要素がすべて空白のとき、次のループへ
	  continue;
	}
	if(left_flag==0){//ブロックの左が空白のときブロックを左に移動
	  blocX -=1;
	  break;
	}
      }
      break;
    case KEY_RIGHT:
      right_flag=0;y=0;
      for(i=2;i>=0;i--){
	for(j=0;j<3;j++){
	  //ブロックの要素が空白でない時
	  if(blockPattern[pattern][j][i] != ' '){
	    y++;
	    if((buffer[blocY+j][blocX+i+1]=='|')||(buffer[blocY+j][blocX+i+1]=='#')){
	      right_flag++;
	    }
	  }
	}
	if(y==0){//ブロックの右の列の要素がすべて空白のとき、次のループへ
	  continue;
	}
	if(right_flag==0){//ブロックの右が空白のときブロックを右に移動
	  blocX +=1;
	  break;
	}
      }
      break;
    case KEY_DOWN:
      blocY +=1;
      break;
    case ' ':
      for(i=0;i<3;i++){
	for(j=0;j<3;j++){
	  block_turn[pattern][i][j]=blockPattern[pattern][2-j][i];
	}
      }
      if(my_turn_check(blocX, blocY, block_turn, pattern, buffer)){
	for(i=0;i<3;i++){
	  for(j=0;j<3;j++){
	        blockPattern[pattern][i][j]=block_turn[pattern]
		  [i][j];
	  }
	}
      }
      break;
    case 'Z':
      if(pattern == 1 ){
	pattern = 2;
      }else if(pattern == 2 ){
	pattern = 1;
      }
      beep();
      break;
    default:
      break;
    }

    for(j=0; j<LINES; j++){
      for(i=0; i<COLS/2; i++){
	attron(COLOR_PAIR(color_buffer[j][i]));
	mvaddch(j, i, buffer[j][i] );
      }
    }
    attron(COLOR_PAIR(pattern+1));
    for(jj=0; jj<3; jj++){
      for(ii=0; ii<3; ii++){

	//ブロックの要素が空白の時、次のループへ
	if(blockPattern[pattern][jj][ii] == ' ')
	  continue;
	mvaddch(blocY+jj, blocX+ii, blockPattern[pattern][jj][ii]);
      }
    }
    if( collisionBottomWall(blocX, blocY, LINES-1, pattern, buffer)){
      for(jj=0; jj<3; jj++){
	for(ii=0; ii<3; ii++){

	  //ブロックの要素が空白の時、次のループへ
	  if(blockPattern[pattern][jj][ii] == ' ')
	    continue;

	  buffer[blocY+jj][blocX+ii] = blockPattern[pattern][jj][ii];
	  color_buffer[blocY+jj][blocX+ii]=pattern+1;
	}
      }
      //ブロック消去
      for(i=0;i<3;i++){
	if(my_clear_block(blocY+i, buffer)){
	  for(j=blocY+i;j>1;j--){
	    for(k=S_SIZE_LEFT+1;k<S_SIZE_RIGHT;k++){
	      buffer[j][k]=buffer[j-1][k];
	    }
	  }
	  score=score+21000-waitCount;
	}
      }
      //レベルの計算と落下速度変化
      if((score>=1000*sc)&&(waitCount>2000)){
	waitCount=waitCount-3000;
	sc++;
	level++;
      }

      //beep();
      blocY = 1;
      blocX = (S_SIZE_LEFT+S_SIZE_RIGHT)/2+1;
      srand((unsigned) time(NULL));
      pattern = patA;
      patA = patB;
      patB = rand()%7;
    }
    sprintf(msg, "X %03d Y %03d Pat %02d", blocX, blocY, pattern );
    attron(COLOR_PAIR(8));
    mvaddstr(2,COLS/2,msg );
    sprintf(msg,"Time        %d",clock()/1000000);
    mvaddstr(3,COLS/2,msg);
    sprintf(msg,"Level       %d",level);
    mvaddstr(4,COLS/2,msg);
    sprintf(msg,"Score       %d",score);
    mvaddstr(5,COLS/2,msg);
    sprintf(msg,"High Score");
    mvaddstr(6,COLS/2,msg);
    sprintf(msg,"Name");
    mvaddstr(7,COLS/2,msg);
    sprintf(msg,"Next Blocks");
    mvaddstr(8,COLS/2,msg);
    sprintf(msg,"First  Second");
    mvaddstr(9,COLS/2+3,msg);
    sprintf(msg,"+---+  +---+");
    mvaddstr(10,COLS/2+3,msg);
    sprintf(msg,"|%c%c%c|  |%c%c%c|",blockPattern[patA][0][0],blockPattern[patA][0][1],blockPattern[patA][0][2],blockPattern[patB][0][0],blockPattern[patB][0][1],blockPattern[patB][0][2]);
    mvaddstr(11,COLS/2+3,msg);
    sprintf(msg,"|%c%c%c|  |%c%c%c|",blockPattern[patA][1][0],blockPattern[patA][1][1],blockPattern[patA][1][2],blockPattern[patB][1][0],blockPattern[patB][1][1],blockPattern[patB][1][2]);
    mvaddstr(12,COLS/2+3,msg);
    sprintf(msg,"|%c%c%c|  |%c%c%c|",blockPattern[patA][2][0],blockPattern[patA][2][1],blockPattern[patA][2][2],blockPattern[patB][2][0],blockPattern[patB][2][1],blockPattern[patB][2][2]);
    mvaddstr(13,COLS/2+3,msg);
    sprintf(msg,"+---+  +---+");
    mvaddstr(14,COLS/2+3,msg);
  
    if(gameover(buffer)){
      for(i=0;i<256;i++){
	for(j=0;j<256;j++){
	  mvaddch(j,i,' ');
	  
	}
      }
      sprintf(msg, "GAME OVER");
      mvaddstr(3,S_SIZE_RIGHT,msg);
      
      
      
      sprintf(msg,"exit is 'Q'");
      mvaddstr(6,S_SIZE_RIGHT,msg);
      //  break;
    }
  }
}
