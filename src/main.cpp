/*
    main函数是主函数，整个文件系统中由main函数调用Kernel类进行文件
    系统的初始化，文件读写，打开关闭等操作，负责管理磁盘，高速缓存，内存等等
    的数据结构，Command类是自己定义的一个命令行类，用来帮助用户更好的使用
    本文件系统，本质上相当于封装好的一些FileManager的API接口，包括一些常见的
    文件创建，打开，写入，读出，关闭等命令接口和文件系统的整个可视化工作。
*/
#include<cstring>
#include<stdio.h>
#include <cstdlib>
#include "Command.h"
#include "Kernel.h"
#include "time.h"
const int update_freq = 100;

void Init()
{
    printf("UNIX FILESYSTEM INITIALIZING...\n");
    FileManager *file_m = &Kernel::Instance().GetFileManager();
    //初始化根目录
    file_m->rootDirInode = g_InodeTable.IGet(FileSystem::ROOTINO);
    FileSystem * file_s = &Kernel::Instance().GetFileSystem();
    //从磁盘文件中读取超级块，读入内存
    file_s->LoadSuperBlock();
    User * u = &Kernel::Instance().GetUser();
    //初始化User
    u->u_error = User::NOERROR;
    //初始目录设置为根目录
    u->u_cdir = g_InodeTable.IGet(FileSystem::ROOTINO);
    u->u_pdir = NULL;
    strcpy(u->u_curdir, "/");
	u->u_dirp = "/";
    //根目录的inode初始化出来不一样
    // printf("user_mode=%d\n",u->u_cdir->i_mode);
	memset(u->u_arg, 0, sizeof(u->u_arg));
	printf("INITIALIZING DONE!\n");
}

int main()
{
    Kernel::Instance().Initialize();//文件系统启动，初始化工作
    Init();
    User *u = &Kernel::Instance().GetUser();
    const char* computerName = std::getenv("COMPUTERNAME");
    Command cmd;
    //内核启动，进行初始化相关工作
    printf("Welcome!\n");
    printf("You can type \"help\" to get more information about the filesystem!\n");
    time_t start_t = time(0);
    while(true){
        time_t now_t = time(0);
        if((now_t - start_t) % update_freq == 0){
            //每100s将内存中的内容更新到磁盘，以防出现错误异常退出时，磁盘无任何数据保留
            FileSystem *file_m = &Kernel::Instance().GetFileSystem();
            //将内存中的SuperBlock同步到磁盘
            file_m -> Update();
        }
        printf("root@%s:~%s$ ",computerName,u->u_curdir);
        u->u_error = User::NOERROR;
        
        char buf[512];
        scanf("%s",buf);
        int ret = cmd.analyze(buf);
        if(ret == EXIT_CMD){
            break;
        }
    }
    return 0;
}