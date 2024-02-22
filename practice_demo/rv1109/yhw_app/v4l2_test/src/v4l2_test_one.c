#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>   
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <linux/videodev2.h>
void enumerate_menu(int fd);
unsigned int CAP_BUF_NUM=4;
int get_now_time(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000+tv.tv_usec/1000;
}
//保存内核映射缓冲区的地址和大小
typedef struct VideoBuffer{
    void *start;
    size_t length;
}VideoBuffer;
unsigned int CAP_W=800;
unsigned int CAP_H=1280;
int main(int argc,char **argv){
    if(argc!=3){
        printf("默认分辨率为%d*%d\n",CAP_W,CAP_H);
        printf("是否需要重新设定y/n：\n");
        char c;
        scanf("%c",&c);
        if(c=='y'){
          printf("请输入新的分辨率：\n");
          scanf("%d%d",&CAP_W,&CAP_H);
          printf("你设置的分辨率为：%d*%d\n",CAP_W,CAP_H);
        }
    }
    int ret,i,j;
    int fd;
    fd=open("/dev/video0",O_RDWR);
    if(-1==fd){
        printf("open video0 failed!\n");
        return -1;
    }
    struct v4l2_capability cap;
    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if(-1==ret){
        printf("ioctl VIDIOC_QUERYCAY ERROR!\n");
        close(fd);
        return -1;
    }
    if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)
    {
        printf("是视频输出设备\n");
    }
    if(cap.capabilities & V4L2_CAP_VIDEO_OVERLAY )
    {
        printf("支持预览功能\n");
    }
    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
    {
       /* 如果为摄像头设备则打印摄像头驱动名字 */
        printf("Driver    Name: %s\n", cap.driver);
        printf("Card      Name: %s\n",cap.card);
    }
    else
    {
        printf("open file is not video\n");
        close(fd);
        return -2;
    }
    //查询支持的视频格式
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE; // 指定需要枚举的类型
    for (i = 0; ; i++)                      // 有可能摄像头支持的图像格式不止一种
    {
        fmtdesc.index = i;
        ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc);
        if (-1 == ret)                      // 获取所有格式完成
        {
            break;
        }
        /* 打印摄像头图像格式 */
        printf("Picture Format: %s\n", fmtdesc.description);
        /* 查询该图像格式所支持的分辨率 */
        struct v4l2_frmsizeenum frmsize;
        frmsize.pixel_format = fmtdesc.pixelformat;
        for (j = 0; ; j++)                  //　该格式支持分辨率不止一种
        {
            frmsize.index = j;
            ret = ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize);
            if (-1 == ret)                  // 获取所有分辨率完成
            {
                break;
            }
            /* 打印分辨率 */
            printf("width: %d height: %d\n",
                    frmsize.discrete.width,frmsize.discrete.height);
        }
    }
    struct v4l2_format    fmt;
    memset(&fmt,0,sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    fmt.fmt.pix.width=CAP_W;
    fmt.fmt.pix.height=CAP_H;
    //fmt.fmt.pix.pixelformat= V4L2_PIX_FMT_YUYV;
    //fmt.fmt.pix.pixelformat= V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.pixelformat= V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field=V4L2_FIELD_NONE;
    if(ioctl(fd,VIDIOC_S_FMT,&fmt)==-1){
        printf("设置视频格式出错!\n");
        return -1;
    }
    struct v4l2_requestbuffers  req;
    req.count=CAP_BUF_NUM;
    req.type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    req.memory=V4L2_MEMORY_MMAP;
    if(-1==ioctl(fd,VIDIOC_REQBUFS,&req)){
        printf("申请帧缓冲出错！\n");
        return -1;
    }
    struct v4l2_buffer buf;
    VideoBuffer *buffers=calloc(req.count,sizeof(*buffers));
    unsigned int num;
    for(num=0;num<req.count;num++){
        memset(&buf,0,sizeof(buf));
        buf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory=V4L2_MEMORY_MMAP;
        buf.index=num;
        if(ioctl(fd,VIDIOC_QUERYBUF,&buf)==-1){
            printf("分配内存出错！\n");
            return -1;
        }
        buffers[num].length=buf.length;
        //内存映射到用户空间
        buffers[num].start=mmap(NULL,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,buf.m.offset);
        if(buffers[num].start==MAP_FAILED){
            printf("mmap error!\n");
            return -1;
        }
        //把帧放入队列
        if(ioctl(fd,VIDIOC_QBUF,&buf)==-1){
            printf("VIDIOC_QBUF error!\n");
            return -1;
        }
    }
    enum v4l2_buf_type type;
    type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    //开始采集
    ioctl (fd, VIDIOC_STREAMON, &type);
    memset(&buf,0,sizeof(buf));
    buf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buf.memory=V4L2_MEMORY_MMAP;
    int count=100;
    FILE *fp;
    fp=fopen("./test.yuv","wb+");
    if(fp==NULL){
       printf("open failed!\n");
       return -1;
    }
    long int lastTime=get_now_time();
    printf("start:%ld ms\n",lastTime);
    while(count--){
        printf("count:%d\n",count);
        //从队列中取出帧
        ret=ioctl(fd,VIDIOC_DQBUF,&buf);
        if(ret<0){  
            printf("VIDIOC_DQBUF FAILED!\n");
            return -1;
        }
        fwrite(buffers[buf.index].start,1,buf.bytesused,fp);
        //把帧放回队列
        ret=ioctl(fd,VIDIOC_QBUF,&buf);
        if(ret<0){
            printf("VIDIOC_QBUF FAILED!\n");
            fclose(fp);
            ioctl(fd,VIDIOC_STREAMOFF,&type);
            //断开内存映射连接
            for(i=0;i<CAP_BUF_NUM;i++){
                munmap(buffers[i].start,buffers[i].length);
            }
            close(fd);
            return -1;
        }
    }
    long int culTime=get_now_time();
    printf("Stop Time:%ld\n",culTime);
    printf("Total time:%ld ms\n",culTime-lastTime);
    fclose(fp);
    ioctl(fd,VIDIOC_STREAMOFF,&type);
    //断开内存映射连接
    for(i=0;i<CAP_BUF_NUM;i++){
        munmap(buffers[i].start,buffers[i].length);
    }
    close(fd);
    return 0;
}