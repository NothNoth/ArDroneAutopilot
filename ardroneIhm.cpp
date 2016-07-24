#include <signal.h>
#include "ardroneIhm.h"
#include "dataCentral.h"
#include "ardroneVideo.h"
#include "ControlLayer.h"

extern "C"
{
  #include <ardrone_api.h>
  #include <ardrone_tool/UI/ardrone_tool_ui.h>
  #include <SDL/SDL_rotozoom.h>
}


using namespace std;

ArdroneIhm * ArdroneIhm::_instance = 0;

ArdroneIhm::ArdroneIhm() :
  _sdlFlightRollPitch(SDLFlightRollPitch(100)),
  _sdlFlightYawAlt(SDLFlightYawAlt(100)) ,
  _sdlFlightFuel(SDLFlightFuel(100)) ,
  _w(640),
  _h(480),
  _fullscreen(false),
  _navdataDemoMode(true)
{
  SDL_Color color;
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0)
    abort();
  _screen = SDL_SetVideoMode(_w, _h, 32,SDL_DOUBLEBUF);
  TTF_Init();
  _font = TTF_OpenFont("./resources/FreeMono.ttf", 12);
  cout << "IHM > Init" << endl;
  SDL_FillRect(_screen, NULL, 0xFFFFFFFF);

  color.r = 0x00;
  color.g = 0x00;
  color.b = 0xFF;
  SDL_Surface * s = SDLCALL TTF_RenderText_Solid(_font, "AutoPilot loaded..", color);
  if (!s)
    cout << "Can't setup fonts.." << endl;
  SDL_BlitSurface(s, NULL, _screen, NULL);
  SDL_FreeSurface(s);
  SDL_Flip(_screen);
  
  _sdlFlightRollPitch.SetMinMaxPitch(-86000, 86000);
  _sdlFlightRollPitch.SetMinMaxRoll(90000, -90000);

  _sdlFlightRollPitch.Debug(false);
  _sdlFlightYawAlt.Debug(false);
  _sdlFlightFuel.Debug(false);
  
  SDL_EnableKeyRepeat(100, 50);
}


ArdroneIhm::~ArdroneIhm()
{
  SDL_Quit();
  TTF_Quit();
  _screen = NULL;
  cout << "IHM > Closed" << endl;
}

ArdroneIhm * ArdroneIhm::GetInstance()
{
  if (!_instance)
    _instance = new ArdroneIhm();

  return _instance;
}

extern void ardrone_at_set_progress_cmd( int32_t enable, float32_t phi, float32_t theta, float32_t gaz, float32_t yaw );

void ArdroneIhm::ProcessKeyboardEvent(SDL_keysym ks)
{
  bool cmdRes;
  SDLKey k = ks.sym;
  SDLMod mod = ks.mod;
  switch(k)
  {
    case SDLK_SPACE:
    {
      int ret;
      if ( DataCentral::GetInstance()->GetAltitude() > 1)
      {
        cmdRes = ControlLayer::GetInstance()->Land();
        cout << "SetUIPadStart :  land " << endl;
      }
      else
      {
        cmdRes = ControlLayer::GetInstance()->TakeOff();
        cout << "SetUIPadStart :  takeoff " << endl;
      }
    }
    break;
    case SDLK_RETURN:
    {
      _fullscreen = !_fullscreen;
      SDL_SetVideoMode(_w, _h, 32, SDL_DOUBLEBUF | (_fullscreen?SDL_FULLSCREEN:0));
    }
    break;
    case SDLK_TAB:
    {
      if (_navdataDemoMode)
      {
        cmdRes = ControlLayer::GetInstance()->SetNavdataDemo(false);
      }
      else
      {
        cmdRes = ControlLayer::GetInstance()->SetNavdataDemo(true);
      }
      _navdataDemoMode = !_navdataDemoMode;
    }
    break;
    case SDLK_F1:
    {
      cmdRes = ControlLayer::GetInstance()->SetCamera(CAMERA_HORI);
    }
    break;
    case SDLK_F2:
    {
      cmdRes = ControlLayer::GetInstance()->SetCamera(CAMERA_VERT);
    }
    break;
    case SDLK_F3:
    {
      cmdRes = ControlLayer::GetInstance()->SetCamera(CAMERA_LARGE_HORI_SMALL_VERT);
    }
    break;
    case SDLK_F4:
    {
      cmdRes = ControlLayer::GetInstance()->SetCamera(CAMERA_LARGE_VERT_SMALL_HORI);
    }
    break;
    case SDLK_F5:
    {
      cmdRes = ControlLayer::GetInstance()->SetCamera(CAMERA_NEXT);
    }
    break;
    case SDLK_a:
    {
      cout << "Starting raw capture" << endl;
      cmdRes = ControlLayer::GetInstance()->StartRawCapture();
    }
    break;
    case SDLK_z:
    {
      cout << "Flat trim set" << endl;
      cmdRes = ControlLayer::GetInstance()->SetFlatTrim();
    }
    break;
    case SDLK_e:
    {
      cmdRes = ControlLayer::GetInstance()->SetEmergency();
      cout << "Emergency sent !!" << endl;
    }
    break;
    case SDLK_r:
    {
      cmdRes = ControlLayer::GetInstance()->CancelEmergency();
      cout << "Emergency cancelled !!" << endl;
    }
    break;
    case SDLK_q:
    {
      cout << "up" << DataCentral::GetInstance()->GetAltitude() << endl;
      if (DataCentral::GetInstance()->GetAltitude() > 6000)
      {
        cmdRes = ControlLayer::GetInstance()->SetAngles(0, 0.0, 0.0, 0.2, 0.0);
        cout << "Altitude restriction enabled!!" << endl;
      }
      else
        cmdRes = ControlLayer::GetInstance()->SetAngles(0, 0.0, 0.0, 0.8, 0.0);
    }
    break;
    case SDLK_w:
    {
      cout << "down" << endl;
      cmdRes = ControlLayer::GetInstance()->SetAngles(0, 0.0, 0.0, -0.8, 0.0);
    }
    break;
    case SDLK_UP:
    {
      cout << "forward" << endl;
      cmdRes = ControlLayer::GetInstance()->SetAngles(1, 0.0, -0.1, 0.0, 0.0);
    }
    break;

    case SDLK_DOWN:
    {
      cout << "back" << endl;
      cmdRes = ControlLayer::GetInstance()->SetAngles(1, 0.0, 0.1, 0.0, 0.0);
    }
    break;

    case SDLK_LEFT:
    {
      if (mod == KMOD_LALT)
      {
        cout << "strafe left" << endl;
        cmdRes = ControlLayer::GetInstance()->SetAngles(1, -0.2, 0.0, 0.0, 0.0);
      }
      else
      {
        cout << "rotate left" << endl;
        cmdRes = ControlLayer::GetInstance()->SetAngles(0, 0.0, 0.0, 0.0, -0.2);
      }
    }
    break;

    case SDLK_RIGHT:
    {
      if (mod == KMOD_LALT)
      {
        cout << "strafe right" << endl;
        cmdRes = ControlLayer::GetInstance()->SetAngles(1, 0.2, 0.0, 0.0, 0.0);
      }
      else
      {
        cout << "rotate right" << endl;
        cmdRes = ControlLayer::GetInstance()->SetAngles(0, 0.0, 0.0, 0.0, 0.2);  
      }
    }
    break;

    default:
    {
    }
  }
}

