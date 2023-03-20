#include "GameTechRenderer.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"
#include "TextureLoader.h"

//this was me
#include <Win32Window.h>


using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

#define SHADOWSIZE 8192

Matrix4 biasMatrix = Matrix4::Translation(Vector3(0.5f, 0.5f, 0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.5f, 0.5f));

GameTechRenderer::GameTechRenderer(GameWorld& world) : OGLRenderer(*Window::GetWindow()), gameWorld(world)	{
	glEnable(GL_DEPTH_TEST);

	debugShader  = new OGLShader("debug.vert", "debug.frag");
	shadowShader = new OGLShader("shadow.vert", "shadow.frag");

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			     SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(1, 1, 1, 1);

	//Set up the light properties
	lightColour = Vector4(0.8f, 0.8f, 0.5f, 1.0f);
	lightRadius = 1000.0f;
	lightPosition = Vector3(0.0f, 53, 160);

	//Skybox!
	skyboxShader = new OGLShader("skybox.vert", "skybox.frag");
	skyboxMesh = new OGLMesh();
	skyboxMesh->SetVertexPositions({Vector3(-1, 1,-1), Vector3(-1,-1,-1) , Vector3(1,-1,-1) , Vector3(1,1,-1) });
	skyboxMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	skyboxMesh->UploadToGPU();

	LoadSkybox();

	glGenVertexArrays(1, &lineVAO);
	glGenVertexArrays(1, &textVAO);

	glGenBuffers(1, &lineVertVBO);
	glGenBuffers(1, &textVertVBO);
	glGenBuffers(1, &textColourVBO);
	glGenBuffers(1, &textTexVBO);

	SetDebugStringBufferSizes(10000);
	SetDebugLineBufferSizes(1000);


	//this was me

	CreateFBOColorDepth(sceneFBO, sceneColor, sceneDepth, sceneHdrTex, GL_RGBA32F,true);


	edgesShader = new OGLShader("smaaEdgeDetection.vert", "smaaEdgeDetectionColor.frag");
	CreateFBOColor(edgesFBO, edgesTex);

	weightCalcShader = new OGLShader("smaaBlendingWeightCalculation.vert", "smaaBlendingWeightCalculation.frag");
	CreateFBOColor(weightCalcFBO, blendTex);

	neighborhoodBlendingShader = new OGLShader("smaaNeighborhoodBlending.vert", "smaaNeighborhoodBlending.frag");
	CreateFBOColor(neighborhoodBlendingFBO, smaaOutput);
#pragma region oldBloom
	//https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
	//downsampleShader = new OGLShader("bloom/downsample.vert", "bloom/downsample.frag");
	//downsampleChain[0].width = windowWidth;
	//downsampleChain[0].height = windowHeight;
	//CreateFBOColor(downsampleFBO, downsampleChain[0].texture);
	//
	//
	//int mipWidth = windowWidth;
	//int mipHeight = windowHeight;
	//bool skipFirst = true;
	//for (BloomMip& mip : downsampleChain) {//todo will need to redo on screen resize
	//	mip.width = mipWidth;
	//	mip.height = mipHeight;
	//	if (skipFirst) {
	//		mip.texture = sceneColor;
	//		skipFirst = false;
	//	}
	//	else {
	//		glGenTextures(1, &(mip.texture));
	//		glBindTexture(GL_TEXTURE_2D, mip.texture);
	//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, mip.width, mip.height, 0, GL_RGB, GL_FLOAT, NULL);
	//	}
	//	
	//	mipWidth /= 2;
	//	mipHeight /= 2;
	//	if (mipWidth == 0 || mipHeight == 0) {
	//		break;
	//	}
	//}
	//mipWidth *= 2;
	//mipHeight *= 2;
	//
	//for (int i = 0; i < upsampleChain.size();i++) {//todo will need to redo on screen resize
	//	BloomMip& mip = upsampleChain[i];
	//	mip.width = downsampleChain[upsampleChain.size() - (i+1)].width;
	//	mip.height = downsampleChain[upsampleChain.size() - (i+1)].height;
	//	glGenTextures(1, &(mip.texture));
	//	glBindTexture(GL_TEXTURE_2D, mip.texture);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, mip.width, mip.height, 0, GL_RGB, GL_FLOAT, NULL);
	//	mipWidth *= 2;
	//	mipHeight *= 2;
	//	if (mipWidth > windowWidth || windowHeight == 0) {
	//		break;
	//	}
	//}
#pragma endregion

	downsampleComputeShader = new OGLComputeShader("bloom/downsample.comp");
	upsampleComputeShader = new OGLComputeShader("bloom/upsample.comp");

	bloomShader = new OGLShader("bloom/bloom.vert", "bloom/bloom.frag");
	 
	//hdrShader = new OGLShader();
	//CreateFBOColor(hdrFBO, tonemappedTexture, GL_RGBA16F);

	unsigned char* flippedAreaTex = new unsigned char[sizeof(areaTexBytes)];

	for (int y = 0; y < AREATEX_HEIGHT; y++)
	{
		int flippedY = AREATEX_HEIGHT - y - 1;

		for (int x = 0; x < AREATEX_WIDTH; x++)
		{
			((uint16_t*)flippedAreaTex)[AREATEX_WIDTH * flippedY + x] = ((uint16_t*)areaTexBytes)[AREATEX_WIDTH * y + x];
		}
	}

	unsigned char* flippedSearchTex = new unsigned char[sizeof(searchTexBytes)];

	for (int y = 0; y < SEARCHTEX_HEIGHT; y++)
	{
		int flippedY = SEARCHTEX_HEIGHT - y - 1;

		for (int x = 0; x < SEARCHTEX_WIDTH; x++)
		{
			(flippedSearchTex)[SEARCHTEX_WIDTH * flippedY + x] = (searchTexBytes)[SEARCHTEX_WIDTH * y + x];
		}
	}

	glGenTextures(1, &areaTex);
	glBindTexture(GL_TEXTURE_2D, areaTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, AREATEX_WIDTH, AREATEX_HEIGHT, 0, GL_RG, GL_UNSIGNED_BYTE, flippedAreaTex);

	delete[] flippedAreaTex;

	glGenTextures(1, &searchTex);
	glBindTexture(GL_TEXTURE_2D, searchTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, flippedSearchTex);

	delete[] flippedSearchTex;

	//https://stackoverflow.com/questions/12105330/how-does-this-simple-fxaa-work
	fxaaShader = new OGLShader("fxaa.vert", "fxaa.frag");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();


	Win32Code::Win32Window* realWindow = (Win32Code::Win32Window*)&hostWindow;

	

	ImGui_ImplWin32_Init(realWindow->GetHandle());
	
	ImGui_ImplOpenGL3_Init("#version 330");
}

