/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef BRFMATERIAL_H
#define BRFMATERIAL_H
#include "vcg/space/box3.h"

#include "brfToken.h"

class BrfMaterial
{
public:
  BrfMaterial();
  static int tokenIndex(){return MATERIAL;}
  char name[255];
  unsigned int flags;

  char shader[255];
  char diffuseA[255];
  char diffuseB[255];
  char bump[255];
  char enviro[255];
  char spec[255];

  float specular;
  float r,g,b;
  int RenderOrder() const;
  void SetRenderOrder(int);

  bool Load(FILE*f,int verbose=0);
  bool Skip(FILE*f);
  void Save(FILE*f) const;
  void SetDefault();
  static bool IsAnimable() { return false; }
  bool HasBump() const;
  bool HasSpec() const;
  static Box3f bbox;

	typedef enum{DIFFUSEA, DIFFUSEB, BUMP, ENVIRO, SPECULAR } TextureType;
	char* getTextureName(int i);
	const char* getTextureName(int i) const;

  typedef enum {UNKNOWN, NOWHERE, COMMON, MODULE, LOCAL} Location; // where the texture dss file is
  Location rgbLocation, bumpLocation, specLocation;
};

#endif // BRFMATERIAL_H
