#pragma once
#include "Image.h"

namespace ColorSpace {
    struct YCbCr {
        float Y;
        float Cb;
        float Cr;
    };
    struct Lab {
        float L;
        float a;
        float b;
    };

    YCbCr toYCbCr(const Pixel& pixel);
    Pixel fromYCbCr(const YCbCr& ycbcr);
    Lab toLab(const Pixel& pixel);
    Pixel fromLab(const Lab& lab);


}