GameTechRenderer::~GameTechRenderer()	{
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
}

void GameTechRenderer::LoadSkybox() {
	string filenames[6] = {
		"/Cubemap/skyrender0004.png",
		"/Cubemap/skyrender0001.png",
		"/Cubemap/skyrender0003.png",
		"/Cubemap/skyrender0006.png",
		"/Cubemap/skyrender0002.png",
		"/Cubemap/skyrender0005.png"
	};

	int width[6]	= { 0 };
	int height[6]	= { 0 };
	int channels[6] = { 0 };
	int flags[6]	= { 0 };

	vector<char*> texData(6, nullptr);

	for (int i = 0; i < 6; ++i) {
		TextureLoader::LoadTexture(filenames[i], texData[i], width[i], height[i], channels[i], flags[i]);
		if (i > 0 && (width[i] != width[0] || height[0] != height[0])) {
			std::cout << __FUNCTION__ << " cubemap input textures don't match in size?\n";
			return;
		}
	}
	glGenTextures(1, &skyboxTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	GLenum type = channels[0] == 4 ? GL_RGBA : GL_RGB;

	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, type, GL_UNSIGNED_BYTE, texData[i]);
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GameTechRenderer::RenderFrame() {
	glEnable(GL_CULL_FACE);
	glClearColor(0,0,0,1);
	BuildObjectList();
	SortObjectList();
	glDisable(GL_CULL_FACE);
	
	RenderShadowMap();
	
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	RenderSkybox();
	RenderCamera();
	
	if(renderFullScreenQuad)RenderFullScreenQuadWithTexture(rayMarchTexture->GetObjectID());//raymarching
	

	if (bloom) {
		Blur();
		RenderBloom();
	}
	
	if(useFXAA)FXAA();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	RenderFullScreenQuadWithTexture(sceneColor);//todo fix rotation
	

	
	glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	NewRenderLines();
	NewRenderText();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	

	
	

	ImGui();
	if(drawCrosshair)DrawCrossHair();
}

void GameTechRenderer::BuildObjectList() {
	activeObjects.clear();

	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->IsActive()) {
				const RenderObject* g = o->GetRenderObject();
				if (g) {
					activeObjects.emplace_back(g);
				}
			}
		}
	);
}

void GameTechRenderer::SortObjectList() {

}

