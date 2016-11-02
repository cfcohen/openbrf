/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <vector>
#include <vcg/space/point4.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>
using namespace vcg;
#include "saveLoad.h"

#include "brfMaterial.h"


BrfMaterial::BrfMaterial()
{
  bbox.SetNull();
  specLocation = bumpLocation = rgbLocation= UNKNOWN;

}

void BrfMaterial::SetDefault(){
  flags = 0;
  //renderOrder = 0;
  sprintf(shader,"simple_shader");
  sprintf(diffuseA,"%s",name);
  sprintf(diffuseB,"none");
  sprintf(bump,"none");
  sprintf(enviro,"none");
  sprintf(spec,"none");
  specular = 0;
  r=g=b=1;
}
bool BrfMaterial::HasBump() const {
  return (strcmp(bump,"none")!=0);
}
bool BrfMaterial::HasSpec() const {
  return (strcmp(spec,"none")!=0);
}

const char* BrfMaterial::getTextureName(int i) const{
	switch (i){
	default:
	case DIFFUSEA: return diffuseA;
	case DIFFUSEB: return diffuseB;
	case BUMP: return bump;
	case ENVIRO: return enviro;
	case SPECULAR: return spec;
	}
}
char* BrfMaterial::getTextureName(int i){
	switch (i){
	default:
	case DIFFUSEA: return diffuseA;
	case DIFFUSEB: return diffuseB;
	case BUMP: return bump;
	case ENVIRO: return enviro;
	case SPECULAR: return spec;
	}
}

bool BrfMaterial::Skip(FILE*f
                       ){
  return Load(f);
  if (!LoadString(f, name)) return false;
  ::Skip<int>(f);

  if (!LoadString(f, shader)) return false;
  if (!LoadString(f, diffuseA)) return false;
  if (!LoadString(f, diffuseB)) return false;
  if (!LoadString(f, bump)) return false;
  if (!LoadString(f, enviro)) return false;
  LoadStringMaybe(f, spec,"none");
  ::Skip(f,16);

  return true;
}


bool BrfMaterial::FlagUniformLighting() const{
    return (flags & (1<<6))!=0;
}

bool BrfMaterial::FlagNoZWrite() const{
    return (flags & (1<<3))!=0;
}

bool BrfMaterial::FlagNoDepthTest() const{
    return (flags & (1<<4))!=0;
}

bool BrfMaterial::FlagAutoNormalize() const{
    return (flags & (1<<11))!=0;
}

bool BrfMaterial::FlagBlend() const{
    return ( ((flags & (7<<8))!=0) && ((flags & (7<<8))!=(7<<8)) ) ; // 7 = "auto mode" -- unclear, consider it NOT blend
}

bool BrfMaterial::FlagAlphaTest() const{
    return (flags & (3<<12))!=0;
}

int  BrfMaterial::FlagRenderOrder() const{
    if (flags & (1<<16)) return -9; // render 1st
    else {
        // -8..+7 encoded as 4 bits difference encoding...
        int res = ( flags >>24 ) & 15;
        if (res>7) res -= 16;
        return res;
        return (int)((flags >>24)&15);
    }
}

float BrfMaterial::FlagAlphaValue() const{
    switch ((flags>>12)&3) {
    default: return 0;
    case  1: return 8/256.0f;
    case  2: return 136/256.0f;
    case  3: return 251/256.0f;
    }
}

#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303

int  BrfMaterial::FlagBlendFuncSrc() const{
    switch ((flags>>8)&7) {
    default:
    case 0: return GL_ONE;
    case 7:
    case 1: return GL_SRC_ALPHA;
    case 2: return GL_SRC_ALPHA;
    case 3: return GL_ZERO;
    case 4: return GL_ONE;
    }
}

int  BrfMaterial::FlagBlendFuncDst() const{
    switch ((flags>>8)&7) {
    default:
    case 0: return GL_ZERO;
    case 7:
    case 1: return GL_ONE_MINUS_SRC_ALPHA;
    case 2: return GL_ONE;
    case 3: return GL_SRC_COLOR;
    case 4: return GL_ONE;
    }
}


void BrfMaterial::SetRenderOrder(int ro){
  assert(ro>=-8 && ro<=7);
  if (ro<0) ro+=16;
  flags&= 0xF0FFFFFF;
  flags+= ro<<24;
}

bool BrfMaterial::Load(FILE*f, int /*verbose*/){
  if (!LoadString(f, name)) return false;
  //if (verbose>0) printf("loading \"%s\"...\n",name);
  LoadUint(f , flags);

  if (!LoadString(f, shader)) return false;
  if (!LoadString(f, diffuseA)) return false;
  if (!LoadString(f, diffuseB)) return false;
  if (!LoadString(f, bump)) return false;
  if (!LoadString(f, enviro)) return false;
  LoadStringMaybe(f, spec,"none");
  LoadFloat(f,specular);
  LoadFloat(f,r);
  LoadFloat(f,g);
  LoadFloat(f,b);
  return true;
}

void BrfMaterial::Save(FILE*f) const{
  SaveString(f, name);
  SaveUint(f , flags);
  SaveString(f, shader);
  SaveString(f, diffuseA);
  SaveString(f, diffuseB);
  SaveString(f, bump);
  SaveString(f, enviro);
  SaveStringNotempty(f, spec, "none");
  SaveFloat(f,specular);
  SaveFloat(f,r);
  SaveFloat(f,g);
  SaveFloat(f,b);
}

vcg::Box3f BrfMaterial::bbox;
