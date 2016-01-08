/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <vector>
#include <vcg/space/box3.h>
#include <vcg/space/point4.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>
using namespace vcg;
#include "saveLoad.h"

#include "brfTexture.h"

vcg::Box3f BrfTexture::bbox;

BrfTexture::BrfTexture()
{
  bbox.SetNull();
}

bool BrfTexture::Load(FILE*f, int verbose){
  if (!LoadString(f, name)) return false;
  if (verbose>0) printf("loading \"%s\"...\n",name);
  LoadUint(f , flags);
  return true;
}

void BrfTexture::Save(FILE* f) const{
  SaveString(f, name);
  SaveUint(f , flags);
}

void BrfTexture::SetDefault(){
  sprintf(name, "%s.dds" ,name);
  flags=0x00000000;
}

int BrfTexture::NFrames() const{
  return ( (flags>>24) & 0xF ) * 4;
}

bool BrfTexture::IsAnimable() const {
  return NFrames()>0;
}

char* BrfTexture::FrameName(int i) const{
  static char res[1024];
  sprintf(res,"%s_%d.dds",name,i);
  return res;
}