void GameTechRenderer::RenderShadowMap() {
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 6, "shadow");
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glCullFace(GL_FRONT);

	BindShader(shadowShader);
	int mvpLocation = glGetUniformLocation(shadowShader->GetProgramID(), "mvpMatrix");

	Matrix4 shadowViewMatrix = Matrix4::BuildViewMatrix(lightPosition, Vector3(0, 0, 0), Vector3(0,1,0));
	Matrix4 shadowProjMatrix = Matrix4::Perspective(100.0f, 500.0f, 1, 45.0f);

	Matrix4 mvMatrix = shadowProjMatrix * shadowViewMatrix;

	shadowMatrix = biasMatrix * mvMatrix; //we'll use this one later on

	for (const auto&i : activeObjects) {

		//moving vertices slightly to combat shadow acne
		GeometryPrimitive prevPrimitive = i->GetMesh()->GetPrimitiveType();
		i->GetMesh()->SetPrimitiveType(GeometryPrimitive::Triangles);
		Transform* tempTransform = (*i).GetTransform();
		Transform newTransform;
		memcpy(&newTransform, tempTransform,sizeof(Transform));
		Vector3 lightDir = (lightPosition - newTransform.GetPosition()).Normalised();
		newTransform.SetPosition(newTransform.GetPosition() - lightDir);
		Matrix4 modelMatrix = newTransform.GetMatrix();


		Matrix4 mvpMatrix	= mvMatrix * modelMatrix;
		glUniformMatrix4fv(mvpLocation, 1, false, (float*)&mvpMatrix);
		BindMesh((*i).GetMesh());
		int layerCount = (*i).GetMesh()->GetSubMeshCount();
		for (int i = 0; i < layerCount; ++i) {
			DrawBoundMesh(i);
		}
		i->GetMesh()->SetPrimitiveType(prevPrimitive);//dont forget this
	}

	glViewport(0, 0, windowWidth, windowHeight);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
	glPopDebugGroup();
}

