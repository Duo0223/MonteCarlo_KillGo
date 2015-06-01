#include "uct_mc.h"


MCTree::MCTree( void )
{
	m_Root = NULL;
	m_Current = NULL;
	m_GameLength = 0;
}

MCTree::~MCTree( void )
{
	m_Current = NULL;
	if( m_Root != NULL )
		delete m_Root;
	m_Root = NULL;
}

///< --- Search
int MCTree::Search( int _Time , int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , int _GameLength ,
		int _GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE] )
{
	///< --- 將目前傳入的盤面當做 root
	m_Root = new MCTreeNode( _Turn );
	this->CopyBoard( _Board , m_Root->board );
	m_Current = m_Root;

	///< --- copy 目前的 歷史盤面，因為做模擬產生合法步需要
	m_GameLength = _GameLength;
	for( int i = 0 ; i < m_GameLength ; ++i )
		this->CopyBoard( _GameRecord[i] , m_GameRecord[i] );

	int Time = _Time - 1;
	const float TimeOfPare1 = 3.0f / 4.0f;
	const float TimeOfPare2 = 1.0f / 4.0f;
	Time = (int)floor( Time * TimeOfPare1 );

	///< --- 進行第一次的模擬
	this->DoLoop( Time );

	///< --- root 下沒有任何子點，回傳 0
	if( m_Root->childVec.empty() )
		return 0;

	///< --- 把找到 PV 中的子點往上更新勝率
	m_Current = this->BackUp( m_Current );

	///< --- 找出更新勝率後，最佳的 Node
	float maxRatio = -1.0f;
	MCTreeNode *pTmpNode = NULL;
	for( unsigned int i = 0 ; i <  m_Root->childVec.size() ; ++i )
	{
		if( maxRatio < m_Root->childVec[i]->mean )
		{
			maxRatio = m_Root->childVec[i]->mean;
			pTmpNode = m_Root->childVec[i];
		}
	}

	///< --- 最佳的 node 依然為同一個
	if( m_Current ==  pTmpNode )
		return m_Current->move;


	///< --- 重新搜尋一次最佳 node
	m_Current = pTmpNode;
	Time = (int)ceil( Time * TimeOfPare2 );
	this->DoLoop( Time );

	///< --- 把找到 新的PV 中的子點往上更新勝率
	m_Current = this->BackUp( m_Current );
	maxRatio = -1.0f;
	pTmpNode = NULL;
	for( unsigned int i = 0 ; i <  m_Root->childVec.size() ; ++i )
	{
		if( maxRatio < m_Root->childVec[i]->mean )
		{
			maxRatio = m_Root->childVec[i]->mean;
			pTmpNode = m_Root->childVec[i];
		}
	}
	return pTmpNode->move;
}

