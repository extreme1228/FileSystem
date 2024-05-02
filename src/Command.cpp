#include"Command.h"

void Command::help()
{
    printf("fformat                                     - 格式化文件系统\n");
	printf("mkdir      <dir name>                       - 创建目录\n");
	printf("cd         <dir name>                       - 进入目录\n");
	printf("ls                                          - 显示当前目录清单\n");
	printf("fcreat     <file name>                      - 创建新文件\n");
	printf("fdelete    <file name>                      - 删除文件\n");
    printf("fopen      <file name>                      - 打开文件\n");
    printf("fcolse     <file name>                      - 关闭文件\n");
	printf("fread      <fd> <buffer> <length>           - 根据文件指针读文件\n");
    printf("fwrite     <fd> <buffer> <length>           - 根据文件指针写文件\n");
    printf("flseek     <fd> <position>                  - 调整文件指针位置\n");
	printf("fin        <out_file_name> <in_file_name>   - 将外部文件读入文件系统\n");
	printf("fout       <in_file_name> <out_file_name>   - 将内部文件读出文件系统\n");
	printf("help                                        - 显示命令清单\n");
	printf("clear                                       - 清屏\n");
	printf("exit                                        - 退出系统\n");
	return;
}

void Command::exit()
{
	//文件系统的退出命令，正常关闭所有文件，安全退出文件系统。
	BufferManager *buff_m = &Kernel::Instance().GetBufferManager();
	buff_m ->Bflush();//clear the buffer
	InodeTable *inode_t = Kernel::Instance().GetFileManager().m_InodeTable;
	inode_t ->UpdateInodeTable();//清除所有进程打开文件。
	FileSystem *file_m = &Kernel::Instance().GetFileSystem();
	//将内存中的SuperBlock同步到磁盘
	file_m -> Update();
	return ;
}

void Command::FFormat()
{
	printf("Are you sure to fromat the filesystem?(All data will be cleared in the disk)[y/n]\n");
	char buf[64];
	scanf("%s",buf);
	if(strcmp(buf,"y") == 0){
		printf("FileSystem Formatting...\n");
		DeviceManager *device_m = &Kernel::Instance().GetDeviceManager();
		device_m ->FormatDisk();
		Kernel::Instance().Initialize();
		printf("Initialize System...");
		FileManager *fileManager = &Kernel::Instance().GetFileManager();
		fileManager->rootDirInode = g_InodeTable.IGet(FileSystem::ROOTINO);
		Kernel::Instance().GetFileSystem().LoadSuperBlock();
		User *u = &Kernel::Instance().GetUser();
		u->u_error = User::NOERROR;
		u->u_cdir = g_InodeTable.IGet(FileSystem::ROOTINO);
		u->u_pdir = NULL;
		strcpy(u->u_curdir, "/");
		u->u_dirp = "/";
		memset(u->u_arg, 0, sizeof(u->u_arg));
		printf("Done.\n");
		return ;
	}
	else if(strcmp(buf,"n") == 0){
		return ;
	}
	else{
		printf("Unknown command \"%s\" !\n");
		return ;
	}
}

void Command::mkdir(char*dir_name)
{
	User *u = &Kernel::Instance().GetUser();
	// printf("dir=%s\n",u->u_curdir);
	u->u_error = User::NOERROR;
	u->system_ret = 0;
	int defaultmode = 040755; //default newmode:- rwx r-w r-w
	u->u_dirp = dir_name;
	u->u_arg[1] = defaultmode;
	u->u_arg[2] = 0;
	FileManager *file_m = &Kernel::Instance().GetFileManager();
	file_m->MkNod();
	// printf("error = %d\n",u->u_error);
	if(u -> u_error == User::EEXIST){
		printf("mkdir: cannot create directory \'%s\': File exists\n",dir_name);
	}
	return ;
}

void get_fa_dir(char*dir_name)
{
	// printf("debug\n");
	if(strcmp(dir_name,"/") == 0){
		//root dir not have fa dir
		return;
	}
	int len = strlen(dir_name);
	int pos = len - 1;
	while(pos>=0 && dir_name[pos] != '/'){
		pos--;
	}
	if(pos == 0){
		dir_name[pos+1] = '\0';
	}
	else{
		dir_name[pos] = '\0';
	}
	// printf("dir = %s\n",dir_name);
	return;

}
void Command::cd()
{
	User *u = &Kernel::Instance().GetUser();
	char dir_name[64];
	scanf("%s",dir_name);
	//特判进入父目录
	if(strcmp(dir_name,"..") == 0){
		strcpy(dir_name,u->u_curdir);
		get_fa_dir(dir_name);
	}
	//传入系统调用参数
	u->system_ret = 0;
	u->u_dirp = dir_name;
	u->u_error = User::NOERROR;
	u->u_arg[0] = (int)dir_name;
	FileManager *file_m = &Kernel::Instance().GetFileManager();
	file_m->ChDir();
	if(u->u_error == User::ENOENT){
		printf("cd: %s: No such file or directory\n",dir_name);
	}
	else if(u->u_error == User::ENOTDIR){
		printf("cd: %s: Not a directory\n",dir_name);
	}
	return ;
	
}

