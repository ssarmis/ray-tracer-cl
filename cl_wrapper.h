#ifndef _CL_WRAPPER_H_
#define _CL_WRAPPER_H_

#include <stdio.h>

#include "general.h"
#include "cl.h"

struct CL {
    cl_context context;
    cl_device_id deviceId;
    cl_command_queue commands;
    cl_program program;
    cl_kernel kernel;
};

static inline CL initializeOpenCLForUsage(){
    CL result = {};
    int error;

    cl_uint numPlatforms;
    cl_platform_id platforms[2];

    clGetPlatformIDs(2, platforms, &numPlatforms);
    printf("%d %d\n", numPlatforms, platforms[0]);


    error = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 1, &result.deviceId, NULL);
    if (error != CL_SUCCESS) {
        printf("Failed to get a device id %d\n", error);
    }

    result.context = clCreateContext(0, 1, &result.deviceId, NULL, NULL, &error);
    if (!result.context) {
        printf("Failed to create a context %d\n", error);
    }

    result.commands = clCreateCommandQueue(result.context, result.deviceId, 0, &error);
    if (!result.commands) {
        printf("Failed to create commands %d\n", error);
    }
    return result;
}

static inline void loadOpenCLSourceFiles(CL* cl, const char** sources, u32* sizes, u8 sourceFilesAmount){
    int error;
    cl->program = clCreateProgramWithSource(cl->context, sourceFilesAmount, sources, NULL, &error);
    if (!cl->program) {
        printf("Error: Failed to create compute program %d\n", error);
    }
}

static inline void enqueuExecutionOfKernel(CL* cl, u32 workGroups){
    u32 localSize;
    u32 globalSize;

    int error;

    error = clGetKernelWorkGroupInfo(cl->kernel, cl->deviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(localSize), &localSize, NULL);
    if (error != CL_SUCCESS) {
        printf("Failed to retrieve kernel work group info %d\n", error);
    }

    printf("Possible local size %d\n", localSize);

    localSize = 1;
    globalSize = localSize * workGroups;

    error = clEnqueueNDRangeKernel(cl->commands, cl->kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);
    if (error) {
        printf("Failed to execute kernel %d\n", error);
    }
}

static inline void buildOpenCLProgram(CL* cl){
    int error;
    error = clBuildProgram(cl->program, 0, NULL, NULL, NULL, NULL);
    if (error != CL_SUCCESS) {
        u32 length;
        char buffer[4096];

        clGetProgramBuildInfo(cl->program, cl->deviceId, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &length);
        printf("%s\n", buffer);
    }
}


static inline void createOpenCLKernel(CL* cl, const char* kernelName){
    int error;
    cl->kernel = clCreateKernel(cl->program, kernelName, &error);
    if (!cl->kernel || error != CL_SUCCESS) {
        printf("Failed to create kernel\n");
    }
}

#endif //_CL_WRAPPER_H_