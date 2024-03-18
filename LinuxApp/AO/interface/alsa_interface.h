#ifndef ALSA_INTERFACE_H_
#define ALSA_INTERFACE_H_

#include <cstdint>

// enum MosSampleRate {
//     MOS_ALSA_SAMPLE_RATE_8KHZ = 8000,
//     MOS_ALSA_SAMPLE_RATE_11KHZ = 11025,
//     MOS_ALSA_SAMPLE_RATE_16KHZ = 16000,
//     MOS_ALSA_SAMPLE_RATE_22KHZ = 22050,
//     MOS_ALSA_SAMPLE_RATE_32KHZ = 32000,
//     MOS_ALSA_SAMPLE_RATE_44KHZ = 44100,
//     MOS_ALSA_SAMPLE_RATE_48KHZ = 48000,
//     MOS_ALSA_SAMPLE_RATE_88KHZ = 88200,
//     MOS_ALSA_SAMPLE_RATE_96KHZ = 96000,
//     MOS_ALSA_SAMPLE_RATE_192KHZ = 192000
// };

// enum MosSndPcmStream {
//     /** Playback stream */
//     MOS_ALSA_SND_PCM_STREAM_PLAYBACK = 0,
//     /** Capture stream */
//     MOS_ALSA_SND_PCM_STREAM_CAPTURE,
//     MOS_ALSA_SND_PCM_STREAM_LAST = MOS_ALSA_SND_PCM_STREAM_CAPTURE
// };

// enum MosSndPcmAccess {
//     /** mmap access with simple interleaved channels */
//     MOS_ALSA_SND_PCM_ACCESS_MMAP_INTERLEAVED = 0,
//     /** mmap access with simple non interleaved channels */
//     MOS_ALSA_SND_PCM_ACCESS_MMAP_NONINTERLEAVED,
//     /** mmap access with complex placement */
//     MOS_ALSA_SND_PCM_ACCESS_MMAP_COMPLEX,
//     /** snd_pcm_readi/snd_pcm_writei access */
//     MOS_ALSA_SND_PCM_ACCESS_RW_INTERLEAVED,
//     /** snd_pcm_readn/snd_pcm_writen access */
//     MOS_ALSA_SND_PCM_ACCESS_RW_NONINTERLEAVED,
//     MOS_ALSA_SND_PCM_ACCESS_LAST = MOS_ALSA_SND_PCM_ACCESS_RW_NONINTERLEAVED
// };

