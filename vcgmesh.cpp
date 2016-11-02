/* OpenBRF -- by marco tarini. Provided under GNU General Public License */



// everything VCG mesh related is implemented here

#include <QGLWidget>

#include <vector>
//#include <vcg/simplex/vertex/base.h>
//#include <vcg/simplex/face/base.h>
//#include <vcg/simplex/edge/base.h>
#include <vcg/complex/complex.h>
//#include <vcg/complex/trimesh/clean.h>


#include "vcgExport.h"
#include "vcgImport.h"
//#include <wrap/io_trimesh/import.h>
//#include <wrap/io_trimesh/export.h>




// simplification
#include <vcg/math/quadric.h>
//#include <vcg/complex/trimesh/clean.h>
#include <vcg/complex/algorithms/update/topology.h>
#include <vcg/complex/algorithms/local_optimization.h>
#include <vcg/complex/algorithms/local_optimization/tri_edge_collapse.h>
#include <vcg/complex/algorithms/local_optimization/tri_edge_collapse_quadric.h>




using namespace vcg;
using namespace std;

#include "vcgmesh.h"

class CEdge;    // dummy prototype never used
class CFace;
class CVertex;

struct MyUsedTypes : public UsedTypes<	Use<CVertex>		::AsVertexType,
                                        Use<CFace>			::AsFaceType,
                                        Use<CEdge>::AsEdgeType  >{};


class CVertex : public Vertex<
    MyUsedTypes,
    vertex::BitFlags,
    vertex::Coord3f,
    vertex::VFAdj,
    vertex::Normal3f,
    vertex::Color4b,
    vertex::TexCoord2f,
    vertex::Mark>
{
public:
  vcg::math::Quadric<double> &Qd() {return q;}
private:
  math::Quadric<double> q;
};
class CFace   : public Face<

   MyUsedTypes,
   vcg::face::VFAdj,
   face::VertexRef,
   face::BitFlags, //face::WedgeTexCoord2f,
   //face::Normal3f, // just for obj importing
   vcg::face::Normal3f, vcg::face::BitFlags, vcg::face::FFAdj,
   vcg::face::Qualityf,
   face::WedgeTexCoord2f> {};
class CMesh   : public vcg::tri::TriMesh< vector<CVertex>, vector<CFace> > {
public:
  void setFace(int i, int vi,int vj,int vk){
    face[i].V(0) = &(vert[vi]);
    face[i].V(1) = &(vert[vj]);
    face[i].V(2) = &(vert[vk]);
  }
};

class CEdge : public Edge<MyUsedTypes,edge::VertexRef> {
public:
  inline CEdge() {};
  inline CEdge( CVertex * v0, CVertex * v1){V(0) = v0; V(1) = v1; };
    static inline CEdge OrderedEdge(CVertex* v0,CVertex* v1){
   if(v0<v1) return CEdge(v0,v1);
   else return CEdge(v1,v0);
  }
};


// MAKE QUAD DOMINANT FOR BrfBodyParts
#include "brfBody.h"
//#include <vcg/complex/trimesh/update/normal.h>
//#include <vcg/complex/trimesh/update/topology.h>
#include <vcg/complex/algorithms/bitquad_creation.h>

static bool noDegenerate(const std::vector<int> &v){
  for (unsigned int i=2; i<v.size(); i++){
    if (v[i]==v[0]) return false;
    if (v[i]==v[i-1]) return false;
    if (v[i-1]==v[0]) return false;
  }
  return true;
}

