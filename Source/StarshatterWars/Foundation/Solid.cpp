/*  Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo, Destroyer Studios LLC

	SUBSYSTEM:    nGenEx.lib
	FILE:         Solid.cpp
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Classes for rendering solid meshes of polygons
*/

#include "Solid.h"
#include "SimScene.h"
#include "DataLoader.h"
#include "SimLight.h"
#include "Shadow.h"
#include "SimProjector.h"
#include "OPCODE.h"

// Unreal core (FVector/FMatrix/FMath/UE_LOG):
#include "CoreMinimal.h"

// C runtime:
#include <cstring>
#include <cstdlib>

#ifdef for
#undef for
#endif

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterSolid, Log, All);

// +--------------------------------------------------------------------+

static bool use_collision_detection = true;

bool  Solid::IsCollisionEnabled() { return use_collision_detection; }
void  Solid::EnableCollision(bool e) { use_collision_detection = e; }

// +--------------------------------------------------------------------+

Opcode::AABBTreeCollider opcode_collider;

class OPCODE_data
{
public:
	OPCODE_data(Surface* s)
	{
		bool status = false;

		if (s) {
			using namespace Opcode;
			opcode_collider.SetFirstContact(true);

			npolys = s->NumPolys();
			nverts = s->NumVerts();
			ntris = s->NumIndices() / 3;

			locs = new IcePoint[nverts];
			tris = new IndexedTriangle[ntris];

			if (locs && tris) {
				int i, n = 0;

				for (i = 0; i < nverts; i++) {
					IcePoint* p = locs + i;

					// Surface VertexSet loc is assumed UE-converted (FVector*):
					const FVector* v = s->GetVertexSet()->loc + i;
					p->Set(v->X, v->Y, v->Z);
				}

				for (i = 0; i < npolys; i++) {
					Poly* p = s->GetPolys() + i;

					if (p->nverts == 3) {
						IndexedTriangle& t = tris[n++];

						t.mVRef[0] = p->verts[0];
						t.mVRef[1] = p->verts[2];
						t.mVRef[2] = p->verts[1];
					}
					else {
						IndexedTriangle& t1 = tris[n++];
						IndexedTriangle& t2 = tris[n++];

						t1.mVRef[0] = p->verts[0];
						t1.mVRef[1] = p->verts[2];
						t1.mVRef[2] = p->verts[1];

						t2.mVRef[0] = p->verts[0];
						t2.mVRef[1] = p->verts[3];
						t2.mVRef[2] = p->verts[2];
					}
				}

				mesh.SetNbVertices(nverts);
				mesh.SetNbTriangles(ntris);
				mesh.SetPointers(tris, locs);

				OPCODECREATE creator;
				creator.mIMesh = &mesh;
				status = model.Build(creator);
			}

			if (!status) {
				UE_LOG(LogStarshatterSolid, Warning, TEXT("OPCODE collision model build failed."));
			}
		}
		else {
			tris = nullptr;
			locs = nullptr;
			npolys = 0;
			nverts = 0;
			ntris = 0;
		}
	}

	~OPCODE_data()
	{
		delete[] tris;
		delete[] locs;
	}

	Opcode::Model         model;
	Opcode::MeshInterface mesh;
	IndexedTriangle* tris;
	IcePoint* locs;
	int                   npolys;
	int                   nverts;
	int                   ntris;
};

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

Solid::Solid()
	: model(nullptr),
	own_model(true),
	roll(0.0f),
	pitch(0.0f),
	yaw(0.0f),
	intersection_poly(nullptr)
{
	shadow = true;
	sprintf_s(name, sizeof(name), "Solid %d", id);
}

Solid::~Solid()
{
	if (own_model)
		delete model;

	shadows.destroy();
}

// +--------------------------------------------------------------------+

void
Solid::Update()
{
}

// +--------------------------------------------------------------------+

void
Solid::SetOrientation(const FMatrix& o)
{
	OrientationMatrix = o;
}