///< --- Search 演算法的主要迴圈
void MCTree::DoLoop( int _Time )
{
	clock_t start_t, end_t, now_t;
	///< --- 記錄開始時間
    now_t = start_t = clock();
	///< --- 計算結束的時間點 ( 保留一秒 )
    end_t = start_t + CLOCKS_PER_SEC *  _Time ;

	while( now_t < end_t )
	{
		///< --- 展開目前 node 的 child
		this->Expand( m_Current );
		
		///< --- 對目前 node 做初步判斷 : 是否合法步為 0，或是已經有一步就獲勝的 node
		MCTreeNode* tmpNode = NULL;
		tmpNode = this->JudgeByNode( m_Current );
		if( tmpNode != NULL )
		{
			m_Current = tmpNode;
			///< --- 填入一些數值
			m_Current->mean = m_Current->score;
			m_Current->visit = 1;
			m_Current->meanR = m_Current->mean;
			return;
		}

		///< --- 對目前 node 的 child 進行模擬
		const int First = 5;
		int childSize = m_Current->childVec.size();
		int SimTimes = childSize * _Time * First;
		m_Current->visit = 0;
		for( int i = 0 ; i < childSize ; ++i )
		{
			m_Current->childVec[i]->visit = 0;
			m_Current->childVec[i]->scoreVec.clear();
			m_Current->childVec[i]->score = 0.0f;
		}
		for( int i = 0 ; i < SimTimes && now_t < end_t ; ++i ) 
		{
			this->Simulation( m_Current  , i % childSize );
			///< --- 取出現在時間
			now_t = clock();
		}

		///< --- 把較差的 child 砍掉
		///< --- 所有的 child 的 標準差都小於預設的目標標準差，模擬結束
		if( this->PruningBadChild( m_Current ) )
		{
			if( m_Current == m_Root )
			{
				int randChildIdx = rand() % m_Current->childVec.size();
				m_Current = m_Current->childVec[ randChildIdx ];
			}
			return;
		}
		///< --- 如果砍完只剩一個 child 那就直接結束
		if( m_Current->childVec.size() == 1 )
		{
			m_Current = m_Current->childVec[0];
			return;
		}

		///< --- 對剩下的 node 再做模擬
		const int Second = 15;
		childSize = m_Current->childVec.size();
		SimTimes = childSize * _Time * Second;

		for( int i = 0 ; i < childSize ; ++i )
		{
			m_Current->childVec[i]->visit = 0;
			m_Current->childVec[i]->scoreVec.clear();
			m_Current->childVec[i]->score = 0.0f;
		}

		for( int i = 0 ; i < SimTimes && now_t < end_t ; ++i ) 
		{
			this->Simulation( m_Current  , i % childSize );
			///< --- 取出現在時間
			now_t = clock();
		}
		
		///< --- 第二次模擬後，把較差的 child 砍掉		
		///< --- 所有的 child 的 標準差都小於預設的目標標準差，模擬結束
		if( this->PruningBadChild( m_Current ) )
		{
			if( m_Current == m_Root )
			{
				int randChildIdx = rand() % m_Current->childVec.size();
				m_Current = m_Current->childVec[ randChildIdx ];
			}
			return;
		}
		///< --- 如果砍完只剩一個 child 那就直接結束
		if( m_Current->childVec.size() == 1 )
		{
			m_Current = m_Current->childVec[0];
			return;
		}

		///< --- 計算 UCB，取得最好 UCB 的 child，繼續長樹
		m_Current = this->GetBestChildByUCB( m_Current );

		///< --- 取出現在時間
		now_t = clock();
	}
}

///< --- 展開子點
void MCTree::Expand( MCTreeNode *_Node )
{
	///< --- 找出合法的移動
	int MoveList[49];
	int NumOfMove = gen_legal_move( _Node->board , _Node->type , m_GameLength , m_GameRecord , MoveList);

	///< --- 對每個合法步判斷是否為需要捨棄掉的合法步
	for( int i = 0 ; i < NumOfMove ; ++i  )
	{
		int x = ( MoveList[ i ] % 100 ) / 10;
		int y = MoveList[ i ] % 10;

		///< --- 不是需要捨棄掉的步，將此步加到 root 的 child 中 
		if( !this->IsEye( _Node->board , _Node->type , x , y ) ) //!this->IsGiveup( _Node->board , _Node->type , x , y ) )
		{
			MCTreeNode *pChild = new MCTreeNode( ChangeTurn( _Node->type ) );
			CopyBoard( _Node->board , pChild->board );
			pChild->move = MoveList[ i ];
			_Node->childVec.push_back( pChild );
			pChild->parent = _Node;
		}
	}

	///< --- 產生 child 的盤面
	for( unsigned int i = 0 ; i < _Node->childVec.size() ; ++i )
	{
		int x = ( _Node->childVec[i]->move % 100 ) / 10;
		int y = _Node->childVec[i]->move % 10;
		update_board( _Node->childVec[i]->board , x , y , _Node->type );
	}
}

