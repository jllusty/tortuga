precision mediump float;

// time
uniform float tiden;

// 2D representation of voxels
uniform sampler2D voxels;
uniform int voxelRes;		// resolution = dimension of height, width, depth
//uniform vec3 voxelPos;		// position of center of voxels, assumed at origin
//uniform vec4 voxelOri;		// orientation, could be a quaternion, assumed standard here

uniform vec2 resolution;
// const ==========================================================================================
vec3  cPos = vec3(3.0,  0.0,  0.0);
vec3  cDir = vec3(-1.0,  0.0, 0.0);
const vec3  cUp  = vec3(0.0,  0.0,  1.0);
const float targetDepth = 1.0;
const vec3  lightDirection = vec3(0.577, 0.577, -0.577);

// struct =========================================================================================
struct Intersection{
	float t;
	float hit;
	vec3  hitPoint;
	vec3  normal;
	vec3  color;
	float u;
	float v;
	float w;
};

struct Sphere{
	vec3  position;
	float radius;
};

struct Plane{
	vec3 position;
	vec3 normal;
};

// global =========================================================================================
Sphere sphere;
Plane plane;

// function =======================================================================================
void intersectSphere(vec3 ray, Sphere s, inout Intersection i){
	vec3  a = cPos - s.position;
	float b = dot(a, ray);
	float c = dot(a, a) - (s.radius * s.radius);
	float d = b * b - c;
	if(d > 0.0){
		float t = -b - sqrt(d);
		if(t > 0.0 && t < i.t){
			i.t = t;
			i.hit = 1.0;
			i.hitPoint = vec3(
				cPos.x + ray.x * t,
				cPos.y + ray.y * t,
				cPos.z + ray.z * t
			);
			i.normal = normalize(i.hitPoint - s.position);
			float diff = clamp(dot(i.normal, lightDirection), 0.1, 1.0);
			i.color = vec3(diff);
		}
	}
}

void intersectPlane(vec3 ray, Plane p, inout Intersection i){
	float d = -dot(p.position, p.normal);
	float v = dot(ray, p.normal);
	float t = -(dot(cPos, p.normal) + d) / v;
	if(t > 0.0 && t < i.t){
		i.t = t;
		i.hit = 1.0;
		i.hitPoint = vec3(
			cPos.x + t * ray.x,
			cPos.y + t * ray.y,
			cPos.z + t * ray.z
		);
		i.normal = p.normal;
		float diff = clamp(dot(i.normal, lightDirection), 0.1, 1.0);
		float m = mod(i.hitPoint.x, 2.0);
		float n = mod(i.hitPoint.z, 2.0);
		if((m > 1.0 && n > 1.0) || (m < 1.0 && n < 1.0)){
			diff -= 0.5;
		}
		
		t = min(i.hitPoint.z, 100.0) * 0.01;
		i.color = vec3(diff + t);
	}
}

void intersectFace(vec3 ray, const vec3 center, const vec3 basisU, 
	const vec3 basisV, inout Intersection i) 
{
	//
	vec3 normal = center;
	vec3 origin = cPos;
	float denom = dot(normal,ray);
	if(denom < 0.0001) {
		float t = dot(center-origin,normal)/denom;
		if(t >= 0.) {
			vec3 hit = origin + ray*t;
			vec3 d = hit-center;
			float u = dot(d,basisU);
			float v = dot(d,basisV);
			if(max(abs(u),abs(v)) <= 1.0) {
				i.hit = 1.0;
				i.hitPoint = hit;
				i.u = i.hitPoint.x;
				i.v = i.hitPoint.y;
				i.w = i.hitPoint.z;
			}
		}
	}
}

float max3(vec3 v) {
	return max(max(v.x,v.y),v.z);
}
void intersectFaces(vec3 ray, inout Intersection i)
{
	vec3 d = cPos;
	if(max3(abs(cPos)) < 1.0) {
		i.hit = 1.0;
		i.hitPoint = cPos;
		i.u = cPos.x;
		i.v = cPos.y;
		i.w = cPos.z;
		return;
	}
	const vec3 front = vec3(1.,0.,0.);
	const vec3 right = vec3(0.,1.,0.);
	const vec3 top = vec3(0.,0.,1.);
	// top/bottom of cube
	intersectFace(ray, top, front, right, i);
	if(i.hit > 0.0) return;
	intersectFace(ray, -top, -front, -right, i);
	if(i.hit > 0.0) return;
	// front/back of cube
	intersectFace(ray, front, right, top, i);
	if(i.hit > 0.0) return;
	intersectFace(ray, -front, -right, -top, i);
	if(i.hit > 0.0) return;
	// right/left of cube
	intersectFace(ray, right, -front, top, i);
	if(i.hit > 0.0) return;
	intersectFace(ray, -right, front, -top, i);
	if(i.hit > 0.0) return;
}

