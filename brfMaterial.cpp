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
  sprintf(diffuseA,name);
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


int BrfMaterial::RenderOrder() const{
  // -8..+7 encoded as 4 bits difference encoding...
  int res = ( flags >>24 ) & 15;
  if (res>7) res -= 16;
  return res;
}

void BrfMaterial::SetRenderOrder(int ro){
  assert(ro>=-8 && ro<=7);
  if (ro<0) ro+=16;
  flags&= 0xF0FFFFFF;
  flags+= ro<<24;
}

bool BrfMaterial::Load(FILE*f, int verbose){
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