void GameTechRenderer::RenderSkybox() {
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	float screenAspect = (float)windowWidth / (float)windowHeight;
	Matrix4 viewMatrix = gameWorld.GetMainCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera()->BuildProjectionMatrix(screenAspect);

	BindShader(skyboxShader);

	int projLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "viewMatrix");
	int texLocation  = glGetUniformLocation(skyboxShader->GetProgramID(), "cubeTex");

	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

	glUniform1i(texLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	BindMesh(skyboxMesh);
	DrawBoundMesh();

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void GameTechRenderer::RenderCamera() {
	float screenAspect = (float)windowWidth / (float)windowHeight;
	Matrix4 viewMatrix = gameWorld.GetMainCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera()->BuildProjectionMatrix(screenAspect);

	OGLShader* activeShader = nullptr;
	int projLocation	= 0;
	int viewLocation	= 0;
	int modelLocation	= 0;
	int colourLocation  = 0;
	int hasVColLocation = 0;
	int hasTexLocation  = 0;
	int shadowLocation  = 0;

	//this was me
	int widthLocation = 0;
	int heightLocation = 0;
	int scaleLocation = 0;

	int useHeightMapLocalLocation = 0;

	int lightPosLocation	= 0;
	int lightColourLocation = 0;
	int lightRadiusLocation = 0;

	int noiseScaleLocation = 0;
	int noiseOffsetSizeLocation = 0;
	int noiseNormalStrengthLocation = 0;
	int noisenormalNoiseMultLocation = 0;

	int cameraLocation = 0;

	int heightMapStrengthLocation = 0;
	int useBumpMapLocation = 0;
	int useMetallicMapLocation = 0;
	int useRoughnessMapLocation = 0;
	int useHeightMapLocation = 0;
	int useEmissionMapLocation = 0;
	int useAOMapLocation = 0;
	int useOpacityMapLocation = 0;
	int useGlossMapLocation = 0;

	int timePassedLocation = 0;
	int timeScaleLocation = 0;

	//TODO - PUT IN FUNCTION
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 12, "renderCamera");

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	glDisable(GL_BLEND);

	for (const auto&i : activeObjects) {
		if (i->onlyForShadows)continue;
		OGLShader* shader = (OGLShader*)(*i).GetShader();
		BindShader(shader);

		

		if (activeShader != shader) {
			projLocation	= glGetUniformLocation(shader->GetProgramID(), "projMatrix");
			viewLocation	= glGetUniformLocation(shader->GetProgramID(), "viewMatrix");
			modelLocation	= glGetUniformLocation(shader->GetProgramID(), "modelMatrix");
			shadowLocation  = glGetUniformLocation(shader->GetProgramID(), "shadowMatrix");
			colourLocation  = glGetUniformLocation(shader->GetProgramID(), "objectColour");
			hasVColLocation = glGetUniformLocation(shader->GetProgramID(), "hasVertexColours");
			hasTexLocation  = glGetUniformLocation(shader->GetProgramID(), "hasTexture");
			scaleLocation  = glGetUniformLocation(shader->GetProgramID(), "scale");
			useHeightMapLocalLocation  = glGetUniformLocation(shader->GetProgramID(), "useHeightMapLocal");
			

			//this was me
			widthLocation = glGetUniformLocation(shader->GetProgramID(), "width");
			heightLocation = glGetUniformLocation(shader->GetProgramID(), "height");

			noiseScaleLocation = glGetUniformLocation(shader->GetProgramID(), "noiseScale");
			noiseOffsetSizeLocation = glGetUniformLocation(shader->GetProgramID(), "noiseOffsetSize");
			noiseNormalStrengthLocation = glGetUniformLocation(shader->GetProgramID(), "noiseNormalStrength");
			noisenormalNoiseMultLocation = glGetUniformLocation(shader->GetProgramID(), "noiseNormalNoiseMult");

			timePassedLocation = glGetUniformLocation(shader->GetProgramID(), "timePassed");
			timeScaleLocation = glGetUniformLocation(shader->GetProgramID(), "timeScale");

			lightPosLocation	= glGetUniformLocation(shader->GetProgramID(), "lightPos");
			lightColourLocation = glGetUniformLocation(shader->GetProgramID(), "lightColour");
			lightRadiusLocation = glGetUniformLocation(shader->GetProgramID(), "lightRadius");

			cameraLocation = glGetUniformLocation(shader->GetProgramID(), "cameraPos");

			heightMapStrengthLocation = glGetUniformLocation(shader->GetProgramID(), "heightMapStrength");
			useBumpMapLocation = glGetUniformLocation(shader->GetProgramID(), "useBumpMap");
			useMetallicMapLocation = glGetUniformLocation(shader->GetProgramID(), "useMetallicMap");
			useRoughnessMapLocation = glGetUniformLocation(shader->GetProgramID(), "useRoughnessMap");
			useHeightMapLocation = glGetUniformLocation(shader->GetProgramID(), "useHeightMap");
			useEmissionMapLocation = glGetUniformLocation(shader->GetProgramID(), "useEmissionMap");
			useAOMapLocation = glGetUniformLocation(shader->GetProgramID(), "useAOMap");
			useOpacityMapLocation = glGetUniformLocation(shader->GetProgramID(), "useOpacityMap");
			useGlossMapLocation = glGetUniformLocation(shader->GetProgramID(), "useGlossMap");

			Vector3 camPos = gameWorld.GetMainCamera()->GetPosition();
			glUniform3fv(cameraLocation, 1, camPos.array);

			glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
			glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

			glUniform3fv(lightPosLocation	, 1, (float*)&lightPosition);
			glUniform4fv(lightColourLocation, 1, (float*)&lightColour);
			glUniform1f(lightRadiusLocation , lightRadius);

			int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");
			glUniform1i(shadowTexLocation, 2);

			

			glUniform3fv(scaleLocation, 1, i->GetTransform()->GetScale().array);

			activeShader = shader;
		}

		Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();
		glUniformMatrix4fv(modelLocation, 1, false, (float*)&modelMatrix);			
		
		Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
		glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);

		Vector4 colour = i->GetColour();
		glUniform4fv(colourLocation, 1, colour.array);

		glUniform1i(hasVColLocation, !(*i).GetMesh()->GetColourData().empty());

		glUniform1i(hasTexLocation, (OGLTexture*)(*i).GetDefaultTexture() ? 1:0);

		//this was me
		Vector2 maskDims = (*i).maskDimensions;
		glUniform1i(widthLocation, maskDims.x);
		glUniform1i(heightLocation, maskDims.y);

		glUniform1f(noiseScaleLocation, noiseScale);
		glUniform1f(noiseOffsetSizeLocation, noiseOffsetSize / 1000.0f * i->GetTransform()->GetScale().Length());
		glUniform1f(noiseNormalStrengthLocation, noiseNormalStrength);
		glUniform1f(noisenormalNoiseMultLocation, noiseNormalNoiseMult);

		glUniform1f(timePassedLocation, timePassed);
		glUniform1f(timeScaleLocation, timeScale);

		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "toneMap"), toneMap);
		glUniform1f(glGetUniformLocation(shader->GetProgramID(), "exposure"), exposure);
		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "emissionStrength"), emissionStrength);

		//glActiveTexture(GL_TEXTURE0);
		//BindTextureToShader((OGLTexture*)(*i).GetDefaultTexture(), "mainTex", 0);
		if (i->isPaintable) {

			glUniform1f(heightMapStrengthLocation, heightMapStrength);
			glUniform1i(useBumpMapLocation, useBumpMap);
			glUniform1i(useMetallicMapLocation, useMetallicMap);
			glUniform1i(useRoughnessMapLocation, useRoughnessMap);
			glUniform1i(useHeightMapLocation, useHeightMap);
			glUniform1i(useEmissionMapLocation, useEmissionMap);
			glUniform1i(useAOMapLocation, useAOMap);
			glUniform1i(useOpacityMapLocation, useOpacityMap);
			glUniform1i(useGlossMapLocation, useGlossMap);

			glUniform1i(useHeightMapLocalLocation, i->useHeightMap);
			glActiveTexture(GL_TEXTURE0);
			glBindImageTexture(0, ((OGLTexture*)i->maskTex)->GetObjectID(), 0, GL_FALSE, NULL, GL_READ_ONLY, GL_R8UI);
			glUniform1i(glGetUniformLocation(shader->GetProgramID(), "maskTex"),0);
			if (i->pbrTextures != nullptr) {
				BindTextureToShader((OGLTexture*)(*i).pbrTextures->base, "baseTex", 1);
				BindTextureToShader((OGLTexture*)(*i).pbrTextures->bump, "bumpTex", 3);
				BindTextureToShader((OGLTexture*)(*i).pbrTextures->metallic, "metallicTex", 4);
				BindTextureToShader((OGLTexture*)(*i).pbrTextures->roughness, "roughnessTex", 5);
				BindTextureToShader((OGLTexture*)(*i).pbrTextures->heightMap, "heightMap", 6);
				BindTextureToShader((OGLTexture*)(*i).pbrTextures->emission, "emissionTex", 7);
				BindTextureToShader((OGLTexture*)(*i).pbrTextures->ao, "AOTex", 8);
				BindTextureToShader((OGLTexture*)(*i).pbrTextures->opacity, "opacityTex", 9);
				BindTextureToShader((OGLTexture*)(*i).pbrTextures->gloss, "glossTex", 10);
			}
			else {
				BindTextureToShader((OGLTexture*)(*i).baseTex, "baseTex", 1);
				BindTextureToShader((OGLTexture*)(*i).bumpTex, "bumpTex", 3);
			}
			
			
			glUniform1i(hasTexLocation,1);

		}
		else {
			if ((OGLTexture*)i->GetDefaultTexture()) {
				glBindImageTexture(0, ((OGLTexture*)i->GetDefaultTexture())->GetObjectID(), 0, GL_FALSE, NULL, GL_READ_ONLY, GL_R8UI);
			}
		}
		
