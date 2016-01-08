/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef VCGMESH_H
#define VCGMESH_H

class BrfMesh;
class BrfSkeleton;

class VcgMesh
{
public:
  VcgMesh();

  // class to wrap vcg meshes...
  static void clear();
  static void add(const BrfMesh&, int i);
  static void add(const BrfSkeleton&);
  static bool save(char* filename);
  static bool load(char* filename);
  static BrfMesh toBrfMesh();
  static bool modifyBrfSkeleton(BrfSkeleton &s);
  static void moveBoneInSkelMesh(int nb, Point3f d);

  static bool gotColor();
  static bool gotTexture();
  static bool gotNormals();
  static bool gotMaterialName();

  static int simplify(int percfaces);

  static const char *lastErrString();
};

#endif // VCGMESH_H
