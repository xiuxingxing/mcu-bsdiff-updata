/*-
 * Copyright 2003-2005 Colin Percival
 * Copyright 2012 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions 
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <limits.h>
#include "bspatch.h"
#include "stdio.h"
#include "stdlib.h"

static int64_t offtin(uint8_t *buf)
{
	int64_t y;

	y=buf[7]&0x7F;
	y=y*256;y+=buf[6];
	y=y*256;y+=buf[5];
	y=y*256;y+=buf[4];
	y=y*256;y+=buf[3];
	y=y*256;y+=buf[2];
	y=y*256;y+=buf[1];
	y=y*256;y+=buf[0];

	if(buf[7]&0x80) y=-y;

	return y;
}
//stream为补丁文件解压后的文件流
int bspatch(const uint8_t* old, int64_t oldsize, uint8_t* new, int64_t newsize, struct bspatch_stream* stream)
{
	uint8_t buf[8];
	int64_t oldpos,newpos;
	int64_t ctrl[3];
	int64_t i;

	oldpos=0;newpos=0;
	while(newpos<newsize) {
		/* Read control data */
		for(i=0;i<=2;i++) {
			if (stream->read(stream, buf, 8))
				return -1;
			ctrl[i]=offtin(buf);
		};

		/* Sanity-check */
		if (ctrl[0]<0 || ctrl[0]>INT_MAX ||
			ctrl[1]<0 || ctrl[1]>INT_MAX ||
			newpos+ctrl[0]>newsize)
			return -1;

		/* Read diff string */
		if (stream->read(stream, new + newpos, ctrl[0]))
			return -1;

		/* Add old data to diff string */
		for(i=0;i<ctrl[0];i++)
			if((oldpos+i>=0) && (oldpos+i<oldsize))
				new[newpos+i]+=old[oldpos+i];

		/* Adjust pointers */
		newpos+=ctrl[0];
		oldpos+=ctrl[0];

		/* Sanity-check */
		if(newpos+ctrl[1]>newsize)
			return -1;

		/* Read extra string */
		if (stream->read(stream, new + newpos, ctrl[1]))
			return -1;

		/* Adjust pointers */
		newpos+=ctrl[1];
		oldpos+=ctrl[2];
	};

	return 0;
}

int read_header(header_t *header,FILE *updata_file)
{
    if(fread(header,1,sizeof(header_t),updata_file)==0)
        return -1;
    return 0;
}



int read_updata_file(const struct bspatch_stream* stream, void* buffer, int size)
{
    FILE *file=(FILE *)(stream->opaque);
    if(fread(buffer,sizeof(uint8_t),size,file)!=size)
        return -1;
    return 0;

}


int main(void)
{
    uint32_t old_file_size,restore_file_size;//文件大小
    uint8_t *old_file_buff,*restore_file_buff;//缓存指针
    FILE *old_file,*restore_file,*updata_file;//文件指针

    header_t header;

//提取旧文件
    if(((old_file = fopen("test_file_old.txt","rb")) == NULL) ||//打开文件不错误
        (fseek(old_file,0,SEEK_END) == -1)||//跳到末尾不错误
        ((old_file_size = ftell(old_file)) == 0)||//获取文件大小不为0
        (fseek(old_file,0,SEEK_SET) == -1)||//跳回开头不错误
        ((old_file_buff=malloc(old_file_size))==NULL)||//申请old文件内存
        (old_file_size!=fread(old_file_buff,sizeof(uint8_t),old_file_size,old_file)))
    {
        printf("old file get fail");
        goto end;
    }
//打开补丁文件
    if((updata_file=fopen("updata_file","rb"))==NULL)
    {
        printf("updata file get fail");
        goto end;
    }
//获取补丁文件头
    read_header(&header,updata_file);

//得到新文件长度
    restore_file_size=header.new_file_size;
    printf("restore_file_size = 0x%x    %dbyte\n",restore_file_size,restore_file_size);
    restore_file_buff=malloc(restore_file_size);
    struct bspatch_stream stream={
        .opaque=updata_file,
        .read=read_updata_file
    };

//差分计算
    if(bspatch(old_file_buff,old_file_size,restore_file_buff,restore_file_size,&stream)!=0)
    {
        printf("bspatch fail");
        goto end;
    }

//写入文件
    if(((restore_file=fopen("restore_file.txt","wb"))==NULL) ||
        (fwrite(restore_file_buff,1,restore_file_size,restore_file)!=restore_file_size))
    {
       printf("restore_file write fail");
       goto end; 
    }




//
    printf("success");

end:
    fclose(old_file);
    fclose(restore_file);
    fclose(updata_file);


    free(old_file_buff);
    free(restore_file_buff);

    return 0;
}






#if defined(BSPATCH_EXECUTABLE)

#include <bzlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

static int bz2_read(const struct bspatch_stream* stream, void* buffer, int length)
{
	int n;
	int bz2err;
	BZFILE* bz2;

	bz2 = (BZFILE*)stream->opaque;
	n = BZ2_bzRead(&bz2err, bz2, buffer, length);
	if (n != length)
		return -1;

	return 0;
}

int main(int argc,char * argv[])
{
	FILE * f;
	int fd;
	int bz2err;
	uint8_t header[24];
	uint8_t *old, *new;
	int64_t oldsize, newsize;
	BZFILE* bz2;
	struct bspatch_stream stream;
	struct stat sb;

	if(argc!=4) errx(1,"usage: %s oldfile newfile patchfile\n",argv[0]);

	/* Open patch file */
	if ((f = fopen(argv[3], "r")) == NULL)
		err(1, "fopen(%s)", argv[3]);

	/* Read header */
	if (fread(header, 1, 24, f) != 24) {
		if (feof(f))
			errx(1, "Corrupt patch\n");
		err(1, "fread(%s)", argv[3]);
	}

	/* Check for appropriate magic */
	if (memcmp(header, "ENDSLEY/BSDIFF43", 16) != 0)
		errx(1, "Corrupt patch\n");

	/* Read lengths from header *///从文件头读取文件长度
	newsize=offtin(header+16);
	if(newsize<0)
		errx(1,"Corrupt patch\n");

	/* Close patch file and re-open it via libbzip2 at the right places */
	if(((fd=open(argv[1],O_RDONLY,0))<0) ||
		((oldsize=lseek(fd,0,SEEK_END))==-1) ||
		((old=malloc(oldsize+1))==NULL) ||//只是用来提取old文件的缓存，事实上可直接读取，oldsize获取的问题仍待解决
		(lseek(fd,0,SEEK_SET)!=0) ||
		(read(fd,old,oldsize)!=oldsize) ||
		(fstat(fd, &sb)) ||
		(close(fd)==-1)) err(1,"%s",argv[1]);
	if((new=malloc(newsize+1))==NULL) err(1,NULL);

	if (NULL == (bz2 = BZ2_bzReadOpen(&bz2err, f, 0, 0, NULL, 0)))
		errx(1, "BZ2_bzReadOpen, bz2err=%d", bz2err);

	stream.read = bz2_read;
	stream.opaque = bz2;
	if (bspatch(old, oldsize, new, newsize, &stream))
		errx(1, "bspatch");

	/* Clean up the bzip2 reads */
	BZ2_bzReadClose(&bz2err, bz2);
	fclose(f);

	/* Write the new file */
	if(((fd=open(argv[2],O_CREAT|O_TRUNC|O_WRONLY,sb.st_mode))<0) ||
		(write(fd,new,newsize)!=newsize) || (close(fd)==-1))
		err(1,"%s",argv[2]);

	free(new);
	free(old);

	return 0;
}

#endif
