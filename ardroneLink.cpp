/**
 * @file main.c
 * @author sylvain.gaeremynck@parrot.com
 * @date 2009/07/01
 */

extern "C"
{
  #include "ardroneLink.h"

  //ARDroneLib
  #include <ardrone_tool/ardrone_time.h>
  #include <ardrone_tool/Navdata/ardrone_navdata_client.h>
  #include <ardrone_tool/Control/ardrone_control.h>
  #include <ardrone_tool/UI/ardrone_input.h>

  //Common
  #include <config.h>
  #include <ardrone_api.h>

  //VP_SDK
  #include <ATcodec/ATcodec_api.h>
  #include <VP_Os/vp_os_print.h>
  #include <VP_Api/vp_api_thread_helper.h>
  #include <VP_Os/vp_os_signal.h>
  #include <ardrone_tool/Video/video_com_stage.h>

  #include <signal.h>
}
#include <iostream>
#include "ardroneIhm.h"
#include "ardroneVideo.h"
#include "dataCentral.h"
#include "BlackBox.h"

using namespace std;

///////////////////// navdata
static int32_t killed = 0;
void signal_exit(int sig);

/* Initialization local variables before event loop  */
inline C_RESULT navdataUpdateInit( void* data )
{
  printf("navdata init\n");
  return C_OK;
}

/* Receving navdata during the event loop */
inline C_RESULT navdataUpdateProcess( const navdata_unpacked_t* const navdata )
{
  DataCentral::GetInstance()->UpdateNavData((navdata_unpacked_t *)navdata);

  return C_OK;
}

/* Relinquish the local resources after the event loop exit */
inline C_RESULT navdataUpdateRelease( void )
{
  printf("navdata release\n");
  return C_OK;
}

BEGIN_NAVDATA_HANDLER_TABLE
  NAVDATA_HANDLER_TABLE_ENTRY(navdataUpdateInit, navdataUpdateProcess, navdataUpdateRelease, NULL)
END_NAVDATA_HANDLER_TABLE



/* Implementing Custom methods for the main function of an ARDrone application */

/* The delegate object calls this method during initialization of an ARDrone application */
C_RESULT ardrone_tool_init_custom(int argc, char **argv)
{
  int i;
  /* Registering for a new device of game controller */
  //ardrone_tool_input_add( &gamepad ); //FIX noth
  BlackBox::GetInstance()->SetOutput("./", "flightData");

  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      if (!strncmp(argv[i], "-videopath=", 11))
      {
        ArdroneVideo::GetInstance()->SetSavePath(string(argv[i]+11));
        BlackBox::GetInstance()->SetOutput(string(argv[i]+11), "capture");
      }
      else if (!strncmp(argv[i], "-encodefile=", 12))
      {
        if (ArdroneVideo::GetInstance()->EncodeFramesFile(string(argv[i]+12)))
        {
          unlink(argv[i]+12);
        }
        exit(0);
      }
      else if (!strcmp(argv[i], "-noencode"))
      {
        ArdroneVideo::GetInstance()->DisableEncoding();
      }
      else
      {      
        cout << "Usage " << argv[0] << endl;
        cout << "\t-videopath= : path for video capture" << endl;
        cout << "\t-noencode : encode video later" << endl; 
        cout << "\t-encodefile= : path for encoding .dup file to mp4" << endl;
        exit(0);
        
      }    
    }
  }
  
  ArdroneVideo::GetInstance()->StartRecording();
  DataCentral::GetInstance();
  BlackBox::GetInstance()->StartLogging(200); //log at 200ms

  START_THREAD( UpdateVideo, NULL );

  START_THREAD( UpdateIhm, NULL );

  signal(SIGINT, signal_exit);
  printf("Tool init\n");
  return C_OK;
}

/* The delegate object calls this method when the event loop exit */
C_RESULT ardrone_tool_shutdown_custom()
{
  killed = 1;
  printf("Tool stop\n");
  /* Relinquish all threads of your application */
  BlackBox::GetInstance()->StopLogging();
cout << "Stopping ihm thread" << endl;
  JOIN_THREAD( UpdateIhm );
  
cout << "Stopping video thread" << endl;
  JOIN_THREAD( UpdateVideo );
  
cout << "Deleting ihm" << endl;
  delete(ArdroneIhm::GetInstance());
cout << "Deleting video" << endl;
  delete(ArdroneVideo::GetInstance());

  return C_OK;
}

/* The event loop calls this method for the exit condition */
bool_t ardrone_tool_exit()
{
//  printf("Tool exit\n");
  return killed;
}

void signal_exit(int sig)
{
  printf("Kill signal\n");
  killed = 1;
}

DEFINE_THREAD_ROUTINE(UpdateIhm, data)
{
  while (!killed)
  {
    ArdroneIhm::GetInstance()->Update();
    usleep(10000);
  }
  return (THREAD_RET)0;
}

DEFINE_THREAD_ROUTINE(UpdateVideo, data)
{
  
  while (!killed)
  {
    ArdroneVideo::GetInstance()->Update(data);
    usleep(100);
  }
  return (THREAD_RET)0;
}


/* Implementing thread table in which you add routines of your application and those provided by the SDK */
BEGIN_THREAD_TABLE
  THREAD_TABLE_ENTRY( ardrone_control, 20 )
  THREAD_TABLE_ENTRY( navdata_update, 20 )
  THREAD_TABLE_ENTRY( UpdateVideo, 20 )
  THREAD_TABLE_ENTRY( UpdateIhm, 20 )
END_THREAD_TABLE

