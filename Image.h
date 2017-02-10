#ifndef _Image_h_
#define _Image_h_

#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#include <cassert>

enum pnm_type {IO_PBMA, IO_PGMA, IO_PPMA, IO_PBM, IO_PGM, IO_PPM};

template <class Item>
class Image {
 public:

  Image();
  Image(int bands, int rows, int cols);
  Image(int bands, int rows, int cols, const Item& init_value);

  ~Image();

  Image(const Image<Item>& image);

  const Image<Item>& operator =(const Image<Item>& image);

  // Get the specified pixel value.
  // Each pixel can be indexed by (column, row) or column*row.
  inline Item GetPixel(int band_index, int row_index, int col_index) const {
    assert(Bands>band_index && Rows>row_index && Cols>col_index &&
           band_index>=0 && row_index>=0 && col_index>=0);
    return(pppItem[band_index][row_index][col_index]);
  }
  
  inline Item GetPixel(int pixel_index) const {
    assert(pixel_index>=0 && pixel_index<Bands*Rows*Cols);
    return(pItem[pixel_index]);
  }

  inline Item GetPixelFold(int band_index, int row_index, int col_index) const;
  inline Item GetPixelWrap(int band_index, int row_index, int col_index) const;
  inline Item GetPixelRepeat(int band_index,int row_index,int col_index) const;
  inline Item GetPixelExtrap(int band_index, int row_index, int col_index) const;
  inline Item GetPixelZero(int band_index, int row_index, int col_index) const;
  inline Item GetPixelDefault(int band_index, int row_index, 
                              int col_index, const Item& item) const;

  // Set the specified pixel to the given value.
  inline void SetPixel(int band_index, int row_index, int col_index,\
		const Item& new_pixel) {
    //assert(Bands>band_index && Rows>row_index && Cols>col_index &&
    //       band_index>=0 && row_index>=0 && col_index>=0);
    if(Bands<band_index) {
      cerr << "Band too big: " << band_index << endl;
      cerr << Bands << " " << Rows << " " << Cols << endl;
      assert(0);
    }
    if(Rows<row_index) {
      cerr << "Row too big: " << row_index << endl;
      cerr << Bands << " " << Rows << " " << Cols << endl;
      assert(0);
    }
    if(Cols<col_index) {
      cerr << "Column too big: " << col_index << endl;
      cerr << Bands << " " << Rows << " " << Cols << endl;
      assert(0);
    }
    pppItem[band_index][row_index][col_index] = new_pixel;
  }

  inline void SetPixel(int pixel_index, const Item& new_pixel) {
    assert(pixel_index>=0 && pixel_index<Bands*Rows*Cols);
    pItem[pixel_index] = new_pixel;
  }
  inline void AddPixel(int band_index, int row_index, int col_index,\
		const Item& adder) {
    assert(Bands>band_index && Rows>row_index && Cols>col_index &&
           band_index>=0 && row_index>=0 && col_index>=0);
    pppItem[band_index][row_index][col_index]=
      pppItem[band_index][row_index][col_index]+adder;
  }

  inline void AddPixel(int pixel_index, const Item& adder) {
    assert(pixel_index>=0 && pixel_index<Bands*Rows*Cols);
    pItem[pixel_index] = pItem[pixel_index]+adder;
  }

  // get the protected member data.
  inline int NumBands() const {return Bands;}
  inline int NumRows() const {return Rows;}
  inline int NumCols() const {return Cols;}
  inline int NumPixels() const {return Bands*Rows*Cols;}

  // Artithmetic overloaded operators
  Image<Item> operator +(const Image<Item>& image) const;
  void operator +=(const Image<Item>& image);
  Image<Item> operator -(const Image<Item>& image) const;
  void operator -=(const Image<Item>& image);
  Image<Item> operator *(const Image<Item>& image) const;
  void operator *=(const Image<Item>& image);
  Image<Item> operator /(const Image<Item>& image) const;
  void operator /=(const Image<Item>& image);

  // Logic overloaded operators
  Image<Item> operator |(const Image<Item>& image) const;
  void operator |=(const Image<Item>& image);
  Image<Item> operator &(const Image<Item>& image) const;
  void operator &=(const Image<Item>& image);

