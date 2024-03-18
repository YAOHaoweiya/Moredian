#ifndef ALSA_CONTROL_H_
#define ALSA_CONTROL_H_

#include <mutex>

#include "alsa_interface.h"
#include "alsa/asoundlib.h"
#include "pad.h"
#include "property.h"
#include "register.h"
//#include "minimp3_ex.h"

class CAlsaControl:public CElement {
public:
    /*pads*/
    CSinkPad<std::string>          SinkPad1;  //需要播放的音频文件 mp3 wav..

    Property<std::string> device {
        [&](std::string x) {m_device = x;},
        [&]{return m_device;}
    };
    Property<std::string> file {
        [&](std::string x) {m_file = x;},
        [&]{return m_file;}
    };
    Property<int> channels {
        [&](int x) {m_channel = x;},
        [&]{return m_channel;}
    };
    Property<int> period {
        [&](int x) {m_period = x;},
        [&]{return m_period;}
    };
    Property<int> period_size {
        [&](int x) {m_period_size = x;},
        [&]{return m_period_size;}
    };
    Property<unsigned int> rate {
        [&](unsigned int x) {m_rate = x;},
        [&]{return m_rate;}
    };
    Property<snd_pcm_stream_t> direction {
        [&](snd_pcm_stream_t x) {m_stream_direction = x;},
        [&]{return m_stream_direction;}
    };
    Property<snd_pcm_access_t> access_type {
        [&](snd_pcm_access_t x) {m_access_type = x;},
        [&]{return m_access_type;}
    };
    Property<snd_pcm_format_t> format {
        [&](snd_pcm_format_t x) {m_format = x;},
        [&]{return m_format;}
    };

    CAlsaControl();
    ~CAlsaControl();

    void Start() override;
    void Stop() override;
private:
    bool running_;
    std::mutex ao_mutex;
    std::string m_file;
    //MosAlsaHw m_hw;
     snd_pcm_t *g_handle;
//    snd_pcm_hw_params_t *m_hwparams;
    //声卡name
    std::string m_device;
    //通道数
    int m_channel;
    //周期数
    int m_period;
    //每个周期大小
    int m_period_size;
    //采样速率
    unsigned int m_rate;
    //帧数
    snd_pcm_uframes_t m_frames;
    //pcm stream direction
    snd_pcm_stream_t m_stream_direction;
    //PCM access type 交错模式或者交错
    snd_pcm_access_t m_access_type;
    //sample format bit per sample
    snd_pcm_format_t m_format;
    //mp3解码
    
    //播放pcm

    //声卡初始化
    int OpenDevice(void);
    int InitDevice(void);
    void UnitDevice(void);
    //解析media file head
    void DecoderFileHead(std::string media_file);
    void StartPlayback(std::string media_file);
    //void SinkPadChain1(std::string media);
};

#endif