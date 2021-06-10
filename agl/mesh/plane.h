// Copyright 2011, OpenGL 4.0 Shading language cookbook (David Wolf 2011)

#ifndef AGL_PLANE_H_
#define AGL_PLANE_H_

#include "agl/triangle_mesh.h"

namespace agl {

class Plane : public TriangleMesh
{
 public:
  Plane(float xsize, float zsize, int xdivs, int zdivs,
      float smax = 1.0f, float tmax = 1.0f);
};

}  // namespace agl
#endif  // AGL_PLANE_H_