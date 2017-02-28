#version 330

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

out vec3 fColor;

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;

uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;

uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uShininessSampler;

flat in vec3 vertex0;
flat in vec3 vertex1;
flat in vec3 vertex2;

uniform float voxelSize;
uniform vec3 origBBox;
uniform int numVoxels;


bool planeBoxOverlap(in vec3 normal, in float d, in float maxVox)
{
        vec3 vMin, vMax;
        if(normal.x > 0.0){
                vMin.x = -maxVox;
                vMax.x = maxVox;
        }
        else{
                vMin.x = maxVox;
                vMax.x = -maxVox;
        }
        if(normal.y > 0.0){
                vMin.y = -maxVox;
                vMax.y = maxVox;
        }
        else{
                vMin.y = maxVox;
                vMax.y = -maxVox;
        }
        if(normal.z > 0.0){
                vMin.z = -maxVox;
                vMax.z = maxVox;
        }
        else{
                vMin.z = maxVox;
                vMax.z = -maxVox;
        }
        if (dot(normal, vMin) + d > 0.0) { return false; }
        if (dot(normal, vMax) + d >= 0.0) { return true; }
        if (dot(normal, vMax) + d >= 0.0) { return true; }
        return false;
}

bool triBoxOverlap(in vec3 voxCenter, in float voxHalfSize, in vec3 vertex0, in vec3 vertex1, in vec3 vertex2)
{
        vec3 v0, v1, v2, e0, e1, e2, fe0, fe1, fe2, normal;
        float minValue, maxValue, p0, p1, p2, rad, d;

        v0 = vertex0 - voxCenter;
        v1 = vertex1 - voxCenter;
        v2 = vertex2 - voxCenter;
        e0 = v1 - v0;
        e1 = v2 - v1;
        e2 = v0 - v2;
        fe0 = abs(e0);

        // AXISTEST_X01(e0.z, e0.y, fe0.z, fe0.y)
        p0 = e0.z * v0.y - e0.y * v0.z;
        p2 = e0.z * v2.y - e0.y * v2.z;
        if (p0 < p2){
                minValue = p0;
                maxValue = p2;
        }
        else{
                minValue = p2;
                maxValue = p0;
        }
        rad = fe0.z * voxHalfSize + fe0.y * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return false;

        // AXISTEST_Y02(e0.z, e0.x, fe0.z, fe0.x)
        p0 = -e0.z * v0.x + e0.x * v0.z;
        p2 = -e0.z * v2.x + e0.x * v2.z;
        if (p0 < p2){
                minValue = p0;
                maxValue = p2;
        }
        else{
                minValue = p2;
                maxValue = p0;
        }
        rad = fe0.z * voxHalfSize + fe0.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return false;

        // AXISTEST_Z12(e0.y, e0.x, fe0.y, fe0.x)
        p1 = e0.y * v1.x - e0.x * v1.y;
        p2 = e0.y * v2.x - e0.x * v2.y;
        if (p2 < p1){
                minValue = p2;
                maxValue = p1;
        }
        else{
                minValue = p1;
                maxValue = p2;
        }
        rad = fe0.y * voxHalfSize + fe0.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return false;
        fe1 = abs(e1);

        // AXISTEST_X01(e1.z, e1.y, fe1.z, fe1.y)
        p0 = e1.z * v0.y - e1.y * v0.z;
        p2 = e1.z * v2.y - e1.y * v2.z;
        if (p0 < p2){
                minValue = p0;
                maxValue = p2;
        }
        else{
                minValue = p2;
                maxValue = p0;
        }
        rad = fe1.z * voxHalfSize + fe1.y * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return false;

        // AXISTEST_Y02(e1.z, e1.x, fe1.z, fe1.x)
        p0 = -e1.z * v0.x + e1.x * v0.z;
        p2 = -e1.z * v2.x + e1.x * v2.z;
        if (p0 < p2){
                minValue = p0;
                maxValue = p2;
        }
        else{
                minValue = p2;
                maxValue = p0;
        }
        rad = fe1.z * voxHalfSize + fe1.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return false;

        // AXISTEST_Z0(e1.y, e1.x, fe1.y, fe1.x)
        p0 = e1.y * v0.x - e1.x * v0.y;
        p1 = e1.y * v1.x - e1.x * v1.y;
        if (p0 < p1){
                minValue = p0;
                maxValue = p1;
        }
        else{
                minValue = p1;
                maxValue = p0;
        }
        rad = fe1.y * voxHalfSize + fe1.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return false;
        fe2 = abs(e2);

        // AXISTEST_X2(e2.z, e2.y, fe2.z, fe2.y)
        p0 = e2.z * v0.y - e2.y * v0.z;
        p1 = e2.z * v1.y - e2.y * v1.z;
        if (p0 < p1){
                minValue = p0;
                maxValue = p1;
        }
        else{
                minValue = p1;
                maxValue = p0;
        }
        rad = fe2.z * voxHalfSize + fe2.y * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return false;

        // AXISTEST_Y1(e2.z, e2.x, fe2.z, fe2.x)
        p0 = -e2.z * v0.x + e2.x * v0.z;
        p1 = -e2.z * v1.x + e2.x * v1.z;
        if (p0 < p1){
                minValue = p0;
                maxValue = p1;
        }
        else{
                minValue = p1;
                maxValue = p0;
        }
        rad = fe2.z * voxHalfSize + fe2.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return false;

        // AXISTEST_Z12(e2.y, e2.x, fe2.y, fe2.x)
        p0 = e2.y * v1.x - e2.x * v1.y;
        p1 = e2.y * v2.x - e2.x * v2.y;
        if (p0 < p1){
                minValue = p0;
                maxValue = p1;
        }
        else{
                minValue = p1;
                maxValue = p0;
        }
        rad = fe2.y * voxHalfSize + fe2.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return false;

        // FINDMINMAX(v0.z, v1.z, v2.z, minValue, maxValue)
        minValue = maxValue = v0.z;
        minValue = min(min(minValue, v1.z), v2.z);
        maxValue = max(max(maxValue, v1.z), v2.z);
        if (minValue > voxHalfSize || maxValue < -voxHalfSize) return false;

        // FINDMINMAX(v0.x, v1.x, v2.x, minValue, maxValue)
        minValue = maxValue = v0.x;
        minValue = min(min(minValue, v1.x), v2.x);
        maxValue = max(max(maxValue, v1.x), v2.x);
        if (minValue > voxHalfSize || maxValue < -voxHalfSize) return false;

        // FINDMINMAX(v0.y, v1.y, v2.y, minValue, maxValue)
        minValue = maxValue = v0.y;
        minValue = min(min(minValue, v1.y), v2.y);
        maxValue = max(max(maxValue, v1.y), v2.y);
        if (minValue > voxHalfSize || maxValue < -voxHalfSize) return false;

        normal = cross(e0, e1);
        d = -dot(normal, v0);
        if (!planeBoxOverlap(normal, d, voxHalfSize) ) return false;
        return true;
}


