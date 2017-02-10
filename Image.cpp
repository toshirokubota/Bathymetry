#ifndef _Image_cpp_
#define _Image_cpp_

#include "Image.h"
#include "mytype.h"
#include <stdio.h>

template <class Item>
void Image<Item>:: ResizeImage(int bands, int rows, int cols) {
  if(pItem)
    _resetItemPointers();

  _setItemPointers(bands, rows, cols);
  Bands = bands;
  Rows = rows;
  Cols = cols;
}

template <class Item>
void Image<Item>:: _setItemPointers(int bands, int rows, int cols) {
  pItem = new Item[bands*rows*cols];
  assert(pItem);

  pppItem = new ptr2D_Data[bands];
  assert(pppItem);
  int i;
  for(i=0; i<bands; ++i) {
    pppItem[i] = new ptr1D_Data [rows];
    assert(pppItem[i]);
  }

  Item* ptr = &(pItem[0]);
  for(i=0; i<bands; ++i) {
    for(int j=0; j<rows; ++j) {
      pppItem[i][j] = ptr;
      ptr += cols;
    }
  }
#ifdef DEBUG_Image
  cerr << "_setItemPointers: " << pItem << "," << pppItem << endl;
#endif
}

template <class Item>
void Image<Item>:: _resetItemPointers() {
#ifdef DEBUG_Image
  cerr << "_resetItemPointers: " << pItem << "," << pppItem << endl;
#endif
  for(int i=0; i<Bands; ++i)
    delete [] pppItem[i];
  delete [] pppItem;

  delete [] pItem;
}

//
// Default Constructor
//

template <class Item>
Image<Item>:: Image()
  : pItem(0)
{
#ifdef DEBUG_Image
  cout << "constructing an image..." << endl;
#endif
  ResizeImage(1, 1, 1);
}

//
// this is kind of redundant.  I wanted to make this as a
// default constructor, but I get weird compiling errors
// regarding >> and << operators...
//
template <class Item>
Image<Item>:: Image(int bands, int rows, int cols)
  : pItem(0)
{
#ifdef DEBUG_Image
  cout << "constructing an image..." << endl;
#endif
  ResizeImage(bands, rows, cols);
}

//
// Constructor
//
template <class Item>
Image<Item>:: Image(int bands, int rows, int cols, 
                      const Item& pixel)
  : pItem(0)
{
#ifdef DEBUG_Image
  cout << "constructing an image (" << rows << "x" << cols;
  cout << ") with initial pixel value=" << pixel << endl;
#endif
  ResizeImage(bands, rows, cols);

  for(int i=0; i < Bands*Rows*Cols; ++i){
    pItem[i] = pixel;
  }

}


template <class Item>
Image<Item>:: Image(const Image<Item>& image)
  : pItem(0)
{
  ResizeImage(image.NumBands(), image.NumRows(), image.NumCols());

  for(int i=0; i < Bands*Rows*Cols; ++i){
    pItem[i] = image.GetPixel(i);
  }
}

//
// A Desctructor:
//

template <class Item>
Image<Item>:: ~Image()
{
#ifdef DEBUG_Image
  cout << "destroying an image..." << endl;
#endif
  _resetItemPointers();
}

//
// Assignment:
//

template <class Item>
const Image<Item>& Image<Item>:: operator =(const Image<Item>& image)
{
  if(&image != this) {
    if(Cols != image.NumCols() || Rows != image.NumRows() ||
       Bands != image.NumBands()) {
      ResizeImage(image.NumBands(), image.NumRows(), image.NumCols());
    }
    
    for(int i=0; i < Bands*Cols*Rows; ++i)
      pItem[i] = image.GetPixel(i);
  }
  return *this;
}

//
// Getting a pixel value and Setting a pixel value
//

