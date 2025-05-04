#include<stdio.h>
#include<graphics.h>	// 引用图形库
#include<conio.h>	// 引用控制台输入输出
#include<stdbool.h> // 引用bool类型
#include<windows.h> // 引用windows.h头文件
#include<mmsystem.h> // 引用鼠标事件
#pragma comment(lib,"winmm.lib") // 引用winmm.lib库
#define INTERVAL 50 // 定义间隔
#define CHESS_GRID_SIZE 70 // 定义棋盘格子大小
#define ROW 10 // 定义行数
#define COL 9 // 定义列数


//定义音效资源
MCI_OPEN_PARMS openBGM;
DWORD bgmld;
bool isMusicPlaying = false;
//音频初始化
void SoundInit() {
	//加载背景音乐
	mciSendString("open \"./sounds/bgm1.mp3\" type mpegvideo alias bgm", NULL, 0, NULL);
	//预加载音效
	mciSendString("open \"./sounds/click.wav\" alias click", NULL, 0, NULL);
	mciSendString("open \"./sounds/move.wav\" alias move", NULL, 0, NULL);
	mciSendString("open \"./sounds/eat.wav\" alias eat", NULL, 0, NULL);
	mciSendString("open \"./sounds/check.wav\" alias check", NULL, 0, NULL);
}

//播放背景音乐
void PlayBGM() {
	mciSendString("play bgm repeat", NULL, 0, NULL);
	// 设置主音量（范围0-1000）
	mciSendString("setaudio bgm volume to 1000", NULL, 0, NULL);
	isMusicPlaying = true;
}

//音效播放函数
void PlaySoundEffect(const char* alias) {
	char cmd[50];
	sprintf(cmd, "play %s from 0", alias);
	mciSendString(cmd, NULL, 0, NULL);
}

//定义时间结构
struct Timer {
	int totalTime;	//总剩余时间（秒）
	int stepTime;	//当前步剩余时间（秒）
}redTimer, blackTimer;
DWORD lastUpdateTime = 0;  // 记录上次更新时间戳（毫秒）
const int INIT_TOTAL_TIME = 600; // 初始总时间（10分钟）
const int INIT_STEP_TIME = 60;   // 初始步时（60秒）

//定义棋子结构体
struct Chess {
	char name[4];	// 棋子名称一个汉字
	int x;	// 棋子x坐标
	int y;	// 棋子y坐标
	char type;	// 棋子类型(红方还是黑方)
	bool flag;	// 棋子是否过河；
}map[ROW][COL];

const char* redChess[7] = { "車","馬","相","仕","帥","炮","兵" };	// 红方棋子
const char* blackChess[7] = { "车","马","象","士","将","砲","卒" };	// 黑方棋子

POINT begin = { -1,-1 }, end = { -1,-1 };	// 记录前后两次点击的坐标
MOUSEMSG msg;
bool isRedTurn = true; // 红方先手

// 定义走法结构体
struct Move {
	int fromI, fromK; // 起始位置
	int toI, toK;     // 目标位置
};

//函数声明
void GameInit();	// 游戏初始化
void GameDraw();	// 游戏绘制
void GameControl();	// 游戏控制
bool CheckMove(int fromI, int fromK, int toI, int toK);	// 检查棋子移动是否合法
bool CheckWin();	// 检查是否胜利


//游戏初始化
void GameInit() {

	//遍历二维数组
	for (int i = 0;i < ROW;i++) {
		int temp = 0, temp1 = 0, temp2 = 1;
		for (int k = 0;k < COL;k++) {
			char chessname[4] = ""; // 定义棋子名称
			char mcolor = 'B';	// 定义棋子颜色黑色
			if (i <= 4) {	// 黑棋初始化
				if (i == 0) {	//第一行棋子初始化
					if (temp <= 4) temp++;
					else {
						temp1 = 4 - temp2;
						temp2++;
					}
					sprintf(chessname, "%s", blackChess[temp1]);
					temp1++;
				}
				//设置砲
				if (i == 2 && (k == 1 || k == 7)) {
					strcpy(chessname, blackChess[5]);
				}
				//设置卒
				if (i == 3 && (k % 2 == 0)) {
					strcpy(chessname, blackChess[6]);
				}
			}
			else {
				mcolor = 'R';	// 定义棋子颜色红色
				if (i == 9) {	//最后一行棋子初始化
					if (temp <= 4) temp++;
					else {
						temp1 = 4 - temp2;
						temp2++;
					}
					sprintf(chessname, "%s", redChess[temp1]);
					temp1++;
				}
				//设置炮
				if (i == 7 && (k == 1 || k == 7)) {
					strcpy(chessname, redChess[5]);
				}
				//设置兵
				if (i == 6 && (k % 2 == 0)) {
					strcpy(chessname, redChess[6]);
				}
			}
			map[i][k].type = mcolor;
			strcpy(map[i][k].name, chessname);
			map[i][k].flag = false;
			map[i][k].x = k * CHESS_GRID_SIZE + INTERVAL;
			map[i][k].y = i * CHESS_GRID_SIZE + INTERVAL;
		}
	}
	// 初始化计时器
	redTimer.totalTime = INIT_TOTAL_TIME;
	redTimer.stepTime = INIT_STEP_TIME;
	blackTimer.totalTime = INIT_TOTAL_TIME;
	blackTimer.stepTime = INIT_STEP_TIME;
	lastUpdateTime = GetTickCount(); // 获取当前系统时间
}