void intersect(vec3 ray, inout Intersection i){
	//intersectSphere(ray, sphere, i);
	intersectPlane(ray, plane, i);
	intersectFaces(ray, i);
}

void sampleVoxels(vec3 ray, inout Intersection i) {
	// get i,j,k of hitPoint, assume we hit the "front", i.e. i = 64
	float res = float(voxelRes);
	float root = ceil(sqrt(res));
	//ivec3 ijk = ivec3(res,int(64.*(i.u-1.0)),int(64.*(i.v-1.0)));
	//i.u = res*(i.u/2.0+0.5);
	//i.v = res*(i.v/2.0+0.5);

	// initial coords
	float iu = res*(i.u/2.0+0.5);
		iu = min(max(iu,0.),res-1.);
	float iv = res*(i.v/2.0+0.5);
		iv = min(max(iv,0.),res-1.);
	float iw = res*(i.w/2.0+0.5);
		iw = min(max(iw,0.),res-1.);

	// DDA until we can't no mo
	float dx = ray.x, dy = ray.y, dz = ray.z;
	float step = max(abs(dx),max(abs(dy),abs(dz)));
	dx /= step; dy /= step; dz /= step;
	int maxSteps = int(res*sqrt(3.));
	for(int c = 0; c < maxSteps; ++c) {
		//ivec3 ijk = ivec3(res,int(64.*(i.u-1.0)),int(64.*(i.v-1.0)));

		// sampler
		float u = iu + float(c)*dx;
		float v = iv + float(c)*dy;
		float w = iw + float(c)*dz;
		if((u<0.)||(u>res-1.)) break;
		if((v<0.)||(v>res-1.)) break;
		if((w<0.)||(w>res-1.)) break;
		float uw = res*floor(mod(w,root));
		float vw = res*floor(w/root);
		// sampler in 2D
		float ii = (u + uw + 0.5)/res/root;
		float jj = (v + vw + 0.5)/res/root;

		// did we hit anything?
		vec4 s = texture2D(voxels,vec2(ii,jj));	
		if(s.x > 0.0) {
			i.color.r = 1.0;
			i.color.b = 1.0;
			i.color.g = 1.0;
			break;
		}
	}
}

// main ===========================================================================================
void main(void){
	// wiggle about
	cPos.y -= 1.0*sin(tiden);
	cPos = 1.0*vec3(cos(tiden), sin(tiden), 1.*sin(tiden));
	cDir = -normalize(cPos);

	// fragment position
	vec2 p = (gl_FragCoord.xy * 2.0 - resolution) / min(resolution.x, resolution.y);
	
	// ray init
	vec3 cSide = cross(cDir, cUp);
	const float pi = 4.0 * atan(1.0);
	const float fov = 60.0/180.*pi;
	float f = 2./2./tan(fov/2.);
	vec3 ray = normalize(cSide * p.x + cUp * p.y + cDir * f);
	
	// sphere init
	sphere.position = vec3(1.0*sin(tiden));
	sphere.radius = 1.0;
	
	// plane init
	plane.position = vec3(0.0, -1.0, 0.0);
	plane.normal = vec3(0.0, 0.0, 1.0);
	
	// intersect init
	Intersection i;
	i.t = 1.0e+30;
	i.hit = 0.0;
	i.hitPoint = vec3(0.0);
	i.normal = vec3(0.0);
	i.color = vec3(0.0);

	vec3 color = vec3(0.);
	// check voxels
	intersectFaces(ray, i);
	if(i.hit>0.0) {
		sampleVoxels(ray,i);
		//gl_FragColor = vec4(i.color,1.);
		color = i.color;
	}
	else {
		// check plane
		//intersectPlane(ray, plane, i);
		//color = (i.hit>0.0) ? i.color : vec3(0.,0.,0.);
		color = vec3(.2,.3,.3);
	}

	// done
	gl_FragColor = vec4(color,1.0);
}