///< --- 對進行模擬
void MCTree::Simulation( MCTreeNode *_Node , int _Idx )
{
	///< --- 現在輪到 哪一方下
	MCTreeNode *pChild = _Node->childVec[ _Idx ];
	int turn = pChild->type;
	int GameLength = 0;
	int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE];

	///< --- 將 Board 的值 copy 到 SimBoard 中
	CopyBoard( pChild->board , pChild->simBoard );
	CopyBoard( pChild->simBoard , GameRecord[ GameLength ] );

	///< --- 用來計算 N
	++_Node->visit;
	++pChild->visit;

	///< --- 判斷是否連續兩方都 pass
	int passCount = 0;

	int NumOfMove = 0;
	int MoveList[ 49 ];

	///< --- 一直下到結束，或是超過 50 步
	while( GameLength < MAX_STEP )  
	{
		///< --- 找出合法的移動	
		NumOfMove = gen_legal_move( pChild->simBoard, turn , GameLength, GameRecord, MoveList);
		vector<int> MoveVec;
		MoveVec.clear();
		
		///< --- 檢查每個合法的移動，看是否需要剪裁，如果不需要，就把該步丟入 Vecotr 中
		for( int i = 0 ; i < NumOfMove ; ++i )
		{
			int x = ( MoveList[i] % 100 ) / 10;
			int y = MoveList[i] % 10;
			///< --- 要留著，丟入 Vecotr 
			if( ! this->IsEye( pChild->simBoard , turn , x , y )  )//! IsGiveup( pChild->simBoard , turn , x , y ) )
				MoveVec.push_back( MoveList[i] );
		}

		///< --- 有一方沒有子 => 結束
		int pieceCountW = this->CountPieceNum( pChild->simBoard , WHITE );
		int pieceCountB = this->CountPieceNum( pChild->simBoard , BLACK );
		if( pieceCountW == 0 || pieceCountB == 0 )
			break;
			
		///< --- 沒有合法步 => PASS
		///< --- 如果 雙方都 PASS => 結束
		if( !MoveVec.empty() )
			passCount = 0;
		else 
		{
			///< --- PASS 換對方
			++passCount;
			turn = this->ChangeTurn( turn );
				
			///< --- 雙方連續 pass
			if( passCount >= 2 )
				break;
			continue;
		}

		///< --- 判斷現在是否有直接獲勝的步
		int nowMove = JudgeByMove( pChild->simBoard , turn , MoveVec );
		if( nowMove == -1 )
		{
			///< --- 從 MoveVec 中隨機挑一個合法步
			int moveIdx = rand() % MoveVec.size();
			nowMove = MoveVec[ moveIdx ];
		}

		///< --- 走 挑選出來的合法步
		int move_x = (nowMove % 100) / 10;
		int move_y = nowMove % 10;
		if ( nowMove < 100 ) 
			pChild->simBoard[move_x][move_y] = turn;
		else
			update_board( pChild->simBoard, move_x, move_y, turn );

		///< --- 記錄 Game Record 避免 repeat
		++GameLength;
		CopyBoard( pChild->simBoard , GameRecord[ GameLength ] );

		///< --- 換對方下
		turn = this->ChangeTurn( turn );
	}

	///< --- 進行模擬結果的判斷
	///< --- 超過 限制步數
	if( GameLength >= MAX_STEP )
	{
		pChild->scoreVec.push_back( 0.0f );
		return;
	}

	///< --- node 為黑棋 
	if( _Node->type == BLACK )
	{
		///< --- 判斷是否吃光白棋
		if( this->CountPieceNum( pChild->simBoard , WHITE ) == 0 )
		{
			pChild->score += WIN_SCORE;
			pChild->scoreVec.push_back( WIN_SCORE );
		}
		else
			pChild->scoreVec.push_back( 0.0f );
	}
	///< --- node 為白棋
	else if( _Node->type == WHITE )  
	{
		///< --- 判斷是否還有子
		if( this->CountPieceNum( pChild->simBoard , WHITE ) != 0 )
		{
			pChild->score += WIN_SCORE;
			pChild->scoreVec.push_back( WIN_SCORE );
		}
		else
			pChild->scoreVec.push_back( 0.0f );
	}
}

