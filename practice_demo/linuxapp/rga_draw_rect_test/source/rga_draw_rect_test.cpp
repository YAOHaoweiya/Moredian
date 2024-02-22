#include <signal.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <cstdint>
#include "pad.h"
#include "factory.h"
#include "isp_interface.h"
#include "image_interface.h"
#include "hal_control.h"
#include "hal_lcd.h"
#include "mos_log.h"

/*
/dev/video31(rkispp_scale0):  rgb摄像头,用于rgb显示
/dev/video32(rkispp_scale1):  rgb摄像头,用于算法处理
/dev/video33(rkispp_scale2):  rgb摄像头,用于二维码解析

/dev/video39(rkispp_scale0):  ir摄像头,用于rgb显示
/dev/video40(rkispp_scale1):  ir摄像头,用于算法处理
*/

#define RGB_CAMERA_WIDTH 1920
#define RGB_CAMERA_HEIGHT 1080
#define IR_CAMERA_WIDTH 1920
#define IR_CAMERA_HEIGHT 1080

#define RGB_RESIZE_WIDTH 1280
#define RGB_RESIZE_HEIGHT 720
#define IR_RESIZE_WIDTH 1280
#define IR_RESIZE_HEIGHT 720

#define RGB_FACE_IMAGE_WIDTH RGB_RESIZE_HEIGHT
#define RGB_FACE_IMAGE_HEIGHT RGB_RESIZE_WIDTH

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

/// @brief  设置isp属性
void SetIspProperties(void)
{
    CElement *isp = ElementFactoryGet("isp");
    if (isp == NULL)
    {
        printf("ElementFactoryGet isp failed\n");
        return;
    }
    // set isp
    isp->SetProperty("iq_file", std::string("/app/iqfiles/"));
    isp->SetProperty("isp0_enable", true);
    isp->SetProperty("isp0_mode", MOS_ISP_WORKING_MODE_NORMAL);
    isp->SetProperty("isp0_framerate", 25);
    isp->SetProperty("isp1_enable", true);
    isp->SetProperty("isp1_mode", MOS_ISP_WORKING_MODE_NORMAL);
    isp->SetProperty("isp1_framerate", 25);
}

/// @brief  设置vi属性
void SetRgbViProperties(void)
{
    CElement *rgb_vi = ElementFactoryGet("rgb_vi");
    if (rgb_vi == NULL)
    {
        printf("ElementFactoryGet vi failed\n");
        return;
    }
    // set vi
    rgb_vi->SetProperty("device", std::string(RGB_CAMERA_DEV));
    rgb_vi->SetProperty("format", MOS_IMAGE_FORMAT_TYPE_NV12);
    rgb_vi->SetProperty("width", RGB_CAMERA_WIDTH);
    rgb_vi->SetProperty("height", RGB_CAMERA_HEIGHT);
    rgb_vi->SetProperty("count", 2);
}

/// @brief  设置rgb属性
void SetRgbResizeProperties(void)
{
    CElement *rgb_resize = ElementFactoryGet("rgb_resize");
    if (rgb_resize == NULL)
    {
        printf("ElementFactoryGet rgb_resize failed\n");
        return;
    }
    rgb_resize->SetProperty("format", MOS_IMAGE_FORMAT_TYPE_NV12);
    rgb_resize->SetProperty("out_width", RGB_RESIZE_WIDTH);
    rgb_resize->SetProperty("out_height", RGB_RESIZE_HEIGHT);
    rgb_resize->SetProperty("count", 2);
}

/// @brief  设置rgb属性
void SetRgbRotationProperties(void)
{
    CElement *rgb_rotation = ElementFactoryGet("rgb_rotation");
    if (rgb_rotation == NULL)
    {
        printf("ElementFactoryGet rgb_rotation failed\n");
        return;
    }
    // set rgb rotation
    rgb_rotation->SetProperty("rotation", MOS_IMAGE_ROTATION_270);
    rgb_rotation->SetProperty("format", MOS_IMAGE_FORMAT_TYPE_NV12);
    rgb_rotation->SetProperty("in_width", RGB_RESIZE_WIDTH);
    rgb_rotation->SetProperty("in_height", RGB_RESIZE_HEIGHT);
    rgb_rotation->SetProperty("count", 2);
}

