#ifndef _FileIO_cpp_
#define _FileIO_cpp_

#include "Image.h"
#include "mytype.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

// Overloaded input operator for class Array;
// inputs values for entire array.
template <class Item>
istream &operator>>( istream& input, Image<Item>& image )
{
  Item item;
  for ( int i = 0; i < image.NumBands*image.NumRows()*image.NumCols(); i++ ) {
    input >> item;
    image.SetPixel(i, item);
  }

  return input;   // enables cin >> x >> y;
}

template <class Item>
ostream& operator<<(ostream& s, const Image<Item>& image)
{
  Item item;
  int rows, cols, bands;

  cols=image.NumCols();
  rows=image.NumRows();
  bands=image.NumBands();

  s << "Bands=" << bands << endl;  
  s << "Rows=" << rows << endl;  
  s << "Cols=" << cols << endl;
  for(int k=0; k<bands; ++k) {
    s << "Band " << k << endl;
    for(int i=0; i<rows; ++i) {
      for(int j=0; j<cols; ++j) {
        item=image.GetPixel(k,i,j);
        s << item << " ";
      }
      s << endl;
    }
  }

  return s;
}

// file IO routine.
// this is a friend of Image class.
//
template <class Item>
ifstream& operator>>( ifstream& input, Image<Item>& image )
{
  int bands, rows, cols;

  input >> bands;
  input >> rows;
  input >> cols;

  //I don't know why but I have to throw one char away
  // in order to read in data properly...
  char dummy;
  input.read(&dummy, 1);

  image.ResizeImage(image.Bands, image.Rows, image.Cols);

  int num_pixel = image.Bands*image.Rows*image.Cols;
  Item *buffer = new Item[num_pixel];
  input.read(reinterpret_cast<char *> (buffer),
             num_pixel*sizeof(Item));

  for(int i=0; i<num_pixel; ++i)
    image.pItem[i] = buffer[i];

  delete [] buffer;

  return input;
}

// file IO routine.
// this is a friend of Image class.
//
template <class Item>
ofstream& operator<<(ofstream& output, const Image<Item>& image)
{
  output << image.Bands << endl;  
  output << image.Rows << endl;  
  output << image.Cols << endl;
 
  int num_pixel=image.Bands*image.Rows*image.Cols;
  Item *buffer = new Item[num_pixel];

  for(int i=0; i<num_pixel; ++i)
    buffer[i] = image.pItem[i];

  output.write(reinterpret_cast<char *> (buffer),
               num_pixel*sizeof(Item));

  delete [] buffer;

  return output;
}

//
// file IO routine
// this routine reads a pnm file and convert it into
// the Image class
//