void Command::ls()
{
	User *u = &Kernel::Instance().GetUser();
	u->u_error = User::NOERROR;
	u -> system_ret = 0;
	int fd = Fopen(u->u_curdir,File::FREAD);
	//目录项大小为32
	char data[32];
	while(true){
		if(Fread(fd,data,32) == 0)
			break;
		else{
			DirectoryEntry *d = (DirectoryEntry*)data;
			if(d->m_ino == 0)continue;
			printf("%s\n",d->m_name);
			memset(data,0,32);
		}
	}
	Fclose(fd);
	return ;
}

int Command::clear()
{
	system("cls"); 
	return OK;
}

int Command::Fcreat(char*file_name)
{
	User *u = &Kernel::Instance().GetUser();
	u->u_error = User::NOERROR;
	u ->system_ret = 0;
	u -> u_dirp = file_name;
	u -> u_arg[1] = 0777;
	FileManager *file_m = &Kernel::Instance().GetFileManager();
	//check if file_name exist?
	file_m -> Creat();
	return u->system_ret;
}

//删除文件后，原来文件的fd好像不能够复用
//这时候需要我们保证删除文件之前先close文件
int Command::Fdelete(char*file_name)
{
	User *u = &Kernel::Instance().GetUser();
	u -> u_error = User::NOERROR;
	u -> system_ret = 0;
	u -> u_dirp = file_name;
	FileManager *file_m = &Kernel::Instance().GetFileManager();
	file_m ->UnLink();
	if(u -> u_error == User::ENOENT){
		printf("fdelete: cannot remove '%s': No such file or directory\n",file_name);
	}
	return OK;
}

int Command::Fopen(char*file_name,int mode)
{
	User *u = &Kernel::Instance().GetUser();
	u->u_error = User::NOERROR;
	u -> system_ret = 0;
	u ->u_error = User::NOERROR;
	u ->u_dirp = file_name;
	u -> u_arg[1] = mode;
	FileManager *file_m = &Kernel::Instance().GetFileManager();
	file_m ->Open();
	return u->system_ret;
}

void Command::Fclose(int fd)
{
	User *u = &Kernel::Instance().GetUser();
	u->u_error = User::NOERROR;
	u->system_ret = 0;
	u->u_arg[0] = fd;
	FileManager *file_m = &Kernel::Instance().GetFileManager();
	file_m->Close();
}

//返回读到多少个字节
int Command::Fread(int fd,char* data,int len)
{
	User *u = &Kernel::Instance().GetUser();
	u->u_error = User::NOERROR;
	u->system_ret = 0;
	u->u_arg[0] = fd;
	u->u_arg[1] = int(data);
	u->u_arg[2] = len;
	FileManager *file_m = &Kernel::Instance().GetFileManager();
	file_m->Read();
	return u->system_ret;
}

int Command::Fwrite(int fd,char* data,int len)
{
	User *u = &Kernel::Instance().GetUser();
	u->u_error = User::NOERROR;
	u->system_ret = 0;
	u->u_arg[0] = fd;
	u->u_arg[1] = int(data);
	u->u_arg[2] = len;
	FileManager *file_m = &Kernel::Instance().GetFileManager();
	// printf("u_error=%d\n",u->u_error);
	file_m -> Write();
	// printf("u_error=%d\n",u->u_error);
	return u->system_ret;
}

int Command :: Flseek(int fd,int pos)
{
	User *u = &Kernel::Instance().GetUser();
	u->u_error = User::NOERROR;
	u->system_ret = 0;
	u->u_arg[0] = fd;
	u->u_arg[1] = pos;
	u->u_arg[2] = 0;//表示fd直接设置到pos
	FileManager *file_m = &Kernel::Instance().GetFileManager();
	file_m->Seek();
	return u->system_ret;
}