/// @brief  设置drm属性
void SetDrmProperties(void)
{
    CElement *md_drm = ElementFactoryGet("md_drm");
    if (md_drm == NULL)
    {
        printf("ElementFactoryGet md_drm failed\n");
        return;
    }
    // drm
    md_drm->SetProperty("device", std::string("/dev/dri/card0"));
    // drm plane primary
    md_drm->SetProperty("plane", MOS_DRM_PLANE_TYPE_OVERLAY);
    md_drm->SetProperty("buffer_format", MOS_IMAGE_FORMAT_TYPE_NV12);
    // 输入源区域
    MosImageRect src_rect;
    src_rect.x = 0;
    src_rect.y = 64;
    src_rect.w = RGB_FACE_IMAGE_WIDTH;
    src_rect.h = RGB_FACE_IMAGE_HEIGHT - src_rect.y * 2;
    md_drm->SetProperty("src_rect", src_rect);
    // 显示区域
    MosImageRect dst_rect;
    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.w = LCD_WIDTH;
    dst_rect.h = LCD_HEIGHT;
    md_drm->SetProperty("dst_rect", dst_rect);
}
void SetRgaDrawProperties(void)
{
    CElement *rga_draw_rect = ElementFactoryGet("rga_draw_rect");
    if (rga_draw_rect == NULL)
    {
        printf("SetRgaDrawProperties rga_draw_rect failed\n");
        return;
    }
    rga_draw_rect->SetProperty("color", static_cast<uint32_t>(0x000000FF));
    rga_draw_rect->SetProperty("rotation", MOS_IMAGE_ROTATION_180);
}
int main(int argc, char *argv[])
{
    CSourcePad<MosImageRects> mos_image_rects_src_pad;

    signal(SIGINT, signal_handle);
    MosLogInit("/tmp/log", 1024 * 512, 1);

    HalLedParam hal_param;
    hal_param.type = HAL_LED_BACKLIGHT;
    hal_param.level = 100;
    HalLedSetAttr(&hal_param);

    hal_param.type = HAL_LED_FILLIN_IR;
    hal_param.level = 15;
    HalLedSetAttr(&hal_param);

    HalLcdClear(LCD_WIDTH, LCD_HEIGHT);

    CElement *isp = ElementFactoryMake("CMdIsp", "isp");
    CElement *rgb_vi = ElementFactoryMake("CMdvi", "rgb_vi");
    CElement *rgb_resize = ElementFactoryMake("CImageResize", "rgb_resize");
    CElement *rgb_rotation = ElementFactoryMake("CImageRotaiton", "rgb_rotation");
    CElement *rga_draw_rect = ElementFactoryMake("CRgaDrawRect", "rga_draw_rect");
    CElement *md_drm = ElementFactoryMake("CMdDrm", "md_drm");

    if (isp == nullptr || rgb_vi == nullptr || rgb_resize == nullptr || rgb_rotation == nullptr ||
        rga_draw_rect == nullptr || md_drm == nullptr)
    {
        ElementFactoryFreeAll();
        printf("create element failed\n");
        return -1;
    }

    SetIspProperties();
    SetRgbViProperties();
    SetRgbResizeProperties();
    SetRgbRotationProperties();
    SetDrmProperties();
    SetRgaDrawProperties();

    rgb_vi->SrcPad(1).Link(rgb_resize->SinkPad(1), "std::shared_ptr<MosImageData>"); // 先resize ,再rotaion
    rgb_resize->SrcPad(1).Link(rgb_rotation->SinkPad(1), "std::shared_ptr<MosImageData>");
    rgb_rotation->SrcPad(1).Link(rga_draw_rect->SinkPad(1), "std::shared_ptr<MosImageData>");
    rga_draw_rect->SrcPad(1).Link(md_drm->SinkPad(1), "std::shared_ptr<MosImageData>");

    mos_image_rects_src_pad.Link(rga_draw_rect->SinkPad(2), "MosImageRects");

    // start  element
    isp->Start();
    rgb_rotation->Start();
    rgb_resize->Start();
    rgb_vi->Start();
    md_drm->Start();
    rga_draw_rect->Start();

    MosImageRects mos_image_rects;

    for (int i = 0; i < 5; i++)
    {
        MosImageRect rect;
        rect.x = 100;
        rect.y = 100 + 100 * i;
        rect.w = 100;
        rect.h = 100;
        mos_image_rects.item.push_back(rect);
    }

    mos_image_rects_src_pad.Push(mos_image_rects);

    running = true;
    while (running)
    {
        sleep(1);
    }

    rga_draw_rect->Stop();
    rgb_rotation->Stop();
    rgb_resize->Stop();
    rgb_vi->Stop();
    md_drm->Stop();
    isp->Stop();

    ElementFactoryFreeAll();

    return 0;
}