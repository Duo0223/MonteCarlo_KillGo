/*
 * This code is provied as a sample code of Hw 2 of "Theory of Computer Game".
 * This code implementeds the basic request operation.
 * The "think" function will randomly output one of the legal move.
 * This code can only be used within the class.
 * */

#include "uct_mc.h"


///< ==== 顯示 棋盤
void display(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int time_limit) 
{
    cout << "time limit=" << time_limit << endl;
    for (int i = 1; i <= BOARDSIZE; ++i) 
	{
		for (int j = 1; j <= BOARDSIZE; ++j) 
		{
			switch(Board[i][j] ) 
			{
			case EMPTY: 
				cout << ".";
				break;
			case BLACK: 
				cout << "X";
				break;
			case WHITE: 
				cout << "O";
				break;
			}
		}
		cout << endl;
    }
    cout << endl;
}

///< ==== 重置 棋盤
void reset(int Board[BOUNDARYSIZE][BOUNDARYSIZE]) 
{
    for (int i = 1 ; i <= BOARDSIZE; ++i) 
	{
		for (int j = 1 ; j <= BOARDSIZE; ++j) 
		{
			Board[i][j] = EMPTY;
		}
    }

    for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
	{
		Board[0][i] = Board[BOUNDARYSIZE-1][i] = Board[i][0] = Board[i][BOUNDARYSIZE-1] = BOUNDARY;
    }
}

///< ==== 傳回 ( X , Y ) 這條 String 的所有 liberty
int find_liberty(int X, int Y, int label, int Board[BOUNDARYSIZE][BOUNDARYSIZE], 
	long long int ConnectBoard[BOUNDARYSIZE][BOUNDARYSIZE]) 
{
    ConnectBoard[X][Y] |= label;

    int total_liberty = 0;

    for (int d = 0 ; d < MAXDIRECTION; ++d ) 
	{
		int DirX = X + DirectionX[ d ];
		int DirY = Y + DirectionY[ d ];

		if ((Board[ DirX ][ DirY ] == EMPTY ) && 
			( ( ConnectBoard[ DirX ][ DirY ] & ( 1ull << label) ) == 0 ) )
		{
			ConnectBoard[ DirX ][ DirY ] |= (1ull << label );
			total_liberty++;
		}
		if (Board[ DirX ][ DirY ] == Board[X][Y] && 
			( ConnectBoard[ DirX ][ DirY ] & ( 1ull << label) ) == 0 ) 
		{
			ConnectBoard[ DirX ][ DirY ] |=( 1ull << label );
			total_liberty += find_liberty( DirX, DirY, label, Board, ConnectBoard);
		}
    }
    return total_liberty;
}

///< === 計算 氣
void count_liberty(int X, int Y, int Board[BOUNDARYSIZE][BOUNDARYSIZE], int Liberties[MAXDIRECTION]) 
{
    long long int ConnectBoard[BOUNDARYSIZE][BOUNDARYSIZE];
    
    for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
	{
		for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
		{
			ConnectBoard[i][j] = 0;
		}
    }

    for (int d = 0 ; d < MAXDIRECTION; ++d) 
	{
		Liberties[d] = 0;
		int DirX = X + DirectionX[ d ];
		int DirY = Y + DirectionY[ d ];
		if (Board[ DirX ][ DirY ] == BLACK ||  
			Board[ DirX ][ DirY ] == WHITE    ) 
		{
			Liberties[ d ] = find_liberty( DirX, DirY, d, Board, ConnectBoard);
		}
    }
}

