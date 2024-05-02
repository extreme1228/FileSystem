#include"Kernel.h"
#include"stdio.h"
#include "string.h"
#include <cstdlib> 
#include "Utility.h"

#define EXIT_CMD -1
#define OK 1

class Command{

    public:
    //分析处理命令
    int analyze(char*buf);

    //文件系统支持的命令
    
    void ls();
    void cd();
    void FFormat();
    void mkdir(char*dir_name);
    int Fcreat(char*file_name);
    int Fopen(char*file_name,int mode);
    void Fclose(int fd);
    int Fread(int fd,char*data,int len);
    int Fwrite(int fd,char* data,int len);
    int Flseek(int fd,int pos);
    int Fdelete(char*file_name);
    void Fin(char*out_file_name,char*in_file_name);
    void Fout(char*in_file_name,char*out_file_name);
    int  clear();
    void help();
    void exit();
};