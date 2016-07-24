extern "C"
{
  #include <SDL/SDL_video.h>
  #include <SDL/SDL_ttf.h>
}


class SDLFlightFuel
{
  public:
    SDLFlightFuel(int dimension);
    ~SDLFlightFuel();
    SDL_Surface * GetSurface();
    int GetWidth() { return _w; }
    int GetHeight() { return _h; }
    void SetFuel(int perc) {_perc = perc;}
    void Debug(bool flag) {_debug = flag;}
  private:
    int _w;
    int _h;
    SDL_Surface * _back;
    SDL_Surface * _current;
    TTF_Font * _font;
    int  _perc;
    bool _debug;
};
