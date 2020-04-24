#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/edo/Kuliah/Sisop/SoalShiftSISOP20_modul4_E09/dir";
static const char *infodir = "/home/edo/fs.log";

void write_log(int numlevel,char *command,char *one,char *two){
    FILE *f = fopen(infodir,"a+");
    
    char loginfo[1000],level[50];
    int year,month,day,hour,minute,second;

    time_t curtime;
    struct tm* loc_time;
    time(&curtime);
    loc_time = localtime(&curtime);
    year  = (loc_time->tm_year + 1900)%1000;
    month = loc_time->tm_mon+1;
    day   = loc_time->tm_mday;
    hour  = loc_time->tm_hour;
    minute= loc_time->tm_min;
    second= loc_time->tm_sec;
    
    memset(level,0,50*sizeof(char));
    if(numlevel==0){    
        strcpy(level,"WARNING");
    } else {
        strcpy(level,"INFO");
    }

    memset(loginfo,0,1000*sizeof(char));
    if(two==NULL){
        sprintf(loginfo,"%s::%d%d%d-%d:%d:%d::%s::%s",level,year,month,day,hour,minute,second,command,one);
    } else {
        sprintf(loginfo,"%s::%d%d%d-%d:%d:%d::%s::%s::%s",level,year,month,day,hour,minute,second,command,one,two);
    }
    

    fprintf(f,"%s\n",loginfo);
    fclose(f);
}

static int xmp_mkdir(const char *path,mode_t mode){
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    write_log(1,"MKDIR",fpath,NULL);
    res = mkdir(fpath,mode);
    if(res==1){
        return -errno;
    }
    return 0;
}

static int xmp_rmdir(const char *path){
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    write_log(0,"RMDIR",fpath,NULL);
    res = rmdir(fpath);
    if(res==-1){
        return -errno;
    }
    return 0;
}

static int xmp_getattr(const char *path,struct stat *stbuf){
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    res = lstat(fpath, stbuf);
    if (res == -1){
        return -errno;
    }

    return 0;
}



static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi){
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }else {
        sprintf(fpath, "%s%s",dirpath,path);
    }
    int res = 0;
    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;
    dp = opendir(fpath);
    if (dp == NULL){
        return -errno;    
    }
    
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        res = (filler(buf, de->d_name, &st, 0));
        if(res!=0) break;
    }
    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi){
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else {
        sprintf(fpath, "%s%s",dirpath,path);
    }
    int res = 0;
    int fd = 0 ;
    (void) fi;
    fd = open(fpath, O_RDONLY);
    if (fd == -1){
        return -errno;    
    }
    
    res = pread(fd, buf, size, offset);
    if (res == -1){
        res = -errno;
    }
    
    close(fd);
    return res;
}

static int xmp_rename(const char *from, const char *to)
{
    int res;
    char from_path[1000],to_path[1000];
    sprintf(from_path,"%s%s",dirpath,from);
    sprintf(to_path,"%s%s",dirpath,to);
    write_log(1,"RENAME",from_path,to_path);
    res = rename(from_path, to_path);
    if(res == -1)
        return -errno;

    return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev){
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    printf("==========\n%s\n===========",fpath);
    write_log(1,"MKNODE",fpath,NULL);
    res = mknod(fpath, mode, rdev);
    if(res == -1)
        return -errno;

    return 0;
}

static int xmp_unlink(const char *path){
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    write_log(0,"UNLINK",fpath,NULL);
    res = unlink(fpath);
    if(res == -1)
        return -errno;

    return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size,
             off_t offset, struct fuse_file_info *fi)
{
    int fd;
    int res;
    char fpath[1000];
    char name[1000];
    // char temp[1000];
    sprintf(name,"%s",path);
    if(strcmp(path,"/") == 0)
    {
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,name);
    
    (void) fi;
    fd = open(fpath, O_WRONLY);
    if (fd == -1)
        return -errno;
 
    res = pwrite(fd, buf, size, offset);
    if (res == -1)
        res = -errno;
 
    close(fd);
    return res;
    
}

// static int xmp_truncate(const char *path, off_t size){
//     int res;
//     char fpath[1000];
//     char name[1000];
//     sprintf(name,"%s",path);
//     sprintf(fpath, "%s%s",dirpath,name);
//     res = truncate(fpath, size);
//     if (res == -1)
//         return -errno;
 
//     return 0;
// }

static struct fuse_operations xmp_oper = {
    .getattr	= xmp_getattr,
    .readdir    = xmp_readdir,
    .mkdir	= xmp_mkdir,
    .rmdir	= xmp_rmdir,
    .rename	= xmp_rename,
    .read	= xmp_read,
    .mknod  = xmp_mknod,
    .unlink = xmp_unlink,
    .write  = xmp_write,
    // .truncate   = xmp_truncate
};

int main(int argc,char *argv[]){
    umask(0);
    return fuse_main(argc,argv,&xmp_oper,NULL);
}