///< === 記錄鄰近格點的狀態
void count_neighboorhood_state(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn, int* empt, int* self, 
	int* oppo ,int* boun, int NeighboorhoodState[MAXDIRECTION]) 
{
    for (int d = 0 ; d < MAXDIRECTION; ++d ) 
	{
		switch(Board[ X + DirectionX[ d ] ][ Y + DirectionY[ d ] ]) 
		{
			case EMPTY:   
				(*empt)++; 
				NeighboorhoodState[ d ] = EMPTY;
				break;
			case BLACK:    
				if (turn == BLACK) 
				{
					(*self)++;
					NeighboorhoodState[ d ] = SELF;
				}
				else 
				{
					(*oppo)++;
					NeighboorhoodState[ d ] = OPPONENT;
				}
				break;
	    case WHITE:    
				if (turn == WHITE) 
				{
					(*self)++;
					NeighboorhoodState[d] = SELF;
				}
				else 
				{
					(*oppo)++;
					NeighboorhoodState[d] = OPPONENT;
				}
				break;
	    case BOUNDARY: 
				(*boun)++;
				NeighboorhoodState[d] = BOUNDARY;
				break;
		}
    }
}

///< === 提子
void remove_piece(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn) 
{
    Board[X][Y] = EMPTY;
    for (int d = 0; d < MAXDIRECTION; ++d) 
	{
		if (Board[ X + DirectionX[ d ] ][ Y + DirectionY[ d ] ] == turn ) 
		{
			remove_piece(Board, X + DirectionX[ d ] , Y + DirectionY[ d ], turn );
		}
    }
}

///< === 更新棋盤
void update_board(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn) 
{
    int num_neighborhood_self = 0;
    int num_neighborhood_oppo = 0;
    int num_neighborhood_empt = 0;
    int num_neighborhood_boun = 0;
    int Liberties[4];
    int NeighboorhoodState[4];

    count_neighboorhood_state(Board, X, Y, turn,
	    &num_neighborhood_empt,
	    &num_neighborhood_self,
	    &num_neighborhood_oppo,
	    &num_neighborhood_boun, NeighboorhoodState);
    
	
    if (num_neighborhood_oppo != 0) 
	{
		count_liberty(X, Y, Board, Liberties);
		for (int d = 0 ; d < MAXDIRECTION; ++d) 
		{
			///< === 對手只有一氣
			if (NeighboorhoodState[d] == OPPONENT && Liberties[d] == 1 && 
				Board[ X + DirectionX[ d ] ][ Y + DirectionY[ d ] ] != EMPTY) 
			{
				remove_piece(Board, X + DirectionX[ d ], Y + DirectionY[ d ], 
					Board[ X + DirectionX[ d ] ][ Y + DirectionY[ d ] ]);
			}
		}
    }
    Board[X][Y] = turn;
}

///< === 更新棋盤，並會判斷是否為合法步
int update_board_check(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn) 
{
    if ( X < 1 || X > 7 || Y < 1 || Y > 7 || Board[X][Y]!=EMPTY)
		return 0;

	int num_neighborhood_self = 0;
    int num_neighborhood_oppo = 0;
    int num_neighborhood_empt = 0;
    int num_neighborhood_boun = 0;
    int Liberties[4];
    int NeighboorhoodState[4];

    count_neighboorhood_state(Board, X, Y, turn,
	    &num_neighborhood_empt,
	    &num_neighborhood_self,
	    &num_neighborhood_oppo,
	    &num_neighborhood_boun, NeighboorhoodState);
    
	///< --- 判斷是否為合法步
    ///< --- Condition 1 : 空的格點
    int legal_flag = 0;

    count_liberty(X, Y, Board, Liberties);

    if (num_neighborhood_empt != 0) 
	{
		legal_flag = 1;
    }
    else 
	{
		///< --- Condition 2: 有自己的 string，且超過一氣
		for (int d = 0; d < MAXDIRECTION; ++d) 
		{
			if (NeighboorhoodState[d] == SELF && Liberties[d] > 1) 
			{
				legal_flag = 1;
			}
		}
		if (legal_flag == 0) 
		{
			///< --- Condition 3: 對方的子只有一氣
			for (int d = 0; d < MAXDIRECTION; ++d) 
			{
				if (NeighboorhoodState[d] == OPPONENT && Liberties[d] == 1) 
				{
					legal_flag = 1;
				}
			}
		}
    }

    if (legal_flag == 1) 
	{
		///< --- 判斷是否有對手的棋子在附近
		if (num_neighborhood_oppo != 0) 
		{
			for (int d = 0 ; d < MAXDIRECTION; ++d) 
			{
				///< --- 如果，對手的棋子只有一氣
				if (NeighboorhoodState[d] == OPPONENT && Liberties[d] == 1 &&
					Board[X+DirectionX[d]][Y+DirectionY[d]]!=EMPTY) 
				{
					remove_piece(Board, X + DirectionX[ d ], Y + DirectionY[ d ], 
						Board[ X + DirectionX[ d ] ][ Y + DirectionY[ d ] ]);
				}
			}
		}
		Board[X][Y] = turn;
    }
    return (legal_flag==1)?1:0;
}

