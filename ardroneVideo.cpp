/*
 * @video_stage.c
 * @author marc-olivier.dzeukou@parrot.com
 * @date 2007/07/27
 *
 * ihm vision thread implementation
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <sys/time.h>
#include <time.h>
#include <iostream>


extern "C"
{
}
#include "ardroneVideo.h"

ArdroneVideo * ArdroneVideo::_instance = 0;


using namespace std;


C_RESULT initNewFrameCb( void *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  return (SUCCESS);
}

C_RESULT updateNewFrameCb( void *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  return ArdroneVideo::GetInstance()->NewFrame(cfg, in, out);
}

C_RESULT destroyNewFrameCb( void *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  return (SUCCESS);
}







const vp_api_stage_funcs_t vp_stages_display_frame_funcs =
{
  NULL,
  (vp_api_stage_open_t)initNewFrameCb,
  (vp_api_stage_transform_t)updateNewFrameCb,
  (vp_api_stage_close_t)destroyNewFrameCb
}; 




ArdroneVideo * ArdroneVideo::GetInstance()
{
  if (!_instance)
    _instance = new ArdroneVideo();

  return _instance;
}


void * _storeThreadStart(void * data)
{
  return ArdroneVideo::GetInstance()->FrameStore(data);
}

ArdroneVideo::ArdroneVideo() :
  _exit(false),
  _w(QVGA_WIDTH),
  _h(QVGA_HEIGHT),
  _savePath("./"),
  _nbFrames(0),
  _framesDump(NULL),
  _disableEncoding(false)
{
  vp_os_mutex_init(&_videoUpdateLock);
  vp_os_mutex_init(&_storePoolLock);
  /// Picture configuration
  _picture.format        = PIX_FMT_YUV420P;

  _picture.width         = QVGA_WIDTH;
  _picture.height        = QVGA_HEIGHT;
  _picture.framerate     = 30;

  _picture.y_buf   = (uint8_t *) vp_os_malloc( QVGA_WIDTH * QVGA_HEIGHT     );
  _picture.cr_buf  = (uint8_t *) vp_os_malloc( QVGA_WIDTH * QVGA_HEIGHT / 4 );
  _picture.cb_buf  = (uint8_t *) vp_os_malloc( QVGA_WIDTH * QVGA_HEIGHT / 4 );

  _picture.y_line_size   = QVGA_WIDTH;
  _picture.cb_line_size  = QVGA_WIDTH / 2;
  _picture.cr_line_size  = QVGA_WIDTH / 2;

  vp_os_memset(&_videoComConfig,          0, sizeof( _videoComConfig ));
  vp_os_memset(&_vlibDecodingConf,          0, sizeof( _vlibDecodingConf ));
  vp_os_memset(&_yuv2rgbconf,  0, sizeof( _yuv2rgbconf ));

  _videoComConfig.com                 = COM_VIDEO();
  _videoComConfig.buffer_size         = 100000;
  _videoComConfig.protocol            = VP_COM_UDP;
  COM_CONFIG_SOCKET_VIDEO(&_videoComConfig.socket, VP_COM_CLIENT, VIDEO_PORT, wifi_ardrone_ip);

  _vlibDecodingConf.width               = QVGA_WIDTH;
  _vlibDecodingConf.height              = QVGA_HEIGHT;
  _vlibDecodingConf.picture             = &_picture;
  _vlibDecodingConf.block_mode_enable   = TRUE;
  _vlibDecodingConf.luma_only           = FALSE;

  _yuv2rgbconf.rgb_format = VP_STAGES_RGB_FORMAT_RGB24;

  _pipeline.nb_stages = 0;

  _stages[_pipeline.nb_stages].type    = VP_API_INPUT_SOCKET;
  _stages[_pipeline.nb_stages].cfg     = (void *)&_videoComConfig;
  _stages[_pipeline.nb_stages].funcs   = video_com_funcs;

  _pipeline.nb_stages++;

  _stages[_pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  _stages[_pipeline.nb_stages].cfg     = (void*)&_vlibDecodingConf;
  _stages[_pipeline.nb_stages].funcs   = vlib_decoding_funcs;

  _pipeline.nb_stages++;

  _stages[_pipeline.nb_stages].type    = VP_API_FILTER_YUV2RGB;
  _stages[_pipeline.nb_stages].cfg     = (void*)&_yuv2rgbconf;
  _stages[_pipeline.nb_stages].funcs   = vp_stages_yuv2rgb_funcs;

  _pipeline.nb_stages++;

  _stages[_pipeline.nb_stages].type    = VP_API_OUTPUT_SDL;
  _stages[_pipeline.nb_stages].cfg     = &_videoComConfig;
  _stages[_pipeline.nb_stages].funcs   = vp_stages_display_frame_funcs;

  _pipeline.nb_stages++;

  _pipeline.stages = &_stages[0];
 
  int res = vp_api_open(&_pipeline, &_pipeline_handle);

  if(! SUCCEED(res) )
  {
    printf("Video > init Error\n"); 
    _initOk = false;
    return;
  }
  _out.status = VP_API_STATUS_PROCESSING;

  _frameData = NULL;
  _initOk = true;
  
}


ArdroneVideo::~ArdroneVideo()
{
  void * ret;
  _exit = true;//Ask FrameEncoder to finish
  vp_os_mutex_lock(&_videoUpdateLock);


  // Final cleanup.
  pthread_join(_storeThread, &ret);//Wait for last frames encoding
  vp_os_mutex_destroy(&_storePoolLock);  
  vp_os_mutex_destroy(&_videoUpdateLock);

  if((!_disableEncoding) && EncodeFramesFile(_savePath+_dumpFileName))
  {
    unlink((_savePath+_dumpFileName).c_str());
  }

  if (_initOk)
  {
    vp_api_close(&_pipeline, &_pipeline_handle);  //FIXME : SEGFAULT...
  }
  cout << "Video > Closed" << endl;
}


void ArdroneVideo::StartRecording()
{
  
  pthread_attr_t stAttr;
  pthread_attr_init(&stAttr);
   if (!_initOk)
    return;
  if (pthread_create(&_storeThread, &stAttr, _storeThreadStart, NULL) != 0)
  {
    printf("Can't create video encoder thread !!\n");
  }  
  
}

void ArdroneVideo::Update(void * data)
{
  if( SUCCEED(vp_api_run(&_pipeline, &_out)) ) 
  {
    if( (_out.status == VP_API_STATUS_PROCESSING || 
         _out.status == VP_API_STATUS_STILL_RUNNING) ) 
    {


    }
    else
    {
      //printf("Video > Update failed\n");
    }
  }
}

int ArdroneVideo::NewFrame(void *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  unsigned char * dup;
  if (!_initOk)
    return SUCCESS;
  vp_os_mutex_lock(&_videoUpdateLock);
  /* Get a reference to the last decoded picture */
  _frameData     = (uint8_t*)in->buffers[0];
  //Duplicate frame for video encoding
  dup = (unsigned char *) vp_os_malloc(QVGA_WIDTH*QVGA_HEIGHT*3);
  memcpy(dup, in->buffers[0], QVGA_WIDTH*QVGA_HEIGHT*3);
  vp_os_mutex_unlock(&_videoUpdateLock);

  vp_os_mutex_lock(&_storePoolLock);
  _storePool.push_back(dup);
  vp_os_mutex_unlock(&_storePoolLock);
  
  
  return (SUCCESS);
}