//		glDisable(GL_CULL_FACE);//todo turn back on
		BindMesh((*i).GetMesh());
		int layerCount = (*i).GetMesh()->GetSubMeshCount();
		for (int i = 0; i < layerCount; ++i) {
			DrawBoundMesh(i);
		}
	}
	glEnable(GL_BLEND);
	glPopDebugGroup();

	
}

void GameTechRenderer::RenderFullScreenQuadWithTexture(GLuint texture) {
	glDepthMask(false);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE); //todo reverse winding order
	BindShader(quadShader);
	
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(quadShader->GetProgramID(), "mainTex"), 0);
	glUniform1i(glGetUniformLocation(quadShader->GetProgramID(), "hasTexture"), true);
	
	glBindTexture(GL_TEXTURE_2D, texture);
	BindMesh(quad->GetMesh());
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDepthMask(true);
}

void GameTechRenderer::RenderBloom() {
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 5, "bloom");
	glDepthMask(false);
	BindShader(bloomShader);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(bloomShader->GetProgramID(), "sceneTex"), 0);
	glBindTexture(GL_TEXTURE_2D, sceneColor);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(bloomShader->GetProgramID(), "hdrTex"), 1);
	glBindTexture(GL_TEXTURE_2D, sceneHdrTex);

	BindMesh(quad->GetMesh());
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDepthMask(true);
	glPopDebugGroup();
}

void GameTechRenderer::FXAA() {
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 4, "fxaa");
	
	EdgeDetection();
	BindShader(fxaaShader);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(fxaaShader->GetProgramID(), "mainTex"), 0);
	glBindTexture(GL_TEXTURE_2D, sceneColor);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(fxaaShader->GetProgramID(), "edgesTex"), 1);
	glBindTexture(GL_TEXTURE_2D, edgesTex);

	glUniform1i(glGetUniformLocation(fxaaShader->GetProgramID(), "width"), windowWidth);
	glUniform1i(glGetUniformLocation(fxaaShader->GetProgramID(), "height"), windowHeight);

	glUniform1i(glGetUniformLocation(fxaaShader->GetProgramID(), "edgeDetection"), edgeDetection);

	BindMesh(quad->GetMesh());
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glPopDebugGroup();
}

void GameTechRenderer::SMAA() {
	EdgeDetection();
	if (renderEdges) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		RenderFullScreenQuadWithTexture(edgesTex);
		return;
	}

	WeightCalculation();
	if (renderBlend) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		RenderFullScreenQuadWithTexture(blendTex);
	}

	NeighborhoodBlending();
	if (renderAA) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		RenderFullScreenQuadWithTexture(smaaOutput);
	}
}

void GameTechRenderer::EdgeDetection() {
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 4, "edge");
	glBindFramebuffer(GL_FRAMEBUFFER, edgesFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	BindShader(edgesShader);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(edgesShader->GetProgramID(), "depthTex"), 0);
	glUniform1i(glGetUniformLocation(edgesShader->GetProgramID(), "width"), windowWidth);
	glUniform1i(glGetUniformLocation(edgesShader->GetProgramID(), "height"), windowHeight);
	glBindTexture(GL_TEXTURE_2D, sceneColor);
	glUniform1f(glGetUniformLocation(edgesShader->GetProgramID(), "SMAA_THRESHOLD"), smaaEdgeThreshold);
	BindMesh(quad->GetMesh());
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopDebugGroup();
}

