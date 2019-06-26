#include "optix.h"
#include "optix_device.h"
#include <optixu/optixu_math_namespace.h>
#include "Payloads.h"
#include "Geometries.h"
#include "Lights.h"
#include "Constants.h"

using namespace optix;

// Declare light buffers
rtBuffer<PointLight> plights;
rtBuffer<DirectionalLight> dlights;

// Declare variables
rtDeclareVariable(Payload, payload, rtPayload, );
rtDeclareVariable(rtObject, root, , );

// Declare attibutes 

// TODO: declare attribute variables here 

RT_PROGRAM void closestHit() {
	float3 result = make_float3(1, 0, 0);


	// TODO: calculate the color based	on lighting


	payload.result = result;
}

rtDeclareVariable(ShadowPayload, shadowPayload, rtPayload, );

RT_PROGRAM void anyHit() {
	

	// TODO: handle the situation that the shadow ray hit any objects


	rtTerminateRay();
}