// enum MosSndPcmFormat {
//     /** Unknown */
//     MOS_ALSA_SND_PCM_FORMAT_UNKNOWN = -1,
//     /** Signed 8 bit */
//     MOS_ALSA_SND_PCM_FORMAT_S8 = 0,
//     /** Unsigned 8 bit */
//     MOS_ALSA_SND_PCM_FORMAT_U8,
//     /** Signed 16 bit Little Endian */
//     MOS_ALSA_SND_PCM_FORMAT_S16_LE,
//     /** Signed 16 bit Big Endian */
//     MOS_ALSA_SND_PCM_FORMAT_S16_BE,
//     /** Unsigned 16 bit Little Endian */
//     MOS_ALSA_SND_PCM_FORMAT_U16_LE,
//     /** Unsigned 16 bit Big Endian */
//     MOS_ALSA_SND_PCM_FORMAT_U16_BE,
//     /** Signed 24 bit Little Endian using low three bytes in 32-bit word */
//     MOS_ALSA_SND_PCM_FORMAT_S24_LE,
//     /** Signed 24 bit Big Endian using low three bytes in 32-bit word */
//     MOS_ALSA_SND_PCM_FORMAT_S24_BE,
//     /** Unsigned 24 bit Little Endian using low three bytes in 32-bit word */
//     MOS_ALSA_SND_PCM_FORMAT_U24_LE,
//     /** Unsigned 24 bit Big Endian using low three bytes in 32-bit word */
//     MOS_ALSA_SND_PCM_FORMAT_U24_BE,
//     /** Signed 32 bit Little Endian */
//     MOS_ALSA_SND_PCM_FORMAT_S32_LE,
//     /** Signed 32 bit Big Endian */
//     MOS_ALSA_SND_PCM_FORMAT_S32_BE,
//     /** Unsigned 32 bit Little Endian */
//     MOS_ALSA_SND_PCM_FORMAT_U32_LE,
//     /** Unsigned 32 bit Big Endian */
//     MOS_ALSA_SND_PCM_FORMAT_U32_BE,
//     /** Float 32 bit Little Endian, Range -1.0 to 1.0 */
//     MOS_ALSA_SND_PCM_FORMAT_FLOAT_LE,
//     /** Float 32 bit Big Endian, Range -1.0 to 1.0 */
//     MOS_ALSA_SND_PCM_FORMAT_FLOAT_BE,
//     /** Float 64 bit Little Endian, Range -1.0 to 1.0 */
//     MOS_ALSA_SND_PCM_FORMAT_FLOAT64_LE,
//     /** Float 64 bit Big Endian, Range -1.0 to 1.0 */
//     MOS_ALSA_SND_PCM_FORMAT_FLOAT64_BE,
//     /** IEC-958 Little Endian */
//     MOS_ALSA_SND_PCM_FORMAT_IEC958_SUBFRAME_LE,
//     /** IEC-958 Big Endian */
//     MOS_ALSA_SND_PCM_FORMAT_IEC958_SUBFRAME_BE,
//     /** Mu-Law */
//     MOS_ALSA_SND_PCM_FORMAT_MU_LAW,
//     /** A-Law */
//     MOS_ALSA_SND_PCM_FORMAT_A_LAW,
//     /** Ima-ADPCM */
//     MOS_ALSA_SND_PCM_FORMAT_IMA_ADPCM,
//     /** MPEG */
//     MOS_ALSA_SND_PCM_FORMAT_MPEG,
//     /** GSM */
//     MOS_ALSA_SND_PCM_FORMAT_GSM,
//     /** Signed 20bit Little Endian in 4bytes format, LSB justified */
//     MOS_ALSA_SND_PCM_FORMAT_S20_LE,
//     /** Signed 20bit Big Endian in 4bytes format, LSB justified */
//     MOS_ALSA_SND_PCM_FORMAT_S20_BE,
//     /** Unsigned 20bit Little Endian in 4bytes format, LSB justified */
//     MOS_ALSA_SND_PCM_FORMAT_U20_LE,
//     /** Unsigned 20bit Big Endian in 4bytes format, LSB justified */
//     MOS_ALSA_SND_PCM_FORMAT_U20_BE,
//     /** Special */
//     MOS_ALSA_SND_PCM_FORMAT_SPECIAL = 31,
//     /** Signed 24bit Little Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_S24_3LE = 32,
//     /** Signed 24bit Big Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_S24_3BE,
//     /** Unsigned 24bit Little Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_U24_3LE,
//     /** Unsigned 24bit Big Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_U24_3BE,
//     /** Signed 20bit Little Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_S20_3LE,
//     /** Signed 20bit Big Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_S20_3BE,
//     /** Unsigned 24bit Little Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_U20_3LE,
//     /** Unsigned 20bit Big Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_U20_3BE,
//     /** Signed 18bit Little Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_S18_3LE,
//     /** Signed 18bit Big Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_S18_3BE,
//     /** Unsigned 18bit Little Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_U18_3LE,
//     /** Unsigned 18bit Big Endian in 3bytes format */
//     MOS_ALSA_SND_PCM_FORMAT_U18_3BE,
//     /* G.723 (ADPCM) 24 kbit/s, 8 samples in 3 bytes */
//     MOS_ALSA_SND_PCM_FORMAT_G723_24,
//     /* G.723 (ADPCM) 24 kbit/s, 1 sample in 1 byte */
//     MOS_ALSA_SND_PCM_FORMAT_G723_24_1B,
//     /* G.723 (ADPCM) 40 kbit/s, 8 samples in 3 bytes */
//     MOS_ALSA_SND_PCM_FORMAT_G723_40,
//     /* G.723 (ADPCM) 40 kbit/s, 1 sample in 1 byte */
//     MOS_ALSA_SND_PCM_FORMAT_G723_40_1B,
//     /* Direct Stream Digital (DSD) in 1-byte samples (x8) */
//     MOS_ALSA_SND_PCM_FORMAT_DSD_U8,
//     /* Direct Stream Digital (DSD) in 2-byte samples (x16) */
//     MOS_ALSA_SND_PCM_FORMAT_DSD_U16_LE,
//     /* Direct Stream Digital (DSD) in 4-byte samples (x32) */
//     MOS_ALSA_SND_PCM_FORMAT_DSD_U32_LE,
//     /* Direct Stream Digital (DSD) in 2-byte samples (x16) */
//     MOS_ALSA_SND_PCM_FORMAT_DSD_U16_BE,
//     /* Direct Stream Digital (DSD) in 4-byte samples (x32) */
//     MOS_ALSA_SND_PCM_FORMAT_DSD_U32_BE,
//     MOS_ALSA_SND_PCM_FORMAT_LAST = MOS_ALSA_SND_PCM_FORMAT_DSD_U32_BE,
// };

// struct MosAlsaHw
// {
//     //handle
//     snd_pcm_t *handle;
//     //specify configution for pcm
//     snd_pcm_hw_params_t *hwparams;
// };

enum MosMediaFileType
{
    MOS_MEDIA_TYPE_MP3 = 0,
    MOS_MEDIA_TYPE_WAV,
    MOS_MEDIA_TYPE_UNKNOWN = -1
};
struct MosMediaWaveHead {
    //riff chunk
    uint8_t RIFF_ID[4];
    uint32_t RIFF_size;
    uint8_t format[4];
    //fmt chunk
    uint8_t Fmt_ID[4];
    uint32_t fmt_size;
    uint16_t audio_format;
    uint16_t chanel_num;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bitspersample;
    //data chunk
    uint8_t Data_ID[4];
    uint32_t data_size;
};

#endif