#include <string>
#include <cstring>
#include <fstream>

#include "mos_log.h"
#include "minimp3_ex.h"
#include "alsa_interface.h"
#include "alsa_control.h"


#define DETECT_FILE_TYPE(file) \
    ((file.find(".mp3") != std::string::npos) ? MOS_MEDIA_TYPE_MP3 : \
    ((file.find(".wav") != std::string::npos) ? MOS_MEDIA_TYPE_WAV : MOS_MEDIA_TYPE_UNKNOWN))

#define PARSE_WAV_HEADER(filePath, header) \
    do { \
        std::ifstream file(filePath, std::ios::binary); \
        if (!file.is_open()) { \
            std::cerr << "Error opening file: " << filePath << std::endl; \
            break; \
        } \
        file.read(reinterpret_cast<char*>(&header), sizeof(MosMediaWaveHead)); \
        file.close(); \
    } while (false)

//构造函数
CAlsaControl::CAlsaControl()
{
    running_ = false;
    m_channel = 2;
    m_rate = 44100;
    m_stream_direction = SND_PCM_STREAM_PLAYBACK;
    m_period = 2;
    m_period_size = 1152;
    m_format = SND_PCM_FORMAT_S16;
    m_access_type = SND_PCM_ACCESS_RW_INTERLEAVED;
    m_frames = (m_period * m_period_size) >>2;
    PROPERTY_REGISTER("device", device);
    PROPERTY_REGISTER("file", file);
    PROPERTY_REGISTER("channels", channels);
    PROPERTY_REGISTER("direction", direction);
    PROPERTY_REGISTER("period", period);
    PROPERTY_REGISTER("period_size", period_size);
    PROPERTY_REGISTER("format", format);
    PROPERTY_REGISTER("access_type", access_type);

    //SINKPAD_REGISTER(1);
    SINKPAD_REGISTER(1);
    //SinkPad1.SetCallback(BIND(CAlsaControl::SinkPadChain1));
}
//析构函数
CAlsaControl::~CAlsaControl()
{

}

void CAlsaControl::Start()
{
    if(running_ == false) {
        running_ = true;
        //open device
         int iRet;
        iRet = OpenDevice();
        if(iRet > 0)
        {
            __MODULE_ERR << "open pcm snd fail";
            return;
        }
        //init device
        iRet = InitDevice();
        if(iRet > 0)
        {
            __MODULE_ERR << "init pcm hw device fail";
            return;
        }
        // while(running_ == true) {
        //     PadQueueStatus status = SinkPad1.Pop(m_file, 0);
        //     if(status == PadQueueStatus::MD_PAD_OK)
        //     {
        //         __MODULE_ERR << "111";
        //     } 
        //     sleep(1);
        // }
        
     //   DecoderFileHead(m_file);  
        StartPlayback(m_file);
    }
        
}
void CAlsaControl::Stop()
{
    if(running_ == true) {
        running_ = false;
        UnitDevice();
    }
        
}

