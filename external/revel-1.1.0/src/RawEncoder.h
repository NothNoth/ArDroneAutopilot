

#ifndef RAWENCODER_H
#define RAWENCODER_H

#include "BaseEncoder.h"

#include <cstdio>
#include <string>

class Revel_RawEncoder : public Revel_BaseEncoder
{
public:
    Revel_RawEncoder();
    ~Revel_RawEncoder();

    // Inherited from Revel_BaseEncoder
    virtual Revel_Error EncodeStart(const std::string& filename,
        const Revel_Params& params);
    virtual Revel_Error EncodeFrame(const Revel_VideoFrame& frame,
        int *frameSize = NULL);
    virtual Revel_Error EncodeEnd(int *totalSize = NULL);


protected:
    // Inherited from Revel_BaseEncoder
    virtual void Reset(void);

private:
    // operator= and copy constructor are disallowed for this class.
    Revel_RawEncoder& operator=(const Revel_RawEncoder& rhs) { return *this; }
    Revel_RawEncoder(const Revel_RawEncoder& rhs) {}


    // Handle to the Raw encoder.
    void *m_RawEncoderHandle;
};

#endif // RAWENCODER_H