template <class Item>
int
Image<Item>:: ReadPnmFile(const char* filename) {
  FILE  *fp;
  size_t bytes_read;
  int   msize, size, val, i, j, k;
  unsigned char *buf;
  char  pmode, c, firstchar, rname[] = "ReadPnmFile";

  printf("Reading data from %s\n", filename);
	
  /* try to open the file */
  if ((fp = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "%s: error loading image from %s.\n", 
	    "ReadPnmFile()", filename);
    fclose(fp);
    return(-1);
  }

  /* Check for the correct magic number */
  fscanf(fp, "%c%c\n", &firstchar, &pmode);
  if (firstchar != 'P') {
    fprintf(stderr, "%s: unknown image file type: %c\n", rname, firstchar);
    fclose(fp);
    return(-1);
  }
  
  pnm_type type;
  int bands;
  switch (pmode) {
  case '1': type = IO_PBMA; bands=1; break;
  case '2': type = IO_PGMA; bands=1; break;
  case '3': type = IO_PPMA; bands=3; break;
  case '4': type = IO_PBM; bands=1; break;
  case '5': type = IO_PGM; bands=1; break;
  case '6': type = IO_PPM; bands=3; break;
  default:
    fprintf(stderr, "%s: unrecognized magic number: %c\n", rname, pmode);
    fclose(fp);
    return(-1);
  }

  /* throw out any comments */
  while ((c = getc(fp)) == '#') 
    while (!feof(fp) && ((c = getc(fp)) != '\n'));
  ungetc(c, fp);
  if (feof(fp)) {
    fprintf(stderr, "%s: unexpected end of file in %s!\n", rname, filename);
    fclose(fp);
    return(-1);
  }
  
  int cols, rows;
  /* read the header information */
  fscanf(fp,"%d %d%c", &cols, &rows, &c);
  if (type != IO_PBM && type != IO_PBMA) fscanf(fp, "%d%c", &msize, &c);

  ResizeImage(bands, rows, cols);

  cout << "Bands=" << Bands << " Rows=" << Rows << ", Cols=" << Cols << endl;

  switch (type) {
  case IO_PBMA: 
  case IO_PGMA:
     size = Cols * Rows;
     for(i = 0; i < size; ++i)
     {
        if (feof(fp)) {
           fprintf(stderr, "%s: unexpected end of file\n", rname);
           return(-1);
        }
        fscanf(fp, "%d", &val);
        pItem[i] = (Item) val;
     }
     break;
  case IO_PPMA:
     if (Bands < 3) {
        fprintf(stderr, "%s: unmatched image depth! (%d)\n", 
           rname, Bands);
        return(-1);
     }
     size = Rows*Cols;
     for(i = 0; i < Rows; ++i) {
       for(j=0; j<Cols; ++j) {
        if (feof(fp)) {
           fprintf(stderr, "%s: unexpected end of file\n", rname);
           return(-1);
        }
        fscanf(fp, "%d", &val);
        SetPixel(0,i,j,val);
        fscanf(fp, "%d", &val);
        SetPixel(1,i,j,val);
        fscanf(fp, "%d", &val);
        SetPixel(2,i,j,val);
       }
     }
     break;
  case IO_PBM: 
     size = ceil((double) .125 * Cols) * Rows;
     buf = (unsigned char*) malloc(size);
     bytes_read = fread(buf, sizeof(unsigned char), size, fp);
     if (bytes_read != size) {
       fprintf(stderr, 
	    "%s: Warning: file not of expected size!\n", rname);
       free(buf);
       return(-1);
     }
     for(i=0, k=0; i<Rows; i++) {
	     for(j=0; j<Cols; j++) {
	        if(j % 8 == 0)
	           val = buf[k++];
	        SetPixel(0,i,j,(Item)((val & 0x80) ? 1 : 0));
	        val <<= 1;
	     }
     }
     free(buf);
     break;
  case IO_PGM:
     size = Rows*Cols;
     buf = (unsigned char*) malloc(size);
     bytes_read = fread(buf, sizeof(unsigned char), size, fp);
     if (bytes_read != size) {
       fprintf(stderr, 
	    "%s: Warning: file not of expected size!\n", rname);
       return(-1);
     }
     for(i=0; i<size; ++i) {
     	 pItem[i]=buf[i];
     }
     free(buf);
     break;
  case IO_PPM:
     size = 3 * Rows*Cols;
     buf = (unsigned char *) malloc(size);
     bytes_read = fread(buf, sizeof(unsigned char), size, fp);
     if (bytes_read != size) {
       fprintf(stderr, 
	    "%s: Warning: file not of expected size!\n", rname);
       free(buf);
       return(-1);
     }
     for(i=0, k=0; i<Rows; ++i) {
	     for(j=0; j<Cols; ++j) {
	     	 SetPixel(0,i,j,(Item) buf[k++]);
	     	 SetPixel(1,i,j,(Item) buf[k++]);
	     	 SetPixel(2,i,j,(Item) buf[k++]);
	     }
     }
     free(buf);
     break;
  default: 
     break;
  }

  fclose(fp);
  return 0;
}

template <class Item>
void
Image<Item>:: ReadPnmFile(ifstream& in) {
  int mode;
  int bands, rows, cols;
  char magic;

  in >> magic;
  in >> mode;
  assert(magic == 'P');
  switch(mode) {
  case 2:
    cerr << "ReadPnmFile:: Ascii data not supported\n";
    abort();
    break;
  case 3:
    cerr << "ReadPnmFile:: Ascii data not supported\n";
    abort();
    break;
  case 5:
    bands=1;
    break;
  case 6:
    bands=3;
    break;
  default:
    cerr << "Mode " << mode << " is not supported." << endl;
    return;
  }

  char chbuffer[MAX_COMMENT_LENGTH];
  in.getline(chbuffer, MAX_COMMENT_LENGTH);
  do { // skip comment lines
    in.getline(chbuffer, MAX_COMMENT_LENGTH);
  }
  while(chbuffer[0] == '#');

  sscanf(chbuffer, "%d %d", &cols, &rows);
  ResizeImage(bands, rows, cols);

  cout << "Bands=" << Bands << " Rows=" << Rows << ", Cols=" << Cols << endl;

  in.getline(chbuffer, MAX_COMMENT_LENGTH); // skip the line with 256

  unsigned char *buffer = new unsigned char[Bands*Rows*Cols];
  in.read((char *)buffer, Bands*Rows*Cols*sizeof(unsigned char));

  int index = 0;
  for(int i=0; i<Rows; ++i) {
    for(int j=0; j<Cols; ++j) {
      for(int k=0; k<Bands; ++k, ++index) {
        pppItem[k][i][j] = (Item) buffer[index];
      }
    }
  }

  delete [] buffer;
}