void ArdroneIhm::Update()
{
  char tmp[255];
  SDL_Color color;
  SDL_Surface *  videoFrame;
  SDL_Rect r;
  
 // Process incoming events
  SDL_Event event;
  while ( SDL_PollEvent(&event) )
  {
    switch (event.type)
    {
      case SDL_MOUSEMOTION:
        //printf("Mouse moved by %d,%d to (%d,%d)\n", event.motion.xrel, event.motion.yrel, event.motion.x, event.motion.y);
      break;
      case SDL_MOUSEBUTTONDOWN:
        //printf("Mouse button %d pressed at (%d,%d)\n", event.button.button, event.button.x, event.button.y);
      break;
      case SDL_QUIT:
          kill(0, SIGINT);
      break;
      case SDL_KEYDOWN:
      {
        ProcessKeyboardEvent(event.key.keysym);
      }
      break;
    }
  }

  //Update ihm
  sprintf(tmp, "In Air : %s [SPACE] - Navdata : %s [TAB]", 
          DataCentral::GetInstance()->IsFlying()?"yes":"no",
          _navdataDemoMode?"Demo":"Full");

  SDL_FillRect(_screen, NULL, 0x00000000);

  //set video
  videoFrame = ArdroneVideo::GetInstance()->GetVideoFrame();
  if (videoFrame && 0)
  {
    SDL_Surface * srf = zoomSurface(videoFrame, (int)(_w/videoFrame->w), (int)(_h/videoFrame->h), 0);
    r.x = 0;
    r.y = 0;
    r.w = -1;
    r.h = -1;
    SDL_BlitSurface(srf, NULL, _screen, &r);

    SDL_FreeSurface(videoFrame);
    SDL_FreeSurface(srf);
  }
  else
  {
    SDL_BlitSurface(videoFrame, NULL, _screen, NULL); 
  }
 

  //render GUI
  color.r = 0x00;
  color.g = 0x00;
  color.b = 0xFF;
  SDL_Surface * s = SDLCALL TTF_RenderText_Solid(_font, tmp, color);
  SDL_BlitSurface(s, NULL, _screen, NULL);
  r.x = _w - _sdlFlightRollPitch.GetWidth();
  r.y = 5;
  r.w = -1;
  r.h = -1;
  _sdlFlightRollPitch.SetRoll(DataCentral::GetInstance()->GetRoll()/1000.0);
  _sdlFlightRollPitch.SetPitch(DataCentral::GetInstance()->GetPitch());
  SDL_BlitSurface(_sdlFlightRollPitch.GetSurface(), NULL, _screen, &r);

  r.x = _w - _sdlFlightYawAlt.GetWidth();
  r.y = 5 + _sdlFlightRollPitch.GetHeight() + 5;
  r.w = -1;
  r.h = -1;
  _sdlFlightYawAlt.SetAlt(DataCentral::GetInstance()->GetAltitude()/10.0);
  _sdlFlightYawAlt.SetYaw(-DataCentral::GetInstance()->GetYaw()/1000.0); // -18000 -- 18000
  SDL_BlitSurface(_sdlFlightYawAlt.GetSurface(), NULL, _screen, &r);

  r.x =  _w - _sdlFlightRollPitch.GetWidth();
  r.y =  5 + _sdlFlightRollPitch.GetHeight() + 5 + _sdlFlightYawAlt.GetHeight() + 5;
  r.w = -1;
  r.h = -1;
  _sdlFlightFuel.SetFuel(DataCentral::GetInstance()->GetBatteryLevel());
  SDL_BlitSurface(_sdlFlightFuel.GetSurface(), NULL, _screen, &r);


  SDL_Flip(_screen);
}