///< === 產生合法步
int gen_legal_move(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int game_length, 
	int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE], int MoveList[NUMINTERSECTION]) 
{
    int NextBoard[BOUNDARYSIZE][BOUNDARYSIZE];
    int num_neighborhood_self = 0;
    int num_neighborhood_oppo = 0;
    int num_neighborhood_empt = 0;
    int num_neighborhood_boun = 0;
    int legal_moves = 0;
    int next_x, next_y;
    int Liberties[4];
    int NeighboorhoodState[4];
    bool eat_move = 0;

    for (int x = 1 ; x <= BOARDSIZE; ++x) 
	{
		for (int y = 1 ; y <= BOARDSIZE; ++y) 
		{
			///< --- 判斷現在狀態
			if (Board[x][y] == 0) 
			{
				///< --- 檢查氣
				num_neighborhood_self = 0;
				num_neighborhood_oppo = 0;
				num_neighborhood_empt = 0;
				num_neighborhood_boun = 0;
				
				///< --- 判斷周圍的格點
				count_neighboorhood_state(Board, x, y, turn,
					&num_neighborhood_empt,
					&num_neighborhood_self,
					&num_neighborhood_oppo,
					&num_neighborhood_boun, NeighboorhoodState);
		
				///< --- 空的:合法步
				next_x = next_y = 0;
				eat_move = 0;
				count_liberty(x, y, Board, Liberties);
		
				///< --- Case 1: 周圍都是空的點
				if (num_neighborhood_empt > 0) 
				{
					next_x = x;
					next_y = y;
		     
					///< --- 可否提子
					for (int d = 0 ; d < MAXDIRECTION; ++d) 
					{
						if (NeighboorhoodState[d] == OPPONENT && Liberties[d] == 1) 
						{
							eat_move = 1;
						}
					}
				}

				///< --- Case 2: 沒有周圍都是空的點
				else 
				{
					///< --- Case 2.1: 找自己的 string
					if (num_neighborhood_self + num_neighborhood_boun == MAXDIRECTION) 
					{
						int check_flag = 0;
						for (int d = 0 ; d < MAXDIRECTION; ++d) 
						{
							///< --- 自己的 string 是否超過一氣
							if (NeighboorhoodState[d]==SELF && Liberties[d] > 1) 
							{
								check_flag = 1;
							}
						}
						if (check_flag == 1) 
						{
							next_x = x;
							next_y = y;
						}
					}	
		    
					///< --- Case 2.2: 是否被對手圍繞
					else if (num_neighborhood_oppo > 0) 
					{
						int check_flag = 0;
						int eat_flag = 0;
						for (int d = 0 ; d < MAXDIRECTION; ++d) 
						{
							///< --- 對手超過一氣
							if (NeighboorhoodState[d]==SELF && Liberties[d] > 1) 
							{
								check_flag = 1;
							}
							///< --- 對手只有一氣
							if (NeighboorhoodState[d]==OPPONENT && Liberties[d] == 1) 
							{
								eat_flag = 1;
							}
						}
						if (check_flag == 1) 
						{
							next_x = x;
							next_y = y;
							if (eat_flag == 1) 
							{
								eat_move = 1;
							}
						}
						else 
						{ 
							if (eat_flag == 1) 
							{
								next_x = x;
								next_y = y;
								eat_move = 1;
							}
						}
					}	
				}
				if (next_x !=0 && next_y !=0) 
				{
					///< --- 把現在的棋盤做複製
					for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
					{
						for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
						{
							NextBoard[i][j] = Board[i][j];
						}
					}
					///< --- 更新棋盤
					if (eat_move == 1) 
					{
						update_board(NextBoard, next_x, next_y, turn);
					}
					else 
					{
						NextBoard[x][y] = turn;
					}
					///< --- 避免 重複盤面
					bool repeat_move = 0;
					for (int t = 0 ; t < game_length; ++t) 
					{
						bool repeat_flag = 1;
						for (int i = 1; i <=BOARDSIZE; ++i) 
						{
							for (int j = 1; j <=BOARDSIZE; ++j) 
							{
								if (NextBoard[i][j] != GameRecord[t][i][j]) 
								{
									repeat_flag = 0;
								}
							}
						}
						if (repeat_flag == 1) 
						{
							repeat_move = 1;
							break;
						}
					}
					if (repeat_move == 0) 
					{
						///< -- 3 數子 zxy, z 代表吃子 ,  位置為 (x, y)
						MoveList[legal_moves] = eat_move * 100 + next_x * 10 + y ;
						legal_moves++;
					}
				}
			}
		}
    }
    return legal_moves;
}