/*
this routine implement GetPixel function with boundary extention.
Pixels outside boundaries are computed using pixel folding
*/
template <class Item>
Item
Image<Item>:: GetPixelFold(int band_index, int row_index, int col_index) const
{
  assert(Rows > 1 && Cols > 1); // the routie can fail in this condition...

  row_index = (row_index<0) ? -row_index: \
    ((row_index>=Rows)? 2*Rows-row_index-2 : row_index);
  col_index = (col_index<0) ? -col_index: \
    ((col_index>=Cols)? 2*Cols-col_index-2 : col_index);

  return(pppItem[band_index][row_index][col_index]);
}

/*
this routine implement GetPixel function with boundary extention.
Pixels outside boundaries are computed using image wrapping
*/
template <class Item>
Item
Image<Item>:: GetPixelWrap(int band_index, int row_index, int col_index) const
{
  while(row_index < 0) {
    row_index += Rows;
  }
  while(row_index >= Rows){
    row_index -= Rows;
  }
  while(col_index < 0) {
    col_index += Cols;
  }
  while(col_index >= Cols){
    col_index -= Cols;
  }

  return(pppItem[band_index][row_index][col_index]);
}

/*
this routine implement GetPixel function with boundary extention.
Pixels outside boundaries are computed using pixel repeatition.
*/
template <class Item>
Item
Image<Item>::GetPixelRepeat(int band_index, int row_index, int col_index)const
{
  row_index = (row_index<0) ? 0: \
    ((row_index>=Rows)? Rows-1: row_index);
  col_index = (col_index<0) ? 0: \
    ((col_index>=Cols)? Cols-1: col_index);

  return(pppItem[band_index][row_index][col_index]);
}

/*
this routine implement GetPixel function with boundary extention.
Pixels outside boundaries are set to Zero.

Note: this may not work on some classes of Item such as Complex.
*/
template <class Item>
Item
Image<Item>::GetPixelZero(int band_index, int row_index, int col_index)const
{
  if(band_index>=0 && band_index < Bands &&\
     row_index>=0 && row_index<Rows && \
     col_index>=0 && col_index<Cols)
    return(pppItem[band_index][row_index][col_index]);
  else {
    return 0;
  }
}

/*
this routine implement GetPixel function with boundary extention.
Pixels outside boundaries are set so that the value is linearly
extrapolated.

Note: this may not work on some classes of Item such as Complex.
*/
template <class Item>
Item
Image<Item>::GetPixelExtrap(int band_index, int row_index, int col_index)const
{
	assert(band_index>=0 && band_index<Bands);
	if(row_index>=0 && row_index<Rows && col_index>=0 && col_index<Cols)
    return(pppItem[band_index][row_index][col_index]);
  else {
  	int erow,brow,ecol,bcol;
  	if(row_index<0) {
  		erow=-row_index;
  		brow=0;
  	}
  	else if(row_index>=Rows) {
  		erow=2*Rows-row_index-2;
  		brow=row_index-1;
  	}
  	else {
  		erow=row_index;
  		brow=row_index;
  	}
  	if(col_index<0) {
  		ecol=-col_index;
  		bcol=0;
  	}
  	else if(col_index>=Cols) {
  		ecol=2*Cols-col_index-2;
  		bcol=col_index-1;
  	}
  	else {
  		ecol=col_index;
  		bcol=col_index;
  	}
  	real eval=pppItem[band_index][erow][ecol];
  	real bval=pppItem[band_index][brow][bcol];
    return bval+(bval-eval);
  }
}

/*
this routine implement GetPixel function with boundary extention.
Pixels outside boundaries are set to the given default value.

*/
template <class Item>
Item
Image<Item>::GetPixelDefault(int band_index, int row_index, 
                             int col_index, const Item& item)const
{
  if(band_index>=0 && band_index < Bands &&\
     row_index>=0 && row_index<Rows && \
     col_index>=0 && col_index<Cols)
    return(pppItem[band_index][row_index][col_index]);
  else {
    return item;
  }
}