//游戏绘制
void GameDraw() {
	setbkcolor(RGB(252, 215, 162));	// 设置背景色
	cleardevice();	// 清屏
	setlinecolor(BLACK); // 设置线条颜色
	setlinestyle(PS_SOLID, 2); // 设置线条样式
	setfillcolor(RGB(252, 215, 162));
	//绘制外边框
	fillrectangle(INTERVAL - 5, INTERVAL - 5, CHESS_GRID_SIZE * 8 + INTERVAL + 5, CHESS_GRID_SIZE * 9 + INTERVAL + 5);
	// 绘制棋盘
	for (int i = 0;i < 10;i++) {
		//画横线
		line(INTERVAL, i * CHESS_GRID_SIZE + INTERVAL, CHESS_GRID_SIZE * 8 + INTERVAL, i * CHESS_GRID_SIZE + INTERVAL);
		//画竖线
		if (i < 9) {
			line(i * CHESS_GRID_SIZE + INTERVAL, INTERVAL, i * CHESS_GRID_SIZE + INTERVAL, 9 * CHESS_GRID_SIZE + INTERVAL);
		}
	}
	//显示楚河，汉界
	fillrectangle(INTERVAL, 4 * CHESS_GRID_SIZE + INTERVAL, 8 * CHESS_GRID_SIZE + INTERVAL, 5 * CHESS_GRID_SIZE + INTERVAL);
	//显示文字
	settextcolor(BLACK);
	settextstyle(50, 0, "楷体");
	char river[25] = "楚 河        汉 界";
	//让文字居中
	int twidth = textwidth(river);
	int theight = textheight(river);
	twidth = (8 * CHESS_GRID_SIZE - twidth) / 2;
	theight = (CHESS_GRID_SIZE - theight) / 2;
	outtextxy(INTERVAL + twidth, 4 * CHESS_GRID_SIZE + theight + INTERVAL, river);
	//画米字
	line(3 * CHESS_GRID_SIZE + INTERVAL, INTERVAL, 5 * CHESS_GRID_SIZE + INTERVAL, 2 * CHESS_GRID_SIZE + INTERVAL);
	line(5 * CHESS_GRID_SIZE + INTERVAL, INTERVAL, 3 * CHESS_GRID_SIZE + INTERVAL, 2 * CHESS_GRID_SIZE + INTERVAL);
	line(3 * CHESS_GRID_SIZE + INTERVAL, 7 * CHESS_GRID_SIZE + INTERVAL, 5 * CHESS_GRID_SIZE + INTERVAL, 9 * CHESS_GRID_SIZE + INTERVAL);
	line(5 * CHESS_GRID_SIZE + INTERVAL, 7 * CHESS_GRID_SIZE + INTERVAL, 3 * CHESS_GRID_SIZE + INTERVAL, 9 * CHESS_GRID_SIZE + INTERVAL);
	//绘制棋子
	settextstyle(40, 0, "楷体");  // 设置字体大小
	for (int i = 0;i < ROW;i++) {
		for (int k = 0;k < COL;k++) {
			if (strcmp(map[i][k].name, "") != 0) {
				//绘制棋子
				if (map[i][k].type == 'B') {
					settextcolor(BLACK);
					setlinecolor(BLACK);
				}
				else {
					settextcolor(RED);
					setlinecolor(RED);
				}
				fillcircle(map[i][k].x, map[i][k].y, 30);
				outtextxy(map[i][k].x - 20, map[i][k].y - 20, map[i][k].name);
				//绘制选中效果
				if (i == begin.y && k == begin.x) {
					static int blink = 0;
					setlinecolor(BLUE);
					circle(map[i][k].x, map[i][k].y, 30);
					circle(map[i][k].x, map[i][k].y, 32);
				}
			}
		}
	}
	//设置文字样式
	settextstyle(20, 0, "楷体");
	settextcolor(BLACK);

	// 绘制计时器
	settextstyle(20, 0, "楷体");
	settextcolor(BLACK);
	char redTimeStr1[50], redTimeStr2[50],blackTimeStr1[50], blackTimeStr2[50];
	// 格式：总时间 MM:SS 步时 SS
	sprintf(redTimeStr1, "红方: 总 %02d:%02d",
		redTimer.totalTime / 60, redTimer.totalTime % 60);
	sprintf(redTimeStr2, "    步时: %02d",redTimer.stepTime);
	sprintf(blackTimeStr1, "黑方: 总 %02d:%02d",
		blackTimer.totalTime / 60, blackTimer.totalTime % 60);
	sprintf(blackTimeStr2, "    步时: %02d", blackTimer.stepTime);
	outtextxy(650, 650, redTimeStr1);
	outtextxy(650, 675, redTimeStr2);
	outtextxy(650, 50, blackTimeStr1);
	outtextxy(650, 75, blackTimeStr2);

}

