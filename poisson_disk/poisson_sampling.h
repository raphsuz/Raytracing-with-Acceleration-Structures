#ifndef _POISSON_SAMPLING_H_
#define _POISSON_SAMPLING_H_
#include <vector>

namespace Poisson_sampling {

struct Vec3 {
    Vec3() { x = y = z = 0; }
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) { }
    float x,y,z;
};

void poisson_disk(float radius,
                  int samples,
                  const std::vector<Vec3>& verts,
                  const std::vector<Vec3>& nors,
                  const std::vector<int>& tris,
                  std::vector<Vec3>& samples_pos,
                  std::vector<Vec3>& samples_nor);

}

#endif