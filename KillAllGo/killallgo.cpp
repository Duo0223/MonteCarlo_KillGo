/*
 * This code is provied as a sample code of Hw 2 of "Theory of Computer Game".
 * This code implementeds the basic request operation.
 * The "think" function will randomly output one of the legal move.
 * This code can only be used within the class.
 * */
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <fstream>

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

#define kCanMove		4

#define SELF             1
#define OPPONENT         2
 
using namespace std;

const int DirectionX[MAXDIRECTION] = {-1, 0, 1, 0};
const int DirectionY[MAXDIRECTION] = { 0, 1, 0,-1};

//ofstream cout("log.txt" , ios::app);

/*
 * This function display the current game board, empty intersection is 0,
 * black intersection is 1, white intersection is 2 and boundary is 3.
 * */
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

			case kCanMove:
				cout << "M";
				break;
			}
		}
		cout << endl;
    }
    cout << endl;
}

/*
 * This function reset the board, the board intersections are labeled with 0,
 * the boundary intersections are labeled with 3.
 * */
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

/*
 * This function return the total number of liberity of the string of (X, Y) and
 * the string will be label with 'label'.
 * */
int find_liberty(int X, int Y, int label, int Board[BOUNDARYSIZE][BOUNDARYSIZE], 
	long long int ConnectBoard[BOUNDARYSIZE][BOUNDARYSIZE]) 
{
    // Label the current intersection
    ConnectBoard[X][Y] |= label;
	
	cout<<"\t\tConnectBoard[X][Y]\t"<<ConnectBoard[X][Y]<<"\tlabel\t"<<label<<endl;

    int total_liberty = 0;

    for (int d = 0 ; d < MAXDIRECTION; ++d ) 
	{
		int DirX = X + DirectionX[ d ];
		int DirY = Y + DirectionY[ d ];

		cout<<"\t\t\tDirX\t"<<DirX<<"\tDirY\t"<<DirY<<endl;

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

/*
 * This function count the liberties of the given intersection's neighboorhod
 * */
void count_liberty(int X, int Y, int Board[BOUNDARYSIZE][BOUNDARYSIZE], int Liberties[MAXDIRECTION]) 
{
    long long int ConnectBoard[BOUNDARYSIZE][BOUNDARYSIZE];
    // Initial the ConnectBoard
    for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
	{
		for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
		{
			ConnectBoard[i][j] = 0;
		}
    }

    // Find the same connect component and its liberity
    for (int d = 0 ; d < MAXDIRECTION; ++d) 
	{
		Liberties[d] = 0;
		int DirX = X + DirectionX[ d ];
		int DirY = Y + DirectionY[ d ];
		if (Board[ DirX ][ DirY ] == BLACK ||  
			Board[ DirX ][ DirY ] == WHITE    ) 
		{
			cout <<"\tfind_liberty\tbefore   "<<endl;
			cout <<"\tDirX\t"<<DirX<<"\tDirY\t"<<DirY<<endl;
			Liberties[ d ] = find_liberty( DirX, DirY, d, Board, ConnectBoard);
			cout <<"\tfind_liberty\tafter   "<<endl;
		}
    }
}

/*
 * This function count the number of empty, self, opponent, and boundary intersections of the neighboorhod
 * and saves the type in NeighboorhoodState.
 * */
void count_neighboorhood_state(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn, int* empt, int* self, 
	int* oppo ,int* boun, int NeighboorhoodState[MAXDIRECTION]) 
{
    for (int d = 0 ; d < MAXDIRECTION; ++d ) 
	{
		// check the number of nonempty neighbor
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

/*
 * This function remove the connect component contains (X, Y) with color "turn" 
 * */
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
/*
 * This function update Board with place turn's piece at (X,Y).
 * Note that this function will not check if (X, Y) is a legal move or not.
 * */
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
    
	// check if there is opponent piece in the neighboorhood
    if (num_neighborhood_oppo != 0) 
	{
		count_liberty(X, Y, Board, Liberties);
		for (int d = 0 ; d < MAXDIRECTION; ++d) 
		{
			// check if there is opponent component only one liberty
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
/*
 * This function update Board with place turn's piece at (X,Y).
 * Note that this function will check if (X, Y) is a legal move or not.
 * */
int update_board_check(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn) 
{
    // Check the given coordination is legal or not
    if ( X < 1 || X > 7 || Y < 1 || Y > 7 || Board[X][Y]!=EMPTY)
		return 0;

	int num_neighborhood_self = 0;
    int num_neighborhood_oppo = 0;
    int num_neighborhood_empt = 0;
    int num_neighborhood_boun = 0;
    int Liberties[4];
    int NeighboorhoodState[4];

	cout <<"\tcount_neighboorhood_state\tbefore\t"<<endl;

    count_neighboorhood_state(Board, X, Y, turn,
	    &num_neighborhood_empt,
	    &num_neighborhood_self,
	    &num_neighborhood_oppo,
	    &num_neighborhood_boun, NeighboorhoodState);

	cout<<"\t num_neighborhood_self \t"<<num_neighborhood_self<<endl;
	cout<<"\t num_neighborhood_oppo \t"<<num_neighborhood_oppo<<endl;
	cout<<"\t num_neighborhood_empt \t"<<num_neighborhood_empt<<endl;
	cout<<"\t num_neighborhood_boun \t"<<num_neighborhood_boun<<endl;

	for( int i = 0 ; i < 4 ; ++i )
	{
		cout<<"\t\t"<<i<<"\t"<<NeighboorhoodState[i]<<endl;
	}

	cout <<"\tcount_neighboorhood_state\tafter"<<endl;
    
	// Check if the move is a legal move
    // Condition 1: there is a empty intersection in the neighboorhood
    int legal_flag = 0;

	cout <<"\tcount_liberty\tbefore"<<endl;
	cout <<"\tX\t"<<X<<"\tY\t"<<Y<<endl;
    count_liberty(X, Y, Board, Liberties);

	for( int i = 0 ; i < 4 ; ++i )
	{
		cout<<"\t\t"<<i<<"\t"<<Liberties[i]<<endl;
	}
	cout <<"\tcount_liberty\tafter"<<endl;

    if (num_neighborhood_empt != 0) 
	{
		legal_flag = 1;
    }
    else 
	{
		// Condition 2: there is a self string has more than one liberty
		for (int d = 0; d < MAXDIRECTION; ++d) 
		{
			if (NeighboorhoodState[d] == SELF && Liberties[d] > 1) 
			{
				legal_flag = 1;
			}
		}
		if (legal_flag == 0) 
		{
			// Condition 3: there is a opponent string has exactly one liberty
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
		// check if there is opponent piece in the neighboorhood
		if (num_neighborhood_oppo != 0) 
		{
			for (int d = 0 ; d < MAXDIRECTION; ++d) 
			{
				// check if there is opponent component only one liberty
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

/*
 * This function return the number of legal moves with clor "turn" and
 * saves all legal moves in MoveList
 * */
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
			// check if current 
			if (Board[x][y] == 0) 
			{
				// check the liberty of the neighborhood intersections
				num_neighborhood_self = 0;
				num_neighborhood_oppo = 0;
				num_neighborhood_empt = 0;
				num_neighborhood_boun = 0;
				
				// count the number of empy, self, opponent, and boundary neighboorhood
				count_neighboorhood_state(Board, x, y, turn,
					&num_neighborhood_empt,
					&num_neighborhood_self,
					&num_neighborhood_oppo,
					&num_neighborhood_boun, NeighboorhoodState);
		
				// check if the emtpy intersection is a legal move
				next_x = next_y = 0;
				eat_move = 0;
				count_liberty(x, y, Board, Liberties);
		
				// Case 1: exist empty intersection in the neighborhood
				if (num_neighborhood_empt > 0) 
				{
					next_x = x;
					next_y = y;
		     
					// check if it is a capture move
					for (int d = 0 ; d < MAXDIRECTION; ++d) 
					{
						if (NeighboorhoodState[d] == OPPONENT && Liberties[d] == 1) 
						{
							eat_move = 1;
						}
					}
				}

				// Case 2: no empty intersection in the neighborhood
				else 
				{
					// Case 2.1: Surround by the self piece
					if (num_neighborhood_self + num_neighborhood_boun == MAXDIRECTION) 
					{
						int check_flag = 0;
						for (int d = 0 ; d < MAXDIRECTION; ++d) 
						{
							// Check if there is one self component which has more than one liberty
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
		    
					// Case 2.2: Surround by opponent or both side's pieces.
					else if (num_neighborhood_oppo > 0) 
					{
						int check_flag = 0;
						int eat_flag = 0;
						for (int d = 0 ; d < MAXDIRECTION; ++d) 
						{
							// Check if there is one self component which has more than one liberty
							if (NeighboorhoodState[d]==SELF && Liberties[d] > 1) 
							{
								check_flag = 1;
							}
							// Check if there is one opponent's component which has exact one liberty
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
							// check_flag == 0
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
					// copy the current board to next board
					for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
					{
						for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
						{
							NextBoard[i][j] = Board[i][j];
						}
					}
					// do the move
					// The move is a capture move and the board needs to be updated.
					if (eat_move == 1) 
					{
						update_board(NextBoard, next_x, next_y, turn);
					}
					else 
					{
						NextBoard[x][y] = turn;
					}
					// Check the history to avoid the repeat board
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
						// 3 digit zxy, z means eat or not, and put at (x, y)
						MoveList[legal_moves] = eat_move * 100 + next_x * 10 + y ;
						legal_moves++;
					}
				}
			}
		}
    }
    return legal_moves;
}
/*
 * This function randomly selects one move from the MoveList.
 * */
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
/*
 * This function update the Board with put 'turn' at (x,y)
 * where x = (move % 100) / 10 and y = move % 10.
 * Note this function will not check 'move' is legal or not.
 * */
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
/* 
 * This function randomly generate one legal move (x, y) with return value x*10+y,
 * if there is no legal move the function will return 0.
 * */
int think(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int time_limit, int game_length, 
	int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]) 
{
    clock_t start_t, end_t, now_t;
    // record start time
    start_t = clock();
    // calculate the time bound
    end_t = start_t + CLOCKS_PER_SEC * time_limit;

    int MoveList[NUMINTERSECTION];
    int num_legal_moves = 0;
    int return_move = 0;

    num_legal_moves = gen_legal_move(Board, turn, game_length, GameRecord, MoveList);

	///< ---- 把選出來的合法步 列印出來
	int CopyBoard[BOUNDARYSIZE][BOUNDARYSIZE];
	for( int ix = 0 ; ix < BOUNDARYSIZE ; ++ix )
	{
		for(int iy = 0 ; iy < BOUNDARYSIZE ;++iy )
		{
			CopyBoard[ix][iy] = Board[ix][iy];
		}
	}

	for( int ii = 0 ; ii < num_legal_moves ; ++ii )
	{
		int movepath = MoveList[ii];

		int move_x = (movepath % 100) / 10;
		int move_y = movepath % 10;

		CopyBoard[move_x][move_y] = kCanMove;
	}

	display( CopyBoard , 0 );

	///< ------------------------------------


    return_move = rand_pick_move(num_legal_moves, MoveList);

    do_move(Board, turn, return_move);

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
    bool GameState = 0; // 0 off, 1 on.
    int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE];
    int game_length = 0;
    reset(Board);
    cout << ">";
    while (cin >> command) 
	{
		cout <<" -----------------------------------------------------------------"<<endl;
		//reset
		if (strcmp(command,"reset") ==0 ) 
		{
			reset(Board);
			game_length = 0;
			GameState = 0;
			cout << ">ok" << endl;
		}

		//time
		if (strcmp(command,"time") == 0 ) 
		{
			cin >> time_limit;
			cout << ">ok" << endl;
		}

		//put
		if (strcmp(command,"put")==0) 
		{
			cin >> bw >> x >> y;
			if ( ( bw[0]=='w' || bw[0]=='b' ) 
				&& 1 <= x && x <= 7 && 1 <= y && y <= 7 ) 
			{
				
				cout <<"        "<<command<<"  "<<bw<<" "<<x<<" "<<y<<"      "<<endl;

				
				if (GameState == 1) 
				{
					// check for repeat board
					for (int i = 0 ; i < BOUNDARYSIZE; ++i) 
					{
						for (int j = 0 ; j < BOUNDARYSIZE; ++j) 
						{
							NextBoard[i][j] = Board[i][j];
						}
					}
					cout<<"           update_board_check            before     "<<endl;
					update_board_check(NextBoard, BOARDSIZE-y+1, x, bw[0]=='b'?1 : 2 );
					cout<<"           update_board_check            after     "<<endl;

					// check for repat board
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
					// record new move
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
					//Board[BOARDSIZE-y+1][x] = bw[0]=='b'?1:2;
				}
				cout << ">ok" << endl;
			}
			else 
			{
				cerr << bw << " " << x << " " << y << endl;
				cout << "put b/w x y:put a black/white stone at (x,y), 1 <= x,y <=7" << endl;
			}
		}
	
		//display
		if (strcmp(command,"display")==0) 
		{
			display(Board, time_limit);
		}

		//start
		if ( strcmp(command,"start") == 0 ) 
		{
			cin >> command;
			if (strcmp(command, "game") == 0 ) 
			{
				GameState = 1;
				// initial game record
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

		// think
		if (strcmp(command,"think")==0) 
		{
			cin >> bw;
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
	
		//quit
		if (strcmp(command,"quit")==0) 
		{
			break;
		}
		cout << ">";
	}

	//cout.close();
	system("pause");
    return 0;
}
