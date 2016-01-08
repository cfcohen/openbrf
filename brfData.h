/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef BRF_DATA
#define BRF_DATA

#include <vector>
#include <stdio.h>

#include <vcg/space/box3.h>
#include <vcg/space/point4.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>
#include <vcg/math/matrix44.h>

using namespace std;
using namespace vcg;

#include "brfToken.h"
#include "brfMesh.h"
#include "brfTexture.h"
#include "brfShader.h"
#include "brfMaterial.h"
#include "brfSkeleton.h"
#include "brfAnimation.h"
#include "brfBody.h"

#include <vector>
using namespace std;

class BrfHitBoxSet;

class BrfData {
public:
	BrfData();
	BrfData(const wchar_t*f,int verbose=0);
	BrfData(FILE*f,int verbose=0);
	vector<BrfMesh> mesh;
	vector<BrfTexture> texture;
	vector<BrfShader> shader;
	vector<BrfMaterial> material;
	vector<BrfSkeleton> skeleton;
	vector<BrfAnimation> animation;
	vector<BrfBody> body;
	bool Load(FILE*f,int verbose=0, int imposeVers = -1);
	bool Load(const wchar_t*filename,int verbose=1, int imposeVers = -1);

	int LoadHitBoxesFromXml(const wchar_t*filename); // loads all hitboxes as collision meshes form skeleton_data.xml
	int SaveHitBoxesToXml(const wchar_t *fin, const wchar_t *fout);
	static char* LastHitBoxesLoadSaveError(const char* st=NULL,const wchar_t* subst1=NULL,const char* subst2=NULL,const char* subst3=NULL); // sets or reads the error

	bool LoadFast(const wchar_t*filename, bool ultrafast); // skips most data
	bool LoadMat(FILE *f);
	void Clear();
	int FirstToken() const;

	bool Save(const wchar_t* f) const;
	bool Save(FILE* f) const;
	void  Merge(const BrfData& b);
	const char* GetName(int i, int token) const;

	int GetFirstUnusedLetter() const; // return first unused alphabet letter in meshes
	BrfMesh GetCompleteSkin(int i) const; // returns a mesh composed of all skin pieces
	const char* GetFirstObjectName() const; // returns name of first object
	const char* GetAllObjectNames() const;
	const char* GetAllObjectNamesAsSceneProps(int *nFound, int *nFoundWithB) const;

	int getOneSkeleton(int nbones, int after);
	int Find(const char* name, int token) const;

	BrfBody* FindBody(const char* name);
	BrfMesh* FindMesh(const char* name);

	int FindTextureWithExt(const char* name);
	bool HasAnyTangentDirs() const;
	void ForgetTextureLocations();
	//const vector<ObjCoord>& GetUsedBy(int i, int token) const;

	unsigned int size(int token) const;
	unsigned int totSize() const;

	template<class BrfType> vector<BrfType>& Vector();

	//bool AddExtraSkelData(const BrfHitBoxSet &s);

	bool IsOneSkelOneHitbox() const;

	void UpdateMetadata();

	int version;
private:
	//template<class BrfType> bool LoadAll(FILE *f, vector<BrfType> &v, int k);
	template<class BrfType> void SaveAll(FILE *f, const vector<BrfType> &v) const;
	int lastLoaded;
	void LoadVersion(FILE *f,int imposeVers);

};

template<class BrfType> static vector<BrfType     > & VectorOf(BrfData& d);
template<> vector<BrfMesh     > & VectorOf<BrfMesh     >(BrfData& d){ return d.mesh;      }
template<> vector<BrfTexture  > & VectorOf<BrfTexture  >(BrfData& d){ return d.texture;   }
template<> vector<BrfShader   > & VectorOf<BrfShader   >(BrfData& d){ return d.shader;    }
template<> vector<BrfMaterial > & VectorOf<BrfMaterial >(BrfData& d){ return d.material;  }
template<> vector<BrfSkeleton > & VectorOf<BrfSkeleton >(BrfData& d){ return d.skeleton;  }
template<> vector<BrfAnimation> & VectorOf<BrfAnimation>(BrfData& d){ return d.animation; }
template<> vector<BrfBody     > & VectorOf<BrfBody     >(BrfData& d){ return d.body;      }

#endif
