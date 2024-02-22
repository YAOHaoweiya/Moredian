#include <stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "./include/rtc.h" /* 包含命令定义 */


int main()
{
	struct rtc_time rtc_time1;
    int fd = 0;
    int cmd;
    int arg = 0;
   // int *p=rtc_time1;
    
    /*可读写方式打开设备文件*/
    fd = open("/dev/rtc0",O_RDWR);
    if (fd < 0)
    {
        printf("Open Dev rtc0 Error!\n");
        return -1;
    }
    
    /* 调用命令RTC_RD_TIME */
    printf("<--- Call RTC_RD_TIME --->\n");
    cmd = RTC_RD_TIME;
    if (ioctl(fd,cmd,&rtc_time1) < 0)
        {
            printf("Call cmd RTC_RD_TIME fail\n");
            return -1;
    }
    printf("RTC date/time: %d/%d/%d %02d:%02d:%02d\n",
        rtc_time1.tm_mday, rtc_time1.tm_mon + 1, rtc_time1.tm_year + 1900,
        rtc_time1.tm_hour, rtc_time1.tm_min, rtc_time1.tm_sec);
	
    
    close(fd);
    return 0;    
}