void Command::Fin(char*out_file_name,char*in_file_name)
{
	int file_mode = 0777;
	int fd = Fcreat(in_file_name);
	char data [1024] = {0};
	FILE *fp = fopen(out_file_name,"rb");
	if(fp == NULL){
		printf("file %s open error,no such file or dirctory\n",out_file_name);
		printf("Fin failed\n");
		return ;
	}
	int num = fread(data,1,1024,fp);
	while(num > 0){
		// printf("debug\n");
		Fwrite(fd,data,num);
		num = fread(data,1,1024,fp);
	}
	fclose(fp);
	Fclose(fd);
	printf("file %s read succesfully to %s\n",out_file_name,in_file_name);
	return ;
}
void Command::Fout(char*in_file_name,char*out_file_name)
{
	User *u = &Kernel::Instance().GetUser();
	int file_mode = File::FREAD | File::FWRITE;
	int fd = Fopen(in_file_name,file_mode);
	if(u->u_error == User::ENOENT){
		printf("file %s open error,no such file or dirctory\n",in_file_name);
		return;	
	}
	char data[1024] = {0};
	FILE *fp = fopen(out_file_name,"wb");
	int num = Fread(fd,data,1024);
	while(num > 0){
		fwrite(data,1,num,fp);
		num = Fread(fd,data,1024);
	}
	Fclose(fd);
	fclose(fp);
	printf("file %s write succesfully to %s\n",in_file_name,out_file_name);
	return ;
}
int Command::analyze(char * buf)
{
	User *u = &Kernel::Instance().GetUser();
    //分析用户输入的命令，做出相对应的表现
	if(strcmp(buf,"fformat") == 0){
		FFormat();
		return OK;
	}
	else if(strcmp(buf,"mkdir") == 0){
		char dir_name[64];
		scanf("%s",dir_name);
		mkdir(dir_name);
		return OK;
	}
	else if(strcmp(buf,"cd") == 0){
		cd();
		return OK;
	}
	else if(strcmp(buf,"ls") == 0){
		ls();
		return OK;
	}
	else if(strcmp(buf,"fcreat") == 0){
		char file_name[64];
		scanf("%s",file_name);
		Fcreat(file_name);
		if( u->u_error == User::EEXIST){
			//file exist
			printf("file %s exist!\n",file_name);
		}
		else if(u -> system_ret <0){
			printf("file %s ceate error\n",file_name);
		}
		else{
			printf("file %s create correct, return fd = %d\n",file_name,u->system_ret);
		}
		return OK;
	}
	else if(strcmp(buf,"fdelete") == 0){
		char file_name[64];
		scanf("%s",file_name);
		Fdelete(file_name);
		return OK;
	}
	else if(strcmp(buf,"fopen") == 0){
		char file_name[64];
		scanf("%s",file_name);
		int file_mode = File::FREAD | File::FWRITE;
		Fopen(file_name,file_mode);
		if(u->u_error == User::ENOENT){
			printf("file %s open error,no such file or dirctory\n",file_name);
		}
		else{
			printf("file %s open correct, return fd = %d\n",file_name,u ->system_ret);
		}
		return OK;
	}
	else if(strcmp(buf,"fclose") == 0){
		int fd;
		scanf("%d",&fd);
		Fclose(fd);
		return OK;
	}
	else if(strcmp(buf,"fread") == 0){
		int fd,len;
		char data[2048] = {'\0'};
		scanf("%d %d",&fd,&len);
		int read_bytes = Fread(fd,data,len);
		if(u->u_error == User::EBADF){
			printf("fd value is out of limit\n");
		}
		else if(u->u_error == User::ENOENT){
			printf("No such or directory\n");
		}
		else if(u->u_error == User::EACCES){
			printf("Permission denied,do not have access to read\n");
		}
		else{
			printf("Read succesfully,read data length is %d bytes\n",read_bytes);
			printf("read data:\n");
			printf("%s\n",data);
		}
		return OK;
	}
	else if(strcmp(buf,"fwrite") == 0){
		int fd,len;
		char data[1024];
		scanf("%d",&fd);scanf("%s",data);scanf("%d",&len);
		int data_len = strlen(data);
		len = Utility::Min(len,data_len);
		int write_bytes = Fwrite(fd,data,len);
		if(u->u_error == User::EBADF){
			printf("fd value is out of limit\n");
		}
		else if(u->u_error == User::ENOENT){
			printf("No such or directory\n");
		}
		else{
			printf("Write succesfully,write data length is %d bytes\n",write_bytes);
		}
		return OK;
	}
	else if(strcmp(buf,"flseek") == 0){
		int fd,pos;
		scanf("%d %d",&fd,&pos);
		Flseek(fd,pos);
		if(u->u_error == User::EBADF){
			printf("fd value is out of limit\n");
		}
		else if(u->u_error == User::ENOENT){
			printf("No such or directory\n");
		}
		else{
			printf("File seek succesfully\n");
		}
		return OK;
	}
	else if(strcmp(buf,"fin") == 0){
		char out_file_name[64],in_file_name[64];
		scanf("%s %s",out_file_name,in_file_name);
		Fin(out_file_name,in_file_name);
		return OK;
	}
	else if(strcmp(buf,"fout") == 0){
		char in_file_name[64],out_file_name[64];
		scanf("%s %s",in_file_name,out_file_name);
		Fout(in_file_name,out_file_name);
		return OK;
	}
	else if(strcmp(buf,"help") == 0){
		help();
		return OK;
	}
	else if(strcmp(buf,"clear") == 0){
		printf("debug clear\n");
		clear();
		return OK;
	}
	else if(strcmp(buf,"exit") == 0){
		exit();
		return EXIT_CMD;
	}
	else{
		printf("%s: command not found\n",buf);
	}
	return OK;
}