void
Solid::SetLuminous(bool l)
{
	luminous = l;

	if (model && luminous) {
		model->luminous = luminous;

		ListIter<Material> iter = model->GetMaterials();

		while (++iter) {
			Material* mtl = iter.value();

			mtl->Ka = Color::Black;
			mtl->Kd = Color::Black;
			mtl->Ks = Color::Black;
			mtl->Ke = Color::White;

			if (mtl->tex_diffuse && !mtl->tex_emissive)
				mtl->tex_emissive = mtl->tex_diffuse;
		}

		ListIter<Surface> s_iter = model->GetSurfaces();
		while (++s_iter) {
			Surface* surface = s_iter.value();
			VertexSet* vset = surface->GetVertexSet();

			for (int i = 0; i < vset->nverts; i++) {
				vset->diffuse[i] = Color::White.Value();
				vset->specular[i] = Color::Black.Value();
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
Solid::SetOrientation(const Solid& match)
{
	if (!model || infinite)
		return;

	OrientationMatrix = match.Orientation();
}

// +--------------------------------------------------------------------+

void
Solid::Render(Video* video, DWORD flags)
{
	if (flags & RENDER_ADDITIVE)
		return;

	if (video && model && model->NumPolys()) {
		DWORD blend_modes = Video::BLEND_SOLID;

		if (flags == RENDER_ALPHA)
			blend_modes = Video::BLEND_ALPHA | Video::BLEND_ADDITIVE;

		video->DrawSolid(this, blend_modes);
	}
}

// +--------------------------------------------------------------------+

void
Solid::SelectDetail(SimProjector* p)
{
}

// +--------------------------------------------------------------------+

void
Solid::ProjectScreenRect(SimProjector* p)
{
	if (model && p) {
		FVector tmp = loc;
		p->Transform(tmp);

		if (tmp.Z > 1) {
			int l = 2000;
			int r = -2000;
			int t = 2000;
			int b = -2000;

			for (int i = 0; i < 6; i++) {
				FVector extent(0, 0, 0);

				if (i < 2)
					extent.X = model->extents[i];
				else if (i < 4)
					extent.Y = model->extents[i];
				else
					extent.Z = model->extents[i];

				// UE: transform by orientation matrix then offset by world location:
				extent = OrientationMatrix.TransformVector(extent) + loc;

				p->Transform(extent);
				p->Project(extent);

				if (extent.X < l) l = (int)extent.X;
				if (extent.X > r) r = (int)extent.X;
				if (extent.Y < t) t = (int)extent.Y;
				if (extent.Y > b) b = (int)extent.Y;
			}

			screen_rect.x = l;
			screen_rect.y = t;
			screen_rect.w = r - l;
			screen_rect.h = b - t;
			return;
		}
	}

	screen_rect.x = 2000;
	screen_rect.y = 2000;
	screen_rect.w = 0;
	screen_rect.h = 0;
}

// +--------------------------------------------------------------------+
// Polygon Interference Detection:

int
Solid::CollidesWith(Graphic& o)
{
	const FVector DeltaLoc = Location() - o.Location();

	// bounding spheres test:
	if (DeltaLoc.Size() > Radius() + o.Radius())
		return 0;

	// possible collision, but no further refinement can be done:
	if (!o.IsSolid())
		return 1;

	Solid& s = (Solid&)o;

	// use the OPCODE library to check for polygon interference:
	if (model && s.model) {
		using namespace Opcode;

		bool contact = false;

		const FMatrix& M1 = OrientationMatrix;
		const FMatrix& M2 = s.OrientationMatrix;

		Matrix4x4 world0;
		Matrix4x4 world1;

		// Preserve legacy row/column layout intent as closely as possible:
		world0.m[0][0] = (float)M1.M[0][0];
		world0.m[0][1] = (float)M1.M[0][1];
		world0.m[0][2] = (float)M1.M[0][2];
		world0.m[0][3] = 0.0f;

		world0.m[1][0] = (float)M1.M[1][0];
		world0.m[1][1] = (float)M1.M[1][1];
		world0.m[1][2] = (float)M1.M[1][2];
		world0.m[1][3] = 0.0f;

		world0.m[2][0] = (float)M1.M[2][0];
		world0.m[2][1] = (float)M1.M[2][1];
		world0.m[2][2] = (float)M1.M[2][2];
		world0.m[2][3] = 0.0f;

		world0.m[3][0] = (float)Location().X;
		world0.m[3][1] = (float)Location().Y;
		world0.m[3][2] = (float)Location().Z;
		world0.m[3][3] = 1.0f;

		// Legacy code used a different indexing for world1; preserve behavior:
		world1.m[0][0] = (float)M2.M[0][0];
		world1.m[0][1] = (float)M2.M[1][0];
		world1.m[0][2] = (float)M2.M[2][0];
		world1.m[0][3] = 0.0f;

		world1.m[1][0] = (float)M2.M[0][1];
		world1.m[1][1] = (float)M2.M[1][1];
		world1.m[1][2] = (float)M2.M[2][1];
		world1.m[1][3] = 0.0f;

		world1.m[2][0] = (float)M2.M[0][2];
		world1.m[2][1] = (float)M2.M[1][2];
		world1.m[2][2] = (float)M2.M[2][2];
		world1.m[2][3] = 0.0f;

		world1.m[3][0] = (float)s.Location().X;
		world1.m[3][1] = (float)s.Location().Y;
		world1.m[3][2] = (float)s.Location().Z;
		world1.m[3][3] = 1.0f;

		ListIter<Surface> s1_iter = model->surfaces;
		while (++s1_iter && !contact) {
			Surface* s1 = s1_iter.value();

			ListIter<Surface> s2_iter = s.model->surfaces;
			while (++s2_iter && !contact) {
				Surface* s2 = s2_iter.value();

				if (s1->opcode && s2->opcode) {
					BVTCache bvt;
					bvt.Model0 = &s1->opcode->model;
					bvt.Model1 = &s2->opcode->model;

					if (opcode_collider.Collide(bvt, &world0, &world1))
						if (opcode_collider.GetContactStatus() != 0)
							contact = true;
				}
			}
		}

		return contact ? 1 : 0;
	}

	return 1;
}

// +--------------------------------------------------------------------+
// Find the intersection of the ray (Q + w*len) with the solid.
// If the ray intersects a polygon of the solid, place the intersection
// point in ipt, and return 1.  Otherwise, return 0.

int
Solid::CheckRayIntersection(FVector Q, FVector w, double len, FVector& ipt,
	bool treat_translucent_polys_as_solid)
{
	int impact = 0;

	if (!model || model->npolys < 1)
		return impact;

	// check right angle spherical distance:
	const FVector d0 = loc - Q;
	const FVector d1 = FVector::CrossProduct(d0, w);
	const double  dlen = (double)d1.Size();    // distance of point from line

	if (dlen > radius)                         // clean miss
		return 0;                               // (no impact)

	// find the point on the ray that is closest to the solid's location:
	const float tClosest = FVector::DotProduct(d0, w);
	const FVector closest = Q + w * tClosest;

	// find the leading edge, and its distance from the location:
	const FVector leading_edge = Q + w * (float)len;
	const FVector leading_delta = leading_edge - loc;
	const double  leading_dist = (double)leading_delta.Size();

	// if the leading edge is not within the bounding sphere,
	if (leading_dist > radius) {
		// check to see if the closest point is between the ray's endpoints:
		const FVector delta1 = closest - Q;
		const FVector delta2 = leading_edge - Q; // this is w*len

		// if the closest point is not between the leading edge and the origin, no intersect:
		if (FVector::DotProduct(delta1, delta2) < 0.0f || (double)delta1.Size() > len) {
			return 0;
		}
	}

	// if not active, that's good enough:
	if (GetScene() == nullptr) {
		ipt = closest;
		return 1;
	}

	// transform ray into object space (legacy behavior preserved):
	const FMatrix& Xform = OrientationMatrix;

	FVector tmp = w;

	w.X = FVector::DotProduct(tmp, FVector((float)Xform.M[0][0], (float)Xform.M[0][1], (float)Xform.M[0][2]));
	w.Y = FVector::DotProduct(tmp, FVector((float)Xform.M[1][0], (float)Xform.M[1][1], (float)Xform.M[1][2]));
	w.Z = FVector::DotProduct(tmp, FVector((float)Xform.M[2][0], (float)Xform.M[2][1], (float)Xform.M[2][2]));

	tmp = Q - loc;

	Q.X = FVector::DotProduct(tmp, FVector((float)Xform.M[0][0], (float)Xform.M[0][1], (float)Xform.M[0][2]));
	Q.Y = FVector::DotProduct(tmp, FVector((float)Xform.M[1][0], (float)Xform.M[1][1], (float)Xform.M[1][2]));
	Q.Z = FVector::DotProduct(tmp, FVector((float)Xform.M[2][0], (float)Xform.M[2][1], (float)Xform.M[2][2]));

	double min = len;
	intersection_poly = nullptr;

	// check each polygon:
	ListIter<Surface> iter = model->surfaces;
	while (++iter) {
		Surface* srf = iter.value();
		Poly* p = srf->GetPolys();

		for (int i = 0; i < srf->NumPolys(); i++) {
			if (!treat_translucent_polys_as_solid && p->material && !p->material->IsSolid()) {
				++p;
				continue;
			}

			const FVector v = p->plane.normal;
			const double  d = p->plane.distance;

			const double denom = (double)FVector::DotProduct(w, v);

			if (denom < -1.0e-5) {
				const FVector P = v * (float)d;
				const double  ilen = (double)FVector::DotProduct((P - Q), v) / denom;

				if (ilen > 0.0 && ilen < min) {
					const FVector intersect = Q + w * (float)ilen;

					if (p->Contains(intersect)) {
						intersection_poly = p;
						ipt = intersect;
						min = ilen;
						impact = 1;
					}
				}
			}

			++p;
		}
	}

	// xform impact point back into world coordinates:
	if (impact) {
		ipt = OrientationMatrix.TransformVector(ipt) + loc;
	}

	return impact;
}

// +--------------------------------------------------------------------+

void
Solid::ClearModel()
{
	if (own_model && model) {
		delete model;
		model = nullptr;
	}

	radius = 0.0f;
}

// +--------------------------------------------------------------------+

void
Solid::UseModel(Model* m)
{
	ClearModel();

	own_model = false;
	model = m;
	radius = m ? m->radius : 0.0f;
}

// +--------------------------------------------------------------------+

bool
Solid::Load(const char* mag_file, double scale)
{
	ClearModel();

	model = new Model;
	own_model = true;

	if (model->Load(mag_file, scale)) {
		radius = model->radius;
		strncpy_s(name, sizeof(name), model->name, sizeof(name) - 1);
		return true;
	}

	ClearModel();
	return false;
}

bool
Solid::Load(ModelFile* mod_file, double scale)
{
	ClearModel();

	model = new Model;
	own_model = true;

	if (model->Load(mod_file, scale)) {
		radius = model->radius;
		return true;
	}

	ClearModel();
	return false;
}

// +--------------------------------------------------------------------+

bool
Solid::Rescale(double scale)
{
	if (!own_model || !model)
		return false;

	radius = 0.0f;

	ListIter<Surface> iter = model->GetSurfaces();
	while (++iter) {
		Surface* s = iter.value();

		for (int v = 0; v < s->NumVerts(); v++) {
			s->vertex_set->loc[v] *= (float)scale;
			s->vloc[v] *= (float)scale;

			const float lvi = s->vloc[v].Size();
			if (lvi > radius)
				radius = lvi;
		}
	}

	model->radius = radius;

	InvalidateSurfaceData();

	return true;
}

// +--------------------------------------------------------------------+

void
Solid::CreateShadows(int nlights)
{
	while (shadows.size() < nlights) {
		shadows.append(new Shadow(this));
	}
}

void
Solid::UpdateShadows(List<SimLight>& lights)
{
	List<SimLight>     active_lights;
	ListIter<SimLight> iter = lights;

	while (++iter) {
		SimLight* light = iter.value();

		if (light->IsActive() && light->CastsShadow()) {
			const double distance = (double)(Location() - light->Location()).Size();
			const double intensity = light->Intensity();

			if (light->Type() == SimLight::LIGHT_POINT) {
				if (distance > KINDA_SMALL_NUMBER && intensity / distance > 1.0)
					active_lights.append(light);
			}
			else if (light->Type() == SimLight::LIGHT_DIRECTIONAL) {
				if (intensity > 0.65)
					active_lights.insert(light);
			}
		}
	}

	iter.attach(active_lights);

	while (++iter) {
		SimLight* light = iter.value();
		const int idx = iter.index();

		if (idx < shadows.size()) {
			shadows[idx]->Update(light);
		}
	}
}

// +--------------------------------------------------------------------+

void
Solid::DeletePrivateData()
{
	if (model)
		model->DeletePrivateData();
}

// +--------------------------------------------------------------------+

void
Solid::InvalidateSurfaceData()
{
	if (!model)
		return;

	const bool invalidate = model->IsDynamic();

	ListIter<Surface> iter = model->GetSurfaces();
	while (++iter) {
		Surface* s = iter.value();
		VideoPrivateData* vpd = s->GetVideoPrivateData();

		if (vpd) {
			if (invalidate) {
				vpd->Invalidate();
			}
			else {
				delete vpd;
				s->SetVideoPrivateData(nullptr);
			}
		}
	}
}

void
Solid::InvalidateSegmentData()
{
	if (!model)
		return;

	const bool invalidate = model->IsDynamic();

	ListIter<Surface> iter = model->GetSurfaces();
	while (++iter) {
		Surface* s = iter.value();

		ListIter<Segment> seg_iter = s->GetSegments();
		while (++seg_iter) {
			Segment* segment = seg_iter.value();
			VideoPrivateData* vpd = segment->GetVideoPrivateData();

			if (vpd) {
				if (invalidate) {
					vpd->Invalidate();
				}
				else {
					delete vpd;
					segment->SetVideoPrivateData(nullptr);
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

bool
Solid::IsDynamic() const
{
	return model ? model->IsDynamic() : false;
}

void
Solid::SetDynamic(bool d)
{
	if (model && own_model)
		model->SetDynamic(d);
}

// +--------------------------------------------------------------------+
// Bitmap -> UTexture2D*: compile-safe stubs (Material must be UE-migrated)

void
Solid::GetAllTextures(List<UTexture2D*>& textures)
{
	if (model)
		model->GetAllTextures(textures);
}

void
Model::GetAllTextures(List<UTexture2D*>& textures)
{
	ListIter<Material> m_iter = materials;
	while (++m_iter) {
		Material* m = m_iter.value();

		if (m->tex_diffuse && !textures.contains(m->tex_diffuse))  textures.append(m->tex_diffuse);
		if (m->tex_specular && !textures.contains(m->tex_specular)) textures.append(m->tex_specular);
		if (m->tex_emissive && !textures.contains(m->tex_emissive)) textures.append(m->tex_emissive);
		if (m->tex_bumpmap && !textures.contains(m->tex_bumpmap))  textures.append(m->tex_bumpmap);
		if (m->tex_detail && !textures.contains(m->tex_detail))   textures.append(m->tex_detail);
	}
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

Model::Model()
	: nverts(0),
	npolys(0),
	radius(0.0f),
	luminous(false),
	dynamic(false)
{
	memset(name, 0, sizeof(name));
}

Model::Model(const Model& m)
	: nverts(0),
	npolys(0),
	radius(0.0f),
	luminous(false),
	dynamic(false)
{
	operator=(m);
}

Model::~Model()
{
	surfaces.destroy();
	materials.destroy();
}

Model&
Model::operator = (const Model& m)
{
	if (this != &m) {
		surfaces.destroy();
		materials.destroy();

		memcpy(name, m.name, Solid::NAMELEN);

		nverts = m.nverts;
		npolys = m.npolys;
		radius = m.radius;
		luminous = m.luminous;
		dynamic = m.dynamic;

		Model* pmod = (Model*)&m;

		ListIter<Material> m_iter = pmod->materials;
		while (++m_iter) {
			Material* matl1 = m_iter.value();
			Material* matl2 = new Material;

			memcpy(matl2, matl1, sizeof(Material));
			matl2->thumbnail = nullptr;

			materials.append(matl2);
		}

		ListIter<Surface> s_iter = pmod->surfaces;
		while (++s_iter) {
			Surface* surf1 = s_iter.value();
			Surface* surf2 = new Surface;

			surf2->Copy(*surf1, this);
			surfaces.append(surf2);
		}
	}

	return *this;
}

// +--------------------------------------------------------------------+

int
Model::NumSegments() const
{
	int nsegments = 0;

	for (int i = 0; i < surfaces.size(); i++) {
		const Surface* s = surfaces[i];
		nsegments += s->NumSegments();
	}

	return nsegments;
}

// +--------------------------------------------------------------------+

inline bool Collinear(const double* a, const double* b, const double* c)
{
	const FVector ab((float)(b[0] - a[0]), (float)(b[1] - a[1]), (float)(b[2] - a[2]));
	const FVector ac((float)(c[0] - a[0]), (float)(c[1] - a[1]), (float)(c[2] - a[2]));
	const FVector cross = FVector::CrossProduct(ab, ac);
	return cross.IsNearlyZero();
}

struct HomogenousPlane
{
	double distance;
	double normal_x;
	double normal_y;
	double normal_z;
	double normal_w;
};

static void LoadPlane(Plane& p, DataLoader* l, BYTE*& fp)
{
	HomogenousPlane tmp{};
	l->fread(&tmp, sizeof(HomogenousPlane), 1, fp);

	p.normal.x = tmp.normal_x;
	p.normal.y = tmp.normal_y;
	p.normal.z = tmp.normal_z;
	p.distance = tmp.distance;
}

static void LoadFlags(DWORD* out_flags, DataLoader* l, BYTE*& fp)
{
	DWORD magic_flags = 0;
	l->fread(&magic_flags, sizeof(DWORD), 1, fp);

	const DWORD magic_mask = 0x0fc3;
	*out_flags = magic_flags & magic_mask;
}

// +--------------------------------------------------------------------+

bool
Model::Load(const char* mag_file, double scale)
{
	BYTE* block = nullptr;
	DataLoader* loader = DataLoader::GetLoader();
	bool        result = false;

	radius = 0.0f;
	extents[0] = extents[1] = extents[2] = extents[3] = extents[4] = extents[5] = 0.0f;

	if (!loader) {
		UE_LOG(LogTemp, Warning, TEXT("MAG Open Failed: no data loader for file '%hs'"), mag_file ? mag_file : "");
		return result;
	}

	const int size = loader->LoadBuffer(mag_file, block);
	BYTE* fp = block;

	if (!size || !block) {
		UE_LOG(LogTemp, Warning, TEXT("MAG Open Failed: could not open file '%hs'"), mag_file ? mag_file : "");
		return result;
	}

	strncpy_s(name, sizeof(name), mag_file ? mag_file : "", 31);
	name[31] = 0;

	char file_id[5]{};
	memcpy(file_id, block, 4);
	file_id[4] = '\0';

	int version = 1;

	if (!strcmp(file_id, "MAG6")) {
		version = 6;
	}
	else if (!strcmp(file_id, "MAG5")) {
		version = 5;
	}
	else if (!strcmp(file_id, "MAG4")) {
		version = 4;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("MAG Open Failed: File '%hs' invalid file type '%hs'"),
			mag_file ? mag_file : "", file_id);
		loader->ReleaseBuffer(block);
		return result;
	}

	surfaces.destroy();
	materials.destroy();
	nverts = 0;
	npolys = 0;

	switch (version) {
	case 4:
	case 5:
		result = LoadMag5(block, size, scale);
		break;

	case 6:
		result = LoadMag6(block, size, scale);
		break;

	default:
		break;
	}

	loader->ReleaseBuffer(block);
	return result;
}

// +--------------------------------------------------------------------+

bool
Model::Load(ModelFile* mod_file, double scale)
{
	return mod_file ? mod_file->Load(this, scale) : false;
}

// +--------------------------------------------------------------------+

static int mcomp(const void* a, const void* b)
{
	Poly* pa = (Poly*)a;
	Poly* pb = (Poly*)b;

	if (pa->sortval == pb->sortval)
		return 0;

	if (pa->sortval < pb->sortval)
		return 1;

	return -1;
}

bool
Model::LoadMag5(BYTE* block, int len, double scale)
{
	bool result = false;

	DataLoader* loader = DataLoader::GetLoader();
	BYTE* fp = block + 4;
	int   ntex = 0;
	int   nsurfs = 0;

	loader->fread(&ntex, sizeof(ntex), 1, fp);
	loader->fread(&nsurfs, sizeof(nsurfs), 1, fp);

	// create a default gray material:
	Material* mtl = new Material;

	if (mtl) {
		mtl->Ka = Color::LightGray;
		mtl->Kd = Color::LightGray;
		mtl->Ks = ColorValue(0.2f, 0.2f, 0.2f);
		mtl->power = 20.0f;

		mtl->ambient_value = 1.0f;
		mtl->ambient_color = Color::LightGray;
		mtl->diffuse_value = 1.0f;
		mtl->diffuse_color = Color::LightGray;
		mtl->specular_value = 0.2f;
		mtl->specular_color = Color::White;
		strcpy_s(mtl->name, "(default)");

		materials.append(mtl);
	}

	// read texture list:
	for (int i = 0; i < ntex; i++) {
		mtl = new Material;
		char tname[32]{};

		if (mtl) {
			mtl->Ka = ColorValue(0.5f, 0.5f, 0.5f);
			mtl->Kd = ColorValue(1.0f, 1.0f, 1.0f);
			mtl->Ks = ColorValue(0.2f, 0.2f, 0.2f);
			mtl->power = 20.0f;

			mtl->ambient_value = 1.0f;
			mtl->ambient_color = Color::Gray;
			mtl->diffuse_value = 1.0f;
			mtl->diffuse_color = Color::White;
			mtl->specular_value = 0.2f;
			mtl->specular_color = Color::White;

			loader->fread(tname, 32, 1, fp);

			// Bitmap -> UTexture2D* (tex_diffuse must be UTexture2D* in Material):
			loader->LoadTexture(tname, mtl->tex_diffuse, /*type*/0, /*mipmaps*/true);

			strcpy_s(mtl->name, tname);

			char* dot = strrchr(mtl->name, '.');
			if (dot) *dot = 0;

			char* plus = strrchr(mtl->name, '+');
			if (plus) *plus = 0;

			materials.append(mtl);
		}
	}

	loader->fread(&nverts, 4, 1, fp);
	loader->fread(&npolys, 4, 1, fp);

	// plan on creating four verts per poly:
	const int mag_nverts = nverts;
	int       next_vert = nverts;

	nverts = npolys * 4;

	Surface* s = new Surface;
	VertexSet* vset = nullptr;

	if (s) {
		strcpy_s(s->name, "default");

		s->model = this;
		s->vertex_set = new VertexSet(nverts);
		s->vloc = new FVector[nverts];

		// UE-safe init (VertexSet loc/nrm are assumed FVector*):
		memset(s->vertex_set->loc, 0, nverts * sizeof(FVector));
		memset(s->vertex_set->nrm, 0, nverts * sizeof(FVector));
		memset(s->vertex_set->diffuse, 0, nverts * sizeof(DWORD));
		memset(s->vertex_set->specular, 0, nverts * sizeof(DWORD));
		memset(s->vertex_set->tu, 0, nverts * sizeof(float));
		memset(s->vertex_set->tv, 0, nverts * sizeof(float));
		memset(s->vertex_set->rw, 0, nverts * sizeof(float));
		memset(s->vloc, 0, nverts * sizeof(FVector));

		s->npolys = npolys;
		s->polys = new Poly[npolys];

		memset(s->polys, 0, sizeof(Poly) * npolys);
		surfaces.append(s);

		vset = s->vertex_set;

		int v = 0;

		// read vertex set:
		for (v = 0; v < mag_nverts; v++) {
			FVector vert(0, 0, 0);
			FVector norm(0, 0, 0);
			DWORD   vstate = 0;

			loader->fread(&vert, sizeof(FVector), 1, fp);
			loader->fread(&norm, sizeof(FVector), 1, fp);
			loader->fread(&vstate, sizeof(DWORD), 1, fp);

			// legacy SwapYZ + scale:
			Swap(vert.Y, vert.Z);
			vert *= (float)scale;

			vset->loc[v] = vert;
			vset->nrm[v] = norm;

			const double d = (double)vert.Size();
			if (d > radius)
				radius = (float)d;

			if (vert.X > extents[0]) extents[0] = vert.X;
			if (vert.X < extents[1]) extents[1] = vert.X;
			if (vert.Y > extents[2]) extents[2] = vert.Y;
			if (vert.Y < extents[3]) extents[3] = vert.Y;
			if (vert.Z > extents[4]) extents[4] = vert.Z;
			if (vert.Z < extents[5]) extents[5] = vert.Z;
		}

		while (v < nverts)
			vset->nrm[v++] = FVector(1, 0, 0);

		// read polys:
		FVector dummy_center(0, 0, 0);
		DWORD   dummy_flags = 0;
		DWORD   dummy_color = 0;
		Plane   dummy_plane;
		int     texture_num = 0;
		int     poly_nverts = 0;
		int     vert_index_buffer[32]{};
		float   texture_index_buffer[32]{};

		for (int n = 0; n < npolys; n++) {
			Poly& poly = s->polys[n];
			poly.vertex_set = vset;

			loader->fread(&dummy_flags, sizeof(DWORD), 1, fp);
			loader->fread(&dummy_center, sizeof(FVector), 1, fp);

			LoadPlane(dummy_plane, loader, fp);

			loader->fread(&dummy_color, sizeof(DWORD), 1, fp);
			loader->fread(&texture_num, sizeof(int), 1, fp);

			if (texture_num >= 0 && texture_num < ntex) {
				const int mtl_num = texture_num + 1;
				poly.material = materials[mtl_num];
				poly.sortval = texture_num;

				const bool flag_translucent = (dummy_flags & 0x04) ? true : false;
				const bool flag_transparent = (dummy_flags & 0x08) ? true : false;

				// luminous
				if (dummy_flags & 2) {
					mtl = materials[mtl_num];

					mtl->Ka = ColorValue(0, 0, 0, 0);
					mtl->Kd = ColorValue(0, 0, 0, 0);
					mtl->Ks = ColorValue(0, 0, 0, 0);
					mtl->Ke = ColorValue(1, 1, 1, 1);

					if (mtl->tex_diffuse && !mtl->tex_emissive)
						mtl->tex_emissive = mtl->tex_diffuse;
				}

				// glowing (additive)
				if (flag_translucent && flag_transparent)
					materials[mtl_num]->blend = Material::MTL_ADDITIVE;

				// translucent (alpha)
				else if (flag_translucent)
					materials[mtl_num]->blend = Material::MTL_TRANSLUCENT;

				// transparent (use alpha)
				else if (flag_transparent)
					materials[mtl_num]->blend = Material::MTL_TRANSLUCENT;
			}
			else {
				poly.material = materials.first();
				poly.sortval = 1000;
			}

			// hack: store flat shaded flag in unused visible byte
			poly.visible = (BYTE)(dummy_flags & 1);

			loader->fread(&poly_nverts, sizeof(int), 1, fp);
			loader->fread(vert_index_buffer, sizeof(int), poly_nverts, fp);

			if (poly_nverts == 3)      s->nindices += 3;
			else if (poly_nverts == 4) s->nindices += 6;

			poly.nverts = poly_nverts;
			for (int vi = 0; vi < poly_nverts; vi++) {
				v = vert_index_buffer[vi];

				if (vset->rw[v] > 0) {
					vset->CopyVertex(next_vert, v);
					v = next_vert++;
				}

				vset->rw[v] = 1;
				poly.verts[vi] = v;
			}

			loader->fread(texture_index_buffer, sizeof(float), poly_nverts, fp); // tu's
			for (int vi = 0; vi < poly_nverts; vi++) {
				v = poly.verts[vi];
				vset->tu[v] = texture_index_buffer[vi];
			}

			loader->fread(texture_index_buffer, sizeof(float), poly_nverts, fp); // tv's
			for (int vi = 0; vi < poly_nverts; vi++) {
				v = poly.verts[vi];
				vset->tv[v] = texture_index_buffer[vi];
			}

			fp += 16;
		}

		// pass 2 (adjust vertex normals for flat polys):
		for (int n = 0; n < npolys; n++) {
			Poly& poly = s->polys[n];

			poly.plane = Plane(
				vset->loc[poly.verts[0]],
				vset->loc[poly.verts[2]],
				vset->loc[poly.verts[1]]
			);

			// hack: retrieve flat shaded flag from unused visible byte
			if (poly.visible) {
				poly_nverts = poly.nverts;

				for (int vi = 0; vi < poly_nverts; vi++) {
					v = poly.verts[vi];
					vset->nrm[v] = poly.plane.normal;
				}
			}
		}

		// sort the polys by material index:
		qsort((void*)s->polys, s->npolys, sizeof(Poly), mcomp);

		// then assign them to cohesive segments:
		Segment* segment = nullptr;

		for (int n = 0; n < npolys; n++) {
			if (segment && segment->material == s->polys[n].material) {
				segment->npolys++;
			}
			else {
				segment = nullptr;
			}

			if (!segment) {
				segment = new Segment;

				segment->npolys = 1;
				segment->polys = &s->polys[n];
				segment->material = segment->polys->material;
				segment->model = this;

				s->segments.append(segment);
			}
		}

		s->BuildHull();
		result = (nverts != 0) && (npolys != 0);
	}

	return result;
}

// +--------------------------------------------------------------------+

struct MaterialMag6
{
	char  name[Material::NAMELEN];
	char  shader[Material::NAMELEN];
	float power;      // highlight sharpness (big=shiny)
	float brilliance; // diffuse power function
	float bump;       // bump level (0=none)
	DWORD blend;      // alpha blend type
	bool  shadow;     // material casts shadow
	bool  luminous;   // verts have their own lighting

	Color ambient_color;
	Color diffuse_color;
	Color specular_color;
	Color emissive_color;

	float ambient_value;
	float diffuse_value;
	float specular_value;
	float emissive_value;

	BYTE  tex_diffuse;
	BYTE  tex_specular;
	BYTE  tex_bumpmap;
	BYTE  tex_emissive;
};

// +--------------------------------------------------------------------+

bool
Model::LoadMag6(BYTE* block, int len, double scale)
{
	bool result = false;

	DataLoader* loader = DataLoader::GetLoader();
	BYTE* fp = block + 4;
	int   ntex = 0;
	int   nmtls = 0;
	int   nsurfs = 0;

	// Bitmap -> UTexture2D*
	List<UTexture2D*> textures;

	loader->fread(&ntex, sizeof(ntex), 1, fp); // size of texture block
	loader->fread(&nmtls, sizeof(nmtls), 1, fp); // number of materials
	loader->fread(&nsurfs, sizeof(nsurfs), 1, fp); // number of surfaces

	// read texture list:
	if (ntex) {
		char* buffer = new char[ntex];
		char* p = buffer;

		loader->fread(buffer, ntex, 1, fp);

		while (p < buffer + ntex) {
			UTexture2D* tex = nullptr;
			loader->LoadTexture(p, tex, /*type*/0, /*mipmaps*/true);
			textures.append(tex);

			p += strlen(p) + 1;
		}

		delete[] buffer;
	}

	for (int i = 0; i < nmtls; i++) {
		MaterialMag6 m6{};
		Material* mtl = new Material;

		loader->fread(&m6, sizeof(m6), 1, fp);

		if (mtl) {
			memcpy(mtl->name, m6.name, Material::NAMELEN);
			memcpy(mtl->shader, m6.shader, Material::NAMELEN);

			mtl->ambient_value = m6.ambient_value;
			mtl->ambient_color = m6.ambient_color;
			mtl->diffuse_value = m6.diffuse_value;
			mtl->diffuse_color = m6.diffuse_color;
			mtl->specular_value = m6.specular_value;
			mtl->specular_color = m6.specular_color;
			mtl->emissive_value = m6.emissive_value;
			mtl->emissive_color = m6.emissive_color;

			mtl->Ka = ColorValue(mtl->ambient_color) * mtl->ambient_value;
			mtl->Kd = ColorValue(mtl->diffuse_color) * mtl->diffuse_value;
			mtl->Ks = ColorValue(mtl->specular_color) * mtl->specular_value;
			mtl->Ke = ColorValue(mtl->emissive_color) * mtl->emissive_value;

			mtl->power = m6.power;
			mtl->brilliance = m6.brilliance;
			mtl->bump = m6.bump;
			mtl->blend = m6.blend;
			mtl->shadow = m6.shadow;
			mtl->luminous = m6.luminous;

			if (m6.tex_diffuse && m6.tex_diffuse <= textures.size())
				mtl->tex_diffuse = textures[m6.tex_diffuse - 1];

			if (m6.tex_specular && m6.tex_specular <= textures.size())
				mtl->tex_specular = textures[m6.tex_specular - 1];

			if (m6.tex_emissive && m6.tex_emissive <= textures.size())
				mtl->tex_emissive = textures[m6.tex_emissive - 1];

			if (m6.tex_bumpmap && m6.tex_bumpmap <= textures.size())
				mtl->tex_bumpmap = textures[m6.tex_bumpmap - 1];

			materials.append(mtl);
		}
	}

	for (int i = 0; i < nsurfs; i++) {
		int  nverts_local = 0;
		int  npolys_local = 0;
		BYTE namelen = 0;
		char surf_name[128]{};

		loader->fread(&nverts_local, 4, 1, fp);
		loader->fread(&npolys_local, 4, 1, fp);
		loader->fread(&namelen, 1, 1, fp);

		// Ensure a valid, null-terminated surface name:
		if (namelen > 0) {
			const int safe_len = (namelen < (BYTE)(sizeof(surf_name) - 1)) ? namelen : (int)(sizeof(surf_name) - 1);
			loader->fread(surf_name, 1, safe_len, fp);
			surf_name[safe_len] = 0;

			if (safe_len < namelen)
				fp += (namelen - safe_len);
		}
		else {
			surf_name[0] = 0;
		}

		Surface* surface = new Surface;
		surface->model = this;
		surface->SetName(surf_name);
		surface->CreateVerts(nverts_local);
		surface->CreatePolys(npolys_local);

		VertexSet* vset = surface->GetVertexSet();
		Poly* polys = surface->GetPolys();

		memset(polys, 0, sizeof(Poly) * npolys_local);

		// read vertex set:
		for (int v = 0; v < nverts_local; v++) {
			loader->fread(&vset->loc[v], sizeof(float), 3, fp);
			loader->fread(&vset->nrm[v], sizeof(float), 3, fp);
			loader->fread(&vset->tu[v], sizeof(float), 1, fp);
			loader->fread(&vset->tv[v], sizeof(float), 1, fp);

			vset->loc[v] *= (float)scale;

			const FVector vert = vset->loc[v];

			const double d = (double)vert.Size();
			if (d > radius)
				radius = (float)d;

			if (vert.X > extents[0]) extents[0] = vert.X;
			if (vert.X < extents[1]) extents[1] = vert.X;
			if (vert.Y > extents[2]) extents[2] = vert.Y;
			if (vert.Y < extents[3]) extents[3] = vert.Y;
			if (vert.Z > extents[4]) extents[4] = vert.Z;
			if (vert.Z < extents[5]) extents[5] = vert.Z;
		}

		// read polys:
		for (int n = 0; n < npolys_local; n++) {
			Poly& poly = polys[n];

			BYTE poly_nverts = 0;
			BYTE material_index = 0;
			WORD poly_verts[8]{};

			loader->fread(&poly_nverts, sizeof(BYTE), 1, fp);
			loader->fread(&material_index, sizeof(BYTE), 1, fp);
			loader->fread(&poly_verts[0], sizeof(WORD), poly_nverts, fp);

			if (poly_nverts >= 3) {
				poly.nverts = poly_nverts;

				for (int vi = 0; vi < poly_nverts; vi++)
					poly.verts[vi] = poly_verts[vi];
			}
			else {
				poly.sortval = 666;
			}

			if (material_index > 0) {
				poly.material = materials[material_index - 1];
				poly.sortval = material_index;
			}
			else if (materials.size()) {
				poly.material = materials.first();
				poly.sortval = 1;
			}
			else {
				poly.sortval = 1000;
			}

			if (poly.nverts == 3)
				surface->AddIndices(3);
			else if (poly.nverts == 4)
				surface->AddIndices(6);

			poly.vertex_set = vset;

			if (poly.nverts >= 3) {
				poly.plane = Plane(
					vset->loc[poly.verts[0]],
					vset->loc[poly.verts[2]],
					vset->loc[poly.verts[1]]
				);
			}
		}

		// sort the polys by material index:
		qsort((void*)polys, npolys_local, sizeof(Poly), mcomp);

		// then assign them to cohesive segments:
		Segment* segment = nullptr;

		for (int n = 0; n < npolys_local; n++) {
			if (segment && segment->material == polys[n].material) {
				segment->npolys++;
			}
			else {
				segment = nullptr;
			}

			if (!segment) {
				segment = new Segment;

				segment->npolys = 1;
				segment->polys = &polys[n];
				segment->material = segment->polys->material;
				segment->model = this;

				surface->GetSegments().append(segment);
			}
		}

		surface->BuildHull();
		surfaces.append(surface);

		this->nverts += nverts_local;
		this->npolys += npolys_local;
	}

	result = (nverts != 0) && (npolys != 0);
	return result;
}

void
Surface::Copy(Surface& s, Model* m)
{
	segments.destroy();

	delete    opcode;
	delete    vertex_set;
	delete[] vloc;
	delete[] polys;
	delete    video_data;

	memcpy(name, s.name, Solid::NAMELEN);

	model = m;
	radius = s.radius;
	nhull = s.nhull;
	npolys = s.npolys;
	nindices = s.nindices;
	state = s.state;
	offset = s.offset;
	orientation = s.orientation;
	opcode = 0;
	video_data = 0;

	vertex_set = s.vertex_set ? s.vertex_set->Clone() : 0;

	if (nhull > 0) {
		vloc = new FVector[nhull];
		memcpy(vloc, s.vloc, nhull * sizeof(FVector));
	}
	else {
		vloc = 0;
	}

	polys = new Poly[npolys];
	memcpy(polys, s.polys, npolys * sizeof(Poly));

	for (int i = 0; i < npolys; i++) {
		polys[i].vertex_set = vertex_set;

		if (s.polys[i].material)
			polys[i].material = (Material*)model->FindMaterial(s.polys[i].material->name);
	}

	ListIter<Segment> iter = s.segments;
	while (++iter) {
		Segment* seg1 = iter.value();
		Segment* seg2 = new Segment;

		seg2->npolys = seg1->npolys;
		seg2->polys = polys + (seg1->polys - s.polys);

		if (seg2->polys[0].material)
			seg2->material = seg2->polys[0].material;

		seg2->model = model;
		seg2->video_data = 0;

		segments.append(seg2);
	}
}

// +--------------------------------------------------------------------+

void
Surface::SetName(const char* n)
{
	const int len = (int)sizeof(name);

	memset(name, 0, len);
	strncpy_s(name, n ? n : "", len - 1);
}

void
Surface::SetHidden(bool b)
{
	if (b) state = state | HIDDEN;
	else   state = state & ~HIDDEN;
}

void
Surface::SetLocked(bool b)
{
	if (b) state = state | LOCKED;
	else   state = state & ~LOCKED;
}

void
Surface::SetSimplified(bool b)
{
	if (b) state = state | SIMPLE;
	else   state = state & ~SIMPLE;
}

void
Surface::CreateVerts(int nverts)
{
	if (!vertex_set && !vloc) {
		vertex_set = new VertexSet(nverts);
		vloc = new FVector[nverts];
	}
}

void
Surface::CreatePolys(int np)
{
	if (!polys && !npolys) {
		npolys = np;
		polys = new Poly[npolys];

		memset(polys, 0, npolys * sizeof(Poly));
	}
}

// +--------------------------------------------------------------------+

Poly*
Surface::AddPolys(int np, int nv)
{
	if (polys && vertex_set &&
		np > 0 && np + npolys < MAX_POLYS &&
		nv > 0 && nv + vertex_set->nverts < MAX_VERTS)
	{
		const int newverts = nv + vertex_set->nverts;
		const int newpolys = np + npolys;

		vertex_set->Resize(newverts, true);

		Poly* pset = new Poly[newpolys];
		Poly* pnew = pset + npolys;

		memcpy(pset, polys, npolys * sizeof(Poly));
		memset(pnew, 0, np * sizeof(Poly));

		if (segments.size() > 0) {
			Segment* seg = segments.last();
			Material* mtl = seg->material;

			for (int i = 0; i < np; i++) {
				Poly* p = pnew + i;
				p->material = mtl;
			}

			seg->npolys += np;
		}

		// Adopt the new poly array and release the old one:
		delete[] polys;
		polys = pset;
		npolys = newpolys;

		return pnew;
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
Surface::ExplodeMesh()
{
	if (!vertex_set || vertex_set->nverts < 3)
		return;

	int nverts = 0;

	// count max verts:
	for (int i = 0; i < npolys; i++) {
		Poly* p = polys + i;
		nverts += p->nverts;
	}

	// create target vertex set:
	VertexSet* vset = new VertexSet(nverts);
	int v = 0;

	// explode verts:
	for (int i = 0; i < npolys; i++) {
		Poly* p = polys + i;
		p->vertex_set = vset;

		for (int j = 0; j < p->nverts; j++) {
			const int vsrc = p->verts[j];

			vset->loc[v] = vertex_set->loc[vsrc];
			vset->nrm[v] = vertex_set->nrm[vsrc];
			vset->tu[v] = vertex_set->tu[vsrc];
			vset->tv[v] = vertex_set->tv[vsrc];
			vset->rw[v] = vertex_set->rw[vsrc];
			vset->diffuse[v] = vertex_set->diffuse[vsrc];
			vset->specular[v] = vertex_set->specular[vsrc];

			p->verts[j] = v++;
		}
	}

	// finalize:
	delete vertex_set;
	vertex_set = vset;

	delete[] vloc;
	vloc = new FVector[nverts];

	ComputeTangents();
	BuildHull();
}

// +--------------------------------------------------------------------+

const double SELECT_EPSILON = 0.05;
const double SELECT_TEXTURE = 0.0001;

static bool MatchVerts(VertexSet* vset, int i, int j)
{
	double         d = 0;
	const FVector& vl1 = vset->loc[i];
	const FVector& vn1 = vset->nrm[i];
	float          tu1 = vset->tu[i];
	float          tv1 = vset->tv[i];
	const FVector& vl2 = vset->loc[j];
	const FVector& vn2 = vset->nrm[j];
	float          tu2 = vset->tu[j];
	float          tv2 = vset->tv[j];

	d = fabs(vl1.X - vl2.X);
	if (d > SELECT_EPSILON) return false;

	d = fabs(vl1.Y - vl2.Y);
	if (d > SELECT_EPSILON) return false;

	d = fabs(vl1.Z - vl2.Z);
	if (d > SELECT_EPSILON) return false;

	d = fabs(vn1.X - vn2.X);
	if (d > SELECT_EPSILON) return false;

	d = fabs(vn1.Y - vn2.Y);
	if (d > SELECT_EPSILON) return false;

	d = fabs(vn1.Z - vn2.Z);
	if (d > SELECT_EPSILON) return false;

	d = fabs(tu1 - tu2);
	if (d > SELECT_TEXTURE) return false;

	d = fabs(tv1 - tv2);
	if (d > SELECT_TEXTURE) return false;

	return true;
}

void
Surface::OptimizeMesh()
{
	if (!vertex_set || vertex_set->nverts < 3)
		return;

	int nverts = vertex_set->nverts;
	int used = 0;
	int final = 0;
	int nmatch = 0;

	// create vertex maps:
	BYTE* vert_map = new BYTE[nverts];
	WORD* vert_dst = new WORD[nverts];

	memset(vert_map, 0, nverts * sizeof(BYTE));
	memset(vert_dst, 0, nverts * sizeof(WORD));

	// count used verts:
	for (int i = 0; i < npolys; i++) {
		Poly* p = polys + i;

		for (int j = 0; j < p->nverts; j++) {
			const WORD vert = p->verts[j];

			if (vert < nverts) {
				vert_map[vert]++;
				used++;
			}
		}
	}

	// create target vertex set:
	VertexSet* vset = new VertexSet(used);
	int v = 0;

	// compress verts:
	for (int i = 0; i < nverts; i++) {
		if (vert_map[i] == 0) continue;

		vert_dst[i] = v;
		vset->loc[v] = vertex_set->loc[i];
		vset->nrm[v] = vertex_set->nrm[i];
		vset->tu[v] = vertex_set->tu[i];
		vset->tv[v] = vertex_set->tv[i];
		vset->rw[v] = vertex_set->rw[i];
		vset->diffuse[v] = vertex_set->diffuse[i];
		vset->specular[v] = vertex_set->specular[i];

		for (int j = i + 1; j < nverts; j++) {
			if (vert_map[j] == 0) continue;

			if (MatchVerts(vertex_set, i, j)) {
				vert_map[j] = 0;
				vert_dst[j] = v;
				nmatch++;
			}
		}

		v++;
	}

	final = v;

	// remap polys:
	for (int n = 0; n < npolys; n++) {
		Poly* p = polys + n;
		p->vertex_set = vset;

		for (int vi = 0; vi < p->nverts; vi++) {
			p->verts[vi] = vert_dst[p->verts[vi]];
		}
	}

	// finalize:
	if (final < nverts) {
		delete vertex_set;
		vertex_set = vset;

		vertex_set->Resize(final, true);
		nverts = final;
	}
	else {
		// no compression benefit; keep original set:
		delete vset;
		vset = 0;
	}

	// clean up and rebuild hull:
	delete[] vert_map;
	delete[] vert_dst;

	delete[] vloc;
	vloc = new FVector[nverts];

	ComputeTangents();
	BuildHull();
}

// +--------------------------------------------------------------------+

void
Surface::ScaleBy(double factor)
{
	offset *= factor;

	if (vertex_set && vertex_set->nverts) {
		for (int i = 0; i < vertex_set->nverts; i++) {
			vertex_set->loc[i] *= (float)factor;
		}
	}
}

// +--------------------------------------------------------------------+

void
Surface::BuildHull()
{
	if (npolys < 1 || !vertex_set || vertex_set->nverts < 1)
		return;

	nhull = 0;

	for (int i = 0; i < npolys; i++) {
		Poly* p = polys + i;

		for (int n = 0; n < p->nverts; n++) {
			WORD v = p->verts[n];
			WORD h;

			for (h = 0; h < nhull; h++) {
				FVector& vl = vertex_set->loc[v];
				FVector& loc = vloc[h];

				double d = vl.X - loc.X;
				if (d < -SELECT_EPSILON || d > SELECT_EPSILON) continue;

				d = vl.Y - loc.Y;
				if (d < -SELECT_EPSILON || d > SELECT_EPSILON) continue;

				d = vl.Z - loc.Z;
				if (d < -SELECT_EPSILON || d > SELECT_EPSILON) continue;

				// found a match:
				break;
			}

			// didn't find a match:
			if (h >= nhull) {
				vloc[h] = vertex_set->loc[v];
				nhull = h + 1;
			}

			p->vlocs[n] = h;
		}
	}

	if (use_collision_detection)
		InitializeCollisionHull();
}

// +--------------------------------------------------------------------+

void
Surface::Normalize()
{
	if (npolys < 1 || !vertex_set || vertex_set->nverts < 1)
		return;

	// STEP ONE: initialize poly planes
	for (int i = 0; i < npolys; i++) {
		Poly* p = polys + i;

		p->plane = Plane(
			vertex_set->loc[p->verts[0]],
			vertex_set->loc[p->verts[2]],
			vertex_set->loc[p->verts[1]]
		);
	}

	// STEP TWO: compute vertex normals by averaging adjacent poly planes
	List<Poly> faces;
	for (int v = 0; v < vertex_set->nverts; v++) {
		faces.clear();
		SelectPolys(faces, vertex_set->loc[v]);

		if (faces.size()) {
			vertex_set->nrm[v] = FVector(0.0f, 0.0f, 0.0f);

			for (int i = 0; i < faces.size(); i++) {
				vertex_set->nrm[v] += faces[i]->plane.normal;
			}

			vertex_set->nrm[v].Normalize();
		}
		else if (vertex_set->loc[v].Size() > 0) {
			vertex_set->nrm[v] = vertex_set->loc[v];
			vertex_set->nrm[v].Normalize();
		}
		else {
			vertex_set->nrm[v] = FVector(0.0f, 1.0f, 0.0f);
		}
	}

	// STEP THREE: adjust vertex normals for poly flatness
	for (int i = 0; i < npolys; i++) {
		Poly* p = polys + i;

		for (int n = 0; n < p->nverts; n++) {
			int v = p->verts[n];

			vertex_set->nrm[v] =
				vertex_set->nrm[v] * (1.0f - p->flatness) +
				p->plane.normal * (p->flatness);
		}
	}
}

void
Surface::SelectPolys(List<Poly>& selection, FVector loc)
{
	for (int i = 0; i < npolys; i++) {
		Poly* p = polys + i;

		for (int n = 0; n < p->nverts; n++) {
			int     v = p->verts[n];
			FVector& vl = vertex_set->loc[v];

			double d = vl.X - loc.X;
			if (d < -SELECT_EPSILON || d > SELECT_EPSILON) continue;

			d = vl.Y - loc.Y;
			if (d < -SELECT_EPSILON || d > SELECT_EPSILON) continue;

			d = vl.Z - loc.Z;
			if (d < -SELECT_EPSILON || d > SELECT_EPSILON) continue;

			selection.append(p);
			break;
		}
	}
}

void
Surface::SelectPolys(List<Poly>& selection, Material* m)
{
	for (int i = 0; i < npolys; i++) {
		Poly* p = polys + i;

		if (p->material == m)
			selection.append(p);
	}
}

void
Surface::ComputeTangents()
{
	FVector tangent(0.0f, 0.0f, 0.0f);
	FVector binormal(0.0f, 0.0f, 0.0f);

	if (!vertex_set || !vertex_set->nverts)
		return;

	if (vertex_set->tangent)
		return;

	vertex_set->CreateTangents();

	for (int i = 0; i < npolys; i++) {
		Poly* p = polys + i;

		CalcGradients(*p, tangent, binormal);

		for (int n = 0; n < p->nverts; n++) {
			vertex_set->tangent[p->verts[n]] = tangent;
			vertex_set->binormal[p->verts[n]] = binormal;
		}
	}
}

void
Surface::CalcGradients(Poly& p, FVector& tangent, FVector& binormal)
{
	// Using Eric Lengyel's approach with minor adaptations.

	VertexSet* vset = p.vertex_set;
	if (!vset || p.nverts < 3)
		return;

	const FVector P = vset->loc[p.verts[1]] - vset->loc[p.verts[0]];
	const FVector Q = vset->loc[p.verts[2]] - vset->loc[p.verts[0]];

	const float s1 = vset->tu[p.verts[1]] - vset->tu[p.verts[0]];
	const float t1 = vset->tv[p.verts[1]] - vset->tv[p.verts[0]];
	const float s2 = vset->tu[p.verts[2]] - vset->tu[p.verts[0]];
	const float t2 = vset->tv[p.verts[2]] - vset->tv[p.verts[0]];

	float tmp = 1.0f;
	const float denom = s1 * t2 - s2 * t1;

	if (fabsf(denom) > 0.0001f)
		tmp = 1.0f / denom;

	tangent.X = (t2 * P.X - t1 * Q.X) * tmp;
	tangent.Y = (t2 * P.Y - t1 * Q.Y) * tmp;
	tangent.Z = (t2 * P.Z - t1 * Q.Z) * tmp;
	tangent.Normalize();

	binormal.X = (s1 * Q.X - s2 * P.X) * tmp;
	binormal.Y = (s1 * Q.Y - s2 * P.Y) * tmp;
	binormal.Z = (s1 * Q.Z - s2 * P.Z) * tmp;
	binormal.Normalize();
}

void
Surface::InitializeCollisionHull()
{
	opcode = new OPCODE_data(this);
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

Segment::Segment()
	: npolys(0), polys(0), material(0), model(0), video_data(0)
{
	// Do not ZeroMemory(this, ...) on a C++ object; it is undefined behavior.
}

Segment::Segment(int n, Poly* p, Material* mtl, Model* mod)
	: npolys(n), polys(p), material(mtl), model(mod), video_data(0)
{
}

Segment::~Segment()
{
	delete video_data;
	video_data = 0;

	// Do not ZeroMemory(this, ...) in a destructor; also undefined behavior.
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

ModelFile::ModelFile(const char* fname)
	: model(0), pname(0), pnverts(0), pnpolys(0), pradius(0)
{
	const int len = (int)sizeof(filename);

	memset(filename, 0, len);
	strncpy_s(filename, fname ? fname : "", len - 1);
	filename[len - 1] = 0;
}

ModelFile::~ModelFile()
{
}

bool
ModelFile::Load(Model* m, double scale)
{
	model = m;

	// expose model innards for child classes:
	if (model) {
		pname = model->name;
		pnverts = &model->nverts;
		pnpolys = &model->npolys;
		pradius = &model->radius;
	}

	return false;
}

bool
ModelFile::Save(Model* m)
{
	model = m;

	// expose model innards for child classes:
	if (model) {
		pname = model->name;
		pnverts = &model->nverts;
		pnpolys = &model->npolys;
		pradius = &model->radius;
	}

	return false;
}

