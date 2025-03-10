#include "FileSystem.h"
#include "Utility.h"
#include "Kernel.h"
#include "OpenFileManager.h"
#include "time.h"

/*==============================class SuperBlock===================================*/
/* 系统全局超级块SuperBlock对象 */
SuperBlock g_spb;

SuperBlock::SuperBlock()
{
	//考虑到每个SuperBlock都有相似的地方，所以我们在构造函数处加上一部分初始化
    s_isize = FileSystem::INODE_ZONE_SIZE;  /* 外存Inode区占用的盘块数 */
    s_fsize = FileSystem::DATA_ZONE_END_SECTOR + 1;
    s_ninode = 100;
    for (int i = 1; i <= s_ninode; i++)
			s_inode[i] = i;
    s_flock = 0;
	s_ilock = 0;
	s_fmod = 0;
	s_ronly = 0;
	s_time = time(0);
}

SuperBlock::~SuperBlock()
{
	//nothing to do here
}



/*==============================class FileSystem===================================*/
FileSystem::FileSystem()
{
	//nothing to do here
}

FileSystem::~FileSystem()
{
	//nothing to do here
}

void FileSystem::Initialize()
{
	this->m_BufferManager = &Kernel::Instance().GetBufferManager();
}

void FileSystem::LoadSuperBlock()
{
	//系统初始化时从myDisk.img中读取超级块
	User& u = Kernel::Instance().GetUser();
	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
	Buf* pBuf;

	for (int i = 0; i < 2; i++)
	{
		int* p = (int *)&g_spb + i * 128;

		pBuf = bufMgr.Bread(FileSystem::SUPER_BLOCK_SECTOR_NUMBER + i);

		Utility::DWordCopy((int *)pBuf->b_addr, p, 128);

		bufMgr.Brelse(pBuf);
	}

	g_spb.s_flock = 0;
	g_spb.s_ilock = 0;
	g_spb.s_ronly = 0;
	g_spb.s_time = time(0);
}

SuperBlock* FileSystem::GetFS()
{
	SuperBlock* m_spb = &g_spb;
	
	if(m_spb->s_nfree > 100 || m_spb->s_ninode > 100)
	{
		m_spb->s_nfree = 0;
		m_spb->s_ninode = 0;
	}
	return m_spb;
}

void FileSystem::Update()
{
	//将内存中的SuperBlock同步到磁盘
	int i;
	SuperBlock* m_spb = &g_spb;
	Buf* pBuf;

	/* 同步SuperBlock到磁盘 */

	/* 如果该SuperBlock内存副本没有被修改，直接管理inode和空闲盘块被上锁或该文件系统是只读文件系统 */
	if(m_spb->s_fmod == 0 || m_spb->s_ilock != 0 || m_spb->s_flock != 0 || m_spb->s_ronly != 0)
	{
		return;
	}

	/* 清SuperBlock修改标志 */
	m_spb->s_fmod = 0;
	/* 写入SuperBlock最后存访时间 */
	m_spb->s_time = time(0);

	/* 
		* 为将要写回到磁盘上去的SuperBlock申请一块缓存，由于缓存块大小为512字节，
		* SuperBlock大小为1024字节，占据2个连续的扇区，所以需要2次写入操作。
		*/
	for(int j = 0; j < 2; j++)
	{
		/* 第一次p指向SuperBlock的第0字节，第二次p指向第512字节 */
		int* p = (int *)m_spb + j * 128;//因为int型是4个字节，所以这里是128个int

		/* 将要写入到设备dev上的SUPER_BLOCK_SECTOR_NUMBER + j扇区中去 */
		pBuf = this->m_BufferManager->GetBlk(FileSystem::SUPER_BLOCK_SECTOR_NUMBER + j);

		/* 将SuperBlock中第0 - 511字节写入缓存区 */
		Utility::DWordCopy(p, (int *)pBuf->b_addr, 128);

		/* 将缓冲区中的数据写到磁盘上 */
		this->m_BufferManager->Bwrite(pBuf);
	}

	
	/* 同步修改过的内存Inode到对应外存Inode */
	g_InodeTable.UpdateInodeTable();

	/* 将延迟写的缓存块写到磁盘上 */
	this->m_BufferManager->Bflush();
}

