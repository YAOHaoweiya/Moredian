#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

//#include "../include/isp_common.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define FMT_NUM_PLANES 1
//定义一个结构体来映射缓冲帧
struct buffer
{
	void *start;
    size_t length;
    int export_fd;
    int sequence;
};

int m_fd;
int m_count = 2;//要申请的缓冲区数量
int m_width=64;
int m_height=1920;
enum v4l2_buf_type m_v4l2_buf_type ;
struct v4l2_capability cap;
struct v4l2_format fmt;
struct v4l2_requestbuffers req;
struct v4l2_buffer buf;
struct v4l2_plane planes[FMT_NUM_PLANES];

struct buffer *tmp_buffers = NULL;//初始化映射缓冲帧的结构体


// /*******************************************************************************************************************/
// /***************关于isp的定义****************/
// bool m_isp0_enable;
// bool m_isp1_enable;
// int m_isp0_init_success;
// int m_isp1_init_success;
// rk_aiq_working_mode_t isp0_working_mode;
// rk_aiq_working_mode_t isp1_working_mode;
// int m_isp0_framerate;
// int m_isp1_framerate;
// char* m_iq_file = NULL;
// /************************end***************/
// void IspSet(void)
// {
// 	char *iqfile="/app/iqfiles/";
// 	m_iq_file=iqfile;
// 	m_isp0_enable=true;
// 	m_isp1_enable=true;
// 	isp0_working_mode=RK_AIQ_WORKING_MODE_ISP_HDR2;
// 	isp0_working_mode=RK_AIQ_WORKING_MODE_ISP_HDR2;
// 	m_isp0_framerate=25;
// 	m_isp1_framerate=25;
// }

// int IspStart(void)
// {
// 	bool multi_cam = false;
//     if(m_isp0_enable && m_isp1_enable)
//         multi_cam = true;
// 	if(m_isp0_enable){
//        m_isp0_init_success =  Ispinit(0,m_isp0_mode,multi_cam ,m_iq_file.c_str(),m_isp0_framerate );
//     }

//     if(m_isp1_enable){
//         m_isp1_init_success = Ispinit(1,m_isp1_mode,multi_cam ,m_iq_file.c_str(),m_isp1_framerate );
//     }
// }
// int IspinitIspinit(int index,rk_aiq_working_mode_t isp_working_mode,bool multi_cam ,char* m_iq_file ,int framerate)
// {

// 	int ret = SAMPLE_COMM_ISP_Init(index, isp_working_mode, static_cast<RK_BOOL>(multi_cam), iq_file_dir.c_str());
//     if(ret < 0){
//         __MODULE_ERR<<"isp "<<index<<" init error";
//         return -1;
//     }
//     SAMPLE_COMM_ISP_Run(index);
//     SAMPLE_COMM_ISP_SetFrameRate(index, framerate);
//     return 0;
// }
// /*************************************************************************************************************************************/

int OpenDevice(void)
{
	m_fd = open("/dev/video0",O_RDWR,0);
	if(m_fd<0)
	{
		printf("%s:open video device error\n",__func__);
	}
	return m_fd;
}

int InitDevice()
{
	int ret;
	CLEAR(cap);
    CLEAR(fmt);
	//查询设备能力
	if(-1==ioctl(m_fd,VIDIOC_QUERYCAP,&cap))
	{
		printf("ioctl VIDIOC_QUERYCAP ERROR\n");
	}
	printf("driver:\t%s\n", cap.driver);
	printf("card:\t%s\n", cap.card);
	printf("bus_info:\t%s\n", cap.bus_info);
	printf("version:\t%d\n", cap.version);
	printf("capability:\t%x\n", cap.capabilities);
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		printf("%s:v4l2_capability do not have V4L2_CAP_VIDEO_CAPTURE\n",__func__);
	}
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
	{
		printf("%s:v4l2_capability do not have V4L2_CAP_VIDEO_CAPTURE_MPLANE\n",__func__);
	}
	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		printf("%s:v4l2_capability do not have V4L2_CAP_STREAMING\n",__func__);
	}

	//设置视频捕获格式
	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
	{
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		printf("%s: set v4l2_format.type:V4L2_BUF_TYPE_VIDEO_CAPTURE\n",__func__);
	}
    else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
	{
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		printf("%s: set v4l2_format.type:V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE\n",__func__);
	}
	m_v4l2_buf_type = fmt.type;
	fmt.fmt.pix.width = m_width;
	fmt.fmt.pix.height = m_height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	ret = ioctl(m_fd, VIDIOC_S_FMT, &fmt);
	if(ret<0)
    {
        printf("%s:ioctl VIDIOC_S_FMT ERROR\n",__func__);
        return -1;
    }
	return ret;
}

