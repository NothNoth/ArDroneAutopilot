#ifndef ARDRONE_IHM_H
#define ARDRONE_IHM_H

#include <iostream>
extern "C"
{
  #include <SDL/SDL.h>
  #include <SDL/SDL_ttf.h>
}
#include "SDLFlightRollPitch.h"
#include "SDLFlightYawAlt.h"
#include "SDLFlightFuel.h"

class ArdroneIhm
{
  public:
    static ArdroneIhm * GetInstance();
    virtual ~ArdroneIhm();
    void Update();

  private:
      static ArdroneIhm * _instance;
      ArdroneIhm();
      void ProcessKeyboardEvent(SDL_keysym ks);

      SDL_Surface * _screen;
      TTF_Font * _font;
      SDLFlightRollPitch _sdlFlightRollPitch;
      SDLFlightYawAlt   _sdlFlightYawAlt; 
      SDLFlightFuel     _sdlFlightFuel; 

      int     _w;
      int     _h;
      bool    _fullscreen;
      bool    _navdataDemoMode;
};

#endif