///< --- 計算 Node 中每個子點的 UCB 值，並回傳有最好 UCB 值的 Node
MCTreeNode *MCTree::GetBestChildByUCB( MCTreeNode *_Node )
{
	static const float cp = 1.0f / MySqrt(2.0f);
    float ucb = -1.0f;
	MCTreeNode *ret = NULL;
	MCTreeNode *pChild = NULL;
	for( unsigned int i = 0 ; i < _Node->childVec.size() ; ++i )
	{ 
		pChild = _Node->childVec[ i ];
		float tmpUcb = pChild->mean + 2 * cp * MySqrt(2 * log((float)_Node->visit ) / pChild->visit);
		if(tmpUcb > ucb)
		{
			ucb = tmpUcb;
			ret = pChild;
		}
	}
	return ret;
}
	
///< --- 計算每個 child 的平均值跟標準差，然後減掉分數太低的 Childe
bool MCTree::PruningBadChild( MCTreeNode *_Node )
{
	const float rd = 1.0f;
	const float dev = 0.25f;

	///< --- 對每個 node 的 child 計算:分數平均值，標準差，meanL，menaR
	int Size = _Node->childVec.size();
	MCTreeNode *pChild = NULL;
	for( int i = 0 ; i < Size ; ++i )
	{
		pChild = _Node->childVec[i];
		///< --- 先歸 0
		pChild->mean = 0.0f;	
		int ScoreSize = pChild->scoreVec.size();
		
		if( ScoreSize == 0 )
			continue;

		///< --- 計算平均
		for( int j = 0 ; j < ScoreSize ; ++j )
			pChild->mean += pChild->scoreVec[j];
		pChild->mean /= (float)ScoreSize;

		///< --- 計算標準差
		pChild->dev = 0.0f;
		for( int j = 0 ; j < ScoreSize ; ++j )
		{
			float diff = ( pChild->scoreVec[j] - pChild->mean );
			pChild->dev += ( diff * diff ); 
		}
		pChild->dev /= (float)ScoreSize;
		pChild->dev = this->MySqrt( pChild->dev );
		if( pChild->dev <= 0.002f )
			pChild->dev = 0.0f;

		///< --- 計算 meanL，menaR
		pChild->meanL = pChild->mean - rd * pChild->dev;
		pChild->meanR = pChild->mean + rd * pChild->dev;
	}

	///< --- pruning 
	vector< MCTreeNode *> clearNodeVec;
	clearNodeVec.clear();
	MCTreeNode *m1 = NULL;
	MCTreeNode *m2 = NULL;
	bool flag = false;
	for( int i = 0 ; i < Size ; ++i )
	{
		m1 = _Node->childVec[ i ];
		flag = false;	///< --- 避免重複加入 m1

		for( int j = i + 1 ; j < Size && !flag ; ++j )
		{
			m2 = _Node->childVec[ j ];
			if( m1 != NULL && m2 != NULL )
			{
				if( m1->meanR < m2->meanL )
				{
					clearNodeVec.push_back( m1 );
					flag = true;
					_Node->childVec[ i ] = NULL;  ///< --- m1
				}
				else if( m2->meanR < m1->meanL )
				{
					clearNodeVec.push_back( m2 );
					_Node->childVec[ j ] = NULL;  ///< --- m2
				}
			}
		}
	}

	///< --- 把存在 clearNodeVec 中的 node 都 delete 掉
	vector< MCTreeNode* >::iterator itr;
	for( itr = clearNodeVec.begin() ; itr != clearNodeVec.end() ; ++itr)
	{
		(*itr)->parent = NULL;
		delete (*itr);
		(*itr) = NULL;
	}

	///< --- 把還在 childVec 的 child 做重新整理，拿到 NULL 的 child
	vector< MCTreeNode *> keepNodeVec;
	keepNodeVec.clear();
	bool isOver = true;
	for( int i = 0 ; i < Size ; ++i  )
	{
		if( _Node->childVec[i] != NULL )
		{
			keepNodeVec.push_back( _Node->childVec[i] );
			if( _Node->childVec[i]->dev > dev  )
				isOver = false;
		}
	}

	_Node->childVec.clear();
	_Node->childVec = keepNodeVec;

	return isOver;
}