//
// Arithmetic operators for Images.
//
template <class Item>
Image<Item>
Image<Item>:: operator+(const Image<Item>& image) const {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  Image<Item> result;
  if(Bands==image.NumBands()) {
    result = image;
    for(int m=0; m<Bands*Rows*Cols; ++m) {
      result.SetPixel(m, pItem[m]+image.GetPixel(m));
    }
  }
  else if(Bands==1) {
    result = image;
    for(int k=0, m=0; k<image.NumBands(); ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, pppItem[0][i][j]+image.GetPixel(m));
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    result = *this;
    for(int k=0, m=0; k<Bands; ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, GetPixel(m)+image.GetPixel(0,i,j));
        }
      }
    }
  }
  return result;
}

template <class Item>
void
Image<Item>:: operator+=(const Image<Item>& image) {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  if(Bands == image.NumBands()) {
    for(int m=0; m<Bands*Rows*Cols; ++m)
      pItem[m] += image.GetPixel(m);
  }
  else if(Bands == 1) {
    Item* oldPixel = new Item[Rows*Cols];
    for(int i=0; i<Rows*Cols; ++i)
      oldPixel[i] = pItem[i];

    ResizeImage(image.NumBands(), Rows, Cols);

    for(int k=0, m=0; k<image.NumBands(); ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j,++n, ++m) {
          pItem[m] = oldPixel[n]+image.GetPixel(m);
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    int m=0;
    for(int k=0; k<Bands; ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m, ++n) {
          pItem[m] += image.GetPixel(n);
        }
      }
    }
  }
}

//
// Arithmetic operators for Images.
//
template <class Item>
Image<Item>
Image<Item>:: operator-(const Image<Item>& image) const {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  Image<Item> result;
  if(Bands==image.NumBands()) {
    result = image;
    for(int m=0; m<Bands*Rows*Cols; ++m) {
      result.SetPixel(m, pItem[m]-image.GetPixel(m));
    }
  }
  else if(Bands==1) {
    result = image;
    for(int k=0, m=0; k<image.NumBands(); ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, pppItem[0][i][j]-image.GetPixel(m));
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    result = *this;
    for(int k=0, m=0; k<Bands; ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, GetPixel(m)-image.GetPixel(0,i,j));
        }
      }
    }
  }
  return result;
}

template <class Item>
void
Image<Item>:: operator-=(const Image<Item>& image) {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  if(Bands == image.NumBands()) {
    for(int m=0; m<Bands*Rows*Cols; ++m)
      pItem[m] -= image.GetPixel(m);
  }
  else if(Bands == 1) {
    Item* oldPixel = new Item[Rows*Cols];
    for(int i=0; i<Rows*Cols; ++i)
      oldPixel[i] = pItem[i];

    ResizeImage(image.NumBands(), Rows, Cols);

    for(int k=0, m=0; k<image.NumBands(); ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j,++n, ++m) {
          pItem[m] = oldPixel[n]-image.GetPixel(m);
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    int m=0;
    for(int k=0; k<Bands; ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m, ++n) {
          pItem[m] -= image.GetPixel(n);
        }
      }
    }
  }
}

//
// Arithmetic operators for Images.
//
template <class Item>
Image<Item>
Image<Item>:: operator*(const Image<Item>& image) const {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  Image<Item> result;
  if(Bands==image.NumBands()) {
    result = image;
    for(int m=0; m<Bands*Rows*Cols; ++m) {
      result.SetPixel(m, pItem[m]*image.GetPixel(m));
    }
  }
  else if(Bands==1) {
    result = image;
    for(int k=0, m=0; k<image.NumBands(); ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, pppItem[0][i][j]*image.GetPixel(m));
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    result = *this;
    for(int k=0, m=0; k<Bands; ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, GetPixel(m)*image.GetPixel(0,i,j));
        }
      }
    }
  }
  return result;
}

template <class Item>
void
Image<Item>:: operator*=(const Image<Item>& image) {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  if(Bands == image.NumBands()) {
    for(int m=0; m<Bands*Rows*Cols; ++m)
      pItem[m] *= image.GetPixel(m);
  }
  else if(Bands == 1) {
    Item* oldPixel = new Item[Rows*Cols];
    for(int i=0; i<Rows*Cols; ++i)
      oldPixel[i] = pItem[i];

    ResizeImage(image.NumBands(), Rows, Cols);

    for(int k=0, m=0; k<image.NumBands(); ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j,++n, ++m) {
          pItem[m] = oldPixel[n]*image.GetPixel(m);
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    int m=0;
    for(int k=0; k<Bands; ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m, ++n) {
          pItem[m] *= image.GetPixel(n);
        }
      }
    }
  }
}

