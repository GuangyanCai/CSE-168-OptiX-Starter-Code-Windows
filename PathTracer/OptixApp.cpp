#include "OptixApp.h"

OptixApp::OptixApp(string sceneFilename) {
	// Create the Optix context
	context = Context::create();

	// Configure the context 
	context->setRayTypeCount(2); // two types of rays: basic ray and shadow ray
	context->setEntryPointCount(1); // only one entry point
	context->setPrintEnabled(true); // enable the use of rtPrintf in programs
	context->setPrintBufferSize(2048);

	// Create the resultBuffer
	resultBuffer = context->createBuffer(RT_BUFFER_OUTPUT); // only device can write
	resultBuffer->setFormat(RT_FORMAT_FLOAT3); // each entry is of type float3

	// Initialize Optix programs
	initPrograms();

	// Build the scene by constructing an Optix graph
	buildScene(sceneFilename);
}

void OptixApp::run() {
	// Validate everything before running 
	context->validate();

	// Start timing the rendering process
	auto start = std::chrono::system_clock::now();

	// Start rendering 
	context->launch(0, width, height);

	// Stop timing
	auto end = std::chrono::system_clock::now();
	auto time_span = std::chrono::duration_cast<std::chrono::milliseconds> (end - start);
	std::cout << "RayTracing takes " << time_span.count() / 1000.f << " seconds." << std::endl;
}

void OptixApp::initPrograms() {
	// Ray generation program
	programs["rayGen"] = context->createProgramFromPTXFile("PTX/PinholeCamera.ptx", "generateRays");
	context->setRayGenerationProgram(0, programs["rayGen"]);

	// Miss progarm
	programs["miss"] = context->createProgramFromPTXFile("PTX/PinholeCamera.ptx", "miss");
	programs["miss"]["backgroundColor"]->setFloat(0.f, 0.f, 0.f);
	context->setMissProgram(BASIC_RAY, programs["miss"]);

	// Exception program
	programs["exc"] = context->createProgramFromPTXFile("PTX/PinholeCamera.ptx", "exception");
	context->setExceptionEnabled(RT_EXCEPTION_ALL, true);
	context->setExceptionProgram(0, programs["exc"]);

	// Triangle programs
	programs["triInt"] = context->createProgramFromPTXFile("PTX/Triangle.ptx", "intersect");
	programs["triBound"] = context->createProgramFromPTXFile("PTX/Triangle.ptx", "bound");

	// Sphere programs 
	programs["sphereInt"] = context->createProgramFromPTXFile("PTX/Sphere.ptx", "intersect");
	programs["sphereBound"] = context->createProgramFromPTXFile("PTX/Sphere.ptx", "bound");

	// Material programs
	programs["materialCH"] = context->createProgramFromPTXFile("PTX/BlinnPhong.ptx", "closestHit");
	programs["materialAH"] = context->createProgramFromPTXFile("PTX/BlinnPhong.ptx", "anyHit");
	material = context->createMaterial();
	material->setClosestHitProgram(BASIC_RAY, programs["materialCH"]);
	material->setAnyHitProgram(SHADOW_RAY, programs["materialAH"]);
}

vector<unsigned char> OptixApp::getResult() {
	// Cast a float number (0 to 1) to a byte (0 to 255)
	auto cast = [](float v) {
		v = v > 1.f ? 1.f : v < 0.f ? 0.f : v;
		return static_cast<unsigned char>(v * 255);
	};

	float3 * bufferData = (float3 *)resultBuffer->map();

	// Store the data into a byte vector
	vector<unsigned char> imageData;
	for (int i = 0; i < width * height; i++) {
		imageData.push_back(cast(bufferData[i].x));
		imageData.push_back(cast(bufferData[i].y));
		imageData.push_back(cast(bufferData[i].z));
		imageData.push_back(255); // alpha channel
	}

	resultBuffer->unmap();

	return imageData;
}

template <class T>
Buffer OptixApp::createBuffer(vector<T> data) {
	Buffer buffer = context->createBuffer(RT_BUFFER_INPUT); // only host can write
	buffer->setFormat(RT_FORMAT_USER); // use user-defined format
	buffer->setElementSize(sizeof(T)); // size of an element
	buffer->setSize(data.size()); // number of elements
	std::memcpy(buffer->map(), data.data(), sizeof(T) * data.size());
	buffer->unmap();
	return buffer;
}

