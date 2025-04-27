class vpuTriangle

{
private:
  /* data */
public:
  int x1, y1; // Vertex 1 coordinates
  int x2, y2; // Vertex 2 coordinates
  int x3, y3; // Vertex 3 coordinates
  int u1, v1; // Texture coordinates for vertex 1
  int u2, v2; // Texture coordinates for vertex 2
  int u3, v3; // Texture coordinates for vertex 3
  int z1, z2, z3; // Depth values for the vertices
vpuTriangle::vpuTriangle(int x1,int y1,int x2,int y2,int x3,int y3,int u1,int v1,int u2,int v2,int u3,int v3,int z1,int z2,int z3)
{
  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;
  this->x3 = x3;
  this->y3 = y3;
  this->u1 = u1;
  this->v1 = v1;
  this->u2 = u2;
  this->v2 = v2;
  this->u3 = u3;
  this->v3 = v3;
  this->z1 = z1;
  this->z2 = z2;
  this->z3 = z3;
}

vpuTriangle::~vpuTriangle()
{
}
};




