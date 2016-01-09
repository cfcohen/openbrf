/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef BRFANIMATION_H
#define BRFANIMATION_H

#include "brfToken.h"

class BrfSkeleton;

class BrfAnimationFrame
{
public:
  int index;
  std::vector<vcg::Point4f> rot;
  Matrix44f getRotationMatrix(int i) const;
  void setRotationMatrix(Matrix44f m, int i);
  Point3f tra;
  std::vector< bool > wasImplicit;

  bool Reskeletonize(const BrfSkeleton& from, const BrfSkeleton& to);
	bool CopyLowerParts(const BrfAnimationFrame& from);
	bool Mirror(const BrfAnimationFrame& from, const std::vector<int>& boneMap);

  void AddBoneHack(int copyfrom);

  BrfAnimationFrame Shuffle( std::vector<int> &map, std::vector<vcg::Point4<float> > &fallback);

};

class BrfAnimation
{
public:
  BrfAnimation();
  static int tokenIndex(){return ANIMATION;}
  char name[255];
  int nbones; // in all frames

  std::vector<BrfAnimationFrame> frame;

  bool Load(FILE*f,int verbose=0);
  bool Skip(FILE*f);
  void Save(FILE*f) const;

  void Export(const wchar_t *f);

  // to dysplay the animation...
  bool IsAnimable() const{return true;}
	bool CopyLowerParts(const BrfAnimation& from);
	bool Mirror(const BrfAnimation& from, const BrfSkeleton& s);


  static Box3f bbox;

  int Break(std::vector<BrfAnimation> &res) const;
  int Break(std::vector<BrfAnimation> &res, const wchar_t* aniFile, wchar_t *fn2) const;

  bool SaveSMD(FILE *f) const;
  bool LoadSMD(FILE *f);

  int ExtractIndexInterval(BrfAnimation&res, int a, int b);
  int RemoveIndexInterval(int a, int b);
  bool Merge(const BrfAnimation& a, const BrfAnimation& b);

  int FirstIndex() const;
  int LastIndex() const;

  bool AutoAssingTimesIfZero();
  bool Reskeletonize(const BrfSkeleton& from, const BrfSkeleton& to);

  void ShiftIndexInterval(int i);

  void GetTimings(std::vector<int> &v);
  void SetTimings(const std::vector<int> &v);

  void AddBoneHack(int copyfrom);

	void ResampleOneEvery(unsigned int nFrames);

  void Shuffle( std::vector<int> &map, std::vector<vcg::Point4<float> > &fallback);

private:
  void EnlongFrames(int nframes);

};

#endif // BRFANIMATION_H