///< --- 模擬結束時，將找到的 Node BackUp 回到 root
MCTreeNode* MCTree::BackUp( MCTreeNode* _Node )
{
	MCTreeNode* ret = _Node;
	int count = 1;
	float vRatio = 0.0f;
	while( ret->parent != m_Root )
	{
		++count;
		///< --- ret 的 node 是由 m_Root 的玩家所下的手
		if( ret->type != m_Root->type  )
			vRatio += ret->mean;
		else if( ret->type == m_Root->type )
			vRatio += (1.0f - ret->mean);

		ret = ret->parent;
	}

	///< --- 更新 勝率
	vRatio += ret->mean;
	ret->mean = (vRatio / count);
	return ret;
}

///< --- Copy 棋盤
void MCTree::CopyBoard( int _Src[BOUNDARYSIZE][BOUNDARYSIZE] , int _Dis[BOUNDARYSIZE][BOUNDARYSIZE] )
{
	for( int i = 0 ; i < BOUNDARYSIZE ; ++i )
		for( int j = 0 ; j < BOUNDARYSIZE ; ++j )
			_Dis[i][j] = _Src[i][j];
}

///< --- 判斷 (X, Y) 是否為眼位
bool MCTree::IsEye( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , int _X , int _Y )
{
	bool flag = true;
	int Count = 0;
	for(int i = 0 ; i < 4 ; ++i)
	{
		int DirX = _X + DirectionX[ i ];
		int DirY = _Y + DirectionY[ i ];
		int NewDirX = _X + NewDirectX[ i ];
		int NewDirY = _Y + NewDirectY[ i ];

		if( _Board[ DirX ][ DirY ] != _Turn && _Board[ DirX ][ DirY ] != BOUNDARY )
			flag = false;

		if( _Board[ NewDirX ][ NewDirY ] == _Turn || _Board[ NewDirX ][ NewDirY ] == BOUNDARY )
			++Count;
	}

	if( Count >= 3 && flag == true )
		return true;

	return false;
}

///< --- 計算棋盤中黑 or 白棋子個數	
int MCTree::CountPieceNum( int _Board[BOUNDARYSIZE][BOUNDARYSIZE]  , int _Turn )
{
	int count = 0;
	for( int i = 0 ; i < BOUNDARYSIZE ; ++i )
		for(int j = 0 ; j < BOUNDARYSIZE ; ++j)
			if( _Board[i][j] == _Turn )
				++count;
	return count;
}

///< --- 判斷是否形成兩眼
bool MCTree::IsTwoEye( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] )
{
	vector<int> eyePosVec;			///< --- 形成一個 eye 的所有棋子的位置
	vector< vector<int> > eyeVec;	///< --- 全部有幾個 eye

	///< --- 找到 有可能是眼位的 格點
	for(int x = 0 ; x < BOUNDARYSIZE ; ++x )
	{
		for( int y = 0 ; y < BOUNDARYSIZE ; ++y )
		{
			if( _Board[x][y] == EMPTY )
			{
				///< --- 判斷 (x , y ) 是否為眼位
				if( this->IsEye(_Board , WHITE , x , y ) )
				{
					///< --- 把形成 該眼位的棋子 都存到 vector 中 
					vector<int> eyePiecePosVec;

					for( int i = 0 ; i < 4 ; ++i )
					{
						int dirX = x + DirectionX[ i ];
						int dirY = y + DirectionY[ i ];

						///< --- 是白棋 才要加到 vector 中
						if( _Board[ dirX ][ dirY ] == WHITE )
						{
							int move = dirX * 10 + dirY;
							eyePiecePosVec.push_back(move);
						}

						dirX = x + NewDirectX[ i ];
						dirY = y + NewDirectY[ i ];
						
						///< --- 是白棋 才要加到 vector 中
						if( _Board[ dirX ][ dirY ] == WHITE )
						{
							int move = dirX * 10 + dirY;
							eyePiecePosVec.push_back(move);
						}
					}

					///< --- 將形成眼位的 棋子列表 加到 eyeVec 中
					eyeVec.push_back(eyePiecePosVec);
				}
			}
		}
	}

	///< --- 如果 沒有兩個眼位 return false
	int Size = eyeVec.size();
	if( Size < 2 )
		return false;

	///< --- 取出第一個眼位的棋子位置，跟其他眼位的棋子位置比較，如果有棋子位置相同，代表為同一個 string
	eyePosVec = eyeVec[0];
	bool flag = false;
	for( int i = 1 ; i < Size ; ++i )
	{
		for( unsigned int j = 0 ; j < eyePosVec.size() ; ++j)
		{
			for( unsigned int k = 0 ; k < eyeVec[i].size() ; ++k )
			{
				if( eyePosVec[j] == eyeVec[i][k])
				{
					flag = true;
					break;
				}
			}
		}
	}
	return flag;
}

