#ifndef _Convolve_h_
#define _Convolve_h_

#include "mytype.h"
#include "Image.h"
//#include "Complex.h"

template<class Item1, class Item2>
Image<Item1>
ImageConvolution(const Image<Item1>& image, const Image<Item2>& filter);


template<class Item1, class Item2>
Image<Item1>
SeparableImageConvolution(const Image<Item1>& image, 
                          const Image<Item2>& verfil, 
                          const Image<Item2>& horfil);

#include "Convolve.cpp"

#endif /* Convolve_h */