// 移动验证函数
bool CheckMove(int fromI, int fromK, int toI, int toK) {
	// 获取棋子信息
	struct Chess fromChess = map[fromI][fromK];
	struct Chess toChess = map[toI][toK];

	// 不能吃己方棋子
	if (toChess.type == fromChess.type && strcmp(toChess.name, "") != 0)
		return false;

	// 根据棋子类型验证规则
	if (strcmp(fromChess.name, "车") == 0 || strcmp(fromChess.name, "車") == 0) { // 车
		if (fromI != toI && fromK != toK) return false;
		int step = (fromI == toI) ? (toK > fromK ? 1 : -1) : (toI > fromI ? 1 : -1);
		int distance = (fromI == toI) ? abs(toK - fromK) : abs(toI - fromI);
		for (int i = 1; i < distance; i++) {
			int x = (fromI == toI) ? fromI : fromI + step * i;
			int y = (fromI == toI) ? fromK + step * i : fromK;
			if (strcmp(map[x][y].name, "") != 0) return false;
		}
		return true;
	}
	else if (strcmp(fromChess.name, "马") == 0 || strcmp(fromChess.name, "馬") == 0) { // 马
		int dx = abs(toK - fromK);
		int dy = abs(toI - fromI);
		if (!((dx == 1 && dy == 2) || (dx == 2 && dy == 1))) return false;
		// 检查蹩马腿
		int blockX = fromI + (toI - fromI) / 2;
		int blockY = fromK + (toK - fromK) / 2;
		if (strcmp(map[blockX][blockY].name, "") != 0) return false;
		return true;
	}
	else if (strcmp(fromChess.name, "相") == 0 || strcmp(fromChess.name, "象") == 0) { // 相/象
		int dx = abs(toK - fromK);
		int dy = abs(toI - fromI);
		if (dx != 2 || dy != 2) return false;
		// 检查田字中心
		int centerX = (fromI + toI) / 2;
		int centerY = (fromK + toK) / 2;
		if (strcmp(map[centerX][centerY].name, "") != 0) return false;
		// 不能过河
		if (fromChess.type == 'B' && toI > 4) return false;
		if (fromChess.type == 'R' && toI < 5) return false;
		return true;
	}
	else if (strcmp(fromChess.name, "仕") == 0 || strcmp(fromChess.name, "士") == 0) { // 士
		// 只能在九宫格斜走
		if (fromChess.type == 'B') {
			if (toI > 2 || toK < 3 || toK > 5) return false;
		}
		else {
			if (toI < 7 || toK < 3 || toK > 5) return false;
		}
		return (abs(toK - fromK) == 1 && abs(toI - fromI) == 1);
	}
	else if (strcmp(fromChess.name, "帥") == 0 || strcmp(fromChess.name, "将") == 0) { // 将帥
		// 九宫格内移动
		if (fromChess.type == 'B') {
			if (toI > 2 || toK < 3 || toK > 5) return false;
		}
		else {
			if (toI < 7 || toK < 3 || toK > 5) return false;
		}
		// 直线移动一步
		if ((abs(toK - fromK) + abs(toI - fromI)) != 1) return false;
		return true;
	}
	else if (strcmp(fromChess.name, "砲") == 0 || strcmp(fromChess.name, "炮") == 0) { // 炮
		if (fromI != toI && fromK != toK) return false;
		int count = 0;
		int step = (fromI == toI) ? (toK > fromK ? 1 : -1) : (toI > fromI ? 1 : -1);
		int distance = (fromI == toI) ? abs(toK - fromK) : abs(toI - fromI);
		for (int i = 1; i < distance; i++) {
			int x = (fromI == toI) ? fromI : fromI + step * i;
			int y = (fromI == toI) ? fromK + step * i : fromK;
			if (strcmp(map[x][y].name, "") != 0) count++;
		}
		// 目标为空则路径无子，否则必须隔一子
		if (strcmp(toChess.name, "") == 0) return (count == 0);
		else return (count == 1);
	}
	else if (strcmp(fromChess.name, "卒") == 0 || strcmp(fromChess.name, "兵") == 0) { // 兵卒
		int direction = (fromChess.type == 'B') ? 1 : -1;
		// 未过河只能前进
		if (!fromChess.flag) {
			if (toI != fromI + direction || toK != fromK) return false;
			// 更新过河状态
			if ((fromChess.type == 'B' && toI >= 5) || (fromChess.type == 'R' && toI <= 4))
				map[fromI][fromK].flag = true;
		}
		else { // 过河后可左右移动
			bool valid = false;
			if (toI == fromI + direction && toK == fromK) valid = true;
			if (toI == fromI && abs(toK - fromK) == 1) valid = true;
			return valid;
		}
		return true;
	}
	return false;
}

