#define RAND_MAX (2147483647) // INT32_MAX

typedef struct RandomState {
    u32 value;
} RandomState;

inline u32 rand(RandomState* random){ // 32bit xorshift
    u32 x = random->value;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

    random->value = x;
	return random->value;
}

inline void recomputeRay(Ray* ray, RandomState* random, r32 distance, Material material, v3 normal){
    ray->color = hadamard(ray->color, hadamard(ray->attenuation, material.color));
    ray->attenuation = hadamard(ray->attenuation, ray->color);

    ray->origin = add(ray->origin, fmulLeft(ray->direction, distance));
    normal = normalize(normal);

    v3 randomVector = {(r32)rand(random) / (r32)RAND_MAX, (r32)rand(random) / (r32)RAND_MAX, (r32)rand(random) / (r32)RAND_MAX};
    ray->direction = sub(ray->direction, fmulRight(2 * dot(ray->direction, normal), normal));
    ray->direction = lerp(ray->direction, randomVector, material.shininess);
}

inline void intersects(Ray* ray, RandomState* random, r32* distanceFromOrigin, const Object* object){
    r32 min = 0.0000001;
    switch (object->type){
        case SPHERE:{
                // x*x + y*y + z*z = r*r
                v3 position = sub(ray->origin, object->origin);
                r32 a = dot(ray->direction, ray->direction);
                r32 b = 2 * dot(position, ray->direction);
                r32 c = dot(position, position) - object->radius * object->radius;

                r32 delta = b * b - 4 * a * c;

                if (delta > 0){
                    r32 t0 = (-b - sqrt(delta)) / (2 * a);
                    r32 t1 = (-b + sqrt(delta)) / (2 * a);

                    r32 t = t0 < t1 ? t0 : t1;

                    if (t > min && t < *distanceFromOrigin){
                        *distanceFromOrigin = t;
                        v3 hitPoint = add(ray->origin, fmulLeft(ray->direction, t));
                        v3 normal = sub(hitPoint, object->origin);
                        recomputeRay(ray, random, t, object->material, normal);
                    }
                }
            }
            break;
    
        default:{
            }
            break;
    }
}

inline void castRay(Ray* ray, RandomState* random, const Space* space){
    ray->color = makev3(0.5, 0.5, 1);

    r32 distanceFromOrigin = FLT_MAX;
    for (int bounce = 0; bounce < 4; ++bounce){
        for(int i = 0; i < space->numberOfObjects; ++i){
            intersects(ray, random, &distanceFromOrigin, &space->objects[i]);
        }
    }
}

typedef struct Region {
    u32 x;
    u32 y;
    u32 width;
    u32 height;
} Region;

typedef struct WorkerContext {
    Space space;
    u32 width;
    u32 height;
    v3 cameraPosition;
} WorkerContext __attribute__((packed));

static inline v3 nToRGB(v3 color){
    v3 result;
    result.x = color.x * 255.0;
    result.y = color.y * 255.0;
    result.z = color.z * 255.0;
    return result;
}

__kernel void regionCast(const WorkerContext context, __global u8* output, __global Region* regions, const u32 seed){
    RandomState random;
    random.value = seed;

    Region currentRegion = regions[get_global_id(0)];

    for(int y = currentRegion.y; y < currentRegion.y + currentRegion.height; ++y){
        r32 ny = 2.0 * ((r32)y / (r32)context.height) - 1.0;
        for(int x = currentRegion.x; x < currentRegion.x + currentRegion.width; ++x){
            r32 nx = 2.0 * ((r32)x / (r32)context.width) - 1.0;

            v3 screenPoint = makev3(nx, ny, 0); // TODO(Sarmis) matrix for lens distorsion in the future

            v3 finalRayColor = makev3(0, 0, 0);
            int rays = 4;
            for (int r = 0; r < rays; ++r){
                Ray ray = {};

                ray.attenuation = makev3(1, 1, 1);
                ray.origin = context.cameraPosition;
                ray.direction = normalize(sub(screenPoint, ray.origin));
                
                v3 randomVector = {(r32)rand(&random) / (r32)RAND_MAX, (r32)rand(&random) / (r32)RAND_MAX, (r32)rand(&random) / (r32)RAND_MAX};
                
                ray.direction = lerp(ray.direction, randomVector, 0.0001);
                
                castRay(&ray, &random, &(context.space));
                finalRayColor = add(finalRayColor, ray.color);
            }

            finalRayColor.x /= (r32)rays;
            finalRayColor.y /= (r32)rays;
            finalRayColor.z /= (r32)rays;

            finalRayColor = nToRGB(finalRayColor);
            output[x * 3 + 0 + (context.height - y) * context.width * 3] = finalRayColor.z;
            output[x * 3 + 1 + (context.height - y) * context.width * 3] = finalRayColor.y;
            output[x * 3 + 2 + (context.height - y) * context.width * 3] = finalRayColor.x;
        }
    }
}

