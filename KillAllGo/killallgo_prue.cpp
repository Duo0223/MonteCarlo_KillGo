/*
 * This code is provied as a sample code of Hw 2 of "Theory of Computer Game".
 * This code implementeds the basic request operation.
 * The "think" function will randomly output one of the legal move.
 * This code can only be used within the class.
 * */

#include "uct_mc.h"


///< ==== ��� �ѽL
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

///< ==== ���m �ѽL
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

///< ==== �Ǧ^ ( X , Y ) �o�� String ���Ҧ� liberty
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

///< === �p�� ��
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

///< === �O���F����I�����A
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

///< === ���l
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

///< === ��s�ѽL
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
			///< === ���u���@��
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

///< === ��s�ѽL�A�÷|�P�_�O�_���X�k�B
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
    
	///< --- �P�_�O�_���X�k�B
    ///< --- Condition 1 : �Ū����I
    int legal_flag = 0;

    count_liberty(X, Y, Board, Liberties);

    if (num_neighborhood_empt != 0) 
	{
		legal_flag = 1;
    }
    else 
	{
		///< --- Condition 2: ���ۤv�� string�A�B�W�L�@��
		for (int d = 0; d < MAXDIRECTION; ++d) 
		{
			if (NeighboorhoodState[d] == SELF && Liberties[d] > 1) 
			{
				legal_flag = 1;
			}
		}
		if (legal_flag == 0) 
		{
			///< --- Condition 3: ��誺�l�u���@��
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
		///< --- �P�_�O�_����⪺�Ѥl�b����
		if (num_neighborhood_oppo != 0) 
		{
			for (int d = 0 ; d < MAXDIRECTION; ++d) 
			{
				///< --- �p�G�A��⪺�Ѥl�u���@��
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

///< === ���ͦX�k�B
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
			///< --- �P�_�{�b���A
			if (Board[x][y] == 0) 
			{
				///< --- �ˬd��
				num_neighborhood_self = 0;
				num_neighborhood_oppo = 0;
				num_neighborhood_empt = 0;
				num_neighborhood_boun = 0;
				
				///< --- �P�_�P�򪺮��I
				count_neighboorhood_state(Board, x, y, turn,
					&num_neighborhood_empt,
					&num_neighborhood_self,
					&num_neighborhood_oppo,
					&num_neighborhood_boun, NeighboorhoodState);
		
				///< --- �Ū�:�X�k�B
				next_x = next_y = 0;
				eat_move = 0;
				count_liberty(x, y, Board, Liberties);
		
				///< --- Case 1: �P�򳣬O�Ū��I
				if (num_neighborhood_empt > 0) 
				{
					next_x = x;
					next_y = y;
		     
					///< --- �i�_���l
					for (int d = 0 ; d < MAXDIRECTION; ++d) 
					{
						if (NeighboorhoodState[d] == OPPONENT && Liberties[d] == 1) 
						{
							eat_move = 1;
						}
					}
				}

				///< --- Case 2: �S���P�򳣬O�Ū��I
				else 
				{
					///< --- Case 2.1: ��ۤv�� string
					if (num_neighborhood_self + num_neighborhood_boun == MAXDIRECTION) 
					{
						int check_flag = 0;
						for (int d = 0 ; d < MAXDIRECTION; ++d) 
						{
							///< --- �ۤv�� string �O�_�W�L�@��
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
		    
					///< --- Case 2.2: �O�_�Q����¶
					else if (num_neighborhood_oppo > 0) 
					{
						int check_flag = 0;
						int eat_flag = 0;
						for (int d = 0 ; d < MAXDIRECTION; ++d) 
						{
							///< --- ���W�L�@��
							if (NeighboorhoodState[d]==SELF && Liberties[d] > 1) 
							{
								check_flag = 1;
							}
							///< --- ���u���@��
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
					///< --- ��{�b���ѽL���ƻs
					for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
					{
						for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
						{
							NextBoard[i][j] = Board[i][j];
						}
					}
					///< --- ��s�ѽL
					if (eat_move == 1) 
					{
						update_board(NextBoard, next_x, next_y, turn);
					}
					else 
					{
						NextBoard[x][y] = turn;
					}
					///< --- �קK ���ƽL��
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
						///< -- 3 �Ƥl zxy, z �N��Y�l ,  ��m�� (x, y)
						MoveList[legal_moves] = eat_move * 100 + next_x * 10 + y ;
						legal_moves++;
					}
				}
			}
		}
    }
    return legal_moves;
}

///< === �q �X�k�B�� �H���D��@�B
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

///< === �U��
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

///< === �q�� ���
int think(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int time_limit, int game_length, 
	int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]) 
{
    int MoveList[NUMINTERSECTION];
    int num_legal_moves = 0;
    int return_move = 0;

	///< --- new �@�� �j�M tree
	MCTree *pTree = new MCTree();
	int whiteNum = pTree->CountPieceNum(Board , WHITE);
	int blackNum = pTree->CountPieceNum(Board , BLACK);

	///< --- �p�G���W���Ѥl�� �p�� 5�A�N��@�}�����A�B�z
	const int OpenGamePieceNum = 5;
	if( whiteNum + blackNum < OpenGamePieceNum )
	{
		///< --- �H���D�@�B
		num_legal_moves = gen_legal_move(Board, turn, game_length, GameRecord, MoveList);
		return_move = rand_pick_move(num_legal_moves, MoveList);
	}
	else
	{
		///< --- �ϥ� MCTree �� search 
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
		///< --- ���m�ѽL
		if (strcmp(command,"reset") ==0 ) 
		{
			reset(Board);
			game_length = 0;
			GameState = 0;
			cout << ">ok" << endl;
		}

		///< --- �]�w��Үɶ�
		if (strcmp(command,"time") == 0 ) 
		{
			cin >> time_limit;
	//		read >> time_limit;
			if( time_limit <= 1 )
				time_limit = 2;
			cout << ">ok" << endl;
		}

		///< --- ��m�Ѥl
		if (strcmp(command,"put")==0) 
		{
			cin >> bw >> x >> y;
			//read >> bw >> x >> y;
			if ( ( bw[0]=='w' || bw[0]=='b' ) 
				&& 1 <= x && x <= 7 && 1 <= y && y <= 7 ) 
			{
				
				if (GameState == 1) 
				{
					///< --- �ˬd repeat
					for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
					{
						for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
						{
							NextBoard[i][j] = Board[i][j];
						}
					}
					
					update_board_check(NextBoard, BOARDSIZE-y+1, x, bw[0]=='b'?1 : 2 );

					///< --- �P�_�O�_�� repeat ������
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
					///< --- �O�� �o��������
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
	
		///< --- �L�X�ѽL
		if (strcmp(command,"display")==0) 
		{
			display(Board, time_limit);
		}

		///< -- �}�l�C��
		if ( strcmp(command,"start") == 0 ) 
		{
			cin >> command;
			//read >> command;
			if (strcmp(command, "game") == 0 ) 
			{
				GameState = 1;
				
				///< --- ��l�ѽL
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

		///< --- �q���U
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
	
		///< --- ����
		if (strcmp(command,"quit")==0) 
		{
			break;
		}
		cout << ">";
	}

	//system("pause");
    return 0;
}
