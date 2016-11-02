/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef GLWIDGETS_H
#define GLWIDGETS_H

#include <QGLWidget>
#include <QOpenGLFunctions_2_0>

#include <QtGui>
#include "brfData.h"
#include "iniData.h"
#include "ddsData.h"

class BrfData;
class QGLShaderProgram;

// for picking
class GlCamera{
public:
	static GlCamera currentCamera(int targetIndex);

	GlCamera();
	double mv[16],pr[16];
	int vp[4];
	int targetIndex; // index of mesh target
	bool isInViewport(int x, int y) const;
	bool getCurrent();
	vcg::Point3f unproject(int x, int y) const;
};

//typedef std::map< std::string, std::string > MapSS;

struct ViewportData{
    std::vector<int> items;
    int bestLod = 100;
};

class GLWidget : public QGLWidget, protected QOpenGLFunctions_2_0
{
	Q_OBJECT
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
public:
	GLWidget(QWidget *parent, IniData& i);
	~GLWidget();

	BrfData* data;
	BrfData* reference; // for things that needs to be present to see other things...
	// e.g. skeletons for animations
	BrfData* hitBoxes; // extra data for skeletons
	IniData &inidata;

	int selected;
	int subsel; // subpiece selected
	int selRefAnimation; // animation selected to view skinned mesh
	int selRefSkin; // skinned mesh
	int selRefSkel; // current skeleton
	int selFrameN; // current selected frame of vertex ani

	int lastSelected;
	bool applyExtraMatrixToAll;
	float extraMatrix[16]; // matric for temp transforms
	void clearExtraMatrix();

	void selectNone();
	void setEditingRef(bool mode);
	TokenEnum displaying;

	typedef struct {
		int type; // 0 = no err;  1 = cannot find material;  2 = cannot find mesh file; 3 = cannot decrypt mesh file
		QString matName;
		QString texName;
	} MaterialError;
	MaterialError lastMatErr;
	void setMaterialError(int newErr);

	void keyPressEvent( QKeyEvent * event );
	void keyReleaseEvent( QKeyEvent * event );
private slots:
	void onTimer();

public slots:
	void setSelection(const QModelIndexList &, int k);
	void setFloatingProbePos(float x, float y, float z);

	void setSubSelected(int k);
	void setRefAnimation(int i);
	void setRefSkin(int i);
	void setViewmode(int i);
	void setViewmodeMult(int i);
	int getViewmodeMult()const{return viewmodeMult;}
	int  getRefSkin() const;
	void setRefSkeleton(int i);
	int  getRefSkeleton() const;
	int  getRefSkelAni() const;
	void setWireframe(int i);
	void setLighting(int i);
	void setTexture(int i);
	void setNormalmap(int i);
    void setTransparency(int i);
    void setSpecularmap(int i);
	void setComparisonMesh(int i);
	void setFloor(int i);
	void setFloorForAni(int i);
	void setRuler(int i);
	void setFloatingProbe(int i);
	void setHitboxes(int i);
	void setRulerLenght(int i);
	void setPlay();
	void setStop();
	void setPause(int i=-1);
	void setStepon();
	void setStepback();
	void setColorPerVert();
	void setColorPerRig();
	void setColorPerWhite();
	void setFrameNumber(int);
	void setDefaultBgColor(QColor bgColor, bool alsoCurrent);

	void renderAoOnMeshes(float brightness, float fromAbove, bool perface, bool inAlpha, bool overwrite);
	void renderTextureColorOnMeshes(bool overwrite);

	void browseTexture();

	int  getFrameNumber() const;
	void showMaterialDiffuseA();
	void showMaterialDiffuseB();
	void showMaterialBump();
	void showMaterialEnviro();
	void showMaterialSpecular();
	void showAlphaTransparent();
	void showAlphaPurple();
	void showAlphaNo();
	void setCommonBBoxOn();
	void setCommonBBoxOff();
	void setInferMaterialOn();
	void setInferMaterialOff();
	void setUseOpenGL2(bool mode);
public:

    /* settings */
    bool useWireframe, useLighting, useTexture , useTransparency, useNormalmap, useFloor, useFloorInAni, usePreviewTangents;
	bool useRuler, useFloatingProbe, useSpecularmap, useHitboxes, useComparisonMesh;
    bool autoComputeTangents;
	bool ghostMode;
	bool fixTexturesOnSight;
	int colorMode, rulerLenght;
	enum{STOP, PAUSE, PLAY} runningState, defaultRunningState;
	enum{DIFFUSEA, DIFFUSEB, BUMP, ENVIRO, SPECULAR } curMaterialTexture;
	enum{TRANSALPHA, PURPLEALPHA, NOALPHA} showAlpha;
	bool commonBBox;
	bool inferMaterial;
	bool useOpenGL2;

