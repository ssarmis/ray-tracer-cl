#include <stdio.h>
#include <malloc.h>
#include <time.h>

#include "general.h"
#include "worker.h"
#include "space.h"
#include "math.h"
#include "bmp.h"

#include "CL/cl.h"
#include "cl_wrapper.h"

#define CHUNKS (1024)

inline void addSphereToSpace(Space& space, Material material, v3 position, r32 radius){
    Object sphere;

    sphere.type = SPHERE;
    sphere.material = material;
    sphere.origin = position;
    sphere.radius = radius;
    space.objects[space.numberOfObjects++] = sphere;
}

void readSourceFile(const char* filename, char*& data, u32& size){
    FILE* file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    data = new char[size + 1];

    fread(data, sizeof(char), size, file);
    data[size] = 0;

    fclose(file);
}

Region* generateRegions(Bitmap bitmap, int chunks){
    Region* regions = new Region[chunks * chunks];
    int i = 0;
    u32 w = bitmap.width / chunks;
    u32 h = bitmap.height / chunks;

    for(u32 y = 0; y < bitmap.height / h; ++y){
        for(u32 x = 0; x < bitmap.width / w; ++x){
            regions[i++] = {x * w, y * h, w, h};
        }
    }
    return regions;
}

int main(int argumentCount, char* arguments[]){
    Bitmap bitmap;

    bitmap.width = 3840;
    bitmap.height = 2160;
    bitmap.data = new u8[bitmap.width * bitmap.height * 3];

    Space world = {};

    v3 cameraPosition = {0, 0.1, -5};
    v3 cameraDirection = {0, 0, 1};

    addSphereToSpace(world, {0, {0.9, 0.9, 0.9}}, {0, -5.5, 0}, 5);
    
    addSphereToSpace(world, {0.4, {0.5, 1, 1}}, {0, 0.2, 0}, 0.4);
    addSphereToSpace(world, {0.8, {1, 1, 0.5}}, {0.6, 0.6, -0.4}, 0.2);
    addSphereToSpace(world, {0.5, {0.5, 0.5, 0.5}}, {0.4, -0.1, -0.8}, 0.1);
    
    printf("Starting\n");

    WorkerContext workerContext;

    workerContext.width = bitmap.width;
    workerContext.height = bitmap.height;
    workerContext.space = world;
    workerContext.cameraPosition = cameraPosition;

    char* sources[6];
    u32 sizes[6];

    readSourceFile("general.h", sources[0], sizes[0]);
    readSourceFile("math.cl", sources[1], sizes[1]);
    readSourceFile("space.h", sources[2], sizes[2]);
    readSourceFile("ray_cast.cl", sources[3], sizes[3]);

    CL cl = initializeOpenCLForUsage();
    loadOpenCLSourceFiles(&cl, (const char**)sources, sizes, 4);
    buildOpenCLProgram(&cl);

    createOpenCLKernel(&cl, "regionCast");
    Region* regionsOrigin = generateRegions(bitmap, CHUNKS);

    cl_mem regions = clCreateBuffer(cl.context, CL_MEM_READ_ONLY, sizeof(Region) * CHUNKS * CHUNKS, NULL, NULL);
    clEnqueueWriteBuffer(cl.commands, regions, CL_FALSE, 0, sizeof(Region) * CHUNKS * CHUNKS, regionsOrigin, 0, NULL, NULL);

    cl_mem output = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, sizeof(u8) * bitmap.width * bitmap.height * 3, NULL, NULL);

    srand(time(NULL));
    u32 random = rand();

    clSetKernelArg(cl.kernel, 0, sizeof(WorkerContext), &workerContext);
    clSetKernelArg(cl.kernel, 1, sizeof(cl_mem), &output);
    clSetKernelArg(cl.kernel, 2, sizeof(cl_mem), &regions);
    clSetKernelArg(cl.kernel, 3, sizeof(u32), &random);

    enqueuExecutionOfKernel(&cl, CHUNKS * CHUNKS);
    
    int error = clEnqueueReadBuffer(cl.commands, output, CL_TRUE, 0, sizeof(u8) * bitmap.width * bitmap.height * 3, bitmap.data, 0, NULL, NULL);
    if (error != CL_SUCCESS) {
        printf("Failed to read output %d\n", error);
    }

    writeBmpFile(bitmap.data, bitmap.width, bitmap.height);

    clFinish(cl.commands);
    clReleaseKernel(cl.kernel);
    clReleaseProgram(cl.program);
    clReleaseCommandQueue(cl.commands);
    clReleaseContext(cl.context);

    delete[] sources[0];
    delete[] sources[1];
    delete[] sources[2];
    delete[] regionsOrigin;
    delete[] bitmap.data;

    return 0;
}

















