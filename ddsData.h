/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef DDSDATA_H
#define DDSDATA_H

typedef struct {
  unsigned int bind;
  int sx;
  int sy;
  int mipmap;
  int filesize;
  int ddxversion;
  int location;
} DdsData;

#endif // DDSDATA_H
