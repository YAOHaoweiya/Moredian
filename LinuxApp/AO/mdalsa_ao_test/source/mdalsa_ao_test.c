#include <string>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "pad.h"
#include "factory.h"
//#include "hal_control.h"
#include "mos_log.h"

bool running = false;
static void signal_handle(int signo)
{
    printf("force exit signo %d !!!\n",signo);
    running = false;
    exit(0);
}

/// @brief  设置Ao属性
void SetAoProperties(void)
{
    CElement *mdalsa_ao = ElementFactoryGet("mdalsa_ao");
    if (mdalsa_ao == NULL)
    {
        printf("ElementFactoryGet mdalsa_ao failed\n");
        return;
    }
    // set isp
    mdalsa_ao->SetProperty("device", std::string("default"));
    mdalsa_ao->SetProperty("file", std::string("/data/1.wav"));
}

int main(int argc, char **argv)
{
    CSourcePad<std::string> ao_src_pad;
    int count = 0;

    signal(SIGINT, signal_handle);
    MosLogInit("/tmp/log",1024*512,1);

    CElement *mdalsa_ao = ElementFactoryMake("CAlsaControl", "mdalsa_ao");
    if(mdalsa_ao == nullptr)
    {
        ElementFactoryFreeAll();
        printf("create element failed\n");
        return -1;
    }
    SetAoProperties();
    ao_src_pad.Link(mdalsa_ao->SinkPad(1));
    mdalsa_ao->Start();

    running = true;
    while (1)
    {
        sleep(1);
    }
    mdalsa_ao->Stop();
    ElementFactoryFreeAll();
    return 0;
}