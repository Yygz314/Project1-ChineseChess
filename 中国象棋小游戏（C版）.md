﻿# 中国象棋小游戏（C版）

> 说明：
>
> - `#include<graphics.h>` 一个在 C/C++ 中用于图形编程的头文件，主要用于创建和操作图形界面。具有绘制图形、设置颜色、鼠标和键盘时间处理等功能。
> - `#include<conio.h>` 提供了对控制台输入/输出的简单操作，如字符读取和屏幕刷新。
> - `#include<windows.h>` 包含了 Windows API 的各种函数和数据结构定义，允许程序直接调用 Windows 操作系统的功能。可用于床咯创建于管理、进程和线程操作、文件操作、系统信息获取等。
> - `#include<mmsystem.h>` 提供了多媒体功能，包括音频和定时器功能。同时需要链接`winmm.lib`库，通过`#pragma comment(lib,"winmm.lib")`实现

## 整体思路

1. 创建图形窗口，绘制中国象棋棋局图案
2. 定义棋子，并在棋局上绘制初始化棋子
3. 实现游戏控制功能。即可通过鼠标操作实现棋子的移动以及红黑双方的交换操作
4. 添加棋子的走法规则，将军的判定以及胜负判定
5. 添加背景音乐、走棋音效等
6. 添加红黑双方计时功能
7. 打包软件

## 实现过程

