/* OpenBRF -- by marco tarini. Provided under GNU General Public License */


#ifndef __VCGLIB_TRIMESH_GENERIC_EXPORT
#define __VCGLIB_TRIMESH_GENERIC_EXPORT

#include <wrap/io_trimesh/export_ply.h>
#include <wrap/io_trimesh/export_stl.h>
#include <wrap/io_trimesh/export_off.h>
#include <wrap/io_trimesh/export_dxf.h>
//#include <wrap/io_trimesh/export_3ds.h>
#include <wrap/io_trimesh/export_obj.h>
#include <wrap/io_trimesh/export_dae.h>

#include <locale>

namespace vcg {
namespace tri {
namespace io {

/**
This class encapsulate a filter for automatically importing meshes by guessing
the right filter according to the extension
*/

template <class OpenMeshType>
class Exporter
{
private:
  enum KnownTypes { KT_UNKNOWN, KT_PLY, KT_STL, KT_DXF, KT_OFF, KT_OBJ, KT_DAE};
static int &LastType()
{
  static int lastType= KT_UNKNOWN;
return lastType;
}

public:
// simple aux function that returns true if a given file has a given extesnion
static bool FileExtension(std::string filename,  std::string extension)
{
  std::locale loc1 ;
  std::use_facet<std::ctype<char> > ( loc1 ).tolower(&*filename.begin(),&(*filename.rbegin()));
  std::use_facet<std::ctype<char> > ( loc1 ).tolower(&*extension.begin(),&(*extension.rbegin()));
  std::string end=filename.substr(filename.length()-extension.length(),extension.length());
  return end==extension;
}
// Open Mesh
static int Save(OpenMeshType &m, const char *filename, CallBackPos *cb=0)
{
 return Save(m,filename,0,cb);
}

// Open Mesh
static int Save(OpenMeshType &m, const char *filename, const int mask, CallBackPos *cb=0)
{
  int err;
  if(FileExtension(filename,"ply"))
  {
    err = ExporterPLY<OpenMeshType>::Save(m,filename,mask);
    LastType()=KT_PLY;
  }
  else if(FileExtension(filename,"stl"))
  {
    err = ExporterSTL<OpenMeshType>::Save(m,filename);
    LastType()=KT_STL;
  }
  else if(FileExtension(filename,"off"))
  {
    err = ExporterOFF<OpenMeshType>::Save(m,filename,mask);
    LastType()=KT_OFF;
  }
  else if(FileExtension(filename,"dxf"))
  {
    err = ExporterDXF<OpenMeshType>::Save(m,filename);
    LastType()=KT_DXF;
  }
  else if(FileExtension(filename,"obj"))
  {
    err = ExporterOBJ<OpenMeshType>::Save(m,filename,mask,cb);
    LastType()=KT_OBJ;
  }
  else if(FileExtension(filename,"dae"))
  {
    err = ExporterDAE<OpenMeshType>::Save(m,filename,mask);
    LastType()=KT_DAE;
  } else {
    err=1;
    LastType()=KT_UNKNOWN;
  }

  return err;
}

static const char *ErrorMsg(int error)
{
  switch(LastType())
  {
    case KT_PLY : return ExporterPLY<OpenMeshType>::ErrorMsg(error); break;
    case KT_STL : return ExporterSTL<OpenMeshType>::ErrorMsg(error); break;
    case KT_OFF : return ExporterOFF<OpenMeshType>::ErrorMsg(error); break;
    case KT_DXF : return ExporterDXF<OpenMeshType>::ErrorMsg(error); break;
    case KT_OBJ : return ExporterOBJ<OpenMeshType>::ErrorMsg(error); break;
  }
  return "Unknown type";
}

}; // end class
} // end Namespace tri
} // end Namespace io
} // end Namespace vcg

#endif