//
//This routine writes Image class image into a pgm file.
//Bands can be either 1 or 3.  Other values are not allowed.
//if norm=1, then the pgm image streatches from 0 to 255 where
//the minimum if this image is mapped to 0 and the maximum pixel
//is mapped to 255.
//

template <class Item>
int
Image<Item>:: WritePnmFile(const char* filename, pnm_type type, int norm) const {

  printf("Writing data to %s\n", filename);

  int bands;
  if(Bands > 1) {
    bands=3;
  }
  else {
    bands=1;
  }
  unsigned char *buffer = new unsigned char[bands*Rows*Cols];

  real max, min;
  if(norm) {  // Normalize the image so that min=0 and max=255
    max = pItem[0];
    min = max;
    for(int i=0; i<Bands*Rows*Cols; ++i) {
      if(pItem[i] > max) max = pItem[i];
      if(pItem[i] < min) min = pItem[i];
    }
    cout << "Normalization: min= " << min << " max=" << max << endl;
  }
  else {
    min = 0;
    max = 255.0;
  }

  if(max - min > 1.0e-5) {
    int index=0;
    for(int i=0; i<Rows; ++i) {
      for(int j=0; j<Cols; ++j) {
        for(int k=0; k<Bands; ++k) {
          buffer[index++] = (unsigned char)(255*(pppItem[k][i][j]-min)/(max-min));
        }
      }
    }
    for(int k=Bands; k<bands; ++k) {
      for(int i=0; i<Rows; ++i) {
        for(int j=0; j<Cols; ++j) {
          buffer[index++] = (unsigned char)0;
        }
      }
    }
  }
  else
    for(int i=0; i<bands*Rows*Cols; ++i)
      buffer[i] = (char) pItem[0];

  /* Open the file */
  FILE *fp;
  size_t size;
  int i, j, k;
  unsigned char *buf, val;
  char rname[] = "write_pnm";

  if ((fp = fopen(filename, "wb")) == NULL){
    fprintf(stderr, "%s: error opening file %s.\n", rname, filename);
    return(-1);
  }

  /* write out magic number for pnm files */
  switch (type) {
  case IO_PBMA: fprintf(fp, "P1\n"); break;
  case IO_PGMA: fprintf(fp, "P2\n"); break;
  case IO_PPMA: fprintf(fp, "P3\n"); break;
  case IO_PBM: fprintf(fp, "P4\n"); break;
  case IO_PGM: fprintf(fp, "P5\n"); break;
  case IO_PPM: fprintf(fp, "P6\n"); break;
  }

  /* write out comment */
  //if (comment != NULL) fprintf(fp, "# %s\n", comment);

  /* write out header info */
  fprintf(fp, "%d %d\n", Cols,Rows);
  if (type != IO_PBM) fprintf(fp, "255\n");

  /* write out data */
  switch(type)
  {
  case IO_PBMA:
     for(i = 0, k = 0; i < Rows; i++)
     {
       for(j = 0, val = 0; j < Cols; j++, k++)
       {
        if(buffer[i]>0)
           fprintf(fp, "1 ");
        else
           fprintf(fp, "0 ");
        if(k % 35 == 34)
           fprintf(fp, "\n");
       }
     }
     break;
  case IO_PGMA:
     for(i = 0, k = 0; i < Rows; i++)
     {
       for(j = 0, val = 0; j < Cols; j++, ++k)
       {
        fprintf(fp, "%u ", buffer[k]);
        if(k % 17 == 16)
           fprintf(fp, "\n");
       }
     }
     break;
  case IO_PPMA:
     for(i = 0, k = 0; i < Rows; i++)
     {
       for(j = 0, val = 0; j < Cols; j++, ++k)
       {
        fprintf(fp, "%u ", buffer[k]);
        fprintf(fp, "%u ", buffer[k+1]);
        fprintf(fp, "%u ", buffer[k+2]);
        if(k % 5 == 4)
           fprintf(fp, "\n");
       }
     }
     break;
  case IO_PBM:
     size = (unsigned int) ceil((double)(Cols * 0.125)) * Rows;
     buf = (unsigned char *) malloc(size);
     int jj;
     for(i = 0, k = 0, jj=0; i < Rows; i++)
     {
       for(j = 0, val = 0; j < Cols; j++,jj++)
       {
        if(buffer[jj] != 0)
           val += 1;
        if(j % 8 == 7)
           buf[k ++] = val;
        val <<= 1;
       }
       if(j % 8 != 0)
       {
         val <<= (7 - (j % 8));
         buf[k ++] = val;
       }
     }
     if ((fwrite(buf, sizeof(unsigned char), size, fp)) != size) {
        fprintf(stderr, "%s: could not write all of image to %s!\n",
           rname, filename);
        free(buf);
        return(-1);
     }
     free(buf);
     break;
  case IO_PGM:
     size = Cols * Rows;
     if ((fwrite(buffer, sizeof(unsigned char), size, fp)) != size) {
        fprintf(stderr, "%s: could not write all of image to %s!\n",
           rname, filename);
     return(-1);
     }
     break;
  case IO_PPM:
     size = 3 * Cols * Rows;
     if ((fwrite(buffer, sizeof(unsigned char), size, fp)) != size) {
        fprintf(stderr, "%s: could not write all of image to %s!\n",
           rname, filename);
     return(-1);
     }
     break;
  default:
     break;
  }
  fclose(fp);

  delete [] buffer;
  
  return 0;
}
template <class Item>
void
Image<Item>:: WritePnmFile(ofstream& out, int norm) const {

  int bands;
  if(Bands > 1) {
    bands=3;
    out << "P6" << endl;
  }
  else {
    bands=1;
    out << "P5" << endl;
  }
  out << Cols << " " << Rows << endl;
  out << "255" << endl;

  unsigned char *buffer = new unsigned char[bands*Rows*Cols];

  real max, min;
  if(norm) {  // Normalize the image so that min=0 and max=255
    max = min = pItem[0];
    for(int i=0; i<Bands*Rows*Cols; ++i) {
      if(pItem[i] > max) max = pItem[i];
      if(pItem[i] < min) min = pItem[i];
    }
    cout << "Normalization: min= " << min << " max=" << max << endl;
  }
  else {
    min = 0;
    max = 255.0;
  }

  if(max - min > 1.0e-5) {
    int index=0;
    for(int i=0; i<Rows; ++i) {
      for(int j=0; j<Cols; ++j) {
        for(int k=0; k<Bands; ++k,++index) {
          buffer[index] = (unsigned char)(255*(pppItem[k][i][j]-min)/(max-min));
        }
        for(int k=Bands; k<bands; ++k,++index) {
          buffer[index] = 0;
        }
      }
    }
  }
  else
    for(int i=0; i<bands*Rows*Cols; ++i)
      buffer[i] = (char) pItem[0];

  out.write((char *)buffer, bands*Rows*Cols*sizeof(unsigned char));
  
  delete [] buffer;
}