///< --- 判斷 (_X , _Y) 這一步是否需要捨棄掉
bool MCTree::IsGiveup( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , int _X , int _Y )
{
	/*
		如果現在是黑下，而且合法步是這種形式的話，則把此合法步捨棄掉
		 X
		X.X
		 X 
	*/
	bool flag = true;
	for( int i = 0 ; i < 4 ; ++i)
	{
		int DirX = _X + DirectionX[ i ];
		int DirY = _Y + DirectionY[ i ];
		if( _Board[ DirX ][ DirY ] != _Turn && _Board[ DirX ][ DirY ] != BOUNDARY )
		{
			flag = false;
			break;
		}
	}
	return flag;
}

///< --- 換手
int MCTree::ChangeTurn( int _Turn )
{
	int ret = ( _Turn == WHITE ) ? BLACK : WHITE;
	return ret;
}

///< --- 判斷是否有子點，以及是否有子點是勝利點
MCTreeNode *MCTree::JudgeByNode( MCTreeNode *_Node )
{
	int Size = _Node->childVec.size();
	MCTreeNode * ret = NULL;
	///< --- 沒有合法步
	if( Size == 0 )
		return _Node;

	///< --- 判斷展開後的 child 中，是否有直接獲勝的點
	for( int i = 0 ; i < Size ; ++i )
	{
		///< --- 更新完 board 後，記錄白棋個數
		int whiteCount = CountPieceNum( _Node->childVec[i]->board , WHITE );

		///< --- 判斷是否獲勝
		if( _Node->type == WHITE)
		{
			///< --- 白方要有兩眼
			if( IsTwoEye( _Node->childVec[i]->board ) )
			{
				ret = _Node->childVec[i];
				ret->score = WIN_SCORE;
				break;
			}
		}
		else
		{
			///< --- 黑方要把白方全滅
			if( whiteCount == 0 )
			{
				ret = _Node->childVec[i];
				ret->score = WIN_SCORE;
				break;
			}
		}
	}
	return ret;
}

///< --- 判斷合法步中，對於目前棋盤是否有直接勝利的步
int MCTree::JudgeByMove( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , vector<int> &_MoveVec  )
{
	int ret = -1;
	int SrcBoard[ BOUNDARYSIZE ][ BOUNDARYSIZE ];
	int JudgeBoard[ BOUNDARYSIZE ][ BOUNDARYSIZE ];
	CopyBoard( _Board , SrcBoard );

	for( unsigned int i = 0 ; i < _MoveVec.size() ; ++i )
	{
		CopyBoard( SrcBoard , JudgeBoard );
		int x = ( _MoveVec[i] % 100 ) / 10;
		int y = _MoveVec[i] % 10;

		if ( _MoveVec[i] < 100 ) 
			JudgeBoard[x][y] = _Turn;
		else
			update_board( JudgeBoard, x, y, _Turn );

		///< --- 更新完 board 後，記錄白棋個數
		int whiteCount = CountPieceNum( JudgeBoard , WHITE );

		///< --- 判斷是否獲勝
		if( _Turn == WHITE)
		{
			///< --- 白方要有兩眼
			if( IsTwoEye( JudgeBoard ) )
			{
				ret = _MoveVec[i];
				break;
			}
		}
		else
		{
			///< --- 黑方要把白方全滅
			if( whiteCount == 0 )
			{
				ret = _MoveVec[i];
				break;
			}
		}
	}
	return ret;
}