#include <iostream>
#include "SDLFlightFuel.h"
extern "C"
{
  #include "SDL/SDL_gfxPrimitives.h"
  #include <math.h>
  #include <SDL/SDL_rotozoom.h>
}

using namespace std;
#define MARGIN 5

#define RMASK 0x00FF0000
#define GMASK 0x0000FF00
#define BMASK 0x000000FF
#define AMASK 0x00000000



SDLFlightFuel::SDLFlightFuel(int dimension) :
  _w(dimension),
  _h(dimension),
  _perc(100),
  _debug(false)
{
  //Rendering surfaces
  _back = SDL_CreateRGBSurface
			(SDL_HWSURFACE, _w, _h, 24, 
			RMASK,GMASK, BMASK, AMASK);
  _current = SDL_CreateRGBSurface
			(SDL_HWSURFACE, _w, _h, 24, 
			RMASK,GMASK, BMASK, AMASK);

  TTF_Init();
  _font = TTF_OpenFont("./resources/FreeMono.ttf", 14);
  SDL_FillRect(_back, NULL, 0x00666666);


  //Prepare back layer
  SDL_FillRect(_back, NULL, 0x00222222);
  filledCircleRGBA(_back, (int)(_w/2), (int) (_h/2), (int) (_w/2) - MARGIN, 0x00, 0xFF, 0x00, 0xFF); 
  SDL_SetColorKey(_back, SDL_SRCCOLORKEY, 0x0000FF00);

  circleRGBA(_back, (int)(_w/2), (int) (_h/2), (int) (_w/2) - MARGIN, 0xAA, 0xAA, 0xAA, 0xFF); 
  filledCircleRGBA(_back, 8, 8, 5, 0x88, 0x88, 0x88, 0xFF); 
  filledCircleRGBA(_back, _w-8, 8, 5, 0x88, 0x88, 0x88, 0xFF); 
  filledCircleRGBA(_back, 8, _h-8, 5, 0x88, 0x88, 0x88, 0xFF); 
  filledCircleRGBA(_back, _w-8, _h-8, 5, 0x88, 0x88, 0x88, 0xFF); 

  filledPieRGBA(_back, _w/2.0, _h/2.0+20, (_w/2.0)-MARGIN*2,//orange
			     -160, -20, 
            0xFF, 0x66, 0x44, 0xFF);
  filledPieRGBA(_back, _w/2.0, _h/2.0+20, (_w/2.0)-MARGIN*2,
			     -160, -140, 
            0xDD, 0x22, 0x22, 0xFF); //red zone

  filledPieRGBA(_back, _w/2.0, _h/2.0+22, (_w/2.0)-15,
			           -160-3, -20+3, 
                  0x22, 0x22, 0x22, 0xFF);//mask
 // filledCircleRGBA(_back, (int)(_w/2), (int) (_h/2.0)+22, 4, 0xFF, 0x66, 0x44, 0xFF); //axis 


}


SDLFlightFuel::~SDLFlightFuel()
{
  SDL_FreeSurface(_back);
  SDL_FreeSurface(_current);
}

SDL_Surface * SDLFlightFuel::GetSurface()
{
  SDL_Rect r;
  int cx, cy;
  int minA, maxA;
  char fuel[32];
  //Draw background
  SDL_FillRect(_current, NULL, 0x00000000);
  SDL_BlitSurface(_back, NULL, _current, NULL); 

  cx =  (int)(_w/2);
  cy = (int) (_h/2.0)+22;
  minA = -160-3;
  maxA = -20+3;
  
  int curA = (int) (_perc * (maxA-minA) / 100.0);
  filledPieRGBA(_current, _w/2.0, _h/2.0+22, (_w/2.0)-15,
			           minA+curA-1, minA+curA+1, 
                  0xFF, 0x22, 0x22, 0xFF);//mask
  

  SDL_Color color;
  sprintf(fuel, "%03d%%", _perc);
  color.r = 0xCC;
  color.g = 0xCC;
  color.b = 0xCC;
  SDL_Surface * s = SDLCALL TTF_RenderText_Solid(_font, fuel, color);
  if (s)
  {
    r.x = _w/2.0 - s->w/2.0;
    r.y = _h - 30;;
    r.w = -1;
    r.h = -1;
    SDL_BlitSurface(s, NULL, _current, &r); 
    SDL_FreeSurface(s);
  }
  
  if (_debug)
  {
    char dbg[64];
    sprintf(dbg, "Bat. %d%%", _perc);
    color.r = 0x00;
    color.g = 0xFF;
    color.b = 0x00;
    SDL_Surface * s = SDLCALL TTF_RenderText_Solid(_font, dbg, color);
    if (s)
    {
      r.x = 0;
      r.y = 0;
      r.w = -1;
      r.h = -1;
      SDL_BlitSurface(s, NULL, _current, &r); 
      SDL_FreeSurface(s);
    }

  }

  return _current;
}