Inode* FileSystem::IAlloc()
{
	SuperBlock* m_spb = this->GetFS();
	Buf* pBuf;
	Inode* pNode;
	User& u = Kernel::Instance().GetUser();
	int ino;	/* 分配到的空闲外存Inode编号 */

	if(m_spb->s_ninode <= 0)
	{

		/* 外存Inode编号从0开始，这不同于Unix V6中外存Inode从1开始编号 */
		ino = -1;

		/* 依次读入磁盘Inode区中的磁盘块，搜索其中空闲外存Inode，记入空闲Inode索引表 */
		for(int i = 0; i < m_spb->s_isize; i++)
		{
			pBuf = this->m_BufferManager->Bread(FileSystem::INODE_ZONE_START_SECTOR + i);

			/* 获取缓冲区首址 */
			int* p = (int *)pBuf->b_addr;

			/* 检查该缓冲区中每个外存Inode的i_mode != 0，表示已经被占用 */
			for(int j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; j++)
			{
				ino++;

				int mode = *( p + j * sizeof(DiskInode)/sizeof(int) );

				/* 该外存Inode已被占用，不能记入空闲Inode索引表 */
				if(mode != 0)
				{
					continue;
				}

				/* 
				 * 如果外存inode的i_mode==0，此时并不能确定
				 * 该inode是空闲的，因为有可能是内存inode没有写到
				 * 磁盘上,所以要继续搜索内存inode中是否有相应的项
				 */
				if( g_InodeTable.IsLoaded(ino) == -1 )
				{
					/* 该外存Inode没有对应的内存拷贝，将其记入空闲Inode索引表 */
					m_spb->s_inode[m_spb->s_ninode++] = ino;

					/* 如果空闲索引表已经装满，则不继续搜索 */
					if(m_spb->s_ninode >= 100)
					{
						break;
					}
				}
			}

			/* 至此已读完当前磁盘块，释放相应的缓存 */
			this->m_BufferManager->Brelse(pBuf);

			/* 如果空闲索引表已经装满，则不继续搜索 */
			if(m_spb->s_ninode >= 100)
			{
				break;
			}
		}
		
		/* 如果在磁盘上没有搜索到任何可用外存Inode，返回NULL */
		if(m_spb->s_ninode <= 0)
		{
			printf("No space on device\n");
			u.u_error = User::ENOSPC;
			return NULL;
		}
	}

	/* 
	 * 上面部分已经保证，除非系统中没有可用外存Inode，
	 * 否则空闲Inode索引表中必定会记录可用外存Inode的编号。
	 */
	while(true)
	{
		/* 从索引表“栈顶”获取空闲外存Inode编号 */
		ino = m_spb->s_inode[--m_spb->s_ninode];

		/* 将空闲Inode读入内存 */
		pNode = g_InodeTable.IGet(ino);
		/* 未能分配到内存inode */
		if(NULL == pNode)
		{
			return NULL;
		}

		/* 如果该Inode空闲,清空Inode中的数据 */
		if(0 == pNode->i_mode)
		{
			pNode->Clean();
			/* 设置SuperBlock被修改标志 */
			m_spb->s_fmod = 1;
			return pNode;
		}
		else	/* 如果该Inode已被占用 */
		{
			g_InodeTable.IPut(pNode);
			continue;	/* while循环 */
		}
	}
	return NULL;	/* GCC likes it! */
}

void FileSystem::IFree(int number)
{
	SuperBlock* m_spb = this->GetFS();

	/* 
	 * 如果超级块直接管理的空闲外存Inode超过100个，
	 * 同样让释放的外存Inode散落在磁盘Inode区中。
	 */
	if(m_spb->s_ninode >= 100)
	{
		return;
	}

	m_spb->s_inode[m_spb->s_ninode++] = number;

	/* 设置SuperBlock被修改标志 */
	m_spb->s_fmod = 1;
}

