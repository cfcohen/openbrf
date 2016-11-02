/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include "brfData.h"

#include "saveLoad.h"
#include "platform.h"

 const char * tokenBrfName[N_TOKEN] = {
  "mesh",
  "texture",
  "shader",
  "material",
  "skeleton",
  "skeleton_anim",
  "body",
};

const char* BrfData::GetFirstObjectName() const{
  if (mesh.size()!=0) return mesh[0].name;
  if (texture.size()!=0) return texture[0].name;
  if (shader.size()!=0) return shader[0].name;
  if (material.size()!=0) return material[0].name;
  if (skeleton.size()!=0) return skeleton[0].name;
  if (animation.size()!=0) return animation[0].name;
  if (body.size()!=0) return body[0].name;
  return NULL;
}

template <class BrfType> void __AppendAll( const std::vector<BrfType> v, std::string& c){
	for (unsigned int i=0; i<v.size(); i++ ) {
		if (!c.empty()) c.append(", ");
		c.append(v[i].name);
	}
}

const char* BrfData::GetAllObjectNames() const{
  static std::string c;
  c.clear();
  __AppendAll(mesh,c);
  __AppendAll(texture,c);
  __AppendAll(shader,c);
  __AppendAll(material,c);
  __AppendAll(skeleton,c);
  __AppendAll(animation,c);
  __AppendAll(body,c);
  return c.c_str();
}

const char* BrfData::GetAllObjectNamesAsSceneProps(int *nFound, int *nFoundWithB) const{

	static std::string c;
	c.clear();
	char lastName[1024]; lastName[0]=0;
	*nFound = 0;
	*nFoundWithB = 0;
	for (unsigned int i=0; i<mesh.size(); i++) {
		char bodyName[1024];
		char baseName[1024];
		sprintf(baseName,"\"%s\"",mesh[i].baseName);
		char fullstr[1024*4];

		if (strcmp(lastName,mesh[i].baseName)==0) continue;

		sprintf(bodyName, "bo_%s",mesh[i].baseName );
		bool hasBody = (Find(bodyName,BODY)!=-1);
		sprintf(fullstr,"\t( %-45s,0,%s,\"%s\",[]),\n",
			baseName,baseName,( (hasBody)?bodyName:"0")
		);
		(*nFound)++;
		if (hasBody) (*nFoundWithB)++;
		c.append(fullstr);

		sprintf(lastName,"%s",mesh[i].baseName);

	}
	if (!*nFound) return NULL;

	return c.c_str();
}

bool BrfData::IsOneSkelOneHitbox() const{
  return 1
    && (skeleton.size()==1)
    && (body.size()==1)
    && (totSize() == 2 )
    && (body[0].part.size()==skeleton[0].bone.size())
    && (0==strcmp(body[0].name,skeleton[0].name))
  ;
}

BrfData::BrfData(){
  version = 0;
  //globalVersion = 0;
}

BrfData::BrfData(FILE*f,int verbose){
  Load(f,verbose);
}
BrfData::BrfData(const wchar_t*f,int verbose){
  Load(f,verbose);
}

int BrfData::GetFirstUnusedLetter() const{
  vector<bool> used(255,false);

  for (unsigned int i=0; i<mesh.size(); i++) {
    char let = mesh[i].name[4];
    used[let]=true;
  }
  for (char i='A'; i<'Z'; i++) if (!used[i]) return i-'A';
  return -1;
}

BrfMesh BrfData::GetCompleteSkin(int pos) const{
  BrfMesh res;
  bool first = true;
  for (unsigned int i=0; i<mesh.size(); i++) {
    char let = mesh[i].name[4];
    if (let==char(pos+'A')) {
      BrfMesh a = mesh[i];
      a.KeepOnlyFrame(0);
      if (first) res=a; else res.Merge(a);
      first=false;
    }
  }
  return res;
}

template <class T>
static void mergeVec(vector<T> &a, const vector<T> &b){
  for (unsigned int i=0; i<b.size(); i++) a.push_back(b[i]);
}