void GameTechRenderer::WeightCalculation() {
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 6, "weight");
	glBindFramebuffer(GL_FRAMEBUFFER, weightCalcFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	BindShader(weightCalcShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, edgesTex);
	glUniform1i(glGetUniformLocation(weightCalcShader->GetProgramID(), "edgesTex"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, areaTex);
	glUniform1i(glGetUniformLocation(weightCalcShader->GetProgramID(), "areaTex"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, searchTex);
	glUniform1i(glGetUniformLocation(weightCalcShader->GetProgramID(), "searchTex"), 2);

	glUniform1i(glGetUniformLocation(weightCalcShader->GetProgramID(), "width"), windowWidth);
	glUniform1i(glGetUniformLocation(weightCalcShader->GetProgramID(), "height"), windowHeight);

	glUniform1f(glGetUniformLocation(weightCalcShader->GetProgramID(), "SMAA_THRESHOLD"), smaaEdgeThreshold);

	BindMesh(quad->GetMesh());
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopDebugGroup();
}

void GameTechRenderer::NeighborhoodBlending() {
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 8, "neighbor");
	glBindFramebuffer(GL_FRAMEBUFFER, neighborhoodBlendingFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	BindShader(neighborhoodBlendingShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneColor);
	glUniform1i(glGetUniformLocation(neighborhoodBlendingShader->GetProgramID(), "colorTex"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, blendTex);
	glUniform1i(glGetUniformLocation(neighborhoodBlendingShader->GetProgramID(), "blendTex"), 1);


	glUniform1i(glGetUniformLocation(neighborhoodBlendingShader->GetProgramID(), "width"), windowWidth);
	glUniform1i(glGetUniformLocation(neighborhoodBlendingShader->GetProgramID(), "height"), windowHeight);

	glUniform1f(glGetUniformLocation(neighborhoodBlendingShader->GetProgramID(), "SMAA_THRESHOLD"), smaaEdgeThreshold);

	BindMesh(quad->GetMesh());
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopDebugGroup();
}

void GameTechRenderer::Blur() {
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 10, "downsample");


	downsampleComputeShader->Bind();

	int width = windowWidth, height = windowHeight;
	std::array<std::pair<int, int>, numBloomMips> sizes;
	for (int i = 0; i < numBloomMips; i++) {
		sizes[i] = std::make_pair(width, height);

		width = max(1, width / 2);
		height = max(1, height / 2);
		if (width == 1 && height == 1) break;

		glUniform1i(glGetUniformLocation(downsampleComputeShader->GetProgramID(), "width"), width);
		glUniform1i(glGetUniformLocation(downsampleComputeShader->GetProgramID(), "height"), height);
		glBindImageTexture(0, sceneHdrTex, i, GL_FALSE, NULL, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(1, sceneHdrTex, i+1, GL_FALSE, NULL, GL_WRITE_ONLY, GL_RGBA32F);
		downsampleComputeShader->Execute(width / 8 + 1, height / 8 + 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	upsampleComputeShader->Bind();

	for (int i = numBloomMips - 1; i > 0; i--) {
		auto [ srcWidth, srcHeight ] = sizes[i];
		auto [ dstWidth, dstHeight ] = sizes[i - 1];

		glUniform1i(glGetUniformLocation(upsampleComputeShader->GetProgramID(), "srcWidth"), srcWidth);
		glUniform1i(glGetUniformLocation(upsampleComputeShader->GetProgramID(), "srcHeight"), srcHeight);
		glUniform1i(glGetUniformLocation(upsampleComputeShader->GetProgramID(), "dstWidth"), dstWidth);
		glUniform1i(glGetUniformLocation(upsampleComputeShader->GetProgramID(), "dstHeight"), dstHeight);
		glUniform1f(glGetUniformLocation(upsampleComputeShader->GetProgramID(), "filterRadius"), upsampleFilterRadius);

		glBindImageTexture(0, sceneHdrTex, i, GL_FALSE, NULL, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(1, sceneHdrTex, i - 1, GL_FALSE, NULL, GL_WRITE_ONLY, GL_RGBA32F);
		upsampleComputeShader->Execute(dstWidth / 8 + 1, dstHeight / 8 + 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

	}

	glPopDebugGroup();
}

MeshGeometry* GameTechRenderer::LoadMesh(const string& name, std::vector<MeshGeometry*>* meshes) {
	OGLMesh* mesh = new OGLMesh(name);
	mesh->SetPrimitiveType(GeometryPrimitive::Patches);
	mesh->UploadToGPU();
	if (meshes != nullptr)meshes->push_back(mesh);
	return mesh;
}



void GameTechRenderer::NewRenderLines() {
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();
	if (lines.empty()) {
		return;
	}
	float screenAspect = (float)windowWidth / (float)windowHeight;
	Matrix4 viewMatrix = gameWorld.GetMainCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera()->BuildProjectionMatrix(screenAspect);
	
	Matrix4 viewProj  = projMatrix * viewMatrix;

	BindShader(debugShader);
	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 0);

	glUniformMatrix4fv(matSlot, 1, false, (float*)viewProj.array);

	debugLineData.clear();

	int frameLineCount = lines.size() * 2;

	SetDebugLineBufferSizes(frameLineCount);

	glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameLineCount * sizeof(Debug::DebugLineEntry), lines.data());
	

	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, frameLineCount);
	glBindVertexArray(0);
}

void GameTechRenderer::NewRenderText() {
	const std::vector<Debug::DebugStringEntry>& strings = Debug::GetDebugStrings();
	if (strings.empty()) {
		return;
	}

	BindShader(debugShader);

	OGLTexture* t = (OGLTexture*)Debug::GetDebugFont()->GetTexture();

	if (t) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t->GetObjectID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);	
		BindTextureToShader(t, "mainTex", 0);
	}
	Matrix4 proj = Matrix4::Orthographic(0.0, 100.0f, 100, 0, -1.0f, 1.0f);

	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	glUniformMatrix4fv(matSlot, 1, false, (float*)proj.array);

	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 1);

	debugTextPos.clear();
	debugTextColours.clear();
	debugTextUVs.clear();

	int frameVertCount = 0;
	for (const auto& s : strings) {
		frameVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}
	SetDebugStringBufferSizes(frameVertCount);

	for (const auto& s : strings) {
		float size = 20.0f;
		Debug::GetDebugFont()->BuildVerticesForString(s.data, s.position, s.colour, size, debugTextPos, debugTextUVs, debugTextColours);
	}


	glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector3), debugTextPos.data());
	glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector4), debugTextColours.data());
	glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector2), debugTextUVs.data());

	glBindVertexArray(textVAO);
	glDrawArrays(GL_TRIANGLES, 0, frameVertCount);
	glBindVertexArray(0);
}



