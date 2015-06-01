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
	///< --- �N�ثe�ǤJ���L���� root
	m_Root = new MCTreeNode( _Turn );
	this->CopyBoard( _Board , m_Root->board );
	m_Current = m_Root;

	///< --- copy �ثe�� ���v�L���A�]�����������ͦX�k�B�ݭn
	m_GameLength = _GameLength;
	for( int i = 0 ; i < m_GameLength ; ++i )
		this->CopyBoard( _GameRecord[i] , m_GameRecord[i] );

	int Time = _Time - 1;
	const float TimeOfPare1 = 3.0f / 4.0f;
	const float TimeOfPare2 = 1.0f / 4.0f;
	Time = (int)floor( Time * TimeOfPare1 );

	///< --- �i��Ĥ@��������
	this->DoLoop( Time );

	///< --- root �U�S������l�I�A�^�� 0
	if( m_Root->childVec.empty() )
		return 0;

	///< --- ���� PV �����l�I���W��s�Ӳv
	m_Current = this->BackUp( m_Current );

	///< --- ��X��s�Ӳv��A�̨Ϊ� Node
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

	///< --- �̨Ϊ� node �̵M���P�@��
	if( m_Current ==  pTmpNode )
		return m_Current->move;


	///< --- ���s�j�M�@���̨� node
	m_Current = pTmpNode;
	Time = (int)ceil( Time * TimeOfPare2 );
	this->DoLoop( Time );

	///< --- ���� �s��PV �����l�I���W��s�Ӳv
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

///< --- Search �t��k���D�n�j��
void MCTree::DoLoop( int _Time )
{
	clock_t start_t, end_t, now_t;
	///< --- �O���}�l�ɶ�
    now_t = start_t = clock();
	///< --- �p�⵲�����ɶ��I ( �O�d�@�� )
    end_t = start_t + CLOCKS_PER_SEC *  _Time ;

	while( now_t < end_t )
	{
		///< --- �i�}�ثe node �� child
		this->Expand( m_Current );
		
		///< --- ��ثe node ����B�P�_ : �O�_�X�k�B�� 0�A�άO�w�g���@�B�N��Ӫ� node
		MCTreeNode* tmpNode = NULL;
		tmpNode = this->JudgeByNode( m_Current );
		if( tmpNode != NULL )
		{
			m_Current = tmpNode;
			///< --- ��J�@�Ǽƭ�
			m_Current->mean = m_Current->score;
			m_Current->visit = 1;
			m_Current->meanR = m_Current->mean;
			return;
		}

		///< --- ��ثe node �� child �i�����
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
			///< --- ���X�{�b�ɶ�
			now_t = clock();
		}

		///< --- ����t�� child �屼
		///< --- �Ҧ��� child �� �зǮt���p��w�]���ؼмзǮt�A��������
		if( this->PruningBadChild( m_Current ) )
		{
			if( m_Current == m_Root )
			{
				int randChildIdx = rand() % m_Current->childVec.size();
				m_Current = m_Current->childVec[ randChildIdx ];
			}
			return;
		}
		///< --- �p�G�姹�u�Ѥ@�� child ���N��������
		if( m_Current->childVec.size() == 1 )
		{
			m_Current = m_Current->childVec[0];
			return;
		}

		///< --- ��ѤU�� node �A������
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
			///< --- ���X�{�b�ɶ�
			now_t = clock();
		}
		
		///< --- �ĤG��������A����t�� child �屼		
		///< --- �Ҧ��� child �� �зǮt���p��w�]���ؼмзǮt�A��������
		if( this->PruningBadChild( m_Current ) )
		{
			if( m_Current == m_Root )
			{
				int randChildIdx = rand() % m_Current->childVec.size();
				m_Current = m_Current->childVec[ randChildIdx ];
			}
			return;
		}
		///< --- �p�G�姹�u�Ѥ@�� child ���N��������
		if( m_Current->childVec.size() == 1 )
		{
			m_Current = m_Current->childVec[0];
			return;
		}

		///< --- �p�� UCB�A���o�̦n UCB �� child�A�~�����
		m_Current = this->GetBestChildByUCB( m_Current );

		///< --- ���X�{�b�ɶ�
		now_t = clock();
	}
}

