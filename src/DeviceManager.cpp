#include "DeviceManager.h"
#include "FileSystem.h"
#include "string.h"

const char* DeviceManager::DISK_FILE_NAME = "1.img";

DeviceManager::DeviceManager()
{
    m_DiskFile = fopen(DISK_FILE_NAME, "rb+");
}

DeviceManager::~DeviceManager()
{
    if(m_DiskFile)
        fclose(m_DiskFile);
}

void DeviceManager::Initialize()
{
    if(m_DiskFile == NULL)
    {
        m_DiskFile = fopen(DISK_FILE_NAME, "wb+");
    }
    else{
        return;
    }
    if(m_DiskFile == NULL)
    {
        printf("Error: Can't open disk file.\n");
        return;
    }
    else{
        // SuperBlock区域初始化
        SuperBlock m_spb;
        fwrite(&m_spb, sizeof(SuperBlock), 1, m_DiskFile);


        //Inode区域初始化
        for(int i=0;i<FileSystem::INODE_ZONE_SIZE*FileSystem::INODE_NUMBER_PER_SECTOR;i++){
            DiskInode tmp_inode;
            if(i == FileSystem::ROOTINO){
                //根目录Inode初始化
                tmp_inode.d_mode |= Inode::IFDIR | Inode::IALLOC | Inode::IEXEC;
				tmp_inode.d_nlink = 1;
                // printf("tmp_mode=%d\n",tmp_inode.d_mode);
            }
            fwrite(&tmp_inode, sizeof(DiskInode), 1, m_DiskFile);
        }

        //数据区域初始化
        unsigned char data_buf[FileSystem::BLOCK_SIZE] = {0};
        unsigned char tmp_spb[FileSystem::BLOCK_SIZE] = {0};


        //按照分组链式索引方法，分配空闲数据盘块和对应的SuperBlock
        m_spb.s_nfree = 99;
        m_spb.s_free[0] = 0;//free[0]指向上一个SuperBlock
        for(int i=0;i<99;i++){
            m_spb.s_free[i+1] = i + FileSystem::DATA_ZONE_START_SECTOR;
            fwrite(data_buf, FileSystem::BLOCK_SIZE, 1, m_DiskFile);
        }
        memcpy(tmp_spb, &(m_spb.s_nfree), sizeof(int)*101);
        fwrite(tmp_spb, FileSystem::BLOCK_SIZE, 1, m_DiskFile);
        //以上完成了第一个SuperBlock和其对应的空闲数据块的初始化，后续循环进行即可

        m_spb.s_nfree = 1;
        m_spb.s_free[0] = FileSystem::DATA_ZONE_START_SECTOR + 99;//指向第一个SuperBlock
        for(int i=100;i<FileSystem::DATA_ZONE_SIZE;i++){
            if(m_spb.s_nfree >= 100){
                memcpy(tmp_spb, &(m_spb.s_nfree), sizeof(int)*101);
                fwrite(tmp_spb, FileSystem::BLOCK_SIZE, 1, m_DiskFile);
                m_spb.s_nfree = 0;
            }
            else{
                fwrite(data_buf, FileSystem::BLOCK_SIZE, 1, m_DiskFile);
            }
            m_spb.s_free[m_spb.s_nfree++] = i + FileSystem::DATA_ZONE_START_SECTOR;
        }

        //最后重新将SuperBlock写回
        fseek(m_DiskFile, 0, SEEK_SET);
        fwrite(&m_spb, sizeof(SuperBlock), 1, m_DiskFile);

    }
}

void DeviceManager::FormatDisk()
{
   //格式化文件系统本质上就是重新初始化一个文件系统
    //这里我们直接删除文件，然后重新创建一个文件
    if(m_DiskFile)
        fclose(m_DiskFile);
    remove(DISK_FILE_NAME);
}


//这里的IO_read是将磁盘中对应地址的文件内容读到了高速缓冲块的内存地址
void DeviceManager::IO_read(Buf* bp)
{
    fseek(m_DiskFile, bp->b_blkno*FileSystem::BLOCK_SIZE, SEEK_SET);
    fread(bp->b_addr, bp->b_wcount, 1, m_DiskFile);
    bp->b_flags &= ~(Buf::B_READ);
	bp->b_flags |= Buf::B_DONE;
    // printf("read from no.%d block\n", bp->b_blkno);
}

//这里的IO_write是将高速缓冲块中的内容写入了磁盘的对应地址
void DeviceManager::IO_write(Buf* bp)
{
    fseek(m_DiskFile, bp->b_blkno*FileSystem::BLOCK_SIZE, SEEK_SET);
    fwrite(bp->b_addr, bp->b_wcount, 1, m_DiskFile);
    bp->b_flags &= ~(Buf::B_WRITE);
    bp->b_flags |= Buf::B_DONE;
    // printf("write to no.%d block\n", bp->b_blkno);
}