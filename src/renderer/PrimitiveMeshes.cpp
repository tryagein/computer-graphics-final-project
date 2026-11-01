#include "renderer/PrimitiveMeshes.hpp"
#include <cmath>
namespace scene_editor {
Mesh makeCube(){std::vector<Vertex>v;std::vector<unsigned>i;const glm::vec3 p[8]={{-.5F,-.5F,-.5F},{.5F,-.5F,-.5F},{.5F,.5F,-.5F},{-.5F,.5F,-.5F},{-.5F,-.5F,.5F},{.5F,-.5F,.5F},{.5F,.5F,.5F},{-.5F,.5F,.5F}};const unsigned faces[6][4]={{0,1,2,3},{5,4,7,6},{4,0,3,7},{1,5,6,2},{3,2,6,7},{4,5,1,0}};for(auto&f:faces){glm::vec3 n=glm::normalize(glm::cross(p[f[1]]-p[f[0]],p[f[2]]-p[f[0]]));unsigned base=unsigned(v.size());for(int j=0;j<4;++j)v.push_back({p[f[j]],n,{float(j==1||j==2),float(j>=2)}});i.insert(i.end(),{base,base+1,base+2,base,base+2,base+3});}Mesh m;m.upload(v,i);return m;}
Mesh makePlane(){std::vector<Vertex>v={{{-.5F,0,-.5F},{0,1,0},{0,0}},{{.5F,0,-.5F},{0,1,0},{1,0}},{{.5F,0,.5F},{0,1,0},{1,1}},{{-.5F,0,.5F},{0,1,0},{0,1}}};Mesh m;m.upload(v,{0,2,1,0,3,2});return m;}
Mesh makeSphere(unsigned rings,unsigned sectors){std::vector<Vertex>v;std::vector<unsigned>i;for(unsigned y=0;y<=rings;++y){float a=3.14159265F*y/rings;for(unsigned x=0;x<=sectors;++x){float b=6.2831853F*x/sectors;glm::vec3 n{std::sin(a)*std::cos(b),std::cos(a),std::sin(a)*std::sin(b)};v.push_back({n*.5F,n,{float(x)/sectors,float(y)/rings}});}}for(unsigned y=0;y<rings;++y)for(unsigned x=0;x<sectors;++x){unsigned a=y*(sectors+1)+x,b=a+sectors+1;i.insert(i.end(),{a,b,a+1,a+1,b,b+1});}Mesh m;m.upload(v,i);return m;}
}