///< --- �i�}�l�I
void MCTree::Expand( MCTreeNode *_Node )
{
	///< --- ��X�X�k������
	int MoveList[49];
	int NumOfMove = gen_legal_move( _Node->board , _Node->type , m_GameLength , m_GameRecord , MoveList);

	///< --- ��C�ӦX�k�B�P�_�O�_���ݭn�˱󱼪��X�k�B
	for( int i = 0 ; i < NumOfMove ; ++i  )
	{
		int x = ( MoveList[ i ] % 100 ) / 10;
		int y = MoveList[ i ] % 10;

		///< --- ���O�ݭn�˱󱼪��B�A�N���B�[�� root �� child �� 
		if( !this->IsEye( _Node->board , _Node->type , x , y ) ) //!this->IsGiveup( _Node->board , _Node->type , x , y ) )
		{
			MCTreeNode *pChild = new MCTreeNode( ChangeTurn( _Node->type ) );
			CopyBoard( _Node->board , pChild->board );
			pChild->move = MoveList[ i ];
			_Node->childVec.push_back( pChild );
			pChild->parent = _Node;
		}
	}

	///< --- ���� child ���L��
	for( unsigned int i = 0 ; i < _Node->childVec.size() ; ++i )
	{
		int x = ( _Node->childVec[i]->move % 100 ) / 10;
		int y = _Node->childVec[i]->move % 10;
		update_board( _Node->childVec[i]->board , x , y , _Node->type );
	}
}

///< --- ��i�����
void MCTree::Simulation( MCTreeNode *_Node , int _Idx )
{
	///< --- �{�b���� ���@��U
	MCTreeNode *pChild = _Node->childVec[ _Idx ];
	int turn = pChild->type;
	int GameLength = 0;
	int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE];

	///< --- �N Board ���� copy �� SimBoard ��
	CopyBoard( pChild->board , pChild->simBoard );
	CopyBoard( pChild->simBoard , GameRecord[ GameLength ] );

	///< --- �Ψӭp�� N
	++_Node->visit;
	++pChild->visit;

	///< --- �P�_�O�_�s���賣 pass
	int passCount = 0;

	int NumOfMove = 0;
	int MoveList[ 49 ];

	///< --- �@���U�쵲���A�άO�W�L 50 �B
	while( GameLength < MAX_STEP )  
	{
		///< --- ��X�X�k������	
		NumOfMove = gen_legal_move( pChild->simBoard, turn , GameLength, GameRecord, MoveList);
		vector<int> MoveVec;
		MoveVec.clear();
		
		///< --- �ˬd�C�ӦX�k�����ʡA�ݬO�_�ݭn�ŵ��A�p�G���ݭn�A�N��ӨB��J Vecotr ��
		for( int i = 0 ; i < NumOfMove ; ++i )
		{
			int x = ( MoveList[i] % 100 ) / 10;
			int y = MoveList[i] % 10;
			///< --- �n�d�ۡA��J Vecotr 
			if( ! this->IsEye( pChild->simBoard , turn , x , y )  )//! IsGiveup( pChild->simBoard , turn , x , y ) )
				MoveVec.push_back( MoveList[i] );
		}

		///< --- ���@��S���l => ����
		int pieceCountW = this->CountPieceNum( pChild->simBoard , WHITE );
		int pieceCountB = this->CountPieceNum( pChild->simBoard , BLACK );
		if( pieceCountW == 0 || pieceCountB == 0 )
			break;
			
		///< --- �S���X�k�B => PASS
		///< --- �p�G ���賣 PASS => ����
		if( !MoveVec.empty() )
			passCount = 0;
		else 
		{
			///< --- PASS �����
			++passCount;
			turn = this->ChangeTurn( turn );
				
			///< --- ����s�� pass
			if( passCount >= 2 )
				break;
			continue;
		}

		///< --- �P�_�{�b�O�_��������Ӫ��B
		int nowMove = JudgeByMove( pChild->simBoard , turn , MoveVec );
		if( nowMove == -1 )
		{
			///< --- �q MoveVec ���H���D�@�ӦX�k�B
			int moveIdx = rand() % MoveVec.size();
			nowMove = MoveVec[ moveIdx ];
		}

		///< --- �� �D��X�Ӫ��X�k�B
		int move_x = (nowMove % 100) / 10;
		int move_y = nowMove % 10;
		if ( nowMove < 100 ) 
			pChild->simBoard[move_x][move_y] = turn;
		else
			update_board( pChild->simBoard, move_x, move_y, turn );

		///< --- �O�� Game Record �קK repeat
		++GameLength;
		CopyBoard( pChild->simBoard , GameRecord[ GameLength ] );

		///< --- �����U
		turn = this->ChangeTurn( turn );
	}

	///< --- �i��������G���P�_
	///< --- �W�L ����B��
	if( GameLength >= MAX_STEP )
	{
		pChild->scoreVec.push_back( 0.0f );
		return;
	}

	///< --- node ���´� 
	if( _Node->type == BLACK )
	{
		///< --- �P�_�O�_�Y���մ�
		if( this->CountPieceNum( pChild->simBoard , WHITE ) == 0 )
		{
			pChild->score += WIN_SCORE;
			pChild->scoreVec.push_back( WIN_SCORE );
		}
		else
			pChild->scoreVec.push_back( 0.0f );
	}
	///< --- node ���մ�
	else if( _Node->type == WHITE )  
	{
		///< --- �P�_�O�_�٦��l
		if( this->CountPieceNum( pChild->simBoard , WHITE ) != 0 )
		{
			pChild->score += WIN_SCORE;
			pChild->scoreVec.push_back( WIN_SCORE );
		}
		else
			pChild->scoreVec.push_back( 0.0f );
	}
}