template <class T>
static int myfind(const vector<T> &b, const char* name) {
  for (unsigned int i=0; i<b.size(); i++) if (strcmp(b[i].name,name)==0) return i;
  return -1;
}


unsigned int BrfData::totSize() const{
  return mesh.size() +
         material.size() +
         shader.size() +
         texture.size() +
         body.size() +
         skeleton.size() +
         animation.size();
}

unsigned int BrfData::size(int token) const{

  switch (token) {
    case MESH: return mesh.size();
    case MATERIAL: return material.size();
    case SHADER: return shader.size();
    case TEXTURE: return texture.size();
    case BODY: return body.size();
    case SKELETON: return skeleton.size();
    case ANIMATION: return animation.size();
  }
  return 0;

}

int BrfData::FindTextureWithExt(const char* name){
  int k = myfind(texture,name);
  if (k>=0) return k;
  char full[1024];
  sprintf(full,"%s.dds",name);
  return myfind(texture, full);
}

bool BrfData::HasAnyTangentDirs() const{
  for (unsigned int i=0; i<mesh.size(); i++) {
    if (mesh[i].StoresTangentField()) return true;
  }
  return false;
}

int BrfData::Find(const char* name, int token) const{
  switch (token) {
    case MESH: return myfind(mesh,name);
    case MATERIAL: return myfind(material,name);
    case SHADER: return myfind(shader,name);
    case TEXTURE: return myfind(texture,name);
    case BODY: return myfind(body,name);
    case SKELETON: return myfind(skeleton,name);
    case ANIMATION: return myfind(animation,name);
  }
  return -1;
}

BrfBody* BrfData::FindBody(const char* name){
  int i= myfind(body,name); if (i<0) return NULL; else return &(body[i]);
}
BrfMesh* BrfData::FindMesh(const char* name){
  int i= myfind(mesh,name); if (i<0) return NULL; else return &(mesh[i]);
}

void  BrfData::Merge(const BrfData& b){
  mergeVec(mesh, b.mesh);
  mergeVec(texture, b.texture);
  mergeVec(shader, b.shader);
  mergeVec(material, b.material);
  mergeVec(skeleton, b.skeleton);
  mergeVec(animation, b.animation);
  mergeVec(body, b.body);
}


bool BrfData::Load(const wchar_t *filename,int verbose, int imposeVersion){
  FILE *f = wfopen(filename,"rb");
  if (!f) return false;
  return Load(f, verbose, imposeVersion);
}



template<class BrfType> void BrfData::SaveAll(FILE *f, const vector<BrfType> &v) const{
  if (v.size()==0) return;
  SaveString(f,tokenBrfName[ BrfType::tokenIndex() ]);
  SaveInt(f, v.size());
  for (unsigned int i=0; i<v.size(); i++) {
     v[i].Save(f);
  }
}

int BrfData::getOneSkeleton(int nbones, int after){

  for (unsigned int i=0; i<skeleton.size(); i++){
    int j = (i+after)%skeleton.size();
    if ((int)skeleton[j].bone.size()>=nbones) { return j; }
  }
  return -1;
}

int globVersion;

bool BrfData::Save(FILE *f) const{
	globVersion = version;
	if (globVersion==1) { SaveString(f, "rfver "); SaveInt(f,1); }
  SaveAll(f,texture);
  SaveAll(f,shader);
  SaveAll(f,material);
  SaveAll(f,mesh);
  SaveAll(f,skeleton);
  SaveAll(f,animation);
  SaveAll(f,body);

  SaveString(f,"end");
  return true;
}

bool BrfData::Save(const wchar_t*fn) const{

  FILE *f = wfopen(fn,"wb");
  if (!f) return false;

  Save(f);

  fclose(f);

  return true;
}


const char* BrfData::GetName(int i, int token) const{
  switch (token) {
    case MESH: return mesh[i].name;
    case MATERIAL: return material[i].name;
    case SHADER: return shader[i].name;
    case TEXTURE: return texture[i].name;
    case BODY: return body[i].name;
    case SKELETON: return skeleton[i].name;
    case ANIMATION: return animation[i].name;
  }
  assert(0);
  return NULL;
}