int InitMap()
{
	CLEAR(req);
	CLEAR(buf);
	//设置申请的缓冲区数量，类型，映射方式
	req.count = m_count;
	req.type = m_v4l2_buf_type;
	req.memory = V4L2_MEMORY_MMAP;
	//申请缓冲区
	if(-1 == ioctl(m_fd,VIDIOC_REQBUFS,&req))
	{
		printf("%s: ioctl VIDIOC_REQBUFS ERROR\n",__func__);
	}
	if (req.count < 2)
    {
        printf("%s: Insufficient buffer memory \n",__func__);
        return -1;
    }
	
	//申请几个缓冲区buf，就用几个tmp_buffer指针记录
	tmp_buffers = (struct buffer *)calloc(req.count,sizeof(struct buffer));
	if(!tmp_buffers)
	{
		printf("%s: tmp_buffers calloc failed!\n",__func__);
		return -1;
	}
	
	//计数用
	int n_buffers;
	//for循环将申请到的帧缓冲映射到用户空间 mmap
	for(n_buffers = 0; n_buffers < req.count; n_buffers++)
	{
		CLEAR(buf);
        CLEAR(planes);
		//设置v4l2_buffer buf
		buf.type = m_v4l2_buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;
		//设置缓冲区长度
		if(m_v4l2_buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		{
			buf.m.planes = planes;
			buf.length = FMT_NUM_PLANES;
		}
		
		//查询v4l2_buffer buf 的地址 查询申请到内核缓冲区的信息
		if(-1 == ioctl(m_fd,VIDIOC_QUERYBUF,&buf))
		{
			printf("%s: ioctl VIDIOC_QUERYBUF error\n",__func__);
		}
		//开始映射
		if(m_v4l2_buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		{
			tmp_buffers[n_buffers].length = buf.m.planes[0].length;
			tmp_buffers[n_buffers].start=
				mmap(NULL,
					buf.m.planes[0].length,
					PROT_READ | PROT_WRITE,
					MAP_SHARED,
					m_fd,
					buf.m.planes[0].m.mem_offset);
		}
		else
		{
			
			tmp_buffers[n_buffers].length = buf.length;
			tmp_buffers[n_buffers].start=
				mmap(NULL,
					buf.length,
					PROT_READ | PROT_WRITE,
					MAP_SHARED,
					m_fd,
					buf.m.offset);
		}
		//check mmap
		if(MAP_FAILED == tmp_buffers[n_buffers].start)
		{
			printf("%s: mmap failed!\n",__func__);
		}
		
		//export buf as dma file descriptor.
		
	}
	return 1;
}

void StartCapture(void)
{
	int i;
    enum v4l2_buf_type type;
	//将申请的缓冲区放入队列，并启动数据流
    for (i = 0; i < m_count; i++)
    {
        struct v4l2_buffer buf;

        CLEAR(buf);
        buf.type = m_v4l2_buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (m_v4l2_buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
        {
            struct v4l2_plane planes[FMT_NUM_PLANES];

            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }
        if (-1 == ioctl(m_fd, VIDIOC_QBUF, &buf))
            printf("%s: ioctl VIDIOC_QBUF ERROR\n",__func__);
    }
    type = m_v4l2_buf_type;
    printf("-------- stream on  -------------\n");
    if (-1 == ioctl(m_fd, VIDIOC_STREAMON, &type))
        printf("%s: ioctl VIDIOC_STREAMON\n",__func__);
}

void WriteFrame(void)
{
	int j;
	int i;
	struct v4l2_buffer buf;
	int fp = open("/mnt/v4l2.yuv",O_WRONLY | O_CREAT | O_TRUNC);
    CLEAR(buf);
    buf.type = m_v4l2_buf_type;
    buf.memory = V4L2_MEMORY_MMAP;
	
    if(m_v4l2_buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
    {
		struct v4l2_plane planes[FMT_NUM_PLANES];
        buf.m.planes = planes;
        buf.length = FMT_NUM_PLANES;
    }
	//for (j = 0; j < 5; j++)
	//{
	//写入文件
		for (i = 0; i < m_count; i++)
		{
			buf.type = m_v4l2_buf_type;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;
			
			//出队	
			if(-1 == ioctl(m_fd,VIDIOC_DQBUF,&buf))
				printf("%s: ioctl VIDIOC_DQBUF ERROR\n",__func__);
			
			if(tmp_buffers[i].start == NULL) printf("%s: tmp_buffers is null!",__func__);
		
			if(-1 == write(fp,tmp_buffers[i].start,tmp_buffers[i].length))
			//if(-1 == write(fp,&buf,buf.length))
			{
				printf("%s: write frame error\n",__func__);
				close(fp);
			}
			else printf("%s: write frame  tmp_buffers[%d]\n",__func__,i);
		
			//入队
			if(-1 == ioctl(m_fd,VIDIOC_QBUF,&buf))
				printf("%s: ioctl VIDIOC_QBUF ERROR\n",__func__);
		
		}
	//}
	close(m_fd);
	close(fp);
}

int main()
{
	/*
	IspSet();
	IspStart();
	*/
	
	//1.打开video设备节点
	OpenDevice();
	//2.查询设备能力，设置视频捕获格式
	InitDevice();
	//3.申请缓冲区，设置后进行内存映射
	InitMap();
	//4. 将buf放入缓冲队列，开始视频流
	StartCapture();
	//5. 
	WriteFrame();
	return 0;
}