///< === 從 合法步中 隨機挑選一步
int rand_pick_move(int num_legal_moves, int MoveList[NUMINTERSECTION]) 
{
    if (num_legal_moves == 0)
		return 0;
    else 
	{
		int move_id = rand()%num_legal_moves;
		return MoveList[move_id];
    }
}

///< === 下棋
void do_move(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int move) 
{
    int move_x = (move % 100) / 10;
    int move_y = move % 10;
    if (move<100) 
	{
		Board[move_x][move_y] = turn;
    }
    else 
	{
		update_board(Board, move_x, move_y, turn);
	}
}

///< === 電腦 思考
int think(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int time_limit, int game_length, 
	int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]) 
{
    int MoveList[NUMINTERSECTION];
    int num_legal_moves = 0;
    int return_move = 0;

	///< --- new 一個 搜尋 tree
	MCTree *pTree = new MCTree();
	int whiteNum = pTree->CountPieceNum(Board , WHITE);
	int blackNum = pTree->CountPieceNum(Board , BLACK);

	///< --- 如果場上的棋子數 小於 5，就當作開局狀態處理
	const int OpenGamePieceNum = 5;
	if( whiteNum + blackNum < OpenGamePieceNum )
	{
		///< --- 隨機挑一步
		num_legal_moves = gen_legal_move(Board, turn, game_length, GameRecord, MoveList);
		return_move = rand_pick_move(num_legal_moves, MoveList);
	}
	else
	{
		///< --- 使用 MCTree 做 search 
		return_move = pTree->Search(time_limit ,Board ,turn ,game_length ,GameRecord);
	}

    do_move(Board, turn, return_move);

	delete pTree;

    return return_move % 100;
}
int main(int argc, char* argv[]) 
{
    int Board[BOUNDARYSIZE][BOUNDARYSIZE];
    int NextBoard[BOUNDARYSIZE][BOUNDARYSIZE];
    int time_limit = DEFAULTTIME;
    char bw[2];
    int x, y;
    char command[COMMANDLENGTH];
    bool GameState = 0;		///<-- 0 off, 1 on.
    int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE];
    int game_length = 0;
    reset(Board);
    cout << ">";

	//static const string DictStr = "sample_test_data/";
	//static const string FileStr[] = { "" , 
	//	"input01.txt" , "input02.txt" , "input03.txt" , 
	//	"input04.txt" , "input05.txt" , "input06.txt"
	//};

	//string filename = DictStr + FileStr[ 6 ];
	//ifstream read(filename);

    while (cin >> command) 
	//while( read >> command )
	{	
		///< --- 重置棋盤
		if (strcmp(command,"reset") ==0 ) 
		{
			reset(Board);
			game_length = 0;
			GameState = 0;
			cout << ">ok" << endl;
		}

		///< --- 設定思考時間
		if (strcmp(command,"time") == 0 ) 
		{
			cin >> time_limit;
	//		read >> time_limit;
			if( time_limit <= 1 )
				time_limit = 2;
			cout << ">ok" << endl;
		}

		///< --- 放置棋子
		if (strcmp(command,"put")==0) 
		{
			cin >> bw >> x >> y;
			//read >> bw >> x >> y;
			if ( ( bw[0]=='w' || bw[0]=='b' ) 
				&& 1 <= x && x <= 7 && 1 <= y && y <= 7 ) 
			{
				
				if (GameState == 1) 
				{
					///< --- 檢查 repeat
					for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
					{
						for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
						{
							NextBoard[i][j] = Board[i][j];
						}
					}
					
					update_board_check(NextBoard, BOARDSIZE-y+1, x, bw[0]=='b'?1 : 2 );

					///< --- 判斷是否為 repeat 的移動
				    int repeat_flag;
				    int check_flag = 1;
					for (int t = 0 ; t < game_length; ++t) 
					{
						repeat_flag = 1;
						for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
						{
							for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
							{
								if (NextBoard[i][j] != GameRecord[t][i][j]) 
								{
									repeat_flag = 0;
								}
							}
						}
						if (repeat_flag == 1) 
						{
							check_flag = 0;
							break;
						}
					}
					///< --- 記錄 這次的移動
					if (check_flag == 1) 
					{
						for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
						{
							for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
							{
								GameRecord[game_length][i][j] = NextBoard[i][j];
								Board[i][j] = NextBoard[i][j];
							}
						}
					}
				}
				else 
				{
					update_board_check(Board, BOARDSIZE-y+1, x, bw[0]=='b'?1:2);
				}
				cout << ">ok" << endl;
			}
			else 
			{
				cerr << bw << " " << x << " " << y << endl;
				cout << "put b/w x y:put a black/white stone at (x,y), 1 <= x,y <=7" << endl;
			}
		}
	
		///< --- 印出棋盤
		if (strcmp(command,"display")==0) 
		{
			display(Board, time_limit);
		}

		///< -- 開始遊戲
		if ( strcmp(command,"start") == 0 ) 
		{
			cin >> command;
			//read >> command;
			if (strcmp(command, "game") == 0 ) 
			{
				GameState = 1;
				
				///< --- 初始棋盤
				game_length = 0;
				for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
				{
					for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
					{
						GameRecord[game_length][i][j] = Board[i][j];
					}
				}
				game_length++;
				cout << ">ok" << endl;
			}
		}

		///< --- 電腦下
		if (strcmp(command,"think")==0) 
		{
			cin >> bw;
			//read >> bw;
			if (GameState == 1) 
			{
				int turn = bw[0]=='b' ? BLACK : WHITE ;
				int move;
				move = think(Board, turn, time_limit, game_length, GameRecord);
				if (move == 0)
					cout << "0 0" << endl;
				else
					cout << (move%10) << " " << 8 - (move/10) << endl;
				for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
				{
					for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
					{
						GameRecord[game_length][i][j] = Board[i][j];
					}
				}
				game_length++;
			}
		}
	
		///< --- 結束
		if (strcmp(command,"quit")==0) 
		{
			break;
		}
		cout << ">";
	}

	//system("pause");
    return 0;
}