TextureBase* GameTechRenderer::LoadTexture(const string& name) {
	return TextureLoader::LoadAPITexture(name);
}

ShaderBase* GameTechRenderer::LoadShader(const string& vertex, const string& fragment) {
	return new OGLShader(vertex, fragment);
}

ShaderBase* GameTechRenderer::LoadShader(const string& vertex, const string& fragment, const string& domain, const string& hull) {
	return new OGLShader(vertex, fragment, "",domain,hull);
}


void GameTechRenderer::SetDebugStringBufferSizes(size_t newVertCount) {
	if (newVertCount > textCount) {
		textCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector4), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector2), nullptr, GL_DYNAMIC_DRAW);

		debugTextPos.reserve(textCount);
		debugTextColours.reserve(textCount);
		debugTextUVs.reserve(textCount);

		glBindVertexArray(textVAO);

		glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, textVertVBO, 0, sizeof(Vector3));

		glVertexAttribFormat(1, 4, GL_FLOAT, false, 0);
		glVertexAttribBinding(1, 1);
		glBindVertexBuffer(1, textColourVBO, 0, sizeof(Vector4));

		glVertexAttribFormat(2, 2, GL_FLOAT, false, 0);
		glVertexAttribBinding(2, 2);
		glBindVertexBuffer(2, textTexVBO, 0, sizeof(Vector2));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}
}

void GameTechRenderer::SetDebugLineBufferSizes(size_t newVertCount) {
	if (newVertCount > lineCount) {
		lineCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
		glBufferData(GL_ARRAY_BUFFER, lineCount * sizeof(Debug::DebugLineEntry), nullptr, GL_DYNAMIC_DRAW);

		debugLineData.reserve(lineCount);

		glBindVertexArray(lineVAO);

		int realStride = sizeof(Debug::DebugLineEntry) / 2;

		glVertexAttribFormat(0, 3, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, start));
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, lineVertVBO, 0, realStride);

		glVertexAttribFormat(1, 4, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, colourA));
		glVertexAttribBinding(1, 0);
		glBindVertexBuffer(1, lineVertVBO, sizeof(Vector4), realStride);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}
}


