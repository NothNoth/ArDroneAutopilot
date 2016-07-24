#include "RawEncoder.h"

#include <cassert>
#include <cstring>
#include <string>
using std::string;


Revel_RawEncoder::Revel_RawEncoder(void)
{
    // Must initialize pointer members before calling Reset!
    m_RawEncoderHandle = NULL;
    strcpy(m_compressor, "raw ");
    Reset();
}


Revel_RawEncoder::~Revel_RawEncoder(void)
{
    
}


void Revel_RawEncoder::Reset(void)
{
    // Must call base Reset() first!
    Revel_BaseEncoder::Reset();

    if (m_RawEncoderHandle != NULL)
    {
      m_RawEncoderHandle = NULL;
    }
}


Revel_Error Revel_RawEncoder::EncodeStart(const string& filename, const Revel_Params& params)
{
    Revel_Error revError = Revel_BaseEncoder::EncodeStart(filename, params);
    if (revError != REVEL_ERR_NONE)
        return revError;
        

    return REVEL_ERR_NONE;
}


Revel_Error Revel_RawEncoder::EncodeFrame(const Revel_VideoFrame& frame, int *frameSize)
{
  Revel_Error revError = Revel_BaseEncoder::EncodeFrame(frame, frameSize);
  if (revError != REVEL_ERR_NONE)
      return revError;

  AVI_write_frame(m_outFile, (char *)frame.pixels,  frame.width * frame.height * frame.bytesPerPixel);
	return REVEL_ERR_NONE;
}


Revel_Error Revel_RawEncoder::EncodeEnd(int *totalSize)
{
    Revel_Error revError = Revel_BaseEncoder::EncodeEnd(totalSize);
    if (revError != REVEL_ERR_NONE)
        return revError;

    return REVEL_ERR_NONE;
}
