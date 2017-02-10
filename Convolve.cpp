#ifndef _Convolve_cpp_
#define _Convolve_cpp_

#include "Convolve.h"

#include <assert.h>

template<class Item1, class Item2>
Image<Item1>
ImageConvolution(const Image<Item1>& image, const Image<Item2>& filter) {

  assert(image.NumBands()==filter.NumBands()|| filter.NumBands()==1);
  Image<Item1> result(image.NumBands(),image.NumRows(), image.NumCols());

  Item1 val, sum;
  int k2;
  // (offy,offx): the center point of the filter
  int offy = (filter.NumRows()-1)/2;
  int offx = (filter.NumCols()-1)/2;
  for(int k=0; k<image.NumBands(); ++k) {
    if(filter.NumBands()==1)
      k2=0;
    else
      k2=k;
    for(int i=0; i<image.NumRows(); ++i) {
      for(int j=0; j<image.NumCols(); ++j) {
        sum=0;
        //s=image.GetPixel(k,i,j);
        for(int i2=0; i2<filter.NumRows(); ++i2) {
          for(int j2=0; j2<filter.NumCols(); ++j2) {
            val = image.GetPixelZero(k,i-i2+offy,j-j2+offx);
            sum += val*filter.GetPixel(k2,i2,j2);
          }
        }
        result.SetPixel(k,i,j,sum);
      }
    }
  }

  return result;
}

/*
This routine performs separble 2D convolution.
The Image objects, verfil and horfil, are the filters.
They have to be Image object with their Rows=1.
Their Bands have to be either 1 or the same with the number of
bands in the input image, (image).
*/
template<class Item1, class Item2>
Image<Item1>
SeparableImageConvolution(const Image<Item1>& image, 
                          const Image<Item2>& verfil, 
                          const Image<Item2>& horfil) {

  assert(image.NumBands()==verfil.NumBands()|| verfil.NumBands()==1);
  assert(verfil.NumBands()==horfil.NumBands());
  assert(verfil.NumRows()==1 && horfil.NumRows()==1);

  Image<Item1> result(image.NumBands(),image.NumRows(), image.NumCols());
  Image<Item1> tmp_im(image.NumBands(),image.NumRows(), image.NumCols());

  Item1 val, sum, s;
  int i, j, k;
  int i2, k2, k3;

  //
  // 1st phase:: Vertical filtering
  //
  int offy = (verfil.NumCols()-1)/2;
  for(k=0; k<image.NumBands(); ++k) {
    if(verfil.NumBands()==1)
      k2=0;
    else
      k2=k;
    for(i=0; i<image.NumRows(); ++i) {
      for(j=0; j<image.NumCols(); ++j) {
        sum = 0;
        for(i2=0; i2<verfil.NumCols(); ++i2) {
          k3=i-i2+offy;
          //val = image.GetPixelDefault(k,i-i2+offy,j,s);
          //if(k3>=0 && k3<image.NumRows())
            //val = image.GetPixel(k,k3,j);
          val = image.GetPixelRepeat(k,k3,j);
          //val = image.GetPixelZero(k,k3,j);
          sum += val*verfil.GetPixel(k2,0,i2);
        }
        tmp_im.SetPixel(k,i,j,sum);
      }
    }
  }

  //
  // 2nd phase:: Horizontal filtering
  //
  int offx = (horfil.NumCols()-1)/2;
  for(k=0; k<image.NumBands(); ++k) {
    if(horfil.NumBands()==1)
      k2=0;
    else
      k2=k;
    for(i=0; i<image.NumRows(); ++i) {
      for(j=0; j<image.NumCols(); ++j) {
        sum=0;
        //s=tmp_im.GetPixel(k,i,j);
        s=0;
        for(i2=0; i2<horfil.NumCols(); ++i2) {
          //val = tmp_im.GetPixelDefault(k,i,j-i2+offx,s);
          val = tmp_im.GetPixelRepeat(k,i,j-i2+offx);
          //val = tmp_im.GetPixelZero(k,i,j-i2+offx);
          sum += val*horfil.GetPixel(k2,0,i2);
        }
        result.SetPixel(k,i,j,sum);
      }
    }
  }

  return result;
}

#endif /* Convolve_cpp */