template<class Item>
void
WriteMultipleBand2PnmFile(const Image<Item>& image, 
                          char* head, char* extension,
                          int normalize) {
  Image<Item> buffer; 
  int b;
  char filename[128];
  ofstream out;
  for(b=0; b<image.NumBands(); b+=3) {
    if(extension!=NULL){
      if(image.NumBands()>3)
        sprintf(filename, "%s.%s%d", head,extension,b);
      else
        sprintf(filename, "%s.%s", head,extension);
    }
    else if(image.NumBands()>3)
      sprintf(filename, "%s%d", head, b);
    else
      sprintf(filename, "%s", head);
    out.open(filename, ios::out);
    buffer=image.ExtractROI(b,Min(b+2,image.NumBands()-1),
                            0,image.NumRows()-1,
                            0,image.NumCols()-1);
    buffer.WritePnmFile(out, normalize);
    out.close();
  }
}

template<class Item>
void
InteractiveImageWrite(const Image<Item>& image, char* filename, int normalize) {

  Image<Item> tmp_im;
  int band;

  for(int done=0; !done; ) {
    cout << "Which band to look at (type negative to quit): ";
    cin >> band;
    if(band < 0)
      done = 1;
    else {
      if(band >= image.NumBands())
        cerr << "Out of limit.  Has to be 0.." << image.NumBands()-1 << endl;
      else {
        tmp_im = image.ExtractROI(band, band, 0, image.NumRows()-1,
                                  0, image.NumCols()-1);

        tmp_im.WritePnmFile(filename,IO_PGM,normalize);
      }
    }
  }
}

//
// Raw file file I/O.
// The header for a raw data file is:
// #bands #rows #cols
// in the first line followed by the data.
//
const int MAX_COMMENT_LENGTH=256;

