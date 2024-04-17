#include "BufferManager.h"
#include "Kernel.h"

BufferManager::BufferManager()
{
	//nothing to do here
}

BufferManager::~BufferManager()
{
	//nothing to do here
}

void BufferManager::Initialize()
{
	int i;
	Buf* bp;
	//初始化，定义自由缓存队列的队首
	this->bFreeList.b_forw = this->bFreeList.b_back = &(this->bFreeList);
	//this->bFreeList.av_forw = this->bFreeList.av_back = &(this->bFreeList);

	for(i = 0; i < NBUF; i++)
	{
		bp = &(this->m_Buf[i]);
		//bp->b_dev = -1;
		bp->b_addr = this->Buffer[i];
		/* 初始化NODEV队列 */
		//向双向链表中加入新的m_Buf[i]
		bp->b_back = &(this->bFreeList);
		bp->b_forw = this->bFreeList.b_forw;
		this->bFreeList.b_forw->b_back = bp;
		this->bFreeList.b_forw = bp;
		/* 初始化自由队列 */
		bp->b_flags = Buf::B_BUSY;
		Brelse(bp);
	}
	//初始化设备管理类，这里其实就是磁盘管理的类
	this->m_DeviceManager = &Kernel::Instance().GetDeviceManager();
	return;
}

Buf* BufferManager::GetBlk(int blkno)
{
	Buf* bp;
    Buf* dp = &(this->bFreeList);
	
    //循环在队列里寻找目标块
    for(bp = dp->b_forw; bp != (Buf *)dp; bp = bp->b_forw){
        if(bp->b_blkno == blkno ){
            return bp;
        }
    }
    //如果没有找到目标块，就在自由队列里找一个空闲块
	bp = this->bFreeList.b_forw;

	/* 如果该字符块是延迟写，将其异步写到磁盘上 */
	if(bp->b_flags & Buf::B_DELWRI)
	{
		bp->b_flags |= Buf::B_ASYNC;
		this->Bwrite(bp);
	}
	/* 注意: 这里清除了所有其他位，只设了B_BUSY */
	bp->b_flags = Buf::B_BUSY;

	/* 从原设备队列中抽出 */
	//双向链表中删除一个元素
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;
	
	//将刚刚从自由缓存队列拿出来使用的缓存块放于队列尾部，符合LRU规则
    bp->b_back = this->bFreeList.b_back;
    this->bFreeList.b_back->b_forw = bp;
    bp->b_forw = &this->bFreeList;
    this->bFreeList.b_back = bp;

	bp->b_blkno = blkno;
	return bp;
}

void BufferManager::Brelse(Buf* bp)
{

	/* 注意以下操作并没有清除B_DELWRI、B_WRITE、B_READ、B_DONE标志
	 * B_DELWRI表示虽然将该控制块释放到自由队列里面，但是有可能还没有些到磁盘上。
	 * B_DONE则是指该缓存的内容正确地反映了存储在或应存储在磁盘上的信息 
	 */
	bp->b_flags &= ~(Buf::B_WANTED | Buf::B_BUSY | Buf::B_ASYNC);
	return;
}


Buf* BufferManager::Bread(int blkno)
{
	Buf* bp;
	/* 根据设备号，字符块号申请缓存 */
	bp = this->GetBlk(blkno);
	/* 如果在设备队列中找到所需缓存，即B_DONE已设置，就不需进行I/O操作 */
	if(bp->b_flags & Buf::B_DONE)
	{
		return bp;
	}
	/* 没有找到相应缓存，构成I/O读请求块 */
	bp->b_flags |= Buf::B_READ;
	bp->b_wcount = BufferManager::BUFFER_SIZE;

	/* 驱动块设备进行I/O操作 */
	this->m_DeviceManager->IO_read(bp);
	return bp;
}

Buf* BufferManager::Breada(short adev, int blkno, int rablkno)
{
	//nothing to do here
}

void BufferManager::Bwrite(Buf *bp)
{
	unsigned int flags;

	flags = bp->b_flags;
	bp->b_flags &= ~(Buf::B_READ | Buf::B_DONE | Buf::B_ERROR | Buf::B_DELWRI);
	bp->b_wcount = BufferManager::BUFFER_SIZE;		/* 512字节 */

	this->m_DeviceManager->IO_write(bp);

	this->Brelse(bp);

    bp->b_back->b_forw = bp->b_forw;
    bp->b_forw->b_back = bp->b_back;

    bp->b_back = this->bFreeList.b_back;
    this->bFreeList.b_back->b_forw = bp;
    bp->b_forw = &this->bFreeList;
    this->bFreeList.b_back = bp;

	return;
}

void BufferManager::Bdwrite(Buf *bp)
{
	/* 置上B_DONE允许其它进程使用该磁盘块内容 */
	bp->b_flags |= (Buf::B_DELWRI | Buf::B_DONE);
	this->Brelse(bp);
	return;
}

void BufferManager::Bawrite(Buf *bp)
{
	/* 标记为异步写 */
	bp->b_flags |= Buf::B_ASYNC;
	this->Bwrite(bp);
	return;
}

void BufferManager::ClrBuf(Buf *bp)
{
	int* pInt = (int *)bp->b_addr;

	/* 将缓冲区中数据清零 */
	for(unsigned int i = 0; i < BufferManager::BUFFER_SIZE / sizeof(int); i++)
	{
		pInt[i] = 0;
	}
	return;
}

void BufferManager::Bflush()
{
    //刷新操作，将所有延迟写的缓存写回磁盘
	Buf* bp;
	for(int i=0;i<NBUF;i++){
        if(bp->b_flags & Buf::B_DELWRI){
            this->Bwrite(bp);
        }
    }
}

void BufferManager::GetError(Buf* bp)
{
	User& u = Kernel::Instance().GetUser();

	if (bp->b_flags & Buf::B_ERROR)
	{
		u.u_error = User::EIO;
	}
	return;
}

void BufferManager::NotAvail(Buf *bp)
{
	/* 设置B_BUSY标志 */
	bp->b_flags |= Buf::B_BUSY;
	return;
}

Buf* BufferManager::InCore(short adev, int blkno)
{
	Buf* bp;
	Buf* dp =&(this->bFreeList);

	for(bp = dp->b_forw; bp != (Buf *)dp; bp = bp->b_forw)
	{
		if(bp->b_blkno == blkno)
			return bp;
	}
	return NULL;
}

Buf& BufferManager::GetBFreeList()
{
	return this->bFreeList;
}

