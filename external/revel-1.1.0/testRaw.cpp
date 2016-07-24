#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <revel.h>


int main(int argc, char * argv[])
{
  Revel_Error revError;
  int encoderHandle;
  int width = 320;
  int height = 240;
  
  revError = Revel_CreateEncoder(&encoderHandle);
  if (revError != REVEL_ERR_NONE)
  {
    printf("Revel Error while creating encoder: %d\n", revError);
    exit(1);
  }

  Revel_Params revParams;
  Revel_InitializeParams(&revParams);
  revParams.width = width;
  revParams.height = height;
  revParams.frameRate = 30.0f;
  revParams.quality = 1.0f;
  revParams.codec = REVEL_CD_RAW;

  revParams.hasAudio = 0;
  revParams.audioChannels = 0;
  revParams.audioRate = 0;
  revParams.audioBits = 0;
  revParams.audioSampleFormat = 0;

  // Initiate encoding
  revError = Revel_EncodeStart(encoderHandle, "testRaw.avi", &revParams);
  if (revError != REVEL_ERR_NONE)
  {
    printf("Revel Error while starting encoding: %d\n", revError);
    exit(1);
  }
  
  
  //Fake frames
  Revel_VideoFrame frame;
  frame.width = width;
  frame.height = height;
  frame.bytesPerPixel = 3;
  frame.pixelFormat = REVEL_PF_BGR;
  frame.pixels = (unsigned char *) malloc(width*height*frame.bytesPerPixel);
  memset(frame.pixels, 0xCD, width*height*frame.bytesPerPixel);
  
  for(int i = 0; i < 10; ++i)
  {
    int frameSize;

    memset(frame.pixels, (char)(i&&0xFF), width*height*frame.bytesPerPixel);
      
    revError = Revel_EncodeFrame(encoderHandle, &frame, &frameSize);
  	if (revError != REVEL_ERR_NONE)
    {
      printf("Revel Error while writing frame: %d\n", revError);
      exit(1);
    }
    printf("Added frame %d size : %d\n", i, frameSize);
  }

  int totalSize;
  revError = Revel_EncodeEnd(encoderHandle, &totalSize);
  if (revError != REVEL_ERR_NONE)
  {
    printf("Revel Error while ending encoding: %d\n", revError);
    exit(1);
  }
  Revel_DestroyEncoder(encoderHandle);

  return 0;
}
