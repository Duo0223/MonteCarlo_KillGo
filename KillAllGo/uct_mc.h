#ifndef _Prue_Mc_h_
#define _Prue_Mc_h_

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <fstream>
#include <vector>
#include <string>

#define BOARDSIZE        7
#define BOUNDARYSIZE     9
#define COMMANDLENGTH   10
#define DEFAULTTIME     10

#define MAXGAMELENGTH 1000
#define MAXSTRING       50
#define MAXDIRECTION     4

#define NUMINTERSECTION 49

#define EMPTY            0
#define BLACK            1
#define WHITE            2
#define BOUNDARY         3

#define SELF             1
#define OPPONENT         2

//#define EAT_SCORE		10
//#define WIN_SCORE		1000

#define WIN_SCORE		1.0f

#define MAX_STEP		75

using namespace std;

static const int DirectionX[ MAXDIRECTION ] = { -1 , 0 , 1 , 0 };
static const int DirectionY[ MAXDIRECTION ] = {  0 , 1 , 0 , -1 };

static const int NewDirectX[ MAXDIRECTION ] = { 1 , 1 , -1 , -1 };
static const int NewDirectY[ MAXDIRECTION ] = { 1 , -1  , 1 , -1 };


///< ============================== basic method ========================================

///< ==== 顯示 棋盤
void display(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int time_limit);

///< ==== 重置 棋盤
void reset(int Board[BOUNDARYSIZE][BOUNDARYSIZE]);

///< ==== 傳回 ( X , Y ) 這條 String 的所有 liberty
int find_liberty(int X, int Y, int label, int Board[BOUNDARYSIZE][BOUNDARYSIZE], 
	long long int ConnectBoard[BOUNDARYSIZE][BOUNDARYSIZE]);

///< === 計算 氣
void count_liberty(int X, int Y, int Board[BOUNDARYSIZE][BOUNDARYSIZE], int Liberties[MAXDIRECTION]);

///< === 記錄鄰近格點的狀態
void count_neighboorhood_state(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn, int* empt, int* self, 
	int* oppo ,int* boun, int NeighboorhoodState[MAXDIRECTION]);

///< === 提子
void remove_piece(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn);

///< === 更新棋盤
void update_board(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn);

///< === 更新棋盤，並會判斷是否為合法步
int update_board_check(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn);

///< === 產生合法步
int gen_legal_move(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int game_length, 
	int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE], int MoveList[NUMINTERSECTION]); 

///< === 從 合法步中 隨機挑選一步
int rand_pick_move(int num_legal_moves, int MoveList[NUMINTERSECTION]);

///< === 下棋
void do_move(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int move);

///< === 電腦 思考
int think(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int time_limit, int game_length, 
	int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]);

///< =========================================================================================


///< --- node struct
struct MCTreeNode
{      
	int type;										///< --- Node 類型 : BLACK or WHITE
	int visit;										///< --- 被拜訪次數
    int move;										///< --- 要走的步

    int board[ BOUNDARYSIZE ][ BOUNDARYSIZE ];		///< --- 盤面
	int simBoard[ BOUNDARYSIZE ][ BOUNDARYSIZE ];	///< --- node 模擬用的盤面

	vector<float>		scoreVec;					///< --- 所有的模擬分數 Vector
	float				score;						///< --- 分數

	float				dev;						///< --- 標準差
	float				mean;						///< --- 平均值

	float				meanL;						///< --- 左邊的 bound		
	float				meanR;						///< --- 右邊的 bound

	vector< MCTreeNode* >	childVec;				///< --- Children Vevtor
	MCTreeNode*				parent;					///< ---  parent 指標

	MCTreeNode( void ) 
	{
		type = visit = move = 0;
		score = dev = mean = meanL = meanR = 0.0f;
		childVec.clear();
		scoreVec.clear();
		parent = NULL;
	}
	MCTreeNode( int _Type ) 
	{
		type = _Type;
		visit = move = 0;
		score = dev = mean = meanL = meanR = 0.0f;
		childVec.clear();
		scoreVec.clear();
		parent = NULL;
	}
    ~MCTreeNode( void ) 
	{
		parent = NULL;
		scoreVec.clear();
		vector< MCTreeNode* >::iterator itr;
		for( itr = childVec.begin() ; itr != childVec.end() ; ++itr)
		{
			(*itr)->parent = NULL;
			delete (*itr);
			(*itr) = NULL;
		}
	}
};

