precision mediump float;

const vec2 resolution = vec2(800.0,600.0);
// const ==========================================================================================
const vec3  cPos = vec3(0.0,  0.0,  3.0);
const vec3  cDir = vec3(0.0,  0.0, -1.0);
const vec3  cUp  = vec3(0.0,  1.0,  0.0);
const float targetDepth = 1.0;
const vec3  lightDirection = vec3(-0.577, 0.577, 0.577);

// struct =========================================================================================
struct Intersection{
	float t;
	float hit;
	vec3  hitPoint;
	vec3  normal;
	vec3  color;
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

void intersect(vec3 ray, inout Intersection i){
	intersectSphere(ray, sphere, i);
	intersectPlane(ray, plane, i);
}

// main ===========================================================================================
void main(void){
	// fragment position
	vec2 p = (gl_FragCoord.xy * 2.0 - resolution) / min(resolution.x, resolution.y);
	
	// ray init
	vec3 cSide = cross(cDir, cUp);
	vec3 ray = normalize(cSide * p.x + cUp * p.y + cDir * targetDepth);
	
	// sphere init
	sphere.position = vec3(0.0);
	sphere.radius = 1.0;
	
	// plane init
	plane.position = vec3(0.0, -1.0, 0.0);
	plane.normal = vec3(0.0, 1.0, 0.0);
	
	// intersect init
	Intersection i;
	i.t = 1.0e+30;
	i.hit = 0.0;
	i.hitPoint = vec3(0.0);
	i.normal = vec3(0.0);
	i.color = vec3(0.0);
	
	// check
	intersect(ray, i);
	if(i.hit > 0.0){
		gl_FragColor = vec4(i.color, 1.0);
	}else{
		gl_FragColor = vec4(vec3(0.0), 1.0);
	}
}