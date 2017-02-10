#include "StdAfx.h"
#include "derivify.h"

istream& operator>>(istream& is, surreal& x)
{
  /***
   *  Straight from Stroustrup's (third edition) complex version.
   *  Much confusion about the ipfx, isfx functions used in
   *  the GNU library and the sentry type that is supposed to
   *  call them indirectly.  Stroustrup implies that this
   *  works fine when streams are tied and other fancy stuff.
   ***/

  double re, im = 0;
  char c = 0;
 
  is >> c;
  if (c == '(') {
    is >> re >> c;
    if(c == ',') is >> im >> c;
    if(c != ')') is.clear(ios::badbit);
  }
  else {
    is.putback(c);
    is >> re;
  }
 
  if (is) x = surreal (re, im);
  return is;
}

ostream& operator<<(ostream& os, const surreal& z)
{
  return os << '(' << Real (z) << ',' << Imag (z) << ')';
}