///< --- 蒙地卡羅 tree
class MCTree
{
public :
	 MCTree( void );
	 ~MCTree( void );

	 ///< --- Search
	 /*
		_Time : 收尋時間
		_Board : 要做模擬的盤面
		_Turn : 目前是誰下 ( 黑 or 白 )
		_GameLength : 目前遊戲的時間長度
		_GameRecord : 歷史盤面
	 */
	int Search( int _Time , int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , int _GameLength ,
		int _GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE] );

	///< --- 計算棋盤中黑 or 白棋子個數	
	/*
		_Board : 要做判斷的棋盤
		_Turn : 黑 or 白
	*/
	int CountPieceNum( int _Board[BOUNDARYSIZE][BOUNDARYSIZE]  , int _Turn );

protected :

	///< --- 展開子點
	/*
		_Node : 要展開子點的 Node
	*/
	void Expand( MCTreeNode *_Node );


	///< --- 對進行模擬
	/*
		_Node : 要對其子點進行模擬的 Node
		_Idx : 要模擬的 Child
	*/
	void Simulation( MCTreeNode *_Node , int _Idx );


	///< --- 計算 Node 中每個子點的 UCB 值，並回傳有最好 UCB 值的 Node
	/*
		_Node : 要做計算的 Node
	*/
	MCTreeNode *GetBestChildByUCB( MCTreeNode *_Node );

	
	///< --- 計算每個 child 的平均值跟標準差，然後減掉分數太低的 Childe
	/*
		_Node : 計算的 Node
	*/
	bool PruningBadChild( MCTreeNode *_Node );

	
	///< --- 模擬結束時，將找到的 Node BackUp 回到 root
	/*
		_Node : 找到的 Node
	*/
	MCTreeNode *BackUp( MCTreeNode* _Node );


	///< --- Search 演算法的主要迴圈
	/*
		_Time : 模擬時間限制
	*/
	void DoLoop( int _Time );


private :

	///< --- Copy 棋盤
	/*
		_Src : 來源棋盤
		_Dis : 目標棋盤
	*/
	void CopyBoard( int _Src[BOUNDARYSIZE][BOUNDARYSIZE] , int _Dis[BOUNDARYSIZE][BOUNDARYSIZE] );


	///< --- 判斷 (X, Y) 是否為眼位
	/*
		_Board : 要做判斷的棋盤
		_Turn : 黑 or 白
		_X , _Y : 位置
	*/
	bool IsEye( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , int _X , int _Y );
	
	///< --- 判斷是否形成兩眼
	/*
		_Board : 要做判斷的棋盤
	*/
	bool IsTwoEye( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] );

	///< --- 判斷 (_X , _Y) 這一步是否需要捨棄掉
	/*
		_Board : 要做判斷的棋盤
		_Turn : 黑 or 白
		_X , _Y : 位置
	*/
	bool IsGiveup( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , int _X , int _Y );

	///< --- 換手
	/*
		_Turn : 現在的玩家
	*/
	int ChangeTurn( int _Turn );

	///< --- 判斷是否有子點，以及是否有子點是勝利點
	MCTreeNode* JudgeByNode( MCTreeNode *_Node );

	///< --- 判斷合法步中，對於目前棋盤是否有直接勝利的步
	int JudgeByMove( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , vector<int> &_MoveVec );

	///< --- MySqrt
	inline float MySqrt(float x)
	{
		float xhalf = 0.5f*x;
		int i = *(int*)&x; 
		i = 0x5f375a86- (i>>1); 
		x = *(float*)&i; 
		x = x*(1.5f-xhalf*x*x); 
		x = x*(1.5f-xhalf*x*x); 
		x = x*(1.5f-xhalf*x*x); 
		return 1/x;
	}

private :

	MCTreeNode *m_Root;			///< --- 整顆 tree 的 root
	MCTreeNode *m_Current;		///< --- 現在在對哪個 Node 進行模擬

	int m_GameLength;			///< --- 目前 進行的局數
	int m_GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]; ///< --- 歷史盤面

};


#endif