//this was me
void GameTechRenderer::ImGui() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("NotSplatoon");
	Vector3 camPos = gameWorld.GetMainCamera()->GetPosition();
	std::string camPosStr = std::to_string(camPos.x) + " "
		+ std::to_string(camPos.y) + " " + std::to_string(camPos.z);
	ImGui::Text(camPosStr.c_str());
	if (ImGui::TreeNode("Ray Marching")) {
		ImGui::Checkbox("Raymarch", imguiptrs.rayMarchBool);
		ImGui::SliderInt("Max Steps", imguiptrs.rayMarchMaxSteps, 1, 1000);
		ImGui::SliderFloat("Hit Distance", imguiptrs.rayMarchHitDistance, 0, 1);
		ImGui::SliderFloat("No Hit Distance", imguiptrs.rayMarchNoHitDistance, 0, 1000);
		ImGui::SliderFloat("Debug Value", imguiptrs.debugValue, -1, 10);
		ImGui::Checkbox("Depth Test", imguiptrs.depthTest);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Paint Testing")) {
		ImGui::SliderFloat3("Position", imguiptrs.testSphereCenter->array, -200, 500);
		ImGui::SliderFloat("Sphere Radius", imguiptrs.testSphereRadius, 0, 2000);
		ImGui::Checkbox("New Method", imguiptrs.newMethod);
		ImGui::SliderFloat("Noise Scale", &noiseScale,0,10);
		ImGui::SliderFloat("Noise Offset Size", &noiseOffsetSize,0,0.1f);
		ImGui::SliderFloat("Noise Normal Strength", &noiseNormalStrength,0,10);
		ImGui::SliderFloat("Noise Normal Multiplier", &noiseNormalNoiseMult,0,10);
		ImGui::SliderFloat("Time Scale", &timeScale,0,1);
		if (ImGui::Button("Move to Center")) { *(imguiptrs.testSphereCenter) = Vector3(0, 0, 0); }

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("PBR")) {
		ImGui::Checkbox("Bump Map", &useBumpMap);
		ImGui::Checkbox("Metallic Map", &useMetallicMap);
		ImGui::Checkbox("Roughness Map", &useRoughnessMap);
		ImGui::Checkbox("Height Map", &useHeightMap);
		ImGui::Checkbox("Emission Map", &useEmissionMap);
		ImGui::Checkbox("AO Map", &useAOMap);
		ImGui::Checkbox("Opacity Map", &useOpacityMap);
		ImGui::Checkbox("Gloss Map", &useGlossMap);
		ImGui::SliderFloat("Heightmap Strength", &heightMapStrength, 0, 10);
		ImGui::SliderInt("Emission Strength", &emissionStrength, 0, 10000);

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("FXAA")) {
		ImGui::Checkbox("Use FXAA", &useFXAA);
		ImGui::Checkbox("Edge Detection", &edgeDetection);
		ImGui::SliderFloat("Edge Threshold", &smaaEdgeThreshold, 0, 0.5);

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Bloom")) {
		ImGui::Checkbox("Render Bloom", &bloom);
		ImGui::SliderFloat("Filter radius", &upsampleFilterRadius, 0, 0.01f, "%.10f");
		ImGui::TreePop();
	}
	

	ImGui::SliderFloat3("Light Position", lightPosition.array, -200, 200);

	ImGui::End();
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GameTechRenderer::DrawCrossHair() {
	glDepthMask(false);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE); //todo reverse winding order
	BindShader(quadShader);
	glUniform1i(glGetUniformLocation(quadShader->GetProgramID(), "rotated"), true);
	glUniform1i(glGetUniformLocation(quadShader->GetProgramID(), "hasTexture"), false);
	BindMesh(crosshair->GetMesh());
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 9, "crosshair");
	glDrawArrays(GL_LINES, 0, 4);
	glPopDebugGroup();
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
}

void GameTechRenderer::CreateFBOColorDepth(GLuint& fbo, GLuint& colorTex, GLuint& depthTex, GLuint& hdrTex,  GLenum colorFormat, bool withMips)
{
	std::unordered_map<GLenum, GLenum> formats{
		{GL_RGBA8,GL_RGBA},
		{GL_RGBA16,GL_RGBA},
		{GL_RGBA16F,GL_RGBA},
		{GL_RGBA32F,GL_RGBA},
	};
	if (!formats.contains(colorFormat)) {
		std::cout << "missing fbo format!!!";
		return;
	}

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 11, "creatingFBO");
	glGenTextures(1, &colorTex);
	glBindTexture(GL_TEXTURE_2D, colorTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, windowWidth, windowHeight, 0, formats[colorFormat], GL_FLOAT, NULL);

	
	glGenTextures(1, &hdrTex);
	glBindTexture(GL_TEXTURE_2D, hdrTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, windowWidth, windowHeight, 0, formats[colorFormat], GL_FLOAT, NULL);
	if (withMips) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numBloomMips);

		int width = windowWidth, height = windowHeight;
		for (int i = 0; i < numBloomMips; i++) {
			width = max(1, width / 2);
			height = max(1, height / 2);
			glTexImage2D(GL_TEXTURE_2D, i + 1, colorFormat, width, height, 0, formats[colorFormat], GL_FLOAT, NULL);
			if (width == 1 && height == 1)break;
		}
		//glGenerateMipmap(GL_TEXTURE_2D);
	}

	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);


	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, hdrTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
	GLenum buffers[2] = { GL_COLOR_ATTACHMENT0 ,GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2,buffers );
	glObjectLabel(GL_FRAMEBUFFER, fbo, -1, std::string("colorDepthFBO").c_str());
	

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "fbo creation error!!!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopDebugGroup();
}

void GameTechRenderer::CreateFBOColor(GLuint& fbo, GLuint& colorTex, GLenum colorFormat)
{
	std::map<GLenum, GLenum> formats{
		{GL_RGBA8,GL_RGBA},
		{GL_RGBA16,GL_RGBA},
		{GL_RGBA16F,GL_RGBA},
	};
	if (!formats.contains(colorFormat)) {
		std::cout << "missing fbo format!!!";
		return;
	}

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 11, "creatingFBO");
	glGenTextures(1, &colorTex);
	glBindTexture(GL_TEXTURE_2D, colorTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, windowWidth, windowHeight, 0, formats[colorFormat], GL_FLOAT, NULL);


	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
	glObjectLabel(GL_FRAMEBUFFER, fbo, -1, std::string("colorFBO").c_str());


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "fbo creation error!!!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopDebugGroup();
}