bool ArdroneVideo::EncodeFramesFile(string path)
{
  FILE * f; 
  int  encoderHandle;
  char outName[255];
  unsigned char * rawFrame = NULL;
  int frameSize = -1;
  short w;
  short h;
  short fps;
  int nbEncodedFrames = 0;
  cout << "opening " << path <<  " for encoding" << endl;

  Revel_Error revError = Revel_CreateEncoder(&encoderHandle);
  if (revError != REVEL_ERR_NONE)
  {
    printf("Revel Error while creating encoder: %d\n", revError);
    return false;
  }

  f = fopen(path.c_str(), "rb");
  if (!f)
    return false;
  if ((fread(&frameSize, 1, sizeof(int), f) != sizeof(int)) || (frameSize <= 0))
  {
    cout << "Invalid header (frame size)" << endl;
    return false;
  }
  if ((fread(&w, 1, sizeof(short), f) != sizeof(short))|| (w <= 0) ||  (w > 1024))
  {
    cout << "Invalid header (w)" << endl;
    return false;
  }
  if ((fread(&h, 1, sizeof(short), f) != sizeof(short)) || (h <= 0) ||  (h > 1024))
  {
    cout << "Invalid header (h)" << endl;
    return false;
  }
  if ((fread(&fps, 1, sizeof(short), f) != sizeof(short)) || (fps <= 0) || (fps > 30))
  {
    cout << "Invalid header (fps)" << endl;
    return false;
  }
  if (((w * h * 2) != frameSize) &&
      ((w * h * 3) != frameSize) &&
      ((w * h * 4) != frameSize))
  {
    cout << "Invalid header (frame size is strange)" << endl;    
    return false;
  }
  cout << "+++++++++++++++" << endl;
  cout << "Dup header:" << endl;
  cout << "> FrameSize : " << frameSize << endl;
  cout << "> Width : " << w << endl;
  cout << "> Height : " << h << endl;
  cout << "> Fps : " << fps << endl;

  Revel_Params revParams;
  Revel_InitializeParams(&revParams);
  revParams.width = w;
  revParams.height = h;
  revParams.frameRate = (float) fps;
  revParams.quality = 1.0f;
  revParams.codec = REVEL_CD_XVID;
  revParams.hasAudio = 0;
  
  //rename xxxx.dmp -> xxx.avi
  strcpy(outName, path.c_str());
  memcpy(outName+strlen(outName)-3, "avi", 3);

  revError = Revel_EncodeStart(encoderHandle, outName, &revParams);
  if (revError != REVEL_ERR_NONE)
  {
    printf("Revel Error while starting encoding: %d\n", revError);
    fclose(f);
    return false;
  }
  
  rawFrame = (unsigned char *) vp_os_malloc(frameSize);
  while (!feof(f))
  {
    if (fread(rawFrame, sizeof(char), frameSize, f) != (unsigned int)frameSize)
    {
      printf("Incomplete frame ! Stopping.\n");
      break;
    }
    unsigned char c;
    //Convert frame RGB to BGR
    int i = 0;
    while (i < w*h*3)
    {
      c = rawFrame[i];
      rawFrame[i]   = rawFrame[i+2];   //B
      rawFrame[i+1] = rawFrame[i+1]; //G
      rawFrame[i+2] = c; //R
      i+=3;
    }
    
    // Encode each frame.
    int frameSize;
    Revel_VideoFrame frame;
    frame.width = w;
    frame.height = h;
    frame.bytesPerPixel = 3;
    frame.pixelFormat = REVEL_PF_BGR;
    frame.pixels = rawFrame;
    revError = Revel_EncodeFrame(encoderHandle, &frame, &frameSize);
    if (revError != REVEL_ERR_NONE)
    {
      printf("Revel Error while writing frame: %d\n", revError);
      break;
    }
    if (nbEncodedFrames && !(nbEncodedFrames%fps) )
      printf("Encoded %.0fs of video..\n", nbEncodedFrames/(float)fps);

    nbEncodedFrames ++;    
  }
  fclose(f);
  vp_os_free(rawFrame);
  
  int totalSize;
  revError = Revel_EncodeEnd(encoderHandle, &totalSize);
  if (revError != REVEL_ERR_NONE)
  {
    printf("Revel Error while ending encoding: %d\n", revError);
  }
  Revel_DestroyEncoder(encoderHandle);

  printf("%d frames encoded -- Total size : %d\n--> %s\n\n", nbEncodedFrames, totalSize, outName);    
  return true;
}