// 检查将帅是否被攻击
bool CheckGeneral() {
	POINT generalPos = { -1,1 };
	char targetType = isRedTurn ? 'B' : 'R';
	const char* generalName = (targetType == 'B') ? "将" : "帥";

	// 查找己方将的位置
	for (int i = 0; i < ROW; ++i) {
		for (int j = 0; j < COL; ++j) {
			if (map[i][j].type == targetType &&
				strcmp(map[i][j].name, generalName) == 0) {
				generalPos.x = j;
				generalPos.y = i;
				break;
			}
		}
	}

	// 遍历所有敌方棋子检查是否可攻击将
	char enemyType = isRedTurn ? 'R' : 'B';
	for (int i = 0; i < ROW; ++i) {
		for (int j = 0; j < COL; ++j) {
			if (map[i][j].type == enemyType &&
				strcmp(map[i][j].name, "") != 0) {
				// 临时保存目标位置棋子
				struct Chess temp = map[generalPos.y][generalPos.x];

				// 模拟移动
				strcpy(map[generalPos.y][generalPos.x].name, map[i][j].name);
				bool canAttack = CheckMove(i, j, generalPos.y, generalPos.x);

				// 恢复现场
				map[generalPos.y][generalPos.x] = temp;

				if (canAttack) return true;
			}
		}
	}
	return false;
}

// 胜利条件判断
bool CheckWin() {
	bool redExist = false, blackExist = false;
	POINT redGeneral = { -1, -1 }, blackGeneral = { -1, -1 };

	// 遍历查找将帅位置
	for (int i = 0; i < ROW; i++) {
		for (int k = 0; k < COL; k++) {
			if (strcmp(map[i][k].name, "帥") == 0) {
				redExist = true;
				redGeneral.x = k;
				redGeneral.y = i;
			}
			if (strcmp(map[i][k].name, "将") == 0) {
				blackExist = true;
				blackGeneral.x = k;
				blackGeneral.y = i;
			}
		}
	}

	// 将帅被吃判断
	if (!redExist) {
		MessageBox(GetHWnd(), "黑方胜利！", "游戏结束", MB_OK);
		return true;
	}
	if (!blackExist) {
		MessageBox(GetHWnd(), "红方胜利！", "游戏结束", MB_OK);
		return true;
	}

	// 将帅对面判断（飞将规则）
	if (redGeneral.x == blackGeneral.x) {
		int minY = (redGeneral.y < blackGeneral.y) ? redGeneral.y : blackGeneral.y;
		int maxY = (redGeneral.y > blackGeneral.y) ? redGeneral.y : blackGeneral.y;
		bool hasBlock = false;

		// 检查中间是否有棋子
		for (int y = minY + 1; y < maxY; y++) {
			if (strcmp(map[y][redGeneral.x].name, "") != 0) {
				hasBlock = true;
				break;
			}
		}

		// 无遮挡且轮到对方走棋时判负
		if (!hasBlock) {
			// 当形成将帅对面时，由当前走棋方的对手获胜
			const char* winner = isRedTurn ? "黑方" : "红方";
			// 字符串处理方式
			char message[50];
			sprintf(message, "%s 胜利！将帅对面！", winner);
			MessageBox(GetHWnd(), message, "游戏结束", MB_OK);
			return true;
		}
	}

	return false;
}

