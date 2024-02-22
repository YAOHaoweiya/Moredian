#include <signal.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <cstdint>
#include "pad.h"
#include "factory.h"
#include "isp_interface.h"
#include "image_interface.h"
#include "mos_log.h"
#include "hal_control.h"
#include "hal_lcd.h"

#define RGB_CAMERA_WIDTH 1920
#define RGB_CAMERA_HEIGHT 1080
#define IR_CAMERA_WIDTH 1920
#define IR_CAMERA_HEIGHT 1080

#define RGB_RESIZE_WIDTH 1280
#define RGB_RESIZE_HEIGHT 800
#define IR_RESIZE_WIDTH 1280
#define IR_RESIZE_HEIGHT 800

//#define RGB_FACE_IMAGE_WIDTH RGB_RESIZE_HEIGHT
//#define RGB_FACE_IMAGE_HEIGHT RGB_RESIZE_WIDTH

#define RGB_FACE_IMAGE_WIDTH RGB_RESIZE_WIDTH
#define RGB_FACE_IMAGE_HEIGHT RGB_RESIZE_HEIGHT

#define IR_FACE_IMAGE_WIDTH IR_RESIZE_HEIGHT
#define IR_FACE_IMAGE_HEIGHT IR_RESIZE_WIDTH
#define LCD_WIDTH 800
#define LCD_HEIGHT 1280

#define RGB_CAMERA_DEV "/dev/video31"
#define IR_CAMERA_DEV "/dev/video39"
bool running = false;
static void signal_handle(int signo)
{
    printf("force exit signo %d !!!\n", signo);
    running = false;
}

/// @brief  设置vi属性
void SetViProperties(void)
{
    CElement *vi = ElementFactoryGet("vi");
    if (vi == NULL)
    {
        printf("ElementFactoryGet vi failed\n");
        return;
    }
    // set vi
    vi->SetProperty("device", std::string(RGB_CAMERA_DEV));
    vi->SetProperty("format", MOS_IMAGE_FORMAT_TYPE_NV12);
    vi->SetProperty("width", RGB_CAMERA_WIDTH);
    vi->SetProperty("height", RGB_CAMERA_HEIGHT);
    vi->SetProperty("count", 2);
}

/// @brief  设置drm属性
void SetDrmProperties(void)
{
    CElement *drm = ElementFactoryGet("drm");
    if (drm == NULL)
    {
        printf("ElementFactoryGet md_drm failed\n");
        return;
    }
    // drm
    drm->SetProperty("device", std::string("/dev/dri/card0"));
    // drm plane primary
    drm->SetProperty("plane", MOS_DRM_PLANE_TYPE_OVERLAY);
    drm->SetProperty("buffer_format", MOS_IMAGE_FORMAT_TYPE_NV12);
    // 输入源区域
    MosImageRect src_rect;
    src_rect.x = 0;
    src_rect.y = 64;
    src_rect.w = RGB_FACE_IMAGE_WIDTH;
    src_rect.h = RGB_FACE_IMAGE_HEIGHT - src_rect.y * 2;
    drm->SetProperty("src_rect", src_rect);
    // 显示区域
    MosImageRect dst_rect;
    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.w = LCD_WIDTH;
    dst_rect.h = LCD_HEIGHT;
    drm->SetProperty("dst_rect", dst_rect);
}
int main(int argc, char *argv[])
{
	signal(SIGINT, signal_handle);
/*	
	HalLedParam hal_param;
    hal_param.type = HAL_LED_BACKLIGHT;
    hal_param.level = 100;
    HalLedSetAttr(&hal_param);

    hal_param.type = HAL_LED_FILLIN_IR;
    hal_param.level = 15;
    HalLedSetAttr(&hal_param);

    HalLcdClear(LCD_WIDTH, LCD_HEIGHT);
*/
	
    CElement* isp = ElementFactoryMake("CMdIsp","isp");
    CElement* vi = ElementFactoryMake("CMdvi","vi");
    CElement* drm = ElementFactoryMake("CMdDrm","drm");
	
	if (isp == nullptr || vi == nullptr || drm == nullptr)
    {
        ElementFactoryFreeAll();
        printf("create element failed\n");
        return -1;
    }
//设置属性
    isp->SetProperty("iq_file",std::string("/app/iqfiles/"));
    isp->SetProperty("isp0_enable",true);
    isp->SetProperty("isp0_mode",MOS_ISP_WORKING_MODE_ISP_HDR);
    isp->SetProperty("isp0_framerate",25);
    isp->SetProperty("isp1_enable",true);
    isp->SetProperty("isp1_mode",MOS_ISP_WORKING_MODE_ISP_HDR);
    isp->SetProperty("isp1_framerate",25);

	SetViProperties();
	SetDrmProperties();
	
	
//链接
	vi->SrcPad(1).Link(drm->SinkPad(1), "std::shared_ptr<MosImageData>");
//Start   
	isp->Start();
	printf("isp start\n");
    vi->Start();
	printf("vi start\n");
	drm->Start();
	printf("drm start\n");
    


	
    running = true;
    while (running)
    {
        sleep(1);
    }
	
	vi->Stop();
	drm->Stop();
	isp->Stop();
	ElementFactoryFreeAll();

    printf("exit......\n");

    return 0;
}