void * ArdroneVideo::FrameStore(void * data)
{
  time_t t;
  int tmp;
  short tmp2;
  char timestr[32];
  time(&t);
  strftime(timestr, 32, "capture_%Y-%m-%d_%H_%M.dmp", localtime(&t));
  _dumpFileName = timestr;
  cout << "Storing raw frames to : " << (_savePath+_dumpFileName) << endl;
  _framesDump = fopen((_savePath+_dumpFileName).c_str(), "wb");
  
  tmp = _w * _h * 3;
  fwrite(&tmp, sizeof(int), 1, _framesDump);
  tmp2 = _w;
  fwrite(&tmp2, sizeof(short), 1, _framesDump);
  tmp2 = _h;
  fwrite(&tmp2, sizeof(short), 1, _framesDump);
  tmp2 = 15;
  fwrite(&tmp2, sizeof(short), 1, _framesDump);

  while (!(_exit && !_storePool.size()))
  {  
    unsigned char * current = NULL;
    if (_exit)
      cout << "Still " << _storePool.size() << " frames to store before exit !" << endl;

    //Extract last frame
    vp_os_mutex_lock(&_storePoolLock);
    if (_storePool.size())
    {
      current = _storePool.front();
      _storePool.pop_front();
    }
    vp_os_mutex_unlock(&_storePoolLock);
    

    if (current)
    {
      fwrite(current, sizeof(char), _w*_h*3, _framesDump);
      _nbFrames ++;
      vp_os_free(current);
    }
    usleep(1000);
  }
  cout << "Frame store stopped." << endl;
  fclose(_framesDump);  
  return (SUCCESS);
}




SDL_Surface * ArdroneVideo::GetVideoFrame()
{
  SDL_Surface * s = NULL;
  vp_os_mutex_lock(&_videoUpdateLock);
  if (_frameData)
  {
     s = SDL_CreateRGBSurfaceFrom(_frameData,
			    QVGA_WIDTH, QVGA_HEIGHT, 24, QVGA_WIDTH*3,
			    0X000000FF, 0x0000FF00, 0x00FF0000, 0x00000000);
  }
  vp_os_mutex_unlock(&_videoUpdateLock);

  return s;
}