//
// Arithmetic operators for Images.
//
template <class Item>
Image<Item>
Image<Item>:: operator/(const Image<Item>& image) const {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  Image<Item> result;
  if(Bands==image.NumBands()) {
    result = image;
    for(int m=0; m<Bands*Rows*Cols; ++m) {
      result.SetPixel(m, pItem[m]/image.GetPixel(m));
    }
  }
  else if(Bands==1) {
    result = image;
    for(int k=0, m=0; k<image.NumBands(); ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, pppItem[0][i][j]/image.GetPixel(m));
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    result = *this;
    for(int k=0, m=0; k<Bands; ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, GetPixel(m)/image.GetPixel(0,i,j));
        }
      }
    }
  }
  return result;
}

template <class Item>
void
Image<Item>:: operator/=(const Image<Item>& image) {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  if(Bands == image.NumBands()) {
    for(int m=0; m<Bands*Rows*Cols; ++m)
      pItem[m] /= image.GetPixel(m);
  }
  else if(Bands == 1) {
    Item* oldPixel = new Item[Rows*Cols];
    for(int i=0; i<Rows*Cols; ++i)
      oldPixel[i] = pItem[i];

    ResizeImage(image.NumBands(), Rows, Cols);

    for(int k=0, m=0; k<image.NumBands(); ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j,++n, ++m) {
          pItem[m] = oldPixel[n]/image.GetPixel(m);
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    int m=0;
    for(int k=0; k<Bands; ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m, ++n) {
          pItem[m] /= image.GetPixel(n);
        }
      }
    }
  }
}

//
// Logic operators for Images.
//
template <class Item>
Image<Item>
Image<Item>:: operator|(const Image<Item>& image) const {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  Image<Item> result;
  if(Bands==image.NumBands()) {
    result = image;
    for(int m=0; m<Bands*Rows*Cols; ++m) {
      result.SetPixel(m, pItem[m]|image.GetPixel(m));
    }
  }
  else if(Bands==1) {
    result = image;
    for(int k=0, m=0; k<image.NumBands(); ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, pppItem[0][i][j]|image.GetPixel(m));
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    result = *this;
    for(int k=0, m=0; k<Bands; ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, GetPixel(m)|image.GetPixel(0,i,j));
        }
      }
    }
  }
  return result;
}

template <class Item>
void
Image<Item>:: operator|=(const Image<Item>& image) {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  if(Bands == image.NumBands()) {
    for(int m=0; m<Bands*Rows*Cols; ++m)
      pItem[m] |= image.GetPixel(m);
  }
  else if(Bands == 1) {
    Item* oldPixel = new Item[Rows*Cols];
    for(int i=0; i<Rows*Cols; ++i)
      oldPixel[i] = pItem[i];

    ResizeImage(image.NumBands(), Rows, Cols);

    for(int k=0, m=0; k<image.NumBands(); ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j,++n, ++m) {
          pItem[m] = oldPixel[n]|image.GetPixel(m);
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    int m=0;
    for(int k=0; k<Bands; ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m, ++n) {
          pItem[m] |= image.GetPixel(n);
        }
      }
    }
  }
}
template <class Item>
Image<Item>
Image<Item>:: operator&(const Image<Item>& image) const {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  Image<Item> result;
  if(Bands==image.NumBands()) {
    result = image;
    for(int m=0; m<Bands*Rows*Cols; ++m) {
      result.SetPixel(m, pItem[m]&image.GetPixel(m));
    }
  }
  else if(Bands==1) {
    result = image;
    for(int k=0, m=0; k<image.NumBands(); ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, pppItem[0][i][j]&image.GetPixel(m));
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    result = *this;
    for(int k=0, m=0; k<Bands; ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m) {
          result.SetPixel(m, GetPixel(m)&image.GetPixel(0,i,j));
        }
      }
    }
  }
  return result;
}

