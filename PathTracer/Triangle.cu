#include "optix.h"
#include "optix_device.h"
#include "Geometries.h"
#include "Constants.h"

rtBuffer<Triangle> triangles; // a buffer of all triangles

rtDeclareVariable(Ray, ray, rtCurrentRay, );

// Attributes to be passed to material programs 

// TODO: declare attribute variables here 

RT_PROGRAM void intersect(int primIndex) {
	// Find the intersection of the current ray and triangle
	Triangle tri = triangles[primIndex];
	float t;


	// TODO: implement triangle intersection test here

	
	// Report intersection (material programs will handle the rest)
	if (rtPotentialIntersection(t)) {
		// Pass attributes


		// TODO: assign attribute variables here


		rtReportIntersection(0);
	}
}

RT_PROGRAM void bound(int primIndex, float result[6]) {
	
}