template <class Item>
void
Image<Item>:: ReadRawFile(ifstream& in) {
  int bands,rows,cols;
  char chbuf[MAX_COMMENT_LENGTH];
  //in.getline(chbuf, MAX_COMMENT_LENGTH);
  //sscanf(chbuf, "%d %d %d\n", &bands,&rows,&cols);

  in >> bands;
  in >> rows;
  in >> cols;
  in.read(chbuf,2);
  //in.getline(chbuf,MAX_COMMENT_LENGTH);
  ResizeImage(bands, rows, cols);

  cout << "Bands=" << Bands << " Rows=" << Rows << ", Cols=" << Cols << endl;

  in.read((unsigned char *)pItem, Bands*Rows*Cols*sizeof(Item));
  cout << pItem[0] << endl;
}

template <class Item>
int
Image<Item>:: ReadRawFile(const char* filename) {
  int bands,rows,cols;
  printf("Reading data from %s\n", filename);
	
  /* try to open the file */
  FILE* fp;
  if ((fp = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "%s: error opening file %s for read.\n", 
	    "ReadRawFile()", filename);
    fclose(fp);
    return(-1);
  }
  cout << "New version of ReadPnmFile()" << endl;
  //fscanf(fp, "%d %d %d\n", &bands,&rows,&cols);
	fread(&bands, sizeof(int), 1, fp);
	fread(&rows, sizeof(int), 1, fp);
	fread(&cols, sizeof(int), 1, fp);

  ResizeImage(bands, rows, cols);

  cout << "Bands=" << Bands << " Rows=" << Rows << ", Cols=" << Cols << endl;
  cout << "Item size=" << sizeof(Item) << endl;
  int bytes_read;
  bytes_read = fread(pItem, sizeof(Item), Bands*Rows*Cols, fp);
  if (bytes_read != Bands*Rows*Cols) {
    fprintf(stderr,
	    "%s: Warning: file %s not of expected size!\n", 
            "ReadRawFile()", filename);
    fprintf(stderr, 
	    "Only %d bytes (size of item=%d) are read\n", bytes_read, sizeof(Item));
	fclose(fp);
    return(-1);
  }

	fclose(fp);
  return(1);
}

template <class Item>
void
Image<Item>:: WriteRawFile(ofstream& out) const {

  cout << "Bands=" << Bands << " Rows=" << Rows << ", Cols=" << Cols << endl;

  out << Bands << " " << Rows << " " << Cols << endl;
  out.write((char *)pItem, Bands*Rows*Cols*sizeof(Item));

}

template <class Item>
int
Image<Item>:: WriteRawFile(const char* filename) const {
  
  cout << "Bands=" << Bands << " Rows=" << Rows << ", Cols=" << Cols << endl;

  printf("Writing data to %s\n", filename);
  cout << "New version of WritePnmFile()" << endl;
  cout << "Item size=" << sizeof(Item) << endl;
	
  /* try to open the file */
  FILE* fp;
  if ((fp = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "%s: error opening file %s for write.\n", 
	    "WriteRawFile()", filename);
    fclose(fp);
    return(-1);
  }
  
  //fprintf(fp, "%d %d %d\n", Bands,Rows,Cols);
	fwrite(&Bands, sizeof(int), 1, fp);
	fwrite(&Rows, sizeof(int), 1, fp);
	fwrite(&Cols, sizeof(int), 1, fp);
  if ((fwrite(pItem, sizeof(Item), Bands*Rows*Cols, fp)) != Bands*Rows*Cols) {
    fprintf(stderr, "%s: could not write all of image to %s!\n",
            "WriteRawFile", filename);
	fclose(fp);
    return(-1);
  }
  fclose(fp);
  return 1;
}
template <class Item>
int
Image<Item>:: WritePureRawFile(const char* filename) const {
  
  cout << "Bands=" << Bands << " Rows=" << Rows << ", Cols=" << Cols << endl;

  printf("Writing data to %s\n", filename);
  cout << "New version of WritePnmFile()" << endl;
  cout << "Item size=" << sizeof(Item) << endl;
	
  /* try to open the file */
  FILE* fp;
  if ((fp = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "%s: error opening file %s for write.\n", 
	    "WriteRawFile()", filename);
    fclose(fp);
    return(-1);
  }
  
  //fprintf(fp, "%d %d %d\n", Bands,Rows,Cols);
	//fwrite(&Bands, sizeof(int), 1, fp);
	//fwrite(&Rows, sizeof(int), 1, fp);
	//fwrite(&Cols, sizeof(int), 1, fp);
  if ((fwrite(pItem, sizeof(Item), Bands*Rows*Cols, fp)) != Bands*Rows*Cols) {
    fprintf(stderr, "%s: could not write all of image to %s!\n",
            "WriteRawFile", filename);
	fclose(fp);
    return(-1);
  }
  fclose(fp);
  return 1;
}
#endif /* FileIO_cpp */