template <class Item>
void
Image<Item>:: operator&=(const Image<Item>& image) {
  assert(Rows==image.NumRows()&&Cols==image.NumCols());
  assert(Bands==image.NumBands() || Bands==1 || image.NumBands()==1);

  if(Bands == image.NumBands()) {
    for(int m=0; m<Bands*Rows*Cols; ++m)
      pItem[m] &= image.GetPixel(m);
  }
  else if(Bands == 1) {
    Item* oldPixel = new Item[Rows*Cols];
    for(int i=0; i<Rows*Cols; ++i)
      oldPixel[i] = pItem[i];

    ResizeImage(image.NumBands(), Rows, Cols);

    for(int k=0, m=0; k<image.NumBands(); ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j,++n, ++m) {
          pItem[m] = oldPixel[n]&image.GetPixel(m);
        }
      }
    }
  }
  else if (image.NumBands()==1) {
    int m=0;
    for(int k=0; k<Bands; ++k) {
      int n=0;
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j, ++m, ++n) {
          pItem[m] &= image.GetPixel(n);
        }
      }
    }
  }
}

//
// Scalar arithmetic operators for Images
//
template <class Item>
Image<Item>
Image<Item>:: operator+(const Item& value) const {

  Image<Item> result(*this);
  for(int i=0; i<Bands*Rows*Cols; ++i)
    result.SetPixel(i, pItem[i]+value);

  return result;
}

template <class Item>
void
Image<Item>:: operator+=(const Item& value) {

  for(int i=0; i<Bands*Rows*Cols; ++i)
    pItem[i] = pItem[i] + value;
}

template <class Item>
Image<Item>
Image<Item>:: operator-(const Item& value) const {

  Image<Item> result(*this);
  for(int i=0; i<Bands*Rows*Cols; ++i)
    result.SetPixel(i, pItem[i]-value);

  return result;
}

template <class Item>
void
Image<Item>:: operator-=(const Item& value) {

  for(int i=0; i<Bands*Rows*Cols; ++i)
    pItem[i] = pItem[i] - value;
}

template <class Item>
Image<Item>
Image<Item>:: operator*(const Item& value) const {

  Image<Item> result(*this);
  for(int i=0; i<Bands*Rows*Cols; ++i)
    result.SetPixel(i, pItem[i]*value);

  return result;
}

template <class Item>
void
Image<Item>:: operator*=(const Item& value) {

  for(int i=0; i<Bands*Rows*Cols; ++i)
    pItem[i] = pItem[i] * value;
}

template <class Item>
Image<Item>
Image<Item>:: operator^(const Item& value) const {

  Image<Item> result(*this);
  for(int i=0; i<Bands*Rows*Cols; ++i)
    result.SetPixel(i, value - pItem[i]);

  return result;
}

template <class Item>
void
Image<Item>:: operator^=(const Item& value) {

  for(int i=0; i<Bands*Rows*Cols; ++i)
    pItem[i] = value - pItem[i];
}

/*
the routine extracts Region of Interest (ROI) from the image.
the parameters specifies the region and they are taken as
inclusive.
*/

template <class Item>
void
Image<Item>:: extractROI(int low_band, int up_band,
                          int low_row, int up_row,
                          int low_col, int up_col) {
  //assertion takes care of unexpected conditions
  assert(low_band<=up_band && low_row<=up_row && low_col<=up_col);
  assert(up_band<Bands && up_row <Rows && up_col < Cols);
  assert(low_band>=0 && low_row>= 0 && low_col>=0);

  Image<Item> tmp_im(*this);
  ResizeImage(up_band-low_band+1, up_row-low_row+1, up_col-low_col+1);

  //copy items with in the ROI
  int k, index;
  for(k=low_band, index=0; k<=up_band; ++k) {
    for(int i=low_row; i<=up_row; ++i) {
      for(int j=low_col; j<=up_col; ++j, ++index) {
        pItem[index] = tmp_im.GetPixel(k,i,j);
      }
    }
  }
}

