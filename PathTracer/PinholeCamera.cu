#include "optix.h"
#include "optix_device.h"
#include <optixu/optixu_math_namespace.h>
#include "Payloads.h"
#include "Constants.h"

using namespace optix;

rtBuffer<float3, 2> resultBuffer; // used to store the render result

rtDeclareVariable(rtObject, root, , ); // Optix graph

rtDeclareVariable(uint2, launchIndex, rtLaunchIndex, ); // a 2d index (x, y)

// Camera info 

// TODO:: delcare camera varaibles here

rtDeclareVariable(int1, depth, , ); // recursion depth

RT_PROGRAM void generateRays() {


	// TODO: calculate the ray direction
	float3 eye = make_float3(0, 0, 0); // change this
	float3 dir = make_float3(0, 0, 1); // change this


	// Shoot a ray to compute the color of the current pixel
	Ray ray = make_Ray(eye, dir, BASIC_RAY, RAY_EPSILON, RT_DEFAULT_MAX);
	Payload payload;
	payload.depth = depth.x;
	rtTrace(root, ray, payload);

	// Write the result
	resultBuffer[launchIndex] = payload.result;
}

rtDeclareVariable(Payload, payload, rtPayload, );
rtDeclareVariable(float3, backgroundColor, , );

RT_PROGRAM void miss() {
	// Set the result to be the background color if miss


	// TODO: change the color to be the background color


	payload.result = make_float3(1, 0, 0);
}

RT_PROGRAM void exception() {
	// Print any exception for debugging
	const unsigned int code = rtGetExceptionCode();
	rtPrintExceptionDetails();
}