void BodyPart2Mesh(const BrfBodyPart &bp, CMesh &m ){
  int nt = 0; // number of triangles
  for (uint i=0; i<bp.face.size(); i++){
    const std::vector<int> &f (bp.face[i]);
    if (noDegenerate(f))
    nt+=bp.face[i].size()-2;
  }
  vcg::tri::Allocator<CMesh>::AddVertices( m , bp.pos.size() );
  vcg::tri::Allocator<CMesh>::AddFaces( m , nt );
  for (uint i=0; i<bp.pos.size(); i++){
    m.vert[i].P() = bp.pos[i];
  }

  // build faces, splitting polys in tris
  nt = 0;
  for (uint i=0; i<bp.face.size(); i++){
    const std::vector<int> &f (bp.face[i]);
    if (noDegenerate(f))
    for (uint j=2; j<f.size(); j++){
      assert(f[0]!=f[1]);
      assert(f[1]!=f[2]);
      assert(f[2]!=f[0]);
      m.face[nt].V(0) = (&(m.vert[0]))+f[0];
      m.face[nt].V(1) = (&(m.vert[0]))+f[j-1];
      m.face[nt].V(2) = (&(m.vert[0]))+f[j];
      nt++;
    }
  }
  vcg::tri::UpdateNormal<CMesh>::PerFaceNormalized(m);
  vcg::tri::UpdateTopology<CMesh>::FaceFace(m);

}

void Mesh2BodyPart(CMesh  &m, BrfBodyPart &bp  ){
  bp.pos.resize(m.vert.size());
  for (uint i=0; i<bp.pos.size(); i++){
     bp.pos[i]=m.vert[i].P();
  }
  bp.face.resize(0);
  for (uint i=0; i<m.face.size(); i++) m.face[i].ClearV();


  for (uint i=0; i<m.face.size(); i++) {

    CFace &f(m.face[i]);
    CVertex* v0 = &(m.vert[0]);
    if (!f.IsV()) {
      for (int w=0; w<3; w++) {
        if (f.IsF(w)) { // faux edge
          // add a quad
          std::vector<int> v(0);
          v.push_back(f.V0(w)-v0);
          v.push_back(f.FFp(w)->V2(f.FFi(w))-v0); // opposite vertex
          v.push_back(f.V1(w)-v0);
          v.push_back(f.V2(w)-v0);
          f.SetV();
          f.FFp(w)->SetV();
          bp.face.push_back(v);
        }
      }
      if (!f.IsV()){
        //add a tri
        std::vector<int> v(0);
        v.push_back(f.V(0)-v0);
        v.push_back(f.V(1)-v0);
        v.push_back(f.V(2)-v0);
        f.SetV();
        bp.face.push_back(v);
      }
    }
  }
}

void BrfBodyPart::MakeQuadDominant(){
  CMesh m;
  BodyPart2Mesh(*this,m);
  vcg::tri::BitQuadCreation<CMesh>::MakeDominant(m,2);
  vcg::tri::BitQuadCreation<CMesh>::SplitNonFlatQuads(m,9 );
  Mesh2BodyPart(m,*this);
}

CMesh mesh;
typedef vcg::tri::BasicVertexPair<CVertex> MyVertexPair;


class MyTriEdgeCollapse: public vcg::tri::TriEdgeCollapseQuadric< CMesh, MyVertexPair, MyTriEdgeCollapse, tri::QInfoStandard<CVertex>  > {
            public:
typedef  vcg::tri::TriEdgeCollapseQuadric< CMesh,  MyVertexPair, MyTriEdgeCollapse, tri::QInfoStandard<CVertex>  > TECQ;
            typedef  CMesh::VertexType::EdgeType EdgeType;
            inline MyTriEdgeCollapse(  const MyVertexPair &p, int i, BaseParameterClass *pp) :TECQ(p,i,pp){}
};