Buf* FileSystem::Alloc()
{
	int blkno;	/* 分配到的空闲磁盘块编号 */
	SuperBlock* m_spb = this->GetFS();
	Buf* pBuf;
	User& u = Kernel::Instance().GetUser();


	/* 从索引表“栈顶”获取空闲磁盘块编号 */
	
	blkno = m_spb->s_free[--m_spb->s_nfree];
	/* 
	 * 若获取磁盘块编号为零，则表示已分配尽所有的空闲磁盘块。
	 * 或者分配到的空闲磁盘块编号不属于数据盘块区域中(由BadBlock()检查)，
	 * 都意味着分配空闲磁盘块操作失败。
	 */
	if(0 == blkno )
	{
		m_spb->s_nfree = 0;
		printf("No space on device\n");
		u.u_error = User::ENOSPC;
		return NULL;
	}
	/* 
	 * 栈已空，新分配到空闲磁盘块中记录了下一组空闲磁盘块的编号,
	 * 将下一组空闲磁盘块的编号读入SuperBlock的空闲磁盘块索引表s_free[100]中。
	 */
	if(m_spb->s_nfree <= 0)
	{

		/* 读入该空闲磁盘块 */
		pBuf = this->m_BufferManager->Bread(blkno);

		/* 从该磁盘块的0字节开始记录，共占据4(s_nfree)+400(s_free[100])个字节 */
		int* p = (int *)pBuf->b_addr;

		/* 首先读出空闲盘块数s_nfree */	
		m_spb->s_nfree = *p++;
		/* 读取缓存中后续位置的数据，写入到SuperBlock空闲盘块索引表s_free[100]中 */
		Utility::DWordCopy(p, m_spb->s_free, 100);

		/* 缓存使用完毕，释放以便被其它进程使用 */
		this->m_BufferManager->Brelse(pBuf);

	}

	/* 普通情况下成功分配到一空闲磁盘块 */
	pBuf = this->m_BufferManager->GetBlk(blkno);	/* 为该磁盘块申请缓存 */
	this->m_BufferManager->ClrBuf(pBuf);	/* 清空缓存中的数据 */
	m_spb->s_fmod = 1;	/* 设置SuperBlock被修改标志 */

	return pBuf;
}

void FileSystem::Free(int blkno)
{
	SuperBlock* m_spb = this->GetFS();
	Buf* pBuf;
	User& u = Kernel::Instance().GetUser();

	/* 
	 * 如果先前系统中已经没有空闲盘块，
	 * 现在释放的是系统中第1块空闲盘块
	 */
	if(m_spb->s_nfree <= 0)
	{
		m_spb->s_nfree = 1;
		m_spb->s_free[0] = 0;	/* 使用0标记空闲盘块链结束标志 */
	}

	/* SuperBlock中直接管理空闲磁盘块号的栈已满 */
	if(m_spb->s_nfree >= 100)
	{
		m_spb->s_flock++;

		/* 
		 * 使用当前Free()函数正要释放的磁盘块，存放前一组100个空闲
		 * 磁盘块的索引表
		 */
		pBuf = this->m_BufferManager->GetBlk(blkno);	/* 为当前正要释放的磁盘块分配缓存 */

		/* 从该磁盘块的0字节开始记录，共占据4(s_nfree)+400(s_free[100])个字节 */
		int* p = (int *)pBuf->b_addr;
		
		/* 首先写入空闲盘块数，除了第一组为99块，后续每组都是100块 */
		*p++ = m_spb->s_nfree;
		/* 将SuperBlock的空闲盘块索引表s_free[100]写入缓存中后续位置 */
		Utility::DWordCopy(m_spb->s_free, p, 100);

		m_spb->s_nfree = 0;
		/* 将存放空闲盘块索引表的“当前释放盘块”写入磁盘，即实现了空闲盘块记录空闲盘块号的目标 */
		this->m_BufferManager->Bwrite(pBuf);

	}
	m_spb->s_free[m_spb->s_nfree++] = blkno;	/* SuperBlock中记录下当前释放盘块号 */
	m_spb->s_fmod = 1;
}

