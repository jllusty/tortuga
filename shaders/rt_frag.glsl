precision highp float;

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
vec3  lightDirection = vec3(-0.577, 0.577, -0.577);

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

struct Plane{
	vec3 position;
	vec3 normal;
};

// global =========================================================================================
Plane plane;

// function =======================================================================================
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
	float res = float(voxelRes);
	float root = ceil(sqrt(res));

	// initial coords, transform to [0,63)^3
	vec3 origin = res*(vec3(i.u,i.v,i.w)+1.)/2.;
	// voxel coords
	ivec3 map = ivec3(origin);
	// values of t s.t. tDelta[i]*ray[i] == 1
	vec3 tDelta = abs(1.0/ray);
	// direction of step
	ivec3 stepAmount = ivec3(0);
	// values of t that hit the next voxel
	vec3 tMax = vec3(0.);
	// side of voxel we hit
	int side = 0;
	for(int j = 0; j < 3; ++j) {
		if(ray[j] < 0.0) {
			stepAmount[j] = -1;
			tMax[j] = fract(origin[j])*tDelta[j];
		}
		else {
			stepAmount[j] = 1;
			tMax[j] = (1.0-fract(origin[j]))*tDelta[j];
		}
	}
	//int maxSteps = 1000;
	// voxel traversal (DDA)
	while(true) {
		if(tMax.x < tMax.y) {
			if(tMax.x < tMax.z) {
				map.x += stepAmount.x;
				tMax.x += tDelta.x;
				side = 0;
			}
			else {
				map.z += stepAmount.z;
				tMax.z += tDelta.z;
				side = 1;
			}
		}
		else {
			if(tMax.y < tMax.z) {
				map.y += stepAmount.y;
				tMax.y += tDelta.y;
				side = 2;
			}
			else {
				map.z += stepAmount.z;
				tMax.z += tDelta.z;
				side = 1;
			}
		}
		// 3D sampler coords
		// if we are outside the boundaries, stop
		if((map.x<0)||(map.x>=voxelRes)) break;
		if((map.y<0)||(map.y>=voxelRes)) break;
		if((map.z<0)||(map.z>=voxelRes)) break;
		// convert to coords in 2D texture
		int uw = voxelRes*int(floor(mod(float(map.z),root)));
		int vw = voxelRes*(map.z/int(root));
		float ii = (float(map.x+uw)+0.5)/res/root;
		float jj = (float(map.y+vw)+0.5)/res/root;

		vec4 s = texture2D(voxels,vec2(ii,jj));	
		// did we hit anything?
		if(s.x > 0.0) {
			float dist = 0.;
			vec3 dest = vec3(0.);
			if(side == 0) {
				dist = (float(map.x) - origin.x + float((1 - stepAmount.x)/2))/ray.x;
			}
			else if (side == 1) {
				dist = (float(map.z) - origin.z + float((1 - stepAmount.z)/2))/ray.z;
			}
			else {
				dist = (float(map.y) - origin.y + float((1 - stepAmount.y)/2))/ray.y;
			}
			dest = origin + dist * ray;
			dest = 2.*(dest-res/2.)/res;
			float dx = fract(dest.x);
			float dy = fract(dest.y);
			float dz = fract(dest.z);

			float ambient = 0.2;
			float diffuse = max(0., dot(lightDirection,dest));

			vec3 color = vec3(dx,dy,dz);

			// move back to edge of voxel
			i.color.rgb = (diffuse+ambient)*color;
			break;
		}
	}
}

vec3 castRay(vec3 ray) {
	// plane init
	//plane.position = vec3(0.0, -1.0, 0.0);
	//plane.normal = vec3(0.0, 0.0, 1.0);
	
	// intersect init
	Intersection i;
	i.t = 1.0e+30;
	i.hit = 0.0;
	i.hitPoint = vec3(0.0);
	i.normal = vec3(0.0);
	i.color = vec3(.2,.3,.3);

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

	return color;
}

// main ===========================================================================================
void main(void){
	// wiggle about
	cPos = 2.0*vec3(cos(tiden/4.),sin(tiden/4.),(1.+cos(tiden/8.))/2.);
	//cPos = 0.75*vec3(1.);
	vec3 target = vec3(0.,0.,0.35);
	cDir = normalize(target-cPos);

	//lightDirection = normalize(vec3(cos(tiden),sin(tiden),1.));
	lightDirection = -cDir;

	// ray init
	vec3 cSide = cross(cDir, cUp);
	const float pi = 4.0 * atan(1.0);
	const float fov = 60.0/180.*pi;
	float f = 2./2./tan(fov/2.);
	// frag pos
	vec2 p = ((gl_FragCoord.xy) * 2.0 - resolution) / min(resolution.x, resolution.y);
	vec3 ray = normalize(cSide * p.x + cUp * p.y + cDir * f);

	// ray casting
	vec3 color = castRay(ray);

	// done
	gl_FragColor = vec4(color,1.0);
}