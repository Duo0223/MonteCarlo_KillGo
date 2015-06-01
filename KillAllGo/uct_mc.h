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

///< ==== ��� �ѽL
void display(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int time_limit);

///< ==== ���m �ѽL
void reset(int Board[BOUNDARYSIZE][BOUNDARYSIZE]);

///< ==== �Ǧ^ ( X , Y ) �o�� String ���Ҧ� liberty
int find_liberty(int X, int Y, int label, int Board[BOUNDARYSIZE][BOUNDARYSIZE], 
	long long int ConnectBoard[BOUNDARYSIZE][BOUNDARYSIZE]);

///< === �p�� ��
void count_liberty(int X, int Y, int Board[BOUNDARYSIZE][BOUNDARYSIZE], int Liberties[MAXDIRECTION]);

///< === �O���F����I�����A
void count_neighboorhood_state(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn, int* empt, int* self, 
	int* oppo ,int* boun, int NeighboorhoodState[MAXDIRECTION]);

///< === ���l
void remove_piece(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn);

///< === ��s�ѽL
void update_board(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn);

///< === ��s�ѽL�A�÷|�P�_�O�_���X�k�B
int update_board_check(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn);

///< === ���ͦX�k�B
int gen_legal_move(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int game_length, 
	int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE], int MoveList[NUMINTERSECTION]); 

///< === �q �X�k�B�� �H���D��@�B
int rand_pick_move(int num_legal_moves, int MoveList[NUMINTERSECTION]);

///< === �U��
void do_move(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int move);

///< === �q�� ���
int think(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int time_limit, int game_length, 
	int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]);

///< =========================================================================================


///< --- node struct
struct MCTreeNode
{      
	int type;										///< --- Node ���� : BLACK or WHITE
	int visit;										///< --- �Q���X����
    int move;										///< --- �n�����B

    int board[ BOUNDARYSIZE ][ BOUNDARYSIZE ];		///< --- �L��
	int simBoard[ BOUNDARYSIZE ][ BOUNDARYSIZE ];	///< --- node �����Ϊ��L��

	vector<float>		scoreVec;					///< --- �Ҧ����������� Vector
	float				score;						///< --- ����

	float				dev;						///< --- �зǮt
	float				mean;						///< --- ������

	float				meanL;						///< --- ���䪺 bound		
	float				meanR;						///< --- �k�䪺 bound

	vector< MCTreeNode* >	childVec;				///< --- Children Vevtor
	MCTreeNode*				parent;					///< ---  parent ����

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

///< --- �X�a�dù tree
class MCTree
{
public :
	 MCTree( void );
	 ~MCTree( void );

	 ///< --- Search
	 /*
		_Time : ���M�ɶ�
		_Board : �n���������L��
		_Turn : �ثe�O�֤U ( �� or �� )
		_GameLength : �ثe�C�����ɶ�����
		_GameRecord : ���v�L��
	 */
	int Search( int _Time , int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , int _GameLength ,
		int _GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE] );

	///< --- �p��ѽL���� or �մѤl�Ӽ�	
	/*
		_Board : �n���P�_���ѽL
		_Turn : �� or ��
	*/
	int CountPieceNum( int _Board[BOUNDARYSIZE][BOUNDARYSIZE]  , int _Turn );

protected :

	///< --- �i�}�l�I
	/*
		_Node : �n�i�}�l�I�� Node
	*/
	void Expand( MCTreeNode *_Node );


	///< --- ��i�����
	/*
		_Node : �n���l�I�i������� Node
		_Idx : �n������ Child
	*/
	void Simulation( MCTreeNode *_Node , int _Idx );


	///< --- �p�� Node ���C�Ӥl�I�� UCB �ȡA�æ^�Ǧ��̦n UCB �Ȫ� Node
	/*
		_Node : �n���p�⪺ Node
	*/
	MCTreeNode *GetBestChildByUCB( MCTreeNode *_Node );

	
	///< --- �p��C�� child �������ȸ�зǮt�A�M�����ƤӧC�� Childe
	/*
		_Node : �p�⪺ Node
	*/
	bool PruningBadChild( MCTreeNode *_Node );

	
	///< --- ���������ɡA�N��쪺 Node BackUp �^�� root
	/*
		_Node : ��쪺 Node
	*/
	MCTreeNode *BackUp( MCTreeNode* _Node );


	///< --- Search �t��k���D�n�j��
	/*
		_Time : �����ɶ�����
	*/
	void DoLoop( int _Time );


private :

	///< --- Copy �ѽL
	/*
		_Src : �ӷ��ѽL
		_Dis : �ؼдѽL
	*/
	void CopyBoard( int _Src[BOUNDARYSIZE][BOUNDARYSIZE] , int _Dis[BOUNDARYSIZE][BOUNDARYSIZE] );


	///< --- �P�_ (X, Y) �O�_������
	/*
		_Board : �n���P�_���ѽL
		_Turn : �� or ��
		_X , _Y : ��m
	*/
	bool IsEye( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , int _X , int _Y );
	
	///< --- �P�_�O�_�Φ��Ⲵ
	/*
		_Board : �n���P�_���ѽL
	*/
	bool IsTwoEye( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] );

	///< --- �P�_ (_X , _Y) �o�@�B�O�_�ݭn�˱�
	/*
		_Board : �n���P�_���ѽL
		_Turn : �� or ��
		_X , _Y : ��m
	*/
	bool IsGiveup( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , int _X , int _Y );

	///< --- ����
	/*
		_Turn : �{�b�����a
	*/
	int ChangeTurn( int _Turn );

	///< --- �P�_�O�_���l�I�A�H�άO�_���l�I�O�ӧQ�I
	MCTreeNode* JudgeByNode( MCTreeNode *_Node );

	///< --- �P�_�X�k�B���A���ثe�ѽL�O�_�������ӧQ���B
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

	MCTreeNode *m_Root;			///< --- ���� tree �� root
	MCTreeNode *m_Current;		///< --- �{�b�b����� Node �i�����

	int m_GameLength;			///< --- �ثe �i�檺����
	int m_GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]; ///< --- ���v�L��

};


#endif