1. 创建图形窗口，绘制中国象棋棋局图案。

   ​	我们先看此步骤完成后的结果。

   ![image-20250428221426901](https://img2023.cnblogs.com/blog/3610883/202505/3610883-20250504213355977-673563499.png)

   ​	只要对`graphics.h`有所了解，创建图形窗口并不困难。	

   ​	我们首先在主函数中直接初始化图形窗口。

   ```C
   #include<stdio.h>
   #include<graphics.h>
   int main(){
   	initgraph(800,800);
   }
   ```

   ​	接下来我们需要定义一个绘制游戏的函数。首先我们可以定义一个行数、列数、间隔以及棋盘格子大小

   ```C
   #define INTERVAL 50 // 定义间隔
   #define CHESS_GRID_SIZE 70 // 定义棋盘格子大小
   #define ROW 10 // 定义行数
   #define COL 9 // 定义列数
   ```

   ​	接下来绘制棋局的格子

   ```C
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
   }
   ```

   ​	接着显示“楚河汉界”文字

   ```C
   void GameDraw(){
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
   }
   ```

   ​	最后画米字完成第一步。

   ```C
   void GameDraw(){
   	//画米字
   	line(3 * CHESS_GRID_SIZE + INTERVAL, INTERVAL, 5 * CHESS_GRID_SIZE + INTERVAL, 2 * CHESS_GRID_SIZE + INTERVAL);
   	line(5 * CHESS_GRID_SIZE + INTERVAL, INTERVAL, 3 * CHESS_GRID_SIZE + INTERVAL, 2 * CHESS_GRID_SIZE + INTERVAL);
   	line(3 * CHESS_GRID_SIZE + INTERVAL, 7 * CHESS_GRID_SIZE + INTERVAL, 5 * CHESS_GRID_SIZE + INTERVAL, 9 * CHESS_GRID_SIZE + INTERVAL);
   	line(5 * CHESS_GRID_SIZE + INTERVAL, 7 * CHESS_GRID_SIZE + INTERVAL, 3 * CHESS_GRID_SIZE + INTERVAL, 9 * CHESS_GRID_SIZE + INTERVAL);  
   }
   ```

2. 定义棋子，并在棋局上绘制初始化棋子

   ​	我们先看此步骤完成后的结果。

   ![image-20250429131204095](https://img2023.cnblogs.com/blog/3610883/202505/3610883-20250504213355180-264358058.png)

​		根据中国象棋游戏规则，棋子分为红黑双方，不同棋子有不同的走法和不同的过河标准。

​		于是我们首先需要定义红黑双方的棋子名称，这里使用两个指向常量的指针数组分别代表红方棋子和黑方棋子。（用指针数组存储每个棋子名称字符串的起始地址，且是只读模式。更灵活高效且易于修改和维护）

```C
const char* redChess[7] = { "車","馬","相","仕","帥","炮","兵" };	// 红方棋子
const char* blackChess[7] = { "车","马","象","士","将","砲","卒" };	// 黑方棋子
```

​		每个棋子的位置由横纵坐标确定，且每个棋子都有名称、坐标、红黑方、是否过河等属性。于是我们定义一个棋子结构体如下：

```C
//定义棋子结构体
struct Chess {
	char name[4];	// 棋子名称一个汉字
	int x;	// 棋子x坐标
	int y;	// 棋子y坐标
	char type;	// 棋子类型(红方还是黑方)
	bool flag;	// 棋子是否过河；
}map[ROW][COL];
```

​		接下来就需要对结构体进行初始化，在相应的位置放入相应的棋子。

```C
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
}
```

​		最后，我们在绘制图形函数中继续添加关于绘制棋子的代码，同时这里以圆圈代表一个棋子。

```C
void DameDraw(){
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

			}
		}
	}   
}
```
3. 实现游戏控制功能。即可通过鼠标操作实现棋子的移动以及红黑双方的交换操作。

   ​	我们先看此步骤完成后的结果。

   ![image-20250429151241329](https://img2023.cnblogs.com/blog/3610883/202505/3610883-20250504213354574-2079430844.png)

   ​	在实现此功能前我们需要了解`#include<windows.h>`头文件。

   - `MOUSEMSG`：是该头文件下的一个结构体，有以下成员

   | 成员变量      | 类型   | 描述                                                         |
   | ------------- | ------ | ------------------------------------------------------------ |
   | `uMsg`        | `uint` | 鼠标消息类型，例如 `WM_MOUSEMOVE`（鼠标移动）、`WM_LBUTTONDOWN`（左按下键）、`WM_RBUTTONDOWN`（右键按下）等。 |
   | `x`           | `int`  | 鼠标事件发生时的 X 坐标。                                    |
   | `y`           | `int`  | 鼠标事件发生时的 Y 坐标。                                    |
   | `time`        | `uint` | 鼠标事件发生时的时间戳。                                     |
   | `dwExtraInfo` | `uint` | 额外信息，通常用于区分相同消息的不同实例。                   |

   ​	我们首先做出如下定义：

   ```C
   POINT begin = { -1,-1 }, end = { -1,-1 };	// 记录前后两次点击的坐标
   MOUSEMSG msg;
   bool isRedTurn = true; // 红方先手
   ```

   ​	接下来实现游戏控制功能。

   ​	首先是检测鼠标的点击

   ```C
   if (MouseHit) {                 // 检测是否有鼠标事件
       msg = GetMouseMsg();        // 获取鼠标消息
       if (msg.uMsg == WM_LBUTTONDOWN) {  // 判断是否为左键按下事件
           // ...后续处理...
       }
   }
   ```

   ​	再转换鼠标坐标为棋盘坐标以及检查点击位置是否合法

   ```C
   int k = (msg.x - INTERVAL + CHESS_GRID_SIZE / 2) / CHESS_GRID_SIZE;
   int i = (msg.y - INTERVAL + CHESS_GRID_SIZE / 2) / CHESS_GRID_SIZE;
   
   if (k < 0 || k >= COL || i < 0 || i >= ROW) return; // 超出棋盘范围则退出
   ```

   ​	接下来是第一次点击选择棋子

   ```C
   if (begin.x == -1) {  // 首次点击，选择起点
       // 必须选择己方棋子
       if ((isRedTurn && map[i][k].type == 'R') || (!isRedTurn && map[i][k].type == 'B')) {
           begin.x = k;
           begin.y = i;
       }
   }
   ```

   ​	最后是第二次点击移动棋子

   ```C
   else {  // 第二次点击，选择终点
       end.x = k;
       end.y = i;
   
       // 检查目标位置是否为空（未完成逻辑）
       int flagg = 0;
       if (strcmp(map[end.y][end.x].name, "") == 0)  flagg++;
   
       // 移动棋子到目标位置
       strcpy(map[end.y][end.x].name, map[begin.y][begin.x].name);
       map[end.y][end.x].type = map[begin.y][begin.x].type;
       map[end.y][end.x].flag = map[begin.y][begin.x].flag;
   
       // 清空起点位置
       strcpy(map[begin.y][begin.x].name, "");
   
       // 切换回合
       isRedTurn = !isRedTurn;
   
       // 重置起点
       begin.x = -1;
   }
   ```

   ​	完整的函数如下：

   ```C
   void GameControl() {
   	if (MouseHit {
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
   					begin.x = k;
   					begin.y = i;
   				}
   			}else {	//第二次点击
   				end.x = k;
   				end.y = i;
   				//执行移动
   				int flagg = 0;
   				if (strcmp(map[end.y][end.x].name, "") == 0)  flagg++;
   				//移动棋子
   				strcpy(map[end.y][end.x].name, map[begin.y][begin.x].name);
   				map[end.y][end.x].type = map[begin.y][begin.x].type;
   				map[end.y][end.x].flag = map[begin.y][begin.x].flag;
   				//清空原位置
   				strcpy(map[begin.y][begin.x].name, "");
   				//交换走棋方
   				isRedTurn = !isRedTurn;
   				begin.x = -1;	//重置选择
   			}
   		}
   	}
   }
   ```

   ​	最后，我们继续在绘制函数中添加对棋子的选中效果

   ```C
   void GameDraw(){
       //绘制选中效果
       if (i == begin.y && k == begin.x) {
           setlinecolor(BLUE);
           circle(map[i][k].x, map[i][k].y, 30);
           circle(map[i][k].x, map[i][k].y, 32);
       }    
   }
   ```

4. 添加棋子的走法规则，将军的判定以及胜负判定

   ​	这一步是整个象棋游戏的关键步骤。我们需要定义一个检验函数，对每一个棋子进行走法检验。

   首先定义该检验函数：

   ```C
   bool CheckMove(int fromI, int fromK, int toI, int toK) {	}
   ```

   - 第一步需要规定禁止吃己方棋子：

     ​	这个不能实现，只需要当判断目标位置存在棋子且该棋子的阵营属性和移动的棋子相同，则禁止移动即可。

     ```C
     struct Chess fromChess = map[fromI][fromK];
     struct Chess toChess = map[toI][toK];
     
     if (toChess.type == fromChess.type && strcmp(toChess.name, "") != 0)
         return false;
     ```

   - 第二步检验车的走法

     - 车的走法就是必须横向或纵向移动（`fromI == toI` 或 `fromK == toK`）
     - 路径上所经过的格子都必须为空即可。

     ```C
     if (strcmp(fromChess.name, "车") == 0 || strcmp(fromChess.name, "車") == 0) {
         // 必须直线移动
         if (fromI != toI && fromK != toK) return false;
     
         // 计算步长和距离
         int step = (fromI == toI) ? (toK > fromK ? 1 : -1) : (toI > fromI ? 1 : -1);
         int distance = (fromI == toI) ? abs(toK - fromK) : abs(toI - fromI);
     
         // 检查路径是否被阻挡
         for (int i = 1; i < distance; i++) {
             int x = (fromI == toI) ? fromI : fromI + step * i;
             int y = (fromI == toI) ? fromK + step * i : fromK;
             if (strcmp(map[x][y].name, "") != 0) return false;
         }
         return true;
     }
     ```

   - 第三步检验马的走法

     - 马的移动方式是日字格（横向1格，纵向2格，或横向2格，纵向1格）。
     - 判断其移动是否存在蹩马脚的情况（即相邻格子必须为空）

     ```C
     else if (strcmp(fromChess.name, "马") == 0 || strcmp(fromChess.name, "馬") == 0) {
         int dx = abs(toK - fromK);
         int dy = abs(toI - fromI);
         // 必须走日字
         if (!((dx == 1 && dy == 2) || (dx == 2 && dy == 1))) return false;
     
         // 检查蹩马腿
         int blockX = fromI + (toI - fromI) / 2;
         int blockY = fromK + (toK - fromK) / 2;
         if (strcmp(map[blockX][blockY].name, "") != 0) return false;
     
         return true;
     }
     ```

   - 第四步检验象的走法

     - 象的移动是田字格（横向和纵向均移动2格）
     - 田字中间必须为空。
     - 象无法过河。

     ```C
     else if (strcmp(fromChess.name, "相") == 0 || strcmp(fromChess.name, "象") == 0) {
         int dx = abs(toK - fromK);
         int dy = abs(toI - fromI);
         // 必须走田字
         if (dx != 2 || dy != 2) return false;
     
         // 检查田字中心是否被阻挡
         int centerX = (fromI + toI) / 2;
         int centerY = (fromK + toK) / 2;
         if (strcmp(map[centerX][centerY].name, "") != 0) return false;
     
         // 不能过河
         if (fromChess.type == 'B' && toI > 4) return false; // 黑象不能过楚河
         if (fromChess.type == 'R' && toI < 5) return false; // 红象不能过汉界
     
         return true;
     }
     ```

   - 第五步检验士的走法

     - 士在己方九宫格内移动
     - 每次只能斜向移动一格（横向和纵向均移动1格）

     ```C
     else if (strcmp(fromChess.name, "仕") == 0 || strcmp(fromChess.name, "士") == 0) {
         // 九宫格范围检查
         if (fromChess.type == 'B') {
             if (toI > 2 || toK < 3 || toK > 5) return false; // 黑方九宫格
         } else {
             if (toI < 7 || toK < 3 || toK > 5) return false; // 红方九宫格
         }
     
         // 斜向移动一格
         return (abs(toK - fromK) == 1 && abs(toI - fromI) == 1);
     }
     ```

   - 第六步检验将的走法

     - 只能在己方九宫格内移动
     - 每次只能横向或纵向移动一格

     ```C
     else if (strcmp(fromChess.name, "帥") == 0 || strcmp(fromChess.name, "将") == 0) {
         // 九宫格范围检查
         if (fromChess.type == 'B') {
             if (toI > 2 || toK < 3 || toK > 5) return false;
         } else {
             if (toI < 7 || toK < 3 || toK > 5) return false;
         }
     
         // 直线移动一步
         if ((abs(toK - fromK) + abs(toI - fromI)) != 1) return false;
         return true;
     }
     ```

   - 第七步检验炮的走法

     - 必须直线移动
     - 若目标为空，路径上不能有任何子
     - 若目标位敌方棋子，路径上必须恰好有一个棋子

     ```C
     else if (strcmp(fromChess.name, "砲") == 0 || strcmp(fromChess.name, "炮") == 0) {
         // 必须直线移动
         if (fromI != toI && fromK != toK) return false;
     
         int count = 0;
         int step = (fromI == toI) ? (toK > fromK ? 1 : -1) : (toI > fromI ? 1 : -1);
         int distance = (fromI == toI) ? abs(toK - fromK) : abs(toI - fromI);
     
         // 统计路径上的棋子数
         for (int i = 1; i < distance; i++) {
             int x = (fromI == toI) ? fromI : fromI + step * i;
             int y = (fromI == toI) ? fromK + step * i : fromK;
             if (strcmp(map[x][y].name, "") != 0) count++;
         }
     
         // 目标为空则路径必须无子，否则必须隔一子
         if (strcmp(toChess.name, "") == 0) {
             return (count == 0);
         } else {
             return (count == 1);
         }
     }
     ```

   - 第八步检验兵的走法

     - 未过河
       - 只能向前移动一格。
       - 过河后更新标志位。
     - 已过河
       - 可向前或横向移动一格。

     ```C
     else if (strcmp(fromChess.name, "卒") == 0 || strcmp(fromChess.name, "兵") == 0) {
         int direction = (fromChess.type == 'B') ? 1 : -1; // 黑卒向下，红兵向上
         // 未过河时只能前进
         if (!fromChess.flag) {
             if (toI != fromI + direction || toK != fromK) return false;
             // 更新过河状态
             if ((fromChess.type == 'B' && toI >= 5) || (fromChess.type == 'R' && toI <= 4)) {
                 map[fromI][fromK].flag = true; // 标记为已过河
             }
         } else { // 过河后可左右或前进
             bool valid = false;
             if (toI == fromI + direction && toK == fromK) valid = true; // 前进
             if (toI == fromI && abs(toK - fromK) == 1) valid = true;    // 横向移动
             return valid;
         }
         return true;
     }
     ```
     
     ​	综上，我们将`CheckMove()`函数插入游戏控制函数中，当且仅当移动合法时可对棋子进行移动。
     
     ​	接下来定义一个将军状态检测函数。即当一方走棋后，检验接下来走棋一方是否处于被将军状态。
     
     ```C
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
     ```
     
     ​	接下来我们进行胜利条件判断。
     
     ​	当将帅面对面或者将帅被吃时，游戏结束。
     
     ```C
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
     ```
     
     综上，我们将这些函数插入游戏控制函数内即可。

5. 添加背景音乐、走棋音效等

   ​	这一步很简单，直接给出代码。

   ```C
   //定义音效资源
   MCI_OPEN_PARMS openBGM;
   DWORD bgmld;
   bool isMusicPlaying = false;
   //音频初始化
   void SoundInit() {
   	//加载背景音乐
   	mciSendString("open \"./sounds/bgm1.wav\" type mpegvideo alias bgm", NULL, 0, NULL);
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
   ```

   

6. 添加红黑双方计时功能

   ​	我们先看此步骤完成后的结果。

   ![image-20250501211644331](https://img2023.cnblogs.com/blog/3610883/202505/3610883-20250504213353620-829414831.png)

   ​	这里我们只是实现最简单的计时功能，即双方步时60秒，总时长10分钟。

   ​	首先我们需要定义时间结构体包括总时间和当前步时。

   ```C
   //定义时间结构
   struct Timer {
   	int totalTime;	//总剩余时间（秒）
   	int stepTime;	//当前步剩余时间（秒）
   }redTimer, blackTimer;
   
   
   DWORD lastUpdateTime = 0;  // 记录上次更新时间戳（毫秒）
   const int INIT_TOTAL_TIME = 600; // 初始总时间（10分钟）
   const int INIT_STEP_TIME = 60;   // 初始步时（60秒）
   ```

   ​	接着我们在游戏初始化函数中添加初始化计时器。

   ```C
   void GameInit(){
   	// 初始化计时器
   	redTimer.totalTime = INIT_TOTAL_TIME;
   	redTimer.stepTime = INIT_STEP_TIME;
   	blackTimer.totalTime = INIT_TOTAL_TIME;
   	blackTimer.stepTime = INIT_STEP_TIME;
   	lastUpdateTime = GetTickCount(); // 获取当前系统时间    
   }
   ```

   ​	在游戏控制函数中添加双方交换时步时的重置

   ```C
   void GameControl(){
   	// 重置当前玩家的步时
       if (isRedTurn) {
       	redTimer.stepTime = INIT_STEP_TIME;
       }
       else {
       	blackTimer.stepTime = INIT_STEP_TIME;
       }  
   }
   ```

   ​	然后我们在游戏绘制函数中将时间显示在棋盘右侧。

   ```C
   void GameGraw(){
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
   ```

   ​	最后，在主函数中实现对时间的更新。

   ​	再结合之前的函数，我们写出主函数即可实现中国象棋的最简化版。

   ```C
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
   	}
   	EndBatchDraw();	// 结束绘制
   	closegraph();	// 关闭图形窗口
   	mciSendString("close all", NULL, 0, NULL); // 关闭所有音频
   	return 0;
   }
   ```
   
   
   
   ---
   
   ​    到这里中国象棋的基本功能我们就实现了。由于这是本作者的第一个项目有点小激动，于是准备先进性项目打包处理。

​	   这里使用传统打包方式，使用Microsoft Visual Studio Installer Projects打包。

1. 安装扩展插件
   - 在VS中安装**Microsoft Visual Studio Installer Projects**扩展
   - 操作路径：扩展 > 管理扩展 > 搜索安装 > 重启VS
2. 创建安装项目
   - 右键解决方案>添加>新建项目
   - 搜索选择"setup Project"模板
   - 配置项目名称
3. 配置安装内容
   - 主程序添加：右键Application Folder>Add>项目输出>选择主输出
   - 资源文件处理：将"sound"音效文件夹拖入Application Folder（确保所有依赖的DLL被包含）
4. 配置快捷方式
   - 右键Application>Add>文件>选择所需的快捷方式图案.ico文件（图片转换成.ico格式可以通过PS转换，需要提前配置[插件](https://gitcode.com/Resource-Bundle-Collection/330f3))
   - 右键主输出>创建快捷方式
   - 将快捷方式拖入User's Desktop和User's Programs Menu
   - 右键快捷方式>属性窗口>Icon>Browse>Browse>选择Application Folder文件中的ico文件>OK
5. 处理依赖项
   - 右键Application Folder>Add>文件>找到vcredist_x64.exe(路径：VS安装目录\VC\Redist\MSVC\版本号)
6. 生成安装包
   - 右键安装项目>生成
   - 在输出目录即可获取Setup.exe和.msi文件

---





