template <class Item>
Image<Item>
Image<Item>:: ExtractROI(int low_band, int up_band,
                          int low_row, int up_row,
                          int low_col, int up_col) const {
  //assertion takes care of unexpected conditions
  assert(low_band<=up_band && low_row<=up_row && low_col<=up_col);
  assert(up_band<Bands && up_row <Rows && up_col < Cols);
  assert(low_band>=0 && low_row>= 0 && low_col>=0);

  Image<Item> result = Image<Item>(up_band-low_band+1, up_row-low_row+1, 
                                   up_col-low_col+1);

  //copy items with in the ROI
  for(int k=low_band, index=0; k<=up_band; ++k) {
    for(int i=low_row; i<=up_row; ++i) {
      for(int j=low_col; j<=up_col; ++j, ++index) {
        result.SetPixel(index, pppItem[k][i][j]);
      }
    }
  }

  return result;
}

/*
the routine expand an image with arbitrary offset.
*/

template <class Item>
void
Image<Item>:: expand(int new_bands, int new_rows, int new_cols,
                      int band_offset, int row_offset, int col_offset,
                      Item pad_value) {
  //assertion takes care of unexpected conditions
  assert(band_offset<new_bands && row_offset<new_rows && col_offset<new_cols);

  Image<Item> temp_im = *this;

  *this = Image<Item>(new_bands, new_rows, new_cols, pad_value);

  for(int k=band_offset,k2=0; k2<temp_im.NumBands()&&k<Bands; ++k,++k2) {
    for(int i=row_offset,i2=0; i2<temp_im.NumRows()&&i<Rows; ++i,++i2) {
      for(int j=col_offset,j2=0; j2<temp_im.NumCols()&&j<Cols; ++j,++j2) {
        SetPixel(k,i,j,temp_im.GetPixel(k2,i2,j2));
      }
    }
  }
}

template <class Item>
Image<Item>
Image<Item>:: Expand(int new_bands, int new_rows, int new_cols,
                      int band_offset, int row_offset, int col_offset,
                      Item pad_value) const {
  //assertion takes care of unexpected conditions
  assert(band_offset<new_bands && row_offset<new_rows && col_offset<new_cols);

  Image<Item> result(new_bands, new_rows, new_cols, pad_value);

  for(int k=band_offset,k2=0; k2<result.NumBands()&&k<Bands; ++k,++k2) {
    for(int i=row_offset,i2=0; i2<result.NumRows()&&i<Rows; ++i,++i2) {
      for(int j=col_offset,j2=0; j2<result.NumCols()&&j<Cols; ++j,++j2) {
        result.SetPixel(k2,i2,j2, pppItem[k][i][j]);
      }
    }
  }
  return result;
}

/*
the routine inserts another image into this image at an arbitrary offset
and return the new image.
*/

template <class Item>
Image<Item>
Image<Item>:: Insert(const Image<Item>& insert,
                     int band_offset, int row_offset, int col_offset) {
  //assertion takes care of unexpected conditions
  assert(band_offset<Bands && row_offset<Rows && col_offset<Cols);

  Image<Item> res(Bands,Rows,Cols);
  //copy items with in the ROI
  for(int k=band_offset,k2=0; k2<insert.NumBands()&&k<Bands; ++k,++k2) {
    for(int i=row_offset,i2=0; i2<insert.NumRows()&&i<Rows; ++i,++i2) {
      for(int j=col_offset,j2=0; j2<insert.NumCols()&&j<Cols; ++j,++j2) {
        res.SetPixel(k,i,j,insert.GetPixel(k2,i2,j2));
      }
    }
  }
  return res;
}

/*
the routine inserts another image into this image at an arbitrary offset
*/

