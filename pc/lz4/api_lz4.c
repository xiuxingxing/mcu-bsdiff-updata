#include "stdio.h"
#include "stdlib.h"
#include "lz4.h"
#include "stdint.h"

#if 0
//通过filename打开文件并加载到buff和size
int read_file(char *file_name,uint8_t **buff,uint32_t *size)
{
    FILE *file;
    if(((file=fopen(file_name,"rb"))==NULL) || 
        (fseek(file,0,SEEK_END)!=0)     ||
        ((*size=ftell(file))==0)        ||
        ((*buff=malloc(*size))==NULL)       ||
        (fseek(file,0,SEEK_SET)!=0)     ||
        (fread(buff,sizeof(uint8_t),*size,file)!=*size) ||
        (fclose(file)!=0)
    )
    {
        printf("open file %s fail\n",file_name);
        return -1;
    }
    printf("open file %s success\n",file_name);
    return 0;
}
#endif



//压缩文件
int lz4_compress_file(char *file_name,char *compress_file_name)
{
    //源文件变量
    FILE *s_file;
    uint32_t s_size;
    uint8_t *s_buff;

    //压缩文件变量
    FILE *c_file;
    uint32_t c_size;
    uint8_t *c_buff;

    //读取源文件
    if(((s_file=fopen(file_name,"rb"))==NULL) || 
        (fseek(s_file,0,SEEK_END)!=0)   ||
        ((s_size=ftell(s_file))==0)     ||
        ((s_buff=malloc(s_size))==NULL) ||
        (fseek(s_file,0,SEEK_SET)!=0)   ||
        (fread(s_buff,sizeof(uint8_t),s_size,s_file)!=s_size) ||
        (fclose(s_file)!=0) )
    {
        printf("open source file %s fail\n",file_name);
        return -1;
    }
    printf("open source file %s success\n",file_name);
    printf("s_size=%d\n\n",s_size);
    for(int i=0;i<s_size;i++)
        printf("%c",s_buff[i]);

    
    //压缩文件
    if(((c_buff=malloc(s_size))==NULL)  ||       //先申请同样大的内存
        ((c_size=LZ4_compress_default(s_buff,c_buff,s_size,s_size))==0) ||//压缩到申请的内存
        ((c_file=fopen(compress_file_name,"wb"))==NULL)    ||
        (fwrite(c_buff,sizeof(uint8_t),c_size,c_file)!=c_size) ||
        (fclose(c_file)!=0)
    )
    {
        
        printf("compress file fail\n");
        return -1;
    }
    printf("compress file success\n");

    
    printf("c_size=%d\n",c_size);
    free(s_buff);
    free(c_buff);

    




    return 0;
}

//预估解压大小
#define DECOMPRESS_SIZE (200*1024)


//解压文件
int lz4_decompress_file(char *compress_file_name,char *file_name)
{
    //TODO lz4无法获取压缩前大小，所以只能自定义或提前保存 建议直接使用200k
    //压缩文件变量
    FILE *c_file;
    uint32_t c_size;
    uint8_t *c_buff;

    //解压文件变量
    FILE *d_file;
    uint32_t d_size;
    uint8_t *d_buff;
    
    //读取压缩文件
    if(((c_file=fopen(compress_file_name,"rb"))==NULL)||
        (fseek(c_file,0,SEEK_END)!=0)   ||
        ((c_size=ftell(c_file))==0)     ||
        (fseek(c_file,0,SEEK_SET)!=0)   ||
        ((c_buff=malloc(c_size))==NULL) ||
        (fread(c_buff,sizeof(uint8_t),c_size,c_file)!=c_size)||
        (fclose(c_file)!=0)  )
    {
        printf("compress file open fail\n");
        return -1;
    }

    //解压文件
    if(((d_buff=malloc(DECOMPRESS_SIZE))==NULL) ||
        ((d_size=LZ4_decompress_safe(c_buff,d_buff,c_size,DECOMPRESS_SIZE))==0) ||
        ((d_file=fopen(file_name,"wb"))==NULL)    ||
        (fwrite(d_buff,sizeof(uint8_t),d_size,d_file)!=d_size) ||
        (fclose(d_file)!=0) )
    {
        printf("decompress file fail\n");
        return -1;

    }

    free(c_buff);
    free(d_buff);

    printf("decompress file success\n");

    return 0;
}


#if 0
int main(void)
{
    char source_file_name[50];

    printf("input file name\n");
    gets(source_file_name);
    lz4_compress_file("test_file_old.txt","compress_file");

    printf("\n\n\n");

    lz4_decompress_file("compress_file","decompress_file.txt");

    return 0;
}
#endif