	float runningSpeed;
	int relTime; // msec, zeroed at stop.
	float floatingProbePulseColor;

signals:
	void signalFrameNumber(int);
	void notifyCheckboardChanged();
	void setTextureData(DdsData d);
	void displayInfo(QString st, int howlong);
	void notifySelectedPoint(float x, float y, float z);
    void signalSelection(std::vector<int> sel);

protected:
	//MapSS *mapMT;
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);
	void mouseClickEvent(QMouseEvent *);
	void wheelEvent(QWheelEvent *event);

	// rendering of stuff
	template<class BrfType> void renderSelected(const std::vector<BrfType>& p);
	// unified rendering of BrfItems...
	void renderBrfItem(const BrfMesh& p);
	void renderBrfItem(const BrfAnimation& p);
	void renderBrfItem(const BrfBody& p);
	void renderBrfItem(const BrfSkeleton& p);
	void renderBrfItem(const BrfTexture& p);
	void renderBrfItem(const BrfMaterial& p);

	void enableMaterial(const BrfMaterial& m);
	void enableDefMaterial();

    bool myBindTexture(const QString &fileName, DdsData &data);

	// basic rendering of Brf Items & c:
	void renderMesh(const BrfMesh& p, float frame);
	void renderMeshSimple(const BrfMesh& p);
	void renderSkinnedMesh(const BrfMesh& p,  const BrfSkeleton& s, const BrfAnimation& a, float frame);
	void renderSkeleton(const BrfSkeleton& p);
	void renderAnimation(const BrfAnimation& p, const BrfSkeleton& s, float frame);
	void renderBody(const BrfBody& p);
	void renderBody(const BrfBody& p, const BrfSkeleton& s, bool ghost); // renders hitboxes p around sketons s
	void renderBody(const BrfBody& p, const BrfSkeleton& s, const BrfAnimation& a, float frame, bool ghost);


    void renderBone(const BrfAnimation& p, const BrfSkeleton& s,  float frame, int i, int lvl); // recursive
    void renderBone(const BrfSkeleton& p, int i, int lvl); // recursive
    void renderBodyPart(const BrfBodyPart &b);
    void renderBodyPart(const BrfBody &b, const BrfSkeleton& s, int i, int lvl); // recursive! (hitboxes)
    void renderBodyPart(const BrfBody &b, const BrfSkeleton& s, const BrfAnimation& a, float frame, int i, int lvl); // recursive! (hitboxes)



	void renderTexture(const char* name, bool addExtension = true);
    void renderSphereWire();
    void renderCylWire(float rad, float h);
    void renderOcta(int brightness) ;
	void renderFloor();
	void renderFloorMaybe();
	void renderRuler();

	void mySetViewport(int viewportIndex);
    int getViewportOf( int x, int y ) const;
	void mySetViewport(int x,int y,int w,int h);
	int nViewportCols, nViewportRows;
    bool bboxReady;
    //int nViewports;
    //int maxSel; // index of the max selected

    void distributeSelectedInViewports();
    void maybeApplyRenderOrder();

	void scaleAsLastBindedTexture();
	void renderAoOnMeshesAllInViewportI(float brightness, float fromAbove, bool perface, bool inAlpha, bool overwrite, int I);


	void glClearCheckBoard();
	// rendering mode (just changes of openGL status):
	void setShadowMode(bool on);
    void setWireframeLightingMode(bool on, bool light, bool text);
	void setTextureName(QString st, int origin, int texUnit);
	static bool fixTextureFormat(QString st);
	void setMaterialName(QString st);
	void setMaterialNameOnlyDiffuse(QString st);
	void setCheckboardTexture();
	void setDummyRgbTexture();
	void setDummySpecTexture();
	void setDummyNormTexture();

	void initOpenGL2();
	bool openGL2ready;
	void initDefaultTextures();
	void initializeGL();

	bool skeletalAnimation();

    bool viewIs2D() const;

public:

	QString texturePath[3];
	QString locateOnDisk(QString nome, const char*ext, BrfMaterial::Location *loc);
	void forgetChachedTextures();
	QString getCurrentShaderDescriptor() const;
	QString getCurrentShaderLog() const;

    //enum{MAXSEL=10000};
    //bool selGroup[MAXSEL];
    //int selViewport[MAXSEL]; // for each selected item, in which viewport it is
    std::vector< ViewportData >  inViewport;

    Box3f globalBox, selectedBox;

	int readCustomShaders();

	int lastSkelAniFrameUsed;

public:
	bool picking;
	int selPointIndex, selPointFace, selPointWedge; // selected mesh, face, wedge
private:
	vcg::Point3f floatingProbe; // tmp
	void renderFloatingProbe();

    //int w, h; // screen size
	QColor currBgColor, defaultBgColor; // bgcolors
	QPoint lastPos; // mouse pos
	bool mouseMoved;
	float phi, theta, dist;
	int tw, th; // texture size, when texture is shown
	bool ta; // textures uses alpha, when texture is shown
	float cx, cy, zoom; // for texture display
	vcg::Point3f avatP, avatV; // pos, vel of avatat
	bool keys[5];


	QTimer *timer;

	bool animating;
	bool bumpmapActivated, bumpmapUsingGreen;
	bool shadowMode;

	int viewmode;
	int viewmodeMult;
	int dummyRgbTexture, dummySpecTexture, dummyNormTexture, checkboardTexture;


	float currViewmodeHelmet;
	float currViewmodeInterior;
	vcg::Point3f lastCenter;
	float lastScale;
	float closingUp;
	QString renderedTexture;
	// fragment programs
	enum { NM_PLAIN = 0, NM_ALPHA, NM_IRON, NM_SHINE, SHADER_IRON, SHADER_MODES, SHADER_FIXEDFUNC , SHADER_CUSTOM };
	//unsigned int
	QGLShaderProgram* shaderProgram[SHADER_MODES][SHADER_MODES];
	bool shaderTried[SHADER_MODES][SHADER_MODES];
	QString shaderLog[SHADER_MODES][SHADER_MODES];
	int lastUsedShader;
	int lastUsedShaderBumpgreen;

	std::vector<GlCamera> camera;

	//void newShaderProgram(QGLShaderProgram& s, const QStirng prefix, const QStirng vs, const QStirng fs);

	QGLShaderProgram* initFramPrograms(int mode, bool green);

	QMap<QString, QGLShaderProgram*> customShaders;
	QGLShaderProgram* currentCustomShader;

    int widthPix() const;
    int heightPix() const;
};

#endif // GLWIDGETS_H