template <class Item>
void
Image<Item>:: insert(const Image<Item>& insert,
                     int band_offset, int row_offset, int col_offset) {
  //assertion takes care of unexpected conditions
  assert(band_offset<Bands && row_offset<Rows && col_offset<Cols);

  //copy items with in the ROI
  for(int k=band_offset,k2=0; k2<insert.NumBands()&&k<Bands; ++k,++k2) {
    for(int i=row_offset,i2=0; i2<insert.NumRows()&&i<Rows; ++i,++i2) {
      for(int j=col_offset,j2=0; j2<insert.NumCols()&&j<Cols; ++j,++j2) {
        pppItem[k][i][j] = insert.GetPixel(k2,i2,j2);
      }
    }
  }
}

/*
the routine increases the size of the image by integer factors 
with pixel duplication.
*/

template <class Item>
Image<Item>
Image<Item>:: ScaleUp(int scale_bands, int scale_rows, int scale_cols) const {
  //assertion takes care of unexpected conditions
  assert(scale_bands>0);
  assert(scale_rows>0);
  assert(scale_cols>0);

  Image<Item> res(Bands*scale_bands,Rows*scale_rows,Cols*scale_cols);

  for(int k=0; k<res.NumBands(); ++k) {
    for(int i=0; i<res.NumRows(); ++i) {
      for(int j=0; j<res.NumCols(); ++j) {
        res.SetPixel(k,i,j,GetPixel(k/scale_bands,i/scale_rows,j/scale_cols));
      }
    }
  }
  return res;
}

/*
the routine increases the size of the image by integer factors 
with pixel duplication.
*/

template <class Item>
void
Image<Item>:: scaleup(int scale_bands, int scale_rows, int scale_cols) {
  //assertion takes care of unexpected conditions
  assert(scale_bands>0);
  assert(scale_rows>0);
  assert(scale_cols>0);


  Image<Item> temp_im = *this;

  *this = Image<Item>(Bands*scale_bands,Rows*scale_rows,Cols*scale_cols);

  for(int k=0; k<Bands; ++k) {
    for(int i=0; i<Rows; ++i) {
      for(int j=0; j<Cols; ++j) {
        SetPixel(k,i,j,temp_im.GetPixel(k/scale_bands,i/scale_rows,j/scale_cols));
      }
    }
  }
}

/*
the routine performs a circular shift on the image.
*/
template <class Item>
void
Image<Item>:: circularShift(int band_shift, int row_shift, int col_shift) {
  //assertion takes care of unexpected conditions
  assert(band_shift<Bands && row_shift<Rows && col_shift<Cols);
  assert(band_shift> -Bands && row_shift> -Rows && col_shift> -Cols);
  band_shift = (band_shift<0) ? Bands+band_shift: band_shift;
  row_shift = (row_shift<0) ? Rows+row_shift: row_shift;
  col_shift = (col_shift<0) ? Cols+col_shift: col_shift;

  Image<Item> tmp_im(*this);

  for(int k=0, k2=band_shift; k<Bands; ++k,++k2) {
    for(int i=0, i2=row_shift; i<Rows; ++i,++i2) {
      for(int j=0,j2=col_shift; j<Cols; ++j,++j2) {
        pppItem[k][i][j] = tmp_im.GetPixel(k2%Bands,i2%Rows,j2%Cols);
      }
    }
  }
}

/*
the routine performs a circular shift on the image.
*/
template <class Item>
Image<Item>
Image<Item>:: CircularShift(int band_shift, int row_shift, int col_shift) {
  //assertion takes care of unexpected conditions
  assert(band_shift<Bands && row_shift<Rows && col_shift<Cols);
  assert(band_shift> -Bands && row_shift> -Rows && col_shift> -Cols);
  band_shift = (band_shift<0) ? Bands+band_shift: band_shift;
  row_shift = (row_shift<0) ? Rows+row_shift: row_shift;
  col_shift = (col_shift<0) ? Cols+col_shift: col_shift;

  Image<Item> result(Bands, Rows, Cols);

  for(int k=0, k2=band_shift; k<Bands; ++k,++k2) {
    for(int i=0, i2=row_shift; i<Rows; ++i,++i2) {
      for(int j=0,j2=col_shift; j<Cols; ++j,++j2) {
        result.SetPixel(k,i,j,pppItem[k2%Bands][i2%Rows][j2%Cols]);
      }
    }
  }
  return result;
}

//
// This routine computes and returns the gradient of an image.
// the result image has twice the number of bands as the original.
// a pair of bands represent the 2D gradient vector where the 1st
// one represent the horizontal direction and the 2nd the vertical 
// direction.
//
// Currently, the gradient is computed by a simple forward difference.
//
template <class Item>
Image<Item>
Image<Item>:: Gradient() const {
  Image<Item> result(Bands*2, Rows, Cols);

  for(int k=0; k<Bands; k++) {
    for(int i=0; i<Rows; ++i) {
      for(int j=0; j<Cols; ++j) {
        Item here = GetPixel(k, i, j);
        Item up = GetPixelRepeat(k,i-1,j);
        Item right = GetPixelRepeat(k,i,j+1);
        result.SetPixel(2*k, i, j, right-here);
        result.SetPixel(2*k+1, i, j, up-here);
      }
    }
  }
  return result;
}

//
// This routine computes and returns the strength of the gradient.
// the strength is computed by L2 norm squared.
//
template <class Item>
Image<Item>
Image<Item>:: GradientStrength() const {
  Item item;  //whatever the default value for item.
  Image<Item> result(Bands, Rows, Cols, item);

  for(int k=0; k<Bands; ++k) {
    for(int i=0; i<Rows; ++i) {
      for(int j=0; j<Cols; ++j) {
        Item here = GetPixel(k, i, j);
        Item down = GetPixelRepeat(k,i+1,j);
        Item right = GetPixelRepeat(k,i,j+1);
        Item val = (here-down)*(here-down)+(here-right)*(here-right);
        result.SetPixel(k, i, j, val);
      }
    }
  }

  return result;
}

//
// This routine computes and returns the strength of the gradient.
// the gradient is 2-points approximation.
// the strength is computed by L2 norm squared.
//
template <class Item>
Image<Item>
Image<Item>:: GradientStrength2() const {
  Item item;  //whatever the default value for item.
  Image<Item> result(Bands, Rows, Cols, item);

  for(int k=0; k<Bands; ++k) {
    for(int i=0; i<Rows; ++i) {
      for(int j=0; j<Cols; ++j) {
        Item up = GetPixelRepeat(k, i-1, j);
        Item down = GetPixelRepeat(k,i+1,j);
        Item right = GetPixelRepeat(k,i,j+1);
        Item left = GetPixelRepeat(k,i,j-1);
        Item val = .25*((up-down)*(up-down)+(left-right)*(left-right));
        result.SetPixel(k, i, j, val);
      }
    }
  }

  return result;
}

template <class Item>
Item** 
Image<Item>:: Image2NRarray(int band) const {
  Item** a;
  a = new ptr1D_Data[Rows]-1;
  for(int i=1; i<=Rows;++i)
    a[i]=pItem+band*Rows*Cols+(i-1)*Cols-1;
  return a;
}


template<class Item>
int
CompareImageSize(const Image<Item>& image1, const Image<Item>& image2) {
  if (image1.NumBands() > image2.NumBands())  return 1;
  else if (image1.NumBands() < image2.NumBands())  return -1;
  else if (image1.NumRows() > image2.NumRows())  return 1;
  else if (image1.NumRows() < image2.NumRows())  return -1;
  else if (image1.NumCols() > image2.NumCols())  return 1;
  else if (image1.NumCols() < image2.NumCols())  return -1;
  else return 0;
}

template<class Item>
int
ComparePlaneSize(const Image<Item>& image1, const Image<Item>& image2) {
  if (image1.NumRows() > image2.NumRows())  return 1;
  else if (image1.NumRows() < image2.NumRows())  return -1;
  else if (image1.NumCols() > image2.NumCols())  return 1;
  else if (image1.NumCols() < image2.NumCols())  return -1;
  else return 0;
}

#endif /* Image_cpp */
