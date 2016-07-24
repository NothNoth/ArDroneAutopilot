#ifndef _ARDRONE_VIDEO_H
#define _ARDRONE_VIDEO_H

#include <config.h>
#include <VP_Api/vp_api_thread_helper.h>
#include <iostream>
#include <list>
extern "C"
{
  #include <SDL/SDL.h>
  #include <VP_Api/vp_api.h>
  #include <VP_Api/vp_api_error.h>
  #include <VP_Api/vp_api_stage.h>
  #include <VP_Api/vp_api_picture.h>
  #include <VLIB/Stages/vlib_stage_decode.h>
  #include <VP_Stages/vp_stages_yuv2rgb.h>
  #include <ardrone_tool/Com/config_com.h>
  #include <VP_Stages/vp_stages_io_file.h>
  #include <VP_Stages/vp_stages_i_camif.h>

  #include <config.h>
  #include <VP_Os/vp_os_print.h>
  #include <VP_Os/vp_os_malloc.h>
  #include <VP_Os/vp_os_delay.h>

  #include <VP_Stages/vp_stages_buffer_to_picture.h>
  #include <ardrone_tool/ardrone_tool.h>

  #include <ardrone_tool/Video/video_com_stage.h>

}

#include <revel.h>
using namespace std;

#define NB_STAGES 10
class ArdroneVideo
{
  public:
    static ArdroneVideo * GetInstance();
    ~ArdroneVideo();
    void Update(void * data);
    SDL_Surface * GetVideoFrame();
    int NewFrame(void *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out);
    void * FrameStore(void * data);
    bool _initOk;
    void SetSavePath(string path) {_savePath = path; cout << "Video will be save to " << path << endl;}
    void StartRecording();
    static bool EncodeFramesFile(string path);
    void DisableEncoding() {_disableEncoding = true;}
  private:
    ArdroneVideo();
    static ArdroneVideo *  _instance;
    unsigned char *        _frameData;
    vp_api_io_pipeline_t   _pipeline;
    vp_api_io_data_t       _out;
    vp_api_io_stage_t      _stages[NB_STAGES];

    vp_api_picture_t       _picture;

    video_com_config_t              _videoComConfig;
    vlib_stage_decoding_config_t    _vlibDecodingConf;
    vp_stages_yuv2rgb_config_t      _yuv2rgbconf;
    PIPELINE_HANDLE         _pipeline_handle;
    vp_os_mutex_t           _videoUpdateLock;
    bool   _exit;
    int    _w;
    int    _h;
    string _savePath;
    int    _nbFrames;
    FILE * _framesDump;
    string _dumpFileName;
    void InitVideo();
    vp_os_mutex_t           _storePoolLock;
    list<unsigned char *>   _storePool;
    pthread_t               _storeThread;
    bool _disableEncoding;
};

#endif // _ARDRONE_VIDEO_H