  // Scalar arithmetic operators
  Image<Item> operator +(const Item& value) const;
  void operator +=(const Item& value);
  Image<Item> operator -(const Item& value) const;
  void operator -=(const Item& value);
  Image<Item> operator *(const Item& value) const;
  void operator *=(const Item& value);
  Image<Item> operator ^(const Item& value) const;
  void operator ^=(const Item& value);

  // Image manipulation operations
  void extractROI(int low_band, int up_band,
                  int low_row, int up_row,
                  int low_col, int up_col);
  Image<Item> ExtractROI(int low_band, int up_band,
                  int low_row, int up_row,
                  int low_col, int up_col) const;
  void insert(const Image<Item>& insert,
              int band_offset, int row_offset, int col_offset);
  Image<Item> Insert(const Image<Item>& insert,
                     int band_offset, int row_offset, int col_offset);
  void expand(int new_bands, int new_rows, int new_cols,
              int band_offset, int row_offset, int col_offset,
              Item pad_value);
  Image<Item> Expand(int new_bands, int new_rows, int new_cols,
              int band_offset, int row_offset, int col_offset,
              Item pad_value) const;
  void scaleup(int scale_bands, int scale_rows, int scale_cols);
  Image<Item> ScaleUp(int scale_bands, int scale_rows, int scale_cols) const;
  void circularShift(int band_shift, int row_shift, int col_shift);
  Image<Item> CircularShift(int band_shift, int row_shift, int col_shift);

	void Clear() {memset((char *)pItem,(char)0,Bands*Rows*Cols*sizeof(Item));}
	void Const(Item c) {for(int i=0; i<Bands*Rows*Cols; ++i) pItem[i]=c;}
	
  // Simple Image Processing routines
  Image<Item> Gradient() const;
  Image<Item> GradientStrength() const;  // 1-point forward
  Image<Item> GradientStrength2() const; // 2-point gradient

  // File IOs
  //friend ifstream& operator>>(ifstream& s, Image<Item>& image);
  //friend ofstream& operator<<(ofstream& s, const Image<Item>& image);

  void ReadPnmFile(ifstream& in);
  int ReadPnmFile(const char* filename);
  void WritePnmFile(ofstream& out, int norm) const;
  int WritePnmFile(const char* filename, pnm_type type, int norm) const;
  void ReadRawFile(ifstream& in);
  void WriteRawFile(ofstream& out) const;
  int ReadRawFile(const char* filename);
  int WriteRawFile(const char* filename) const;
  int WritePureRawFile(const char* filename) const;

  /*
    Numerical Recipe Interface
    This annoying conversion is the result of Numerical Recepe
    C routines being written originally in Fortran.
    Thus, the array indexing has to be handled with care. I hate it.
  */
  Item** Image2NRarray(int band) const;
    
  Item* Image2NRvector(int band, int row) const {
    return pItem+band*Rows*Cols+row*Cols-1;
  }
  void ResizeImage(int bands, int rows, int cols);

 protected:
  typedef Item**  ptr2D_Data;
  typedef Item*   ptr1D_Data;

 private:

  int Bands, Rows, Cols;

  ptr2D_Data* pppItem;
  Item* pItem;

  void _setItemPointers(int bands, int rows, int cols);
  void _resetItemPointers();
};

// Overloading iostream operators for Image.
template <class Item>
istream& operator>>(istream& s, Image<Item>& image);

template <class Item>
ostream& operator<<(ostream& s, const Image<Item>& image);

// Interactive image write routine.
template<class Item>
void
InteractiveImageWrite(const Image<Item>& image, 
                      char* filename, int normalize);

template<class Item>
void
WriteMultipleBand2PnmFile(const Image<Item>& image, 
                          char* head, char* extension, int normalize);

template<class Item>
int
CompareImageSize(const Image<Item>& image1, const Image<Item>& image2);

template<class Item>
int
ComparePlaneSize(const Image<Item>& image1, const Image<Item>& image2);

#include "Image.cpp"
#include "FileIO.cpp"

#include "mytype.h"
typedef Image<real> RealImage;
typedef Image<uchar> ByteImage;
typedef Image<int> IntImage;

#endif /* Image_h */