int BrfData::FirstToken() const{
  if(mesh.size()) return MESH;
  if(material.size()) return MATERIAL;
  if(shader.size()) return SHADER;
  if(texture.size()) return TEXTURE;
  if(body.size()) return BODY;
  if(skeleton.size()) return SKELETON;
  if(animation.size()) return ANIMATION;
  return -1;
}

void BrfData::ForgetTextureLocations(){
  for (unsigned int i=0; i<material.size(); i++) {
    material[i].rgbLocation=BrfMaterial::UNKNOWN;
    material[i].bumpLocation=BrfMaterial::UNKNOWN;
    material[i].specLocation=BrfMaterial::UNKNOWN;
  }
}

void BrfData::Clear(){
  mesh.clear();
  texture.clear();
  shader.clear();
  material.clear();
  skeleton.clear();
  animation.clear();
  body.clear();
}

void  BrfData::LoadVersion(FILE*f,int imposeVers){
  LoadInt(f,version);
  if (imposeVers>-1) version = imposeVers;
  globVersion=version;

}


bool BrfData::Load(FILE*f,int verbose,int imposeVers){

  Clear();

  version = 0;
  globVersion = version;
  while (1) {
    char str[255];
    if (!LoadString(f, str)) return false;
    if (verbose>1) printf("Read \"%s\"\n",str);
    if (!strcmp(str,"end")) break;
    else if (!strcmp(str,"rfver ")) LoadVersion(f,imposeVers);
    else if (!strcmp(str,"mesh"))  {
      if (!LoadVector(f,mesh)) return false;
    //  int k; LoadInt(f,k); mesh.resize(1); mesh[0].Load(f); return true;
    }
    else if (!strcmp(str,"texture")) {if (!LoadVector(f,texture)) return false;}
    else if (!strcmp(str,"shader")) {if (!LoadVector(f,shader)) return false;}
    else if (!strcmp(str,"material")) {if (!LoadVector(f,material)) return false;}
    else if (!strcmp(str,"skeleton")) {if (!LoadVector(f,skeleton)) return false;}
    else if (!strcmp(str,"skeleton_anim")) {if (!LoadVector(f,animation)) return false;}
    else if (!strcmp(str,"body")) {if (!LoadVector(f,body)) return false; }
    else {
      //printf("ERROR! Unknown token \"%s\"\n",str);
      fflush(stdout);
      fclose(f);
      return false;
    }

  }
  fclose(f);
  return true;
}


bool BrfData::LoadFast(const wchar_t*filename, bool faster){
  FILE *f = wfopen(filename,"rb");
  if (!f) return false;

  version = 0;
  globVersion = version;

  while (1) {
    char str[255];
    if (!LoadString(f, str)) return false;
    if (!strcmp(str,"end")) break;
    else if (!strcmp(str,"rfver ")) LoadVersion(f,-1);
    else if (!strcmp(str,"shader")) {if (!SkipVector(f,shader)) return false;}
    else if (!strcmp(str,"texture")) {if (!LoadVector(f,texture)) return false; }
    else if (!strcmp(str,"material")) {if (!SkipVector(f,material)) return false; }
    else if (!strcmp(str,"mesh")) { if (faster) break; if (!SkipVector(f,mesh)) return false;}
    else if (!strcmp(str,"skeleton")) {if (!SkipVector(f,skeleton)) return false;}
    else if (!strcmp(str,"skeleton_anim")) { if (faster) break; if (!SkipVector(f,animation)) return false;}
    else if (!strcmp(str,"body")) { if (faster) break; if (!SkipVector(f,body)) return false;}
    else {
      //printf("ERROR! Unknown token \"%s\"\n",str);
      fflush(stdout);
      fclose(f);
      return false;
    }

  }
  fclose(f);
  return true;
}