///< --- �p�� Node ���C�Ӥl�I�� UCB �ȡA�æ^�Ǧ��̦n UCB �Ȫ� Node
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
	
///< --- �p��C�� child �������ȸ�зǮt�A�M�����ƤӧC�� Childe
bool MCTree::PruningBadChild( MCTreeNode *_Node )
{
	const float rd = 1.0f;
	const float dev = 0.25f;

	///< --- ��C�� node �� child �p��:���ƥ����ȡA�зǮt�AmeanL�AmenaR
	int Size = _Node->childVec.size();
	MCTreeNode *pChild = NULL;
	for( int i = 0 ; i < Size ; ++i )
	{
		pChild = _Node->childVec[i];
		///< --- ���k 0
		pChild->mean = 0.0f;	
		int ScoreSize = pChild->scoreVec.size();
		
		if( ScoreSize == 0 )
			continue;

		///< --- �p�⥭��
		for( int j = 0 ; j < ScoreSize ; ++j )
			pChild->mean += pChild->scoreVec[j];
		pChild->mean /= (float)ScoreSize;

		///< --- �p��зǮt
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

		///< --- �p�� meanL�AmenaR
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
		flag = false;	///< --- �קK���ƥ[�J m1

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

	///< --- ��s�b clearNodeVec ���� node �� delete ��
	vector< MCTreeNode* >::iterator itr;
	for( itr = clearNodeVec.begin() ; itr != clearNodeVec.end() ; ++itr)
	{
		(*itr)->parent = NULL;
		delete (*itr);
		(*itr) = NULL;
	}

	///< --- ���٦b childVec �� child �����s��z�A���� NULL �� child
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

///< --- ���������ɡA�N��쪺 Node BackUp �^�� root
MCTreeNode* MCTree::BackUp( MCTreeNode* _Node )
{
	MCTreeNode* ret = _Node;
	int count = 1;
	float vRatio = 0.0f;
	while( ret->parent != m_Root )
	{
		++count;
		///< --- ret �� node �O�� m_Root �����a�ҤU����
		if( ret->type != m_Root->type  )
			vRatio += ret->mean;
		else if( ret->type == m_Root->type )
			vRatio += (1.0f - ret->mean);

		ret = ret->parent;
	}

	///< --- ��s �Ӳv
	vRatio += ret->mean;
	ret->mean = (vRatio / count);
	return ret;
}

///< --- Copy �ѽL
void MCTree::CopyBoard( int _Src[BOUNDARYSIZE][BOUNDARYSIZE] , int _Dis[BOUNDARYSIZE][BOUNDARYSIZE] )
{
	for( int i = 0 ; i < BOUNDARYSIZE ; ++i )
		for( int j = 0 ; j < BOUNDARYSIZE ; ++j )
			_Dis[i][j] = _Src[i][j];
}

///< --- �P�_ (X, Y) �O�_������
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

///< --- �p��ѽL���� or �մѤl�Ӽ�	
int MCTree::CountPieceNum( int _Board[BOUNDARYSIZE][BOUNDARYSIZE]  , int _Turn )
{
	int count = 0;
	for( int i = 0 ; i < BOUNDARYSIZE ; ++i )
		for(int j = 0 ; j < BOUNDARYSIZE ; ++j)
			if( _Board[i][j] == _Turn )
				++count;
	return count;
}

///< --- �P�_�O�_�Φ��Ⲵ
bool MCTree::IsTwoEye( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] )
{
	vector<int> eyePosVec;			///< --- �Φ��@�� eye ���Ҧ��Ѥl����m
	vector< vector<int> > eyeVec;	///< --- �������X�� eye

	///< --- ��� ���i��O���쪺 ���I
	for(int x = 0 ; x < BOUNDARYSIZE ; ++x )
	{
		for( int y = 0 ; y < BOUNDARYSIZE ; ++y )
		{
			if( _Board[x][y] == EMPTY )
			{
				///< --- �P�_ (x , y ) �O�_������
				if( this->IsEye(_Board , WHITE , x , y ) )
				{
					///< --- ��Φ� �Ӳ��쪺�Ѥl ���s�� vector �� 
					vector<int> eyePiecePosVec;

					for( int i = 0 ; i < 4 ; ++i )
					{
						int dirX = x + DirectionX[ i ];
						int dirY = y + DirectionY[ i ];

						///< --- �O�մ� �~�n�[�� vector ��
						if( _Board[ dirX ][ dirY ] == WHITE )
						{
							int move = dirX * 10 + dirY;
							eyePiecePosVec.push_back(move);
						}

						dirX = x + NewDirectX[ i ];
						dirY = y + NewDirectY[ i ];
						
						///< --- �O�մ� �~�n�[�� vector ��
						if( _Board[ dirX ][ dirY ] == WHITE )
						{
							int move = dirX * 10 + dirY;
							eyePiecePosVec.push_back(move);
						}
					}

					///< --- �N�Φ����쪺 �Ѥl�C�� �[�� eyeVec ��
					eyeVec.push_back(eyePiecePosVec);
				}
			}
		}
	}

	///< --- �p�G �S����Ӳ��� return false
	int Size = eyeVec.size();
	if( Size < 2 )
		return false;

	///< --- ���X�Ĥ@�Ӳ��쪺�Ѥl��m�A���L���쪺�Ѥl��m����A�p�G���Ѥl��m�ۦP�A�N���P�@�� string
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

///< --- �P�_ (_X , _Y) �o�@�B�O�_�ݭn�˱�
bool MCTree::IsGiveup( int _Board[BOUNDARYSIZE][BOUNDARYSIZE] , int _Turn , int _X , int _Y )
{
	/*
		�p�G�{�b�O�¤U�A�ӥB�X�k�B�O�o�اΦ����ܡA�h�⦹�X�k�B�˱�
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

///< --- ����
int MCTree::ChangeTurn( int _Turn )
{
	int ret = ( _Turn == WHITE ) ? BLACK : WHITE;
	return ret;
}

///< --- �P�_�O�_���l�I�A�H�άO�_���l�I�O�ӧQ�I
MCTreeNode *MCTree::JudgeByNode( MCTreeNode *_Node )
{
	int Size = _Node->childVec.size();
	MCTreeNode * ret = NULL;
	///< --- �S���X�k�B
	if( Size == 0 )
		return _Node;

	///< --- �P�_�i�}�᪺ child ���A�O�_��������Ӫ��I
	for( int i = 0 ; i < Size ; ++i )
	{
		///< --- ��s�� board ��A�O���մѭӼ�
		int whiteCount = CountPieceNum( _Node->childVec[i]->board , WHITE );

		///< --- �P�_�O�_���
		if( _Node->type == WHITE)
		{
			///< --- �դ�n���Ⲵ
			if( IsTwoEye( _Node->childVec[i]->board ) )
			{
				ret = _Node->childVec[i];
				ret->score = WIN_SCORE;
				break;
			}
		}
		else
		{
			///< --- �¤�n��դ����
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

///< --- �P�_�X�k�B���A���ثe�ѽL�O�_�������ӧQ���B
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

		///< --- ��s�� board ��A�O���մѭӼ�
		int whiteCount = CountPieceNum( JudgeBoard , WHITE );

		///< --- �P�_�O�_���
		if( _Turn == WHITE)
		{
			///< --- �դ�n���Ⲵ
			if( IsTwoEye( JudgeBoard ) )
			{
				ret = _MoveVec[i];
				break;
			}
		}
		else
		{
			///< --- �¤�n��դ����
			if( whiteCount == 0 )
			{
				ret = _MoveVec[i];
				break;
			}
		}
	}
	return ret;
}