int triBoxOverlap3(in vec3 voxCenter, in float voxHalfSize, in vec3 vertex0, in vec3 vertex1, in vec3 vertex2)
{
        vec3 v0, v1, v2, e0, e1, e2, fe0, fe1, fe2, normal;
        float minValue, maxValue, p0, p1, p2, rad, d;

        v0 = vertex0 - voxCenter;
        v1 = vertex1 - voxCenter;
        v2 = vertex2 - voxCenter;
        e0 = v1 - v0;
        e1 = v2 - v1;
        e2 = v0 - v2;
        fe0 = abs(e0);

        // AXISTEST_X01(e0.z, e0.y, fe0.z, fe0.y)
        p0 = e0.z * v0.y - e0.y * v0.z;
        p2 = e0.z * v2.y - e0.y * v2.z;
        if (p0 < p2){
                minValue = p0;
                maxValue = p2;
        }
        else{
                minValue = p2;
                maxValue = p0;
        }
        rad = fe0.z * voxHalfSize + fe0.y * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return 1;

        // AXISTEST_Y02(e0.z, e0.x, fe0.z, fe0.x)
        p0 = -e0.z * v0.x + e0.x * v0.z;
        p2 = -e0.z * v2.x + e0.x * v2.z;
        if (p0 < p2){
                minValue = p0;
                maxValue = p2;
        }
        else{
                minValue = p2;
                maxValue = p0;
        }
        rad = fe0.z * voxHalfSize + fe0.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return 2;

        // AXISTEST_Z12(e0.y, e0.x, fe0.y, fe0.x)
        p1 = e0.y * v1.x - e0.x * v1.y;
        p2 = e0.y * v2.x - e0.x * v2.y;
        if (p2 < p1){
                minValue = p2;
                maxValue = p1;
        }
        else{
                minValue = p1;
                maxValue = p2;
        }
        rad = fe0.y * voxHalfSize + fe0.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return 3;
        fe1 = abs(e1);

        // AXISTEST_X01(e1.z, e1.y, fe1.z, fe1.y)
        p0 = e1.z * v0.y - e1.y * v0.z;
        p2 = e1.z * v2.y - e1.y * v2.z;
        if (p0 < p2){
                minValue = p0;
                maxValue = p2;
        }
        else{
                minValue = p2;
                maxValue = p0;
        }
        rad = fe1.z * voxHalfSize + fe1.y * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return 4;

        // AXISTEST_Y02(e1.z, e1.x, fe1.z, fe1.x)
        p0 = -e1.z * v0.x + e1.x * v0.z;
        p2 = -e1.z * v2.x + e1.x * v2.z;
        if (p0 < p2){
                minValue = p0;
                maxValue = p2;
        }
        else{
                minValue = p2;
                maxValue = p0;
        }
        rad = fe1.z * voxHalfSize + fe1.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return 5;

        // AXISTEST_Z0(e1.y, e1.x, fe1.y, fe1.x)
        p0 = e1.y * v0.x - e1.x * v0.y;
        p1 = e1.y * v1.x - e1.x * v1.y;
        if (p0 < p1){
                minValue = p0;
                maxValue = p1;
        }
        else{
                minValue = p1;
                maxValue = p0;
        }
        rad = fe1.y * voxHalfSize + fe1.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return 6;
        fe2 = abs(e2);

        // AXISTEST_X2(e2.z, e2.y, fe2.z, fe2.y)
        p0 = e2.z * v0.y - e2.y * v0.z;
        p1 = e2.z * v1.y - e2.y * v1.z;
        if (p0 < p1){
                minValue = p0;
                maxValue = p1;
        }
        else{
                minValue = p1;
                maxValue = p0;
        }
        rad = fe2.z * voxHalfSize + fe2.y * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return 7;

        // AXISTEST_Y1(e2.z, e2.x, fe2.z, fe2.x)
        p0 = -e2.z * v0.x + e2.x * v0.z;
        p1 = -e2.z * v1.x + e2.x * v1.z;
        if (p0 < p1){
                minValue = p0;
                maxValue = p1;
        }
        else{
                minValue = p1;
                maxValue = p0;
        }
        rad = fe2.z * voxHalfSize + fe2.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return 8;

        // AXISTEST_Z12(e2.y, e2.x, fe2.y, fe2.x)
        p0 = e2.y * v1.x - e2.x * v1.y;
        p1 = e2.y * v2.x - e2.x * v2.y;
        if (p0 < p1){
                minValue = p0;
                maxValue = p1;
        }
        else{
                minValue = p1;
                maxValue = p0;
        }
        rad = fe2.y * voxHalfSize + fe2.x * voxHalfSize;
        if (minValue > rad || maxValue < -rad) return 9;

        // FINDMINMAX(v0.z, v1.z, v2.z, minValue, maxValue)
        minValue = maxValue = v0.z;
        minValue = min(min(minValue, v1.z), v2.z);
        maxValue = max(max(maxValue, v1.z), v2.z);
        if (minValue > voxHalfSize || maxValue < -voxHalfSize) return 10;

        // FINDMINMAX(v0.x, v1.x, v2.x, minValue, maxValue)
        minValue = maxValue = v0.x;
        minValue = min(min(minValue, v1.x), v2.x);
        maxValue = max(max(maxValue, v1.x), v2.x);
        if (minValue > voxHalfSize || maxValue < -voxHalfSize) return 11;

        // FINDMINMAX(v0.y, v1.y, v2.y, minValue, maxValue)
        minValue = maxValue = v0.y;
        minValue = min(min(minValue, v1.y), v2.y);
        maxValue = max(max(maxValue, v1.y), v2.y);
        if (minValue > voxHalfSize || maxValue < -voxHalfSize) return 12;

        normal = cross(e0, e1);
        d = -dot(normal, v0);
        if (!planeBoxOverlap(normal, d, voxHalfSize) ) return 13;
        return 0;
}