int CAlsaControl::OpenDevice()
{
    // if(snd_pcm_open(&m_handle, m_device.c_str(), m_stream_direction, 0) < 0)
    // {
    //     __MODULE_ERR << "open pcm card fail";
    //     return -1;
    // }
    // return 0;
    return 0;
}
void CAlsaControl::UnitDevice()
{
    /* Stop PCM device and drop pending frames */
    snd_pcm_drop(g_handle);
    /* Stop PCM device after pending frames have been played */ 
    snd_pcm_drain(g_handle);
    //close PCM
    snd_pcm_close(g_handle);
}
int CAlsaControl::InitDevice()
{
    int rc;
    int dir;
    unsigned int val;
    snd_pcm_uframes_t frames;
    snd_pcm_hw_params_t *params;

    rc = snd_pcm_open(&g_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        return -1;
    }

    snd_pcm_hw_params_alloca(&params);

    snd_pcm_hw_params_any(g_handle, params);

    snd_pcm_hw_params_set_access(g_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    snd_pcm_hw_params_set_format(g_handle, params, SND_PCM_FORMAT_S16_LE);

    snd_pcm_hw_params_set_channels(g_handle, params, 2);

    val = 44100;
    snd_pcm_hw_params_set_rate_near(g_handle, params, &val, &dir);

    frames = 32;
    snd_pcm_hw_params_set_period_size_near(g_handle, params, &frames, &dir);

    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    __MODULE_ERR << "get frames :" << frames;
    rc = snd_pcm_hw_params(g_handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        return -1;
    }


    // int dir;
    // //alloc pcm hw params
    // snd_pcm_hw_params_alloca(&m_hwparams);
    // /* Init hwparams with full configuration space */
    // if(snd_pcm_hw_params_any(m_handle, m_hwparams) < 0)
    // {
    //     __MODULE_ERR << "snd_pcm_hw_params_any fail";
    //     return -1;      
    // }
    //  //设置数据访问模式
    // if(snd_pcm_hw_params_set_access(m_handle, m_hwparams, m_access_type) < 0)
    // {
    //     __MODULE_ERR << "snd_pcm_hw_params_set_access fail";
    //     return -1;         
    // }
    // //set sample format bit per sample
    // if(snd_pcm_hw_params_set_format(m_handle, m_hwparams, m_format) < 0)
    // {
    //     __MODULE_ERR << "snd_pcm_hw_params_set_format fail";
    //     return -1;
    // }
    // //set channel num
    // if(snd_pcm_hw_params_set_channels(m_handle, m_hwparams, m_channel) < 0)
    // {
    //     __MODULE_ERR << "snd_pcm_hw_params_set_channels fail";
    //     return -1;
    // }
    // // //set period
    // // if(snd_pcm_hw_params_set_periods(m_handle, m_hwparams, m_period, 0) < 0)
    // // {
    // //     __MODULE_ERR << "snd_pcm_hw_params_set_periods fail";
    // //     return -1;
    // // }

    // //set sample rate
    // if(snd_pcm_hw_params_set_rate_near(m_handle, m_hwparams, &m_rate, 0) < 0)
    // {
    //     __MODULE_ERR << "snd_pcm_hw_params_set_rate_near fail";
    //     return -1;
    // }  
    //     long unsigned int value;
    // //每帧大小  每bit样本长度 * channel_num / 8bit
    // //buffer size 按帧计算器 周期数 * 每个周期时间
    // /* Set buffer size (in frames). The resulting latency is given by */
    //     /* latency = periodsize * periods / (rate * bytes_per_frame)     */
    // //pcm->frames = (pcm->period_szie * pcm->periods) >> 2;
    // // if (snd_pcm_hw_params_set_buffer_size(m_handle, m_hwparams, m_frames) == 0) {
    // //     __MODULE_ERR << "Error setting buffersize";
    // //    // return -1;
    // // }
    // snd_pcm_hw_params_set_period_size_near(m_handle, m_hwparams, &m_frames, &dir);
    // /* Apply HW parameter settings to PCM device and prepare device  */
    //  if (snd_pcm_hw_params(m_handle, m_hwparams) < 0) {
    //      __MODULE_ERR << "Error setting HW params";
    //      return -1;
    //  }
    
    // snd_pcm_hw_params_get_period_size(m_hwparams, &m_frames, &dir);
    return 0;
}

void CAlsaControl::DecoderFileHead(std::string media_file)
{
    MosMediaFileType type;

    type = DETECT_FILE_TYPE(media_file);
    __MODULE_ERR << "media_type" << type;
    switch(type)
    {
        case MOS_MEDIA_TYPE_MP3:
            //mp3 解码head
            #if 0
            mp3dec_file_info_t info;
            mp3dec_t mp3d;
            if(!mp3dec_load(&mp3d, media_file.c_str(), &info, NULL, NULL))
            {
                //get pcm channel and rate
                if(info.channels != 0 || info.hz != 0 || info.channels != m_channel || info.hz != m_rate)
                {
                    m_rate = info.hz;
                    m_channel = info.channels;
                    //set hw params
                    //set channel num
                    // snd_pcm_hw_params_set_channels(m_handle, m_hwparams, m_channel);
                    // //set sample rate
                    // snd_pcm_hw_params_set_rate_near(m_handle, m_hwparams, &m_rate, 0);
                    // //apply hw settings and prepare pcm
                    // snd_pcm_hw_params(m_handle, m_hwparams);
                }
            }
            #endif
            break;
        case MOS_MEDIA_TYPE_WAV:
            MosMediaWaveHead wavheader;
            memset(&wavheader, 0, sizeof(MosMediaWaveHead));
            PARSE_WAV_HEADER(media_file, wavheader);
            __MODULE_ERR << "wav sample rate: " << wavheader.sample_rate;
            __MODULE_ERR << "wav channel_num: " << wavheader.chanel_num;
            if((wavheader.chanel_num != 0 && wavheader.sample_rate != 0 )&& (wavheader.chanel_num != m_channel || wavheader.sample_rate != m_rate))
            {
                // m_rate = wavheader.sample_rate;
                // m_channel = wavheader.chanel_num;
                //set hw params
                //set channel num
                // snd_pcm_hw_params_set_channels(m_handle, m_hwparams, m_channel);
                // //set sample rate
                // snd_pcm_hw_params_set_rate_near(m_handle, m_hwparams, &m_rate, 0);
                //apply hw settings and prepare pcm
                // snd_pcm_hw_params(m_handle, m_hwparams);
            }
            break;
        case MOS_MEDIA_TYPE_UNKNOWN:
            __MODULE_ERR << "unknown media file type";
            break;
        default:
            __MODULE_INFO << "no support media file";
    }
}

void CAlsaControl::StartPlayback(std::string media_file)
{
    MosMediaFileType type;
    int ret;
    std::streamsize read_size;
    type = DETECT_FILE_TYPE(media_file);
    __MODULE_ERR << "media_type" << type;
    switch(type)
    {
        case MOS_MEDIA_TYPE_MP3:
            __MODULE_ERR << "no support media file";
            break;
        case MOS_MEDIA_TYPE_WAV:
            // std::ifstream file(media_file.c_str(), std::ios::binary);
            // if(!file.is_open()) {
            //     std::cerr << "Error opening file: " << media_file.c_str() << std::endl;
            //     return;
            // }
            // //申请内存
            // int buffer_size = m_period * m_period_size;
            // char *buffer = new char[buffer_size];
            // // 定位到文件的第 wavhead tail byte
            // file.read(buffer, sizeof(MosMediaWaveHead));
            // do{
            //     memset(buffer, 0, buffer_size);
            //     file.read(buffer, buffer_size);
            //     read_size = file.gcount();
            //     __MODULE_ERR << "read size " << read_size;
            //     while ((ret = snd_pcm_writei(m_handle, buffer, m_frames)) < 0) {
            //        // snd_pcm_prepare(m_handle);
            //         __MODULE_INFO << "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>";
            //     }
            // }while(read_size);
            // file.close();
            // delete[] buffer;
            // break;

            int buffer_size = 128 * 4;
            char *buff =(char *)malloc(buffer_size);
            FILE *fp = fopen(media_file.c_str(), "rb");
            fread(buff, 1, sizeof(MosMediaWaveHead), fp);
            while(1)
            {
                memset(buff, 0, buffer_size);
                ret = fread(buff, 1, buffer_size, fp);
               // __MODULE_ERR << "read buff len " << ret;
                // if(ret == 0){
                //     printf("end of music file input! \n");
                //     //exit(1);
                // }
            
                // if(ret < 0){
                //     printf("read pcm from file! \n");
                // //  exit(1);
                // }
                if(ret > 0) {
                    snd_pcm_writei(g_handle, buff, snd_pcm_bytes_to_frames(g_handle, ret));
                }
            }
            free(buff);
            buff = NULL;
        // default:
        //     __MODULE_ERR << "no support media file";
    }
}

REGISTER_ELEMENT("CAlsaControl", CAlsaControl)