void OptixApp::buildScene(string sceneFilename) {
	// Attempt to open the scene file 
	ifstream in(sceneFilename);
	if (!in) {
		// Unable to open the file. Check if the filename is correct.
		throw runtime_error("Unable to Open Input Data File " + sceneFilename);
	}

	// Variables to hold the data inside the scene file. Set default values here.
	string str, cmd;
	vector<float3> vertices;
	stack<Matrix4x4> transfstack;
	transfstack.push(Matrix4x4::identity());
	vector<Triangle> triangles;
	vector<Sphere> spheres;
	vector<DirectionalLight> dlights;
	vector<PointLight> plights;
	float3 ambient = make_float3(0); 
	float3 diffuse = make_float3(0);
	float3 specular = make_float3(0);
	float3 emission = make_float3(0);
	float3 attenuation = make_float3(1, 0, 0);
	float shininess = 1;
	unsigned int depth = 5;

	// Helper functions
	auto rightMultiply = [&transfstack](const Matrix4x4 &M) {
		Matrix4x4 &T = transfstack.top();
		T = T * M;
	};
	auto readvals = [](stringstream &s, const int numvals, float *values) -> bool {
		for (int i = 0; i < numvals; i++) {
			s >> values[i];
			if (s.fail()) {
				std::cout << "Failed reading value " << i << " will skip" << std::endl;
				return false;
			}
		}
		return true;
	};

	while (std::getline(in, str)) {
		// Ruled out comment and blank lines
		if ((str.find_first_not_of(" \t\r\n") == string::npos) || (str[0] == '#')) {
			continue;
		}

		stringstream s(str);
		s >> cmd;
		float values[12];

		if (cmd == "size" && readvals(s, 2, values)) {
			width = (unsigned int)values[0];
			height = (unsigned int)values[1];
			resultBuffer->setSize(width, height);
			programs["rayGen"]["resultBuffer"]->set(resultBuffer);
		}
		else if (cmd == "maxdepth" && readvals(s, 1, values)) {
			depth = (unsigned int)values[0];
		}
		else if (cmd == "output") {
			string fn;
			s >> fn;
			if (s.fail()) {
				std::cout << "Failed reading value " << 0 << " will skip\n";
			}
			else {
				outputFilename = fn;
			}
		}

		// TODO: read other commands and process them accordingly
	}

	// Set maximum recursion depth.  
	context->setMaxTraceDepth(depth + 2); 
	programs["rayGen"]["depth"]->setInt(depth);

	// Create buffers and pass them to Optix programs that the buffers
	Buffer triBuffer = createBuffer(triangles);
	programs["triInt"]["triangles"]->set(triBuffer);
	programs["triBound"]["triangles"]->set(triBuffer);

	Buffer sphereBuffer = createBuffer(spheres);
	programs["sphereInt"]["spheres"]->set(sphereBuffer);
	programs["sphereBound"]["spheres"]->set(sphereBuffer);

	Buffer plightBuffer = createBuffer(plights);
	programs["materialCH"]["plights"]->set(plightBuffer);

	Buffer dlightBuffer = createBuffer(dlights);
	programs["materialCH"]["dlights"]->set(dlightBuffer);

	// Construct the Optix graph. It should look like:
	//      root
	//       ||
	//       GG
	//    //    \\
	//   triGI   sphereGI
	//  //  \\    //   \\
	// triGeo material sphereGeo
	Geometry triGeo = context->createGeometry();
	triGeo->setPrimitiveCount(triangles.size());
	triGeo->setIntersectionProgram(programs["triInt"]);
	triGeo->setBoundingBoxProgram(programs["triBound"]);

	Geometry sphereGeo = context->createGeometry();
	sphereGeo->setPrimitiveCount(spheres.size());
	sphereGeo->setIntersectionProgram(programs["sphereInt"]);
	sphereGeo->setBoundingBoxProgram(programs["sphereBound"]);

	GeometryInstance triGI = context->createGeometryInstance();
	triGI->setGeometry(triGeo);
	triGI->setMaterialCount(1);
	triGI->setMaterial(0, material);

	GeometryInstance sphereGI = context->createGeometryInstance();
	sphereGI->setGeometry(sphereGeo);
	sphereGI->setMaterialCount(1);
	sphereGI->setMaterial(0, material);
	
	GeometryGroup GG = context->createGeometryGroup();
	GG->setAcceleration(context->createAcceleration("NoAccel"));
	GG->setChildCount(2);
	GG->setChild(0, triGI);
	GG->setChild(1, sphereGI);

	Group root = context->createGroup();
	root->setAcceleration(context->createAcceleration("NoAccel"));
	root->setChildCount(1);
	root->setChild(0, GG);
	programs["rayGen"]["root"]->set(root);
	programs["materialCH"]["root"]->set(root);
}