#define X 0
#define Y 1
#define Z 2

#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2];

#define FINDMINMAX(x0,x1,x2,min,max) \
  min = max = x0;   \
  if(x1<min) min=x1;\
  if(x1>max) max=x1;\
  if(x2<min) min=x2;\
  if(x2>max) max=x2;

int planeBoxOverlap2(float normal[3],float d, vec3 maxbox)
{
  int q;
  float vmin[3],vmax[3];
  for(q=X;q<=Z;q++)
  {
    if(normal[q]>0.0f)
    {
      vmin[q]=-maxbox[q];
      vmax[q]=maxbox[q];
    }
    else
    {
      vmin[q]=maxbox[q];
      vmax[q]=-maxbox[q];
    }
  }
  if(DOT(normal,vmin)+d>0.0f) return 0;
  if(DOT(normal,vmax)+d>=0.0f) return 1;

  return 0;
}


/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)             \
    p0 = a*v0[Y] - b*v0[Z];                    \
    p2 = a*v2[Y] - b*v2[Z];                    \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)              \
    p0 = a*v0[Y] - b*v0[Z];                    \
    p1 = a*v1[Y] - b*v1[Z];                    \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)             \
    p0 = -a*v0[X] + b*v0[Z];                   \
    p2 = -a*v2[X] + b*v2[Z];                       \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)              \
    p0 = -a*v0[X] + b*v0[Z];                   \
    p1 = -a*v1[X] + b*v1[Z];                       \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)             \
    p1 = a*v1[X] - b*v1[Y];                    \
    p2 = a*v2[X] - b*v2[Y];                    \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
    if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)              \
    p0 = a*v0[X] - b*v0[Y];                \
    p1 = a*v1[X] - b*v1[Y];                    \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
    if(min>rad || max<-rad) return 0;

int triBoxOverlap2(vec3 boxcenter,vec3 boxhalfsize,vec3 triverts[3])
{

  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */
   float v0[3],v1[3],v2[3];
   float min,max,d,p0,p1,p2,rad,fex,fey,fez;
   float normal[3],e0[3],e1[3],e2[3];

   /* This is the fastest branch on Sun */
   /* move everything so that the boxcenter is in (0,0,0) */
   SUB(v0,triverts[0],boxcenter);
   SUB(v1,triverts[1],boxcenter);
   SUB(v2,triverts[2],boxcenter);

   /* compute triangle edges */
   SUB(e0,v1,v0);      /* tri edge 0 */
   SUB(e1,v2,v1);      /* tri edge 1 */
   SUB(e2,v0,v2);      /* tri edge 2 */

   /* Bullet 3:  */
   /*  test the 9 tests first (this was faster) */
   fex = abs(e0[X]);
   fey = abs(e0[Y]);
   fez = abs(e0[Z]);
   AXISTEST_X01(e0[Z], e0[Y], fez, fey);
   AXISTEST_Y02(e0[Z], e0[X], fez, fex);
   AXISTEST_Z12(e0[Y], e0[X], fey, fex);

   fex = abs(e1[X]);
   fey = abs(e1[Y]);
   fez = abs(e1[Z]);
   AXISTEST_X01(e1[Z], e1[Y], fez, fey);
   AXISTEST_Y02(e1[Z], e1[X], fez, fex);
   AXISTEST_Z0(e1[Y], e1[X], fey, fex);

   fex = abs(e2[X]);
   fey = abs(e2[Y]);
   fez = abs(e2[Z]);
   AXISTEST_X2(e2[Z], e2[Y], fez, fey);
   AXISTEST_Y1(e2[Z], e2[X], fez, fex);
   AXISTEST_Z12(e2[Y], e2[X], fey, fex);

   /* Bullet 1: */
   /*  first test overlap in the {x,y,z}-directions */
   /*  find min, max of the triangle each direction, and test for overlap in */
   /*  that direction -- this is equivalent to testing a minimal AABB around */
   /*  the triangle against the AABB */

   /* test in X-direction */
   FINDMINMAX(v0[X],v1[X],v2[X],min,max);
   if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;

   /* test in Y-direction */
   FINDMINMAX(v0[Y],v1[Y],v2[Y],min,max);
   if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;

   /* test in Z-direction */
   FINDMINMAX(v0[Z],v1[Z],v2[Z],min,max);
   if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;

   /* Bullet 2: */
   /*  test if the box intersects the plane of the triangle */
   /*  compute plane equation of triangle: normal*x+d=0 */
   CROSS(normal,e0,e1);
   d=-DOT(normal,v0);  /* plane eq: normal.x+d=0 */
   if(0 == planeBoxOverlap2(normal,d,boxhalfsize)) return 0;

   return 1;   /* box and triangle overlaps */
}