int VcgMesh::simplify(int percFaces){
  int origFn = mesh.fn;
  tri::TriEdgeCollapseQuadricParameter qparams;// = MyTriEdgeCollapse::QParameter();//Params() ;
  //MyTriEdgeCollapse::SetDefaultParams();
  qparams.QualityThr  =.3;
  float TargetError=numeric_limits<float>::max();
  //MyTriEdgeCollapse::Params().SafeHeapUpdate=true;
  //qparams.SafeHeapUpdate = true;
  qparams.QualityCheck	= true;
  qparams.NormalCheck	= true;
  qparams.OptimalPlacement	= false;
  qparams.ScaleIndependent	= true;
  qparams.PreserveBoundary	= false;
  qparams.PreserveTopology	= false;
  //qparams.PreserveBoundaryMild	= false ;
  qparams.BoundaryWeight  = 500;


  //qparams.QualityThr	= atof(argv[i]+2);
  //qparams.NormalThrRad = math::ToRad(atof(argv[i]+2));
  //qparams.BoundaryWeight  = atof(argv[i]+2);
  //TargetError = float(atof(argv[i]+2));
  //CleaningFlag=true;


  /*
  if(Cleaning){
      int dup = tri::Clean<CMesh>::RemoveDuplicateVertex(mesh);
      int unref =  tri::Clean<CMesh>::RemoveUnreferencedVertex(mesh);
      //printf("Removed %i duplicate and %i unreferenced vertices from mesh \n",dup,unref);
  }
  */

  vcg::tri::UpdateBounding<CMesh>::Box(mesh);
  vcg::tri::UpdateTopology<CMesh>::VertexFace(mesh);

  vcg::LocalOptimization<CMesh> DeciSession(mesh,&qparams);

  DeciSession.Init<MyTriEdgeCollapse >();
  int finalSize = mesh.fn * percFaces / 100;

  DeciSession.SetTargetSimplices(finalSize);
  DeciSession.SetTimeBudget(0.5f);
  //if(TargetError< numeric_limits<float>::max() ) DeciSession.SetTargetMetric(TargetError);

  while(
    DeciSession.DoOptimization() &&
    mesh.fn>finalSize &&
    DeciSession.currMetric < TargetError
  ) qDebug("...");
//    printf("Current Mesh size %7i heap sz %9i err %9g \r",mesh.fn,DeciSession.h.size(),DeciSession.currMetric);

  tri::Allocator<CMesh>::CompactFaceVector(mesh);
  tri::Allocator<CMesh>::CompactVertexVector(mesh);

  return 100*mesh.fn / ((origFn)?origFn:1);


}

#include "brfMesh.h"
#include "brfSkeleton.h"


VcgMesh::VcgMesh()
{}

static const int mask =
  vcg::tri::io::Mask::IOM_VERTCOLOR |
  //vcg::tri::io::Mask::IOM_WEDGCOLOR |
  vcg::tri::io::Mask::IOM_VERTTEXCOORD|
  vcg::tri::io::Mask::IOM_WEDGTEXCOORD|
  vcg::tri::io::Mask::IOM_VERTCOORD |
  vcg::tri::io::Mask::IOM_VERTNORMAL |
  vcg::tri::io::Mask::IOM_FACEINDEX
  ;

static int lastMask;
static int lastErr;
static bool lastMeterial=false;

void setGotColor(bool b){
  if (b)
    lastMask |= vcg::tri::io::Mask::IOM_VERTCOLOR;
  else
    lastMask &= ~vcg::tri::io::Mask::IOM_VERTCOLOR;
}

void setGotTexture(bool b){
  if (b)
    lastMask |= vcg::tri::io::Mask::IOM_VERTTEXCOORD;
  else
    lastMask &= ~(vcg::tri::io::Mask::IOM_WEDGTEXCOORD|vcg::tri::io::Mask::IOM_VERTTEXCOORD);
}

void setGotMaterialName(bool b){
  lastMeterial = b;
}

void setGotNormals(bool b){
  if (b)
    lastMask |= vcg::tri::io::Mask::IOM_VERTNORMAL;
  else
    lastMask &= ~vcg::tri::io::Mask::IOM_VERTNORMAL;
}


bool VcgMesh::gotColor(){
 return lastMask & vcg::tri::io::Mask::IOM_VERTCOLOR;
}
bool VcgMesh::gotTexture(){
 return lastMask & (vcg::tri::io::Mask::IOM_VERTTEXCOORD | vcg::tri::io::Mask::IOM_WEDGTEXCOORD);
}

bool VcgMesh::gotMaterialName(){
 return lastMeterial;
}
static bool mustUseWT(){
  return
    (lastMask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD)
    &&
    !(lastMask & vcg::tri::io::Mask::IOM_VERTTEXCOORD);
}

static bool mustUseVT(){
  return
    (lastMask & vcg::tri::io::Mask::IOM_VERTTEXCOORD);
}


bool VcgMesh::gotNormals(){
 return lastMask & vcg::tri::io::Mask::IOM_VERTNORMAL;
}

const char *VcgMesh::lastErrString(){
  typedef vcg::tri::io::Importer<CMesh> Imp;
  return Imp::ErrorMsg(lastErr);
}

bool VcgMesh::load(char* filename){
  typedef vcg::tri::io::Importer<CMesh> Imp;
  lastMask=mask;
  vcg::tri::io::Importer<CMesh>::LoadMask(filename,lastMask);
  mesh.Clear();
  return !Imp::ErrorCritical(lastErr=Imp::Open(mesh,filename,lastMask));
}

bool VcgMesh::save(char* filename){
  return (vcg::tri::io::Exporter<CMesh>::Save(mesh,filename,mask)==0);
}

static Color4b Int2Col(unsigned int h){
  return Color4b((h>>16)&255, (h>>8)&255, (h>>0)&255, (h>>24)&255);
}

static unsigned int Col2Int(const Color4b & c){
  unsigned char b[4]={c[2],c[1],c[0],c[3]};
  uint32_t x = (uint32_t)*b;
  return x;
/*  return (((unsigned int)(c[3]))<<24) + (((unsigned int)(c[1]))<<16) +
         (((unsigned int)(c[2]))<< 8) + ((unsigned int)(c[0]));*/
}

class CoordSyst{
public:
  Point3f base;
  Point3f x,y,z;
  Point3f P(Point3f p) {
    return x*p[0] + y*p[1] + z*p[2] + base;
  }
  Point3f V(Point3f p) {
    return x*p[0] + y*p[1] + z*p[2];
  }
  BrfBone B(BrfBone b){
    BrfBone res = b;
    res.t = P(b.t);
    res.x = V(b.x);
    res.y = V(b.y);
    res.z = V(b.z);
    return res;
  }
  CoordSyst operator *(const CoordSyst &b){
    return CoordSyst( P(b.base), V(b.x), V(b.y), V(b.z));
  }
  CoordSyst Inverse() {
    CoordSyst tmp;
    tmp.x=Point3f(x[0],y[0],z[0]);
    tmp.y=Point3f(x[1],y[1],z[1]);
    tmp.z=Point3f(x[2],y[2],z[2]);
    tmp.base=Point3f(0,0,0);
    tmp.base = tmp.P(-base);
    return tmp;
  }
  CoordSyst(){}
  CoordSyst(Point3f o, Point3f _x, Point3f _y, Point3f _z){
    base = o; x = _x; y=_y; z=_z;
  }
  CoordSyst(Point3f a, Point3f b, Point3f c){
    base = a;
    b-=a;
    c-=a;
    x=b;
    y=b^c;
    z=x^y;
    x.Normalize();
    z.Normalize();
    y.Normalize();
  }
};

static void normalizeTriple(Point3f &x, Point3f &y, Point3f &z){
  y=z^x;
  z=x^y;
  x.Normalize();
  z.Normalize();
  y.Normalize();
}

void makeCoordSyst(const BrfSkeleton &s, int bi,  vector<CoordSyst> &cs, CoordSyst c0){
  c0= c0*CoordSyst(s.bone[bi].t, s.bone[bi].x,s.bone[bi].y,s.bone[bi].z);
  cs[bi]=c0;
  for (unsigned int k=0; k<s.bone[bi].next.size(); k++){
    makeCoordSyst(s,s.bone[bi].next[k],cs,c0);
  }
}

void applyInverseCoordSyst(BrfSkeleton &s, int bi,  CoordSyst c0){
  s.bone[bi] = c0.Inverse().B( s.bone[bi] );
  c0= c0*CoordSyst(s.bone[bi].t, s.bone[bi].x,s.bone[bi].y,s.bone[bi].z);


  for (unsigned int k=0; k<s.bone[bi].next.size(); k++){
    applyInverseCoordSyst(s,s.bone[bi].next[k],c0);
  }
}

bool VcgMesh::modifyBrfSkeleton(BrfSkeleton &s){
  int bn = (int)s.bone.size();
  if (mesh.fn!=bn*8) return false;
  if (mesh.vn!=bn*6) return false;
  Point3f undef(666,666,666);
  vector<Point3f> v(6, undef);
  /*if (mustUseWT()) {
    for (CMesh::FaceIterator f = mesh.face.begin(); f!=mesh.face.end(); f++) {
      f->V(0)->T() = f->WT(0);
      f->V(1)->T() = f->WT(1);
      f->V(2)->T() = f->WT(2);
    }
  }*/
  vector< vector<Point3f> > data (s.bone.size(), v);
  for (CMesh::VertexIterator v = mesh.vert.begin(); v!=mesh.vert.end(); v++) {
    int bi = (int)v->T().P()[0]; // bone index
    int vi = (int)v->T().P()[1];
    if (bi<0 || bi>=bn) {
      return false;
    }
    if (vi<0 || vi>=6) {
      return false;
    }
    if (data[bi][vi]!=undef) {
      return false;
    }
    data[bi][vi]=v->P();
  }

  for (int bi=0; bi<bn; bi++) {
    BrfBone b = s.bone[bi];
    b.t=Point3f(0,0,0);
    for (int i=0; i<6; i++) {
      b.t += data[bi][i];
    }
    b.t/=6;
    float
      X=BrfSkeleton::BoneSizeX(),
      Y=BrfSkeleton::BoneSizeY(),
      Z=BrfSkeleton::BoneSizeZ();
    b.x = (data[bi][0]-data[bi][3])/(X*2);
    b.y = (data[bi][1]-data[bi][4])/(Y*2);
    b.z = (data[bi][2]-data[bi][5])/(Z*2);
    normalizeTriple(b.x,b.y,b.z);
    s.bone[bi] = b;
  }

  CoordSyst c0;
  c0.base=Point3f(0,0,0);
  c0.x   =Point3f(1,0,0);
  c0.y   =Point3f(0,1,0);
  c0.z   =Point3f(0,0,1);


  applyInverseCoordSyst(s, s.root, c0);
  return true;
}

// save a skeleton as a mesh
void VcgMesh::add(const BrfSkeleton &s){
  vector<CoordSyst> cs(s.bone.size());
  CoordSyst c0;
  c0.base=Point3f(0,0,0);
  c0.x   =Point3f(1,0,0);
  c0.y   =Point3f(0,1,0);
  c0.z   =Point3f(0,0,1);
  makeCoordSyst(s, s.root,  cs, c0);

  mesh.Clear();
  vcg::tri::Allocator<CMesh>::AddFaces( mesh , s.bone.size()*8 );
  vcg::tri::Allocator<CMesh>::AddVertices( mesh , s.bone.size()*6 );
  for (unsigned int i=0; i<s.bone.size(); i++) {
    mesh.setFace(i*8+0, i*6+0, i*6+1, i*6+2);
    mesh.setFace(i*8+1, i*6+3, i*6+2, i*6+1);
    mesh.setFace(i*8+2, i*6+1, i*6+0, i*6+5);
    mesh.setFace(i*8+3, i*6+2, i*6+4, i*6+0);
    mesh.setFace(i*8+4, i*6+5, i*6+4, i*6+3);
    mesh.setFace(i*8+5, i*6+2, i*6+3, i*6+4);
    mesh.setFace(i*8+6, i*6+3, i*6+1, i*6+5);
    mesh.setFace(i*8+7, i*6+4, i*6+5, i*6+0);
    /*
    for (int h=0; h<8; h++)
    for (int w=0; w<8; w++) {
      int a=mesh.face[i*8+h].V(w)-&(mesh.vert[0]);
      mesh.face[i*8+h].WT(w).P() = Point2f(i,a); // encode node id in pos
    }*/
    float
      X=BrfSkeleton::BoneSizeX(),
      Y=BrfSkeleton::BoneSizeY(),
      Z=BrfSkeleton::BoneSizeZ();
    mesh.vert[i*6+0].P()=cs[ i ].P( Point3f(X,0,0) );
    mesh.vert[i*6+1].P()=cs[ i ].P( Point3f(0,Y,0) );
    mesh.vert[i*6+2].P()=cs[ i ].P( Point3f(0,0,Z) );
    mesh.vert[i*6+3].P()=cs[ i ].P(-Point3f(X,0,0) );
    mesh.vert[i*6+4].P()=cs[ i ].P(-Point3f(0,Y,0) );
    mesh.vert[i*6+5].P()=cs[ i ].P(-Point3f(0,0,Z) );

    mesh.vert[i*6+0].N()=cs[ i ].V( Point3f(1,0,0) );
    mesh.vert[i*6+1].N()=cs[ i ].V( Point3f(0,1,0) );
    mesh.vert[i*6+2].N()=cs[ i ].V( Point3f(0,0,1) );
    mesh.vert[i*6+3].N()=cs[ i ].V(-Point3f(1,0,0) );
    mesh.vert[i*6+4].N()=cs[ i ].V(-Point3f(0,1,0) );
    mesh.vert[i*6+5].N()=cs[ i ].V(-Point3f(0,0,1) );

    for (int v=0; v<6; v++) {
      mesh.vert[i*6+v].T().P()=Point2f(i,v);
    }
    for (int f=0; f<8; f++)
    for (int w=0; w<3; w++) {
      mesh.face[i*8+f].WT(w) = mesh.face[i*8+f].V(w)->T();
    }

  }
}

void VcgMesh::moveBoneInSkelMesh(int nb, Point3f d){
  for (unsigned int i=0; i<mesh.vert.size(); i++) {
    if (mesh.vert[i].T().P()[0]==nb) mesh.vert[i].P()+=d;
  }
}

static Point2f _flipY(const Point2f p){
  return Point2f(p[0],1-p[1]);
}
static Point3f _flipZ(const Point3f p){
  return Point3f(p[0],p[1],-p[2]);
}

void VcgMesh::add(const BrfMesh &b, int fi){
  mesh.Clear();
  mesh.textures.clear();
  if ((int)b.frame.size()<=fi) fi=b.frame.size()-1;


  mesh.textures.push_back(b.material);
  CMesh::FaceIterator f=vcg::tri::Allocator<CMesh>::AddFaces( mesh , b.face.size() );
  lastMask =
    vcg::tri::io::Mask::IOM_VERTNORMAL |
    vcg::tri::io::Mask::IOM_VERTCOLOR;

#if 0
  // one vcg::vert per brf::pos
  CMesh::VertexIterator v=vcg::tri::Allocator<CMesh>::AddVertices( mesh , b.frame[0].pos.size() );
  int k=0;
  for (CMesh::FaceIterator f=mesh.face.begin(); f!=mesh.face.end(); f++,k++) {
    for (int h=0; h<3; h++) {
      int vi = b.face[k].index[2-h];
      f->V(h) = &(mesh.vert[0]) + b.vert[ vi ].index;
      //f->WC(h) = Int2Col(b.vert[ vi ].col);
      f->WT(h).P() = b.vert[vi].ta;

      int pi = b.vert[ vi ].index;
      f->V(h)->N() = b.frame[0].norm[ pi ];
      f->V(h)->P() = b.frame[0].pos [ pi ];
    }
    lastMask |= vcg::tri::io::Mask::IOM_WEDGTEXCOORD;
  }
#else
#if 1
  // one vcg::vert per brf::vert
  // CMesh::VertexIterator v=vcg::tri::Allocator<CMesh>::AddVertices( mesh , b.vert.size() );
  int k=0;
  for (CMesh::FaceIterator f=mesh.face.begin(); f!=mesh.face.end(); f++,k++) {
    for (int h=0; h<3; h++) {
      int vi = b.face[k].index[h];
      f->V(h) = &(mesh.vert[0]) + vi;

      f->WT(h).P() = _flipY(b.vert[vi].ta); // texture also x wedge, always

    }
  }
  k=0;
  for (CMesh::VertexIterator v=mesh.vert.begin();v!=mesh.vert.end(); v++,k++) {
      int pi = b.vert[ k ].index;
      v->N() = _flipZ(b.frame[fi].norm[ k ]);
      v->P() = _flipZ(b.frame[fi].pos [ pi ]);
      v->C() = Int2Col( b.vert[ k ].col );
      v->T().P() = _flipY(  b.vert[ k ].ta );
  }
  lastMask |= vcg::tri::io::Mask::IOM_VERTTEXCOORD;

#else
  // three vcg::vert per face
#endif
#endif

}

BrfMesh VcgMesh::toBrfMesh(){

  BrfMesh b;
  b.name[0]=0;
  b.material[0]=0;

  if ((lastMeterial=(mesh.textures.size()>0)))

    sprintf(b.material,"%s",mesh.textures[0].c_str());
  else
    sprintf(b.material,"undefined");

  b.face.resize( mesh.fn );
  b.vert.resize( mesh.vn ); // one vert x wedge!


  b.frame.resize(1);
  b.frame[0].pos.resize( mesh.vn );
  b.frame[0].norm.resize( mesh.vn );
//        b.frame[0].pos.push_back( f->V(h)->P() );
//      b.frame[0].norm.push_back( f->V(h)->N() );

  int k;
  // copy faces

  k=0;
  for (CMesh::FaceIterator f=mesh.face.begin(); f!=mesh.face.end(); f++) if (!f->IsD()) {
    for (int h=0; h<3; h++) {

      int vi  = b.face[k].index[h] = f->V(h)-&(mesh.vert[0]);
      if (mustUseWT()) b.vert[ vi ] .ta = b.vert[ vi ] .tb = _flipY(f->WT(h).P()); // text x wedge
    }
    k++;
  }

  /*
  if (mustUseWT()) qDebug("Using WT");
  if (mustUseVT()) qDebug("Using VT");
  */
  k=0;
  for (CMesh::VertexIterator v=mesh.vert.begin();v!=mesh.vert.end(); v++) if (!v->IsD()) {
          //int vi = f->V(h)-&mesh.vert[0];
    b.vert[k].col = //0xFFFFFFFF;
    (gotColor())? Col2Int( v->C() )
               :0xFFFFFFFF;
    b.vert[k].index = k;
    b.frame[0].norm[k] = _flipZ(v->N());
    b.frame[0].pos[k] = _flipZ(v->P());
    if (mustUseVT()) {
      //qDebug("%f %f",v->T().P()[0],v->T().P()[1]);
      b.vert[k].ta = b.vert[k].tb  = _flipY(v->T().P()); // text x vert?
    }
    k++;
  }

  b.AdjustNormDuplicates();
  b.skinning.clear();
  b.flags=0;
  if (!gotNormals()) b.ComputeNormals();
  b.AfterLoad();

  return b;
}
