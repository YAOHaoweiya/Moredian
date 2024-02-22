#include <stdio.h>
#include <unistd.h> 
#include <fcntl.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
/*VIDIOC_QUERYCAP ---- struct v4l2_capability*/
/*
header path:./kernel/include/uapi/linux/videodev2.h
struct v4l2_capability {
	__u8	driver[16];  //name of the driver module
	__u8	card[32];    //name of the card
	__u8	bus_info[32];//name of the bus
	__u32   version;     //kernel version
	__u32	capabilities;//capabilities of the physical device as a whole
	__u32	device_caps; //capabilities aeccessed via this particular device
	__u32	reserved[3];
};
*/
 
int main(int argc, char *argv[])
{
	if(argc < 2){
		printf("this process arg error\n");
		return -1;
	}
	printf("camera device:%s\n", argv[1]);
	
	int cam_fd = open(argv[1], O_RDWR);
	if(cam_fd < 0){
		printf("open the camera device:%s is error: %s\n", argv[1], strerror(errno));
		return -2;
	}
	
	struct v4l2_capability cap;
	if(ioctl(cam_fd, VIDIOC_QUERYCAP, &cap) == -1){
		printf("Unable query the ability of the device %s\n ", argv[1]);
		return -3;
	}
	printf("driver:\t%s\n", cap.driver);
	printf("card:\t%s\n", cap.card);
	printf("bus_info:\t%s\n", cap.bus_info);
	printf("version:\t%d\n", cap.version);
	printf("capability:\t%x\n", cap.capabilities);
	
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)){
		printf("the device %s is not support capture\n", argv[1]);
	}else{
		printf("the device %s is support capture\n", argv[1]);
	}
	if((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING){
		printf("the device %s is support streaming\n", argv[1]);
	}else{
		printf("the device %s is not support streaming\n", argv[1]);
	}
	
	printf("device_caps:\t%x\n", cap.device_caps);
	
	
	close(cam_fd);
	return 0;
}