void main()
{
    //vec3 ka = uKa * vec3(texture(uKaSampler, vTexCoords));
    //vec3 kd = uKd * vec3(texture(uKdSampler, vTexCoords));
    //vec3 ks = uKs * vec3(texture(uKsSampler, vTexCoords));
    //float shininess = uShininess * vec3(texture(uShininessSampler, vTexCoords)).x;

    //vec3 normal = normalize(vViewSpaceNormal);
    //vec3 eyeDir = normalize(-vViewSpacePosition);

    //float distToPointLight = length(uPointLightPosition - vViewSpacePosition);
    //vec3 dirToPointLight = (uPointLightPosition - vViewSpacePosition) / distToPointLight;
    //vec3 pointLightIncidentLight = uPointLightIntensity / (distToPointLight * distToPointLight);

    //// half vectors, for blinn-phong shading
    //vec3 hPointLight = normalize(eyeDir + dirToPointLight);
    //vec3 hDirLight = normalize(eyeDir + uDirectionalLightDir);

    //float dothPointLight = shininess == 0 ? 1.f : max(0.f, dot(normal, hPointLight));
    //float dothDirLight = shininess == 0 ? 1.f :max(0.f, dot(normal, hDirLight));

    //if (shininess != 1.f && shininess != 0.f)
    //{
    //    dothPointLight = pow(dothPointLight, shininess);
    //    dothDirLight = pow(dothDirLight, shininess);
    //}

    //fColor = ka;
    //fColor += kd * (uDirectionalLightIntensity * max(0.f, dot(normal, uDirectionalLightDir)) + pointLightIncidentLight * max(0., dot(normal, dirToPointLight)));
    //fColor += ks * (uDirectionalLightIntensity * dothDirLight + pointLightIncidentLight * dothPointLight);

    vec3 voxCenter;
    float targetDepth;
    float halfVoxelSize = voxelSize * 0.5;

    voxCenter.xy = origBBox.xy + vec2(voxelSize) * gl_FragCoord.xy;
    voxCenter.z = origBBox.z;

    vec3 centerT = (vertex0 + vertex1 + vertex2) / 3.0;

   // for (int index = 0; index <= numVoxels; ++index){
   //     //for (float index = voxDepthRange.x; index <= voxDepthRange.y; ++index){
   //         voxCenter.z = origBBox.z + voxelSize * (index + 0.5);
   //         vec3 triverts[3] = vec3[](vertex0, vertex1, vertex2);
   //         if (1 == triBoxOverlap2(voxCenter, vec3(halfVoxelSize), triverts)){
   //             fColor = vec3(1, 1, 1);
   //             return;
   //         }
   //}

   //vec3 triverts[3] = vec3[](vec3(-10000, -10000, 0), vec3(10000, -10000, 0), vec3(0, 10000, 0));
   //if (1 == triBoxOverlap2(voxCenter, vec3(halfVoxelSize, halfVoxelSize, 100.0 * halfVoxelSize * (numVoxels + 1.0)), triverts)){
   //     fColor = vec3(1, 1, 1);
   //     return;
   // }

   vec3 triverts[3] = vec3[](vec3(-1, -1, 0), vec3(1, -1, 0), vec3(0, 1, 0));
   int r = triBoxOverlap3(vec3(0), 1, triverts[0], triverts[1], triverts[2]);
   if (0 == r){
        fColor = vec3(1, 1, 1);
        return;
    }

    fColor = vec3(0, 0, 0);

   switch(r)
   {
   case 1:
    fColor = vec3(1, 0, 0);
    case 2:
    fColor = vec3(0, 1, 0);
    case 3:
    fColor = vec3(0, 0, 1);
    case 4:
    fColor = vec3(1, 1, 1);
    case 5:
    fColor = vec3(1, 0, 1);
    case 6:
    fColor = vec3(0, 1, 1);
    case 7:
    fColor = vec3(1, 0, 0);
    case 8:
    fColor = vec3(0, 1, 0);
    case 9:
    fColor = vec3(0, 0, 1);
    case 10:
    fColor = vec3(1, 1, 0);
    case 11:
    fColor = vec3(1, 0, 1);
    case 12:
    fColor = vec3(0, 1, 1);
    case 13:
    fColor = vec3(0.5, 0.5, 0.5);
    }


}