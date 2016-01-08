/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef BRFTEXTURE_H
#define BRFTEXTURE_H

#include "brfToken.h"

class BrfTexture
{
public:
  BrfTexture();

  static int tokenIndex(){return TEXTURE;}
  char name[255];
  unsigned int flags;

  bool Load(FILE*f,int verbose=0);
  void Save(FILE*f) const;
  bool IsAnimable() const;
  static Box3f bbox;
  char* FrameName(int i) const;

  void SetDefault();
  int NFrames() const;

};

#endif // BRFTEXTURE_H
