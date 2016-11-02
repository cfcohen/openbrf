/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef BRFSKELETON_H
#define BRFSKELETON_H

#include "brfToken.h"
#include <vcg/space/box3.h>
#include <vector>

namespace vcg{
  template<typename T> class Matrix44;
  template<typename T> class Point4;
}


class BrfBone
{
public:
  BrfBone();
  BrfBone(FILE *f, int verbose=0){
    Load(f,verbose);
  }
  char name[255];


  vcg::Point3f x,y,z,t;

  // 56 bytes of data.... 3x4 matrix, 12 floats + int + int?
  int attach,b;

  bool Load(FILE*f,int verbose=0);
  void Save(FILE*f) const;
  void Export(FILE*f);
  static bool Skip(FILE*f);

  //void Adjust(); // adjust to new axis ordering...

  vcg::Matrix44<float> getRotationMatrix() const;
  void  setRotationMatrix(vcg::Matrix44<float>);

  std::vector<int> next;
};

class BrfAnimationFrame;
class BrfMesh;
class BrfBody;

class BrfSkeleton
{      
public:
  BrfSkeleton();


  static int tokenIndex(){return SKELETON;}
  char name[255];
  unsigned int flags;

  bool Load(FILE*f,int verbose=0);
  void Save(FILE*f) const;
  bool Skip(FILE*f);

  std::vector<BrfBone> bone;
  int root;
  void BuildTree();
  bool IsAnimable() const{return false;}
  vcg::Box3f bbox;

  bool SaveSMD(FILE *f) const;
  bool LoadSMD(FILE *f);
  std::vector<vcg::Matrix44<float> >  GetBoneMatrices(const BrfAnimationFrame &fr) const;
  std::vector<vcg::Matrix44<float> >  GetBoneMatrices() const;
  std::vector<vcg::Matrix44<float> >  GetBoneMatricesInverse() const;

  void BuildDefaultMesh(BrfMesh & output) const; // builds a mesh with just an octa x bone...
  vcg::Matrix44<float> getRotationMatrix(int i) const {return bone[i].getRotationMatrix();}
  void setRotationMatrix(vcg::Matrix44<float> m,int i) {bone[i].setRotationMatrix(m);}

  // for translations and points:
  static vcg::Point3f adjustCoordSyst(vcg::Point3f);

  // for rotations (full):
  static vcg::Point4f   adjustCoordSyst(vcg::Point4f); // for quaternions
  static BrfBone        adjustCoordSyst(BrfBone);
  static vcg::Matrix44f adjustCoordSyst(vcg::Matrix44f);

  // for rotations (half):
  static vcg::Point4f   adjustCoordSystHalf(vcg::Point4f);
  static      BrfBone   adjustCoordSystHalf(BrfBone);
  static vcg::Matrix44f adjustCoordSystHalf(vcg::Matrix44f);

  static float BoneSizeX();
  static float BoneSizeY();
  static float BoneSizeZ();
  int FindBoneByName(const char * name) const;
  int FindSpecularBoneOf(int bonei) const; /* judges by the name */

  std::vector<int> Bone2BoneMap(const BrfSkeleton & s) const;
  std::vector<vcg::Point4<float> > BoneRotations() const;

  // layout hitboxes according to bones position (or does the inverse)
  bool LayoutHitboxes(const BrfBody &in, BrfBody &out, bool inverse) const;

  void Scale( float f );
private:

  void Export(const wchar_t *f); // for intrnal use
  void SetBoneMatrices(const BrfAnimationFrame &fr, int boneIndex,
                       std::vector<vcg::Matrix44<float> > &boneMatrV, const vcg::Matrix44<float>  &curr) const;
  void SetBoneMatrices(int boneIndex,
                       std::vector<vcg::Matrix44<float> > &boneMatrV, const vcg::Matrix44<float>  &curr) const;
  void SetBoneMatricesInverse(int boneIndex,
                       std::vector<vcg::Matrix44<float> > &boneMatrV, const vcg::Matrix44<float>  &curr) const;
};

#endif // BRFSKELETON_H