void GameControl() {

	if (MouseHit) {
		msg = GetMouseMsg();
		if (msg.uMsg == WM_LBUTTONDOWN) {
			//转换为棋盘坐标
			int k = (msg.x - INTERVAL + CHESS_GRID_SIZE / 2) / CHESS_GRID_SIZE;
			int i = (msg.y - INTERVAL + CHESS_GRID_SIZE / 2) / CHESS_GRID_SIZE;
			//判断是否在棋盘内
			if (k < 0 || k >= COL || i < 0 || i >= ROW) return;
			if (begin.x == -1) {	//第一次点击
				//必须选择己方棋子
				if ((isRedTurn && map[i][k].type == 'R') || (!isRedTurn && map[i][k].type == 'B')) {
					PlaySoundEffect("click"); // 播放点击音效
					begin.x = k;
					begin.y = i;
				}
			}
			else {	//第二次点击
				end.x = k;
				end.y = i;
				//执行移动
				if (CheckMove(begin.y, begin.x, end.y, end.x)) {
					int flagg = 0;
					if (strcmp(map[end.y][end.x].name, "") == 0)  flagg++;
					//移动棋子
					strcpy(map[end.y][end.x].name, map[begin.y][begin.x].name);
					map[end.y][end.x].type = map[begin.y][begin.x].type;
					map[end.y][end.x].flag = map[begin.y][begin.x].flag;

					//清空原位置
					strcpy(map[begin.y][begin.x].name, "");
					// 将军状态检测
					bool currentCheck = CheckGeneral(); // 需要实现将军检测函数
					if (currentCheck) {
						flagg++;
						PlaySoundEffect("check"); // 播放将军音效
					}
					else if (flagg == 0) PlaySoundEffect("eat"); // 播放吃子音效
					else PlaySoundEffect("move"); // 播放移动音效
					//lastCheckState = currentCheck;

					//判断胜负
					if (CheckWin()) {
						closegraph();	// 关闭图形窗口
						exit(0);	// 退出程序
					}

					//交换走棋方
					isRedTurn = !isRedTurn;

					// 重置当前玩家的步时
					if (isRedTurn) {
						redTimer.stepTime = INIT_STEP_TIME;
					}
					else {
						blackTimer.stepTime = INIT_STEP_TIME;
					}

				}
				begin.x = -1;	//重置选择
			}
		}
	}
}


int main() {
	initgraph(800, 800, SHOWCONSOLE);	// 初始化图形窗口

	SoundInit();	// 初始化音效
	PlayBGM();	// 播放背景音乐
	GameInit();	// 初始化游戏

	while (true) {
		GameControl();  // 触发时间扣除

		// 更新时间逻辑（每秒更新一次）
		DWORD currentTime = GetTickCount();
		DWORD elapsed = currentTime - lastUpdateTime;
		if (elapsed >= 1000) { // 超过1秒
			int seconds = elapsed / 1000; // 计算经过的整秒数
			// 扣除当前玩家的总时间和步时
			if (isRedTurn) {
				redTimer.totalTime -= seconds;
				redTimer.stepTime -= seconds;
			}
			else {
				blackTimer.totalTime -= seconds;
				blackTimer.stepTime -= seconds;
			}

			lastUpdateTime = currentTime;

			// 检查超时
			if (redTimer.totalTime <= 0 || redTimer.stepTime <= 0) {
				MessageBox(GetHWnd(), "红方超时，黑方胜利！", "游戏结束", MB_OK);
				exit(0);
			}
			if (blackTimer.totalTime <= 0 || blackTimer.stepTime <= 0) {
				MessageBox(GetHWnd(), "黑方超时，红方胜利！", "游戏结束", MB_OK);
				exit(0);
			}
		}

		// 双缓冲绘制流程
		BeginBatchDraw();  // 开始批量绘制
		GameDraw();
		EndBatchDraw();    // 结束批量绘制

		Sleep(10); // 延时 防止CPU过高占用
	}

	EndBatchDraw();	// 结束绘制
	closegraph();	// 关闭图形窗口
	mciSendString("close all", NULL, 0, NULL); // 关闭所有音频
	return 0;
}



