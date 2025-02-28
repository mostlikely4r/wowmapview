#ifndef MODEL_H
#define MODEL_H

#include "vec3d.h"

class Model;
class Bone;
Vec3D fixCoordSystem(Vec3D v);

#include "manager.h"
#include "mpq.h"
#include "video.h"

#include "modelheaders.h"
#include "quaternion.h"
#include "matrix.h"

#include <vector>

#include "animated.h"
#include "particle.h"


class Bone {
	Vec3D pivot;
	int parent;

	Animated<Vec3D> trans;
	Animated<Quaternion> rot;
	Animated<Vec3D> scale;

public:
	bool billboard;
	Matrix mat;
	Matrix mrot;

	bool calc;
	void calcMatrix(Bone* allbones, int anim, int time);
	void init(MPQFile &f, ModelBoneDef &b, int *global);
    void init(MPQFile &f, ModelBoneDefTBC &b, int *global);

};


class TextureAnim {
	Animated<Vec3D> trans, rot, scale;

public:
	Vec3D tval, rval, sval;

	void calc(int anim, int time);
	void init(MPQFile &f, ModelTexAnimDef &mta, int *global);
	void setup();
};

struct ModelColor {
	Animated<Vec3D> color;
	AnimatedShort opacity;

	void init(MPQFile &f, ModelColorDef &mcd, int *global);
};

struct ModelTransparency {
	AnimatedShort trans;

	void init(MPQFile &f, ModelTransDef &mtd, int *global);
};

// copied from the .mdl docs? this might be completely wrong
enum BlendModes {
	BM_OPAQUE,
	BM_TRANSPARENT,
	BM_ALPHA_BLEND,
	BM_ADDITIVE,
	BM_ADDITIVE_ALPHA,
	BM_MODULATE
};

struct ModelRenderPass {
	uint16 indexStart, indexCount, vertexStart, vertexEnd;
	TextureID texture, texture2;
	bool usetex2, useenvmap, cull, trans, unlit, nozwrite;
	float p;
	
	int16 texanim, color, opacity, blendmode;
	int16 order;

	bool init(Model *m);
	void deinit();

	bool operator< (const ModelRenderPass &m) const
	{
		//return !trans;
		if (order<m.order) return true;
		else if (order>m.order) return false;
		else return blendmode == m.blendmode ? (p<m.p) : blendmode < m.blendmode;
	}
};

struct ModelCamera {
	bool ok;

	Vec3D pos, target;
	float nearclip, farclip, fov;
	Animated<Vec3D> tPos, tTarget;
	Animated<float> rot;

	void init(MPQFile &f, ModelCameraDef &mcd, int *global);
	void setup(int time=0);

	ModelCamera():ok(false) {}
};

struct ModelLight {
	int type, parent;
	Vec3D pos, tpos, dir, tdir;
	Animated<Vec3D> diffColor, ambColor;
	Animated<float> diffIntensity, ambIntensity;

	void init(MPQFile &f, ModelLightDef &mld, int *global);
	void setup(int time, GLuint l);
};

class Model: public ManagedItem {

	GLuint dlist;
	GLuint vbuf, nbuf, tbuf;
	size_t vbufsize;
	bool animGeometry,animTextures,animBones;

	bool forceAnim;

	void init(MPQFile &f);

	TextureAnim *texanims;
	int *globalSequences;
	ModelColor *colors;
	ModelTransparency *transparency;
	ModelLight *lights;
	ParticleSystem *particleSystems;
	RibbonEmitter *ribbons;

	void drawModel();
	void initCommon(MPQFile &f);
	bool isAnimated(MPQFile &f);
	void initAnimated(MPQFile &f);
	void initStatic(MPQFile &f);

	ModelVertex *origVertices;
	Vec3D *vertices, *normals;
	uint16 *indices;
	size_t nIndices;
	std::vector<ModelRenderPass> passes;

	void calcBones(int anim, int time);

	void lightsOn(GLuint lbase);
	void lightsOff(GLuint lbase);

public:
	bool animated;
	ModelHeader header;
	ModelAnimation* anims;

	void animate(int anim);

	ModelCamera cam;
	Bone *bones;
	TextureID *textures;

	size_t GetTextureCount() const { return header.nTextures; }
	TextureID GetTexture(size_t index) const { return textures[index]; }
	bool HasTextures() const { return textures != nullptr && header.nTextures > 0; }

	bool ok;
	bool ind;

	float rad;
	float trans;
	bool animcalc;
	int anim, animtime;

	Model(std::string name, bool forceAnim=false);
	~Model();
	void draw();
	void updateEmitters(float dt);

	friend struct ModelRenderPass;
};

class ModelManager: public SimpleManager {
public:
	int add(std::string name);

	ModelManager() : v(0) {}

	int v;

	void resetAnim();
	void updateEmitters(float dt);

};


class ModelInstance {
public:
	Model *model;

	int id;

	Vec3D pos, dir;
	unsigned int d1, scale;

	float frot,w,sc;

	int light;
	Vec3D ldir;
	Vec3D lcol;

	ModelInstance() {}
	ModelInstance(Model *m, MPQFile &f);
    void init2(Model *m, MPQFile &f);
	void draw();
	void draw2(const Vec3D& ofs, const float rot);

};

#endif
