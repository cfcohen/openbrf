/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <vector>
#include <stdio.h>
#include <vcg/space/point4.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>
using namespace vcg;
#include "brfShader.h"

#include "saveLoad.h"

BrfShaderOpt::BrfShaderOpt()
{
  map=colorOp=alphaOp=flags=0;
}


BrfShader::BrfShader()
{
}
//FILE *fff=wfopen("prova.txt","wt");

bool BrfShaderOpt::Load(FILE*f, int /*verbose*/){
  //map, colorOp, alphaOp, flags
  LoadInt(f,map);
  LoadUint(f,colorOp);
  LoadUint(f,alphaOp);
  LoadUint(f,flags);
  //fprintf(fff,"%d %u %u %u\n",map,colorOp, alphaOp, flags);
  return true;
}

void BrfShaderOpt::Save(FILE*f) const{
  //map, colorOp, alphaOp, flags
  SaveInt(f,map);
  SaveUint(f,colorOp);
  SaveUint(f,alphaOp);
  SaveUint(f,flags);
}

void BrfShader::SetDefault(){
  requires = 0;
  sprintf(technique,"%s",name);
  fallback[0]=0;
  flags = 0;
  BrfShaderOpt o;
  o.map = 0;
  o.flags = 0;
  o.alphaOp = 1;
  o.colorOp = 2;
  opt.clear();
  opt.push_back(o);
}

bool BrfShader::Skip(FILE*f){
  if (!LoadString(f, name)) return false;
  ::Skip(f,8);
  if (!LoadString(f, technique)) return false;

  unsigned int k;
  LoadUint(f , k);
  assert(k<=1);
  if (k) { if (!LoadString(f , fallback)) return false;}
  else fallback[0]=0;

  SkipVectorF<BrfShaderOpt>(f);
  return true;
}

bool BrfShader::Load(FILE*f, int verbose){
  if (!LoadString(f, name)) return false;
  if (verbose>0) printf("loading \"%s\"...\n",name);
  LoadUint(f , flags);
  LoadUint(f , requires);
  if (!LoadString(f, technique)) return false;

  unsigned int k;
  LoadUint(f , k);
  assert(k<=1);
  if (k) { if (!LoadString(f , fallback)) return false; }
  else fallback[0]=0;

  //fprintf(fff,"--%s--\n",technique);
  if (!LoadVector(f,opt)) return false;
  return true;
}

void BrfShader::Save(FILE*f) const{
  SaveString(f, name);
  SaveUint(f , flags);
  SaveUint(f , requires);
  SaveString(f, technique);
  if (fallback[0]==0) SaveUint(f,0);
  else {
    SaveUint(f,1);
    SaveString(f, fallback);
  }
  SaveVector(f,opt);
}
