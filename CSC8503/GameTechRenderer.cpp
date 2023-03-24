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

#define SHADOWSIZE 2048

Matrix4 biasMatrix = Matrix4::Translation(Vector3(0.5f, 0.5f, 0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.5f, 0.5f));

GameTechRenderer::GameTechRenderer(Window* window) :
	OGLRenderer(*window)
{
	//Initialize settings
	rayMarchingSettings.maxSteps = 500;
	rayMarchingSettings.hitDistance = 0.001f;
	rayMarchingSettings.noHitDistance = 1000;
	rayMarchingSettings.debugValue = 1;
	rayMarchingSettings.rayMarchDepthTest = true;

	lightColour = Vector4(0.8f, 0.8f, 0.5f, 1.0f);
	lightRadius = 1000.0f;
	lightPosition = Vector3(0.0f, 53, 160);

	//Load all shaders
	debugShader = new OGLShader("debug.vert", "debug.frag");
	shadowShader = new OGLShader("shadow.vert", "shadow.frag");
	quadShader = new OGLShader("quad.vert", "quad.frag");
	rayMarchComputeShader = new OGLComputeShader("rayMarchCompute.glsl");
	edgesShader = new OGLShader("smaaEdgeDetection.vert", "smaaEdgeDetectionColor.frag");
	fxaaShader = new OGLShader("fxaa.vert", "fxaa.frag"); //https://stackoverflow.com/questions/12105330/how-does-this-simple-fxaa-work
	paintCollisionShader = new OGLComputeShader("paintBoardCollision.comp");
	maskRasterizerShader = new OGLComputeShader("paintMaskRasterizer.comp");
	drawPaintCollisionsShader = new OGLShader("debugPaintCollisions.vert", "debugPaintCollisions.frag");

	LoadSkybox();
	CreateDebugData();
	CreateShadowFbo();

	//Load all render objects
	quad = new RenderObject(nullptr, OGLMesh::GenerateQuadWithIndices(), nullptr, quadShader);

	//Create OpenGL resources
	glGenBuffers(1, &rayMarchSphereSSBO);
	glGenBuffers(1, &paintCollideUniforms);
	glGenBuffers(1, &paintCollideTriangleIds);
	glGenBuffers(1, &maskRasterUniforms);
	glGenTextures(1, &rayMarchTexture);
	glGenVertexArrays(1, &paintCollisionDrawVAO);

	//Create view-dependent resources
	this->renderWidth = windowWidth;
	this->renderHeight = windowHeight;

	CreateViewDependent();

	InitializeImGui();

	glEnable(GL_DEPTH_TEST);
}

GameTechRenderer::~GameTechRenderer() {
	DestroyViewDependent();

	glDeleteVertexArrays(1, &paintCollisionDrawVAO);
	glDeleteTextures(1, &shadowTex);
	glDeleteTextures(1, &rayMarchTexture);
	glDeleteBuffers(1, &maskRasterUniforms);
	glDeleteBuffers(1, &paintCollideTriangleIds);
	glDeleteBuffers(1, &paintCollideUniforms);
	glDeleteBuffers(1, &rayMarchSphereSSBO);

	delete quad;

	DestroyShadowFbo();
	DestroyDebugData();
	UnloadSkybox();

	delete drawPaintCollisionsShader;
	delete paintCollisionShader;
	delete maskRasterizerShader;
	delete fxaaShader;
	delete edgesShader;
	delete rayMarchComputeShader;
	delete debugShader;
	delete shadowShader;
	delete quadShader;
}

MeshGeometry* GameTechRenderer::LoadMesh(const string& name, std::vector<MeshGeometry*>* meshes) {
	OGLMesh* mesh = new OGLMesh(name);
	mesh->SetPrimitiveType(GeometryPrimitive::Patches);
	mesh->UploadToGPU();
	if (meshes != nullptr)meshes->push_back(mesh);
	return mesh;
}

TextureBase* GameTechRenderer::LoadTexture(const string& name) {
	return TextureLoader::LoadAPITexture(name);
}

ShaderBase* GameTechRenderer::LoadShader(const string& vertex, const string& fragment) {
	return new OGLShader(vertex, fragment);
}

ShaderBase* GameTechRenderer::LoadShader(const string& vertex, const string& fragment, const string& domain, const string& hull) {
	return new OGLShader(vertex, fragment, "", domain, hull);
}

void GameTechRenderer::BeginImGui()
{
	ImGuiIO& io = ImGui::GetIO();

	//Tell ImGui to ignore mouse movement when the cursor is hidden
	if (!Window::GetWindow()->IsMouseLocked())
		io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	else
		io.ConfigFlags |= ImGuiConfigFlags_NoMouse;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void GameTechRenderer::EndImGui()
{
	ImGui::EndFrame();
}

void GameTechRenderer::OnWindowResize(int w, int h)
{
	DestroyViewDependent();

	OGLRenderer::OnWindowResize(w, h);
	this->renderWidth = w;
	this->windowHeight = h;

	CreateViewDependent();
}

void GameTechRenderer::BeginFrame()
{

}

void GameTechRenderer::RenderFrame()
{
	//////////////////////////////////////////////////////////////////
	// Prepare the frame (upload data, run compute shaders, etc.)
	//////////////////////////////////////////////////////////////////
	if (rayMarchingSettings.enabled)
	{
		SendRayMarchData();
	}

	FindTrianglesToPaint();

	//////////////////////////////////////////////////////////////////
	// Draw the scene
	//////////////////////////////////////////////////////////////////
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_CULL_FACE);

	RenderShadowMap();

	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, renderWidth, renderHeight);

	RenderSkybox();
	RenderCamera();

	if (rayMarchingSettings.enabled)
	{
		//NOTE(Jason): I'm if a memory barrier is necessary. The driver might automactically place barriers
		//when switching FBOs.
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 9, "Ray march");
		ExecuteRayMarching();

		glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

		RenderFullScreenQuadWithTexture(rayMarchTexture);
		glPopDebugGroup();
	}

	//////////////////////////////////////////////////////////////////
	// Apply post-processing
	//////////////////////////////////////////////////////////////////
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, renderWidth, renderHeight);
	
	RenderFullScreenQuadWithTexture(sceneColor);//todo fix rotation

	if (fxaaEnabled)
	{
		RenderFXAAPass();
	}

	//Render debug stuff
	glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	NewRenderLines();
	NewRenderText();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Produce draw data and render it
	ImGui::Render();

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 5, "ImGui");
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glPopDebugGroup();
}

void GameTechRenderer::EndFrame()
{
	marchedSpheresToDraw.clear();
	activeObjects.clear();
	paintedSpots.clear();
}

void GameTechRenderer::AttachPaintMask(GameObject* object, int width, int height)
{
	object->GetRenderObject()->isPaintable = true;
	object->GetRenderObject()->maskTex = new OGLTexture();
	object->GetRenderObject()->maskDimensions = { (float)width, (float)height };

	GLuint maskId = ((OGLTexture*)object->GetRenderObject()->maskTex)->GetObjectID();

	glBindTexture(GL_TEXTURE_2D, maskId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	const uint32_t FILL = 0;
	glClearTexImage(maskId, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &FILL);
}

void GameTechRenderer::CreateViewDependent()
{
	//Create the ray march texture
	glBindTexture(GL_TEXTURE_2D, rayMarchTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, renderWidth, renderHeight, 0, GL_RGBA, GL_FLOAT, nullptr);

	//Create scene FBO
	CreateFBOColorDepth(sceneFBO, sceneColor, sceneDepth, GL_RGBA16F);

	//Create edge detection FBO
	CreateFBOColor(edgesFBO, edgesTex);
}

void GameTechRenderer::DestroyViewDependent()
{
	glDeleteTextures(1, &sceneColor);
	glDeleteTextures(1, &sceneDepth);
	glDeleteFramebuffers(1, &sceneFBO);

	glDeleteTextures(1, &edgesTex);
	glDeleteFramebuffers(1, &edgesFBO);
}

void GameTechRenderer::InitializeImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();

	//OGLRenderer will create a backbuffer that converts from linear to sRGB automatically. Since
	//there is no post-processing or gamma correction that we apply to ImGui when rendering, we need
	//pre-gamma correct all the colors here
	for (int i = 0; i < ImGuiCol_COUNT; ++i)
	{
		const float gamma = 2.2f;

		ImVec4 color = ImGui::GetStyle().Colors[i];

		color.x = pow(color.x, gamma);
		color.y = pow(color.y, gamma);
		color.z = pow(color.z, gamma);

		ImGui::GetStyle().Colors[i] = color;
	}

	ImGui_ImplWin32_Init(((Win32Code::Win32Window&)hostWindow).GetHandle());
	ImGui_ImplOpenGL3_Init("#version 330");
}

void GameTechRenderer::LoadSkybox()
{
	skyboxShader = new OGLShader("skybox.vert", "skybox.frag");

	skyboxMesh = new OGLMesh();
	skyboxMesh->SetVertexPositions({ Vector3(-1, 1,-1), Vector3(-1,-1,-1) , Vector3(1,-1,-1) , Vector3(1,1,-1) });
	skyboxMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	skyboxMesh->UploadToGPU();

	string filenames[6] = {
		"/Cubemap/skyrender0004.png",
		"/Cubemap/skyrender0001.png",
		"/Cubemap/skyrender0003.png",
		"/Cubemap/skyrender0006.png",
		"/Cubemap/skyrender0002.png",
		"/Cubemap/skyrender0005.png"
	};

	int width[6] = { 0 };
	int height[6] = { 0 };
	int channels[6] = { 0 };
	int flags[6] = { 0 };

	std::vector<char*> texData(6, nullptr);

	for (int i = 0; i < 6; ++i) 
	{
		TextureLoader::LoadTexture(filenames[i], texData[i], width[i], height[i], channels[i], flags[i],false);

		if (i > 0 && (width[i] != width[0] || height[0] != height[0])) {
			std::cout << __FUNCTION__ << " cubemap input textures don't match in size?\n";
			return;
		}
	}

	GLenum type = channels[0] == 4 ? GL_RGBA : GL_RGB;
	
	glGenTextures(1, &skyboxTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, type, GL_UNSIGNED_BYTE, texData[i]);
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GameTechRenderer::UnloadSkybox()
{
	glDeleteTextures(1, &skyboxTex);

	delete skyboxMesh;
	delete skyboxShader;
}

void GameTechRenderer::CreateShadowFbo()
{
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::DestroyShadowFbo()
{
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
}

void GameTechRenderer::CreateDebugData()
{
	glGenVertexArrays(1, &debugData.lineVAO);
	glGenVertexArrays(1, &debugData.textVAO);

	glGenBuffers(1, &debugData.lineVertVBO);
	glGenBuffers(1, &debugData.textVertVBO);
	glGenBuffers(1, &debugData.textColourVBO);
	glGenBuffers(1, &debugData.textTexVBO);

	SetDebugStringBufferSizes(10000);
	SetDebugLineBufferSizes(1000);
}

void GameTechRenderer::DestroyDebugData()
{
	glDeleteVertexArrays(1, &debugData.lineVAO);
	glDeleteVertexArrays(1, &debugData.textVAO);

	glDeleteBuffers(1, &debugData.lineVertVBO);
	glDeleteBuffers(1, &debugData.textVertVBO);
	glDeleteBuffers(1, &debugData.textColourVBO);
	glDeleteBuffers(1, &debugData.textTexVBO);
}

void GameTechRenderer::CreateFBOColorDepth(GLuint& fbo, GLuint& colorTex, GLuint& depthTex, GLenum colorFormat)
{
	std::map<GLenum, GLenum> formats{
		{GL_RGBA8,GL_RGBA},
		{GL_RGBA16,GL_RGBA},
		{GL_RGBA16F,GL_RGBA},
	};

	if (formats.find(colorFormat) == formats.end()) {
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
	glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, renderWidth, renderHeight, 0, formats[colorFormat], GL_FLOAT, NULL);

	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderWidth, renderHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
	glObjectLabel(GL_FRAMEBUFFER, fbo, -1, std::string("colorDepthFBO").c_str());

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "fbo creation error!!!\n";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopDebugGroup();
}

void GameTechRenderer::CreateFBOColor(GLuint& fbo, GLuint& colorTex, GLenum colorFormat)
{
	std::unordered_map<GLenum, GLenum> formats{
		{GL_RGBA8,GL_RGBA},
		{GL_RGBA16,GL_RGBA},
		{GL_RGBA16F,GL_RGBA},
	};

	if (formats.find(colorFormat) == formats.end()) {
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
	glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, renderWidth, renderHeight, 0, formats[colorFormat], GL_FLOAT, NULL);

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

void GameTechRenderer::NewRenderLines() {
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();
	if (lines.empty()) return;

	float screenAspect = (float)renderWidth / (float)renderHeight;
	Matrix4 viewMatrix = activeCamera->BuildViewMatrix();
	Matrix4 projMatrix = activeCamera->BuildProjectionMatrix(screenAspect);
	Matrix4 viewProj = projMatrix * viewMatrix;

	BindShader(debugShader);

	GLint matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	GLint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");

	glUniform1i(texSlot, 0);
	glUniformMatrix4fv(matSlot, 1, false, (float*)viewProj.array);

	debugData.debugLineData.clear();

	int frameLineCount = (int)lines.size() * 2;
	SetDebugLineBufferSizes(frameLineCount);

	glBindBuffer(GL_ARRAY_BUFFER, debugData.lineVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameLineCount * sizeof(Debug::DebugLineEntry), lines.data());

	glBindVertexArray(debugData.lineVAO);
	glDrawArrays(GL_LINES, 0, frameLineCount);
	glBindVertexArray(0);
}

void GameTechRenderer::NewRenderText() {
	const std::vector<Debug::DebugStringEntry>& strings = Debug::GetDebugStrings();
	if (strings.empty()) {
		return;
	}

	BindShader(debugShader);

	OGLTexture* debugFont = (OGLTexture*)Debug::GetDebugFont()->GetTexture();

	if (debugFont) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, debugFont->GetObjectID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		BindTextureToShader(debugFont, "mainTex", 0);
	}

	Matrix4 proj = Matrix4::Orthographic(0.0, 100.0f, 100, 0, -1.0f, 1.0f);

	GLint matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	GLint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");

	glUniformMatrix4fv(matSlot, 1, false, (float*)proj.array);
	glUniform1i(texSlot, 1);

	debugData.debugTextPos.clear();
	debugData.debugTextColours.clear();
	debugData.debugTextUVs.clear();

	int frameVertCount = 0;
	for (const auto& s : strings) {
		frameVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}

	SetDebugStringBufferSizes(frameVertCount);

	for (const auto& s : strings) {
		float size = 20.0f;
		Debug::GetDebugFont()->BuildVerticesForString(s.data, s.position, s.colour, size, debugData.debugTextPos, debugData.debugTextUVs, debugData.debugTextColours);
	}

	glBindBuffer(GL_ARRAY_BUFFER, debugData.textVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector3), debugData.debugTextPos.data());
	glBindBuffer(GL_ARRAY_BUFFER, debugData.textColourVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector4), debugData.debugTextColours.data());
	glBindBuffer(GL_ARRAY_BUFFER, debugData.textTexVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector2), debugData.debugTextUVs.data());

	glBindVertexArray(debugData.textVAO);
	glDrawArrays(GL_TRIANGLES, 0, frameVertCount);
	glBindVertexArray(0);
}

void GameTechRenderer::SetDebugStringBufferSizes(size_t newVertCount)
{
	if (newVertCount > debugData.textCount)
	{
		debugData.textCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, debugData.textVertVBO);
		glBufferData(GL_ARRAY_BUFFER, debugData.textCount * sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, debugData.textColourVBO);
		glBufferData(GL_ARRAY_BUFFER, debugData.textCount * sizeof(Vector4), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, debugData.textTexVBO);
		glBufferData(GL_ARRAY_BUFFER, debugData.textCount * sizeof(Vector2), nullptr, GL_DYNAMIC_DRAW);

		debugData.debugTextPos.reserve(debugData.textCount);
		debugData.debugTextColours.reserve(debugData.textCount);
		debugData.debugTextUVs.reserve(debugData.textCount);

		glBindVertexArray(debugData.textVAO);

		glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, debugData.textVertVBO, 0, sizeof(Vector3));

		glVertexAttribFormat(1, 4, GL_FLOAT, false, 0);
		glVertexAttribBinding(1, 1);
		glBindVertexBuffer(1, debugData.textColourVBO, 0, sizeof(Vector4));

		glVertexAttribFormat(2, 2, GL_FLOAT, false, 0);
		glVertexAttribBinding(2, 2);
		glBindVertexBuffer(2, debugData.textTexVBO, 0, sizeof(Vector2));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}
}

void GameTechRenderer::SetDebugLineBufferSizes(size_t newVertCount)
{
	if (newVertCount > debugData.lineCount)
	{
		debugData.lineCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, debugData.lineVertVBO);
		glBufferData(GL_ARRAY_BUFFER, debugData.lineCount * sizeof(Debug::DebugLineEntry), nullptr, GL_DYNAMIC_DRAW);

		debugData.debugLineData.reserve(debugData.lineCount);

		glBindVertexArray(debugData.lineVAO);

		int realStride = sizeof(Debug::DebugLineEntry) / 2;

		glVertexAttribFormat(0, 3, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, start));
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, debugData.lineVertVBO, 0, realStride);

		glVertexAttribFormat(1, 4, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, colourA));
		glVertexAttribBinding(1, 0);
		glBindVertexBuffer(1, debugData.lineVertVBO, sizeof(Vector4), realStride);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}
}

void GameTechRenderer::SendRayMarchData()
{
	struct SphereToSend
	{
		float x, y, z;
		float radius;
		float r, g, b;
	};

	std::vector<SphereToSend> spheresToSend(marchedSpheresToDraw.size());

	for (size_t i = 0; i < marchedSpheresToDraw.size(); i++)
	{
		RayMarchedSphere* in = &marchedSpheresToDraw[i];
		SphereToSend* out = &spheresToSend[i];

		out->x = in->position.x;
		out->y = in->position.y;
		out->z = in->position.z;
		out->radius = in->radius;
		out->r = in->color.x;
		out->g = in->color.y;
		out->b = in->color.z;
	}

	//NOTE(Jason): I'm using GL_STATIC_DRAW to force OpenGL (as much as we can) to put this on device-local
	//memory. It could hurt performance if the SSBO ends up on the CPU, even if we are updating to often.
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, rayMarchSphereSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, spheresToSend.size() * sizeof(SphereToSend), spheresToSend.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GameTechRenderer::ExecuteRayMarching()
{
	Matrix4 viewMatrix = activeCamera->BuildViewMatrix();
	Matrix4 projMatrix = activeCamera->BuildProjectionMatrix((float)renderWidth / (float)renderHeight);
	Vector3 cameraPos = activeCamera->GetPosition();

	rayMarchComputeShader->Bind();

	//Getting the uniform location like this is slow! We should cache uniform in a map or a struct for better performance
	int projLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "viewMatrix");
	int cameraPosLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "cameraPos");
	int maxStepsLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "maxSteps");
	int hitDistanceLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "hitDistance");
	int noHitDistanceLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "noHitDistance");
	int viewportWidthLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "viewportWidth");
	int viewportHeightLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "viewportHeight");
	int numSpheresLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "numSpheres");
	int depthTexLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "depthTex");
	int nearPlaneLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "nearPlane");
	int farPlaneLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "farPlane");
	int debugValueLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "debugValue");
	int depthTestValueLocation = glGetUniformLocation(rayMarchComputeShader->GetProgramID(), "depthTest");

	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);
	glUniform3fv(cameraPosLocation, 1, cameraPos.array);
	glUniform1i(maxStepsLocation, rayMarchingSettings.maxSteps);
	glUniform1f(hitDistanceLocation, rayMarchingSettings.hitDistance);
	glUniform1f(noHitDistanceLocation, rayMarchingSettings.noHitDistance);
	glUniform1i(viewportWidthLocation, renderWidth);
	glUniform1i(viewportHeightLocation, renderHeight);
	glUniform1i(numSpheresLocation, (GLint)marchedSpheresToDraw.size()/*+2*/);//one for debug sphere
	glUniform1f(nearPlaneLocation, activeCamera->GetNearPlane());
	glUniform1f(farPlaneLocation, activeCamera->GetFarPlane());
	glUniform1f(debugValueLocation, rayMarchingSettings.debugValue);
	glUniform1i(depthTestValueLocation, rayMarchingSettings.rayMarchDepthTest);
	glUniform1i(depthTexLocation, 1);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, rayMarchSphereSSBO);

	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, rayMarchTexture, 0, GL_FALSE, NULL, GL_WRITE_ONLY, GL_RGBA16);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sceneDepth);

	rayMarchComputeShader->Execute(renderWidth/8+1, renderHeight/8+1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameTechRenderer::FindTrianglesToPaint()
{
	struct
	{
		Matrix4 modelMatrix;
		Vector4 sphere;
		uint32_t totalTriangleCount = 0;
		uint32_t writtenTriangleCount = 0;
		uint32_t padding0 = 1;
		uint32_t padding1 = 1;
	} collUniforms;

	struct
	{
		Matrix4 modelMatrix;
		Vector4 sphere;
		uint32_t teamID;
	} maskUniforms;

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 17, "Update Paint Mask");

	for (PaintSpot spot : paintedSpots)
	{
		RenderObject* renderObject = spot.object->GetRenderObject();
		if (!renderObject->isPaintable) continue;

		OGLTexture* maskTexture = (OGLTexture*)renderObject->maskTex;
		OGLMesh* mesh = (OGLMesh*)renderObject->GetMesh();

		uint32_t triangleCount = mesh->GetIndexCount() / 3;

		collUniforms = {};
		collUniforms.modelMatrix = spot.object->GetTransform().GetMatrix();
		collUniforms.sphere = Vector4(spot.position.x, spot.position.y, spot.position.z, spot.radius);
		collUniforms.totalTriangleCount = triangleCount;
		collUniforms.writtenTriangleCount = 0;

		maskUniforms = {};
		maskUniforms.modelMatrix = collUniforms.modelMatrix;
		maskUniforms.sphere = collUniforms.sphere;
		maskUniforms.teamID = spot.teamID;

		/* Find all overlapping triangles */
		paintCollisionShader->Bind();

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, paintCollideUniforms);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(collUniforms), &collUniforms, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, paintCollideTriangleIds);
		glBufferData(GL_SHADER_STORAGE_BUFFER, triangleCount * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, paintCollideUniforms);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, paintCollideTriangleIds);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mesh->attributeBuffers[VertexAttribute::Positions]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mesh->indexBuffer);

		paintCollisionShader->Execute(triangleCount / 64 + 1);

		glMemoryBarrier(GL_UNIFORM_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);

		/* Find all overlapping triangles */
		maskRasterizerShader->Bind();

		glBindBuffer(GL_UNIFORM_BUFFER, maskRasterUniforms);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(maskUniforms), &maskUniforms, GL_STATIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, maskRasterUniforms);
		glBindImageTexture(1, maskTexture->GetObjectID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mesh->attributeBuffers[VertexAttribute::Positions]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mesh->attributeBuffers[VertexAttribute::TextureCoords]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mesh->indexBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, paintCollideTriangleIds);

		glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, paintCollideUniforms);
		glDispatchComputeIndirect(offsetof(decltype(collUniforms), writtenTriangleCount));

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	glPopDebugGroup();
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

		GeometryPrimitive prevPrimitive = i->GetMesh()->GetPrimitiveType();
		i->GetMesh()->SetPrimitiveType(GeometryPrimitive::Triangles);
		Transform* tempTransform = (*i).GetTransform();
		Transform newTransform;
		memcpy(&newTransform, tempTransform,sizeof(Transform));
		Vector3 lightDir = (lightPosition - newTransform.GetPosition()).Normalised();
		newTransform.SetPosition(newTransform.GetPosition() - lightDir);
		Matrix4 modelMatrix = newTransform.GetMatrix();

		if (i->isAnimated) {
			glUniform1i(glGetUniformLocation(shadowShader->GetProgramID(), "isAnimated"), true);
			int j = glGetUniformLocation(((OGLShader*)i->GetShader())->GetProgramID(), "joints");
			glUniformMatrix4fv(j, (GLsizei)i->frameMatrices.size(), false, (float*)i->frameMatrices.data());
		}
		else {
			glUniform1i(glGetUniformLocation(shadowShader->GetProgramID(), "isAnimated"), false);
		}


		Matrix4 mvpMatrix	= mvMatrix * modelMatrix;
		glUniformMatrix4fv(mvpLocation, 1, false, (float*)&mvpMatrix);
		BindMesh((*i).GetMesh());
		int layerCount = (*i).GetMesh()->GetSubMeshCount();
		for (int i = 0; i < layerCount; ++i) {
			DrawBoundMesh(i);
		}
		i->GetMesh()->SetPrimitiveType(prevPrimitive);//dont forget this
	}

	glViewport(0, 0, renderWidth, renderWidth);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
	glPopDebugGroup();
}

void GameTechRenderer::RenderCamera() {
	float screenAspect = (float)renderWidth / (float)renderHeight;
	Matrix4 viewMatrix = activeCamera->BuildViewMatrix();
	Matrix4 projMatrix = activeCamera->BuildProjectionMatrix(screenAspect);

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
		if (i->onlyForShadows) continue;

		OGLShader* shader = (OGLShader*)(*i).GetShader();
		BindShader(shader);

		if (activeShader != shader) {
			projLocation = glGetUniformLocation(shader->GetProgramID(), "projMatrix");
			viewLocation = glGetUniformLocation(shader->GetProgramID(), "viewMatrix");
			modelLocation = glGetUniformLocation(shader->GetProgramID(), "modelMatrix");
			shadowLocation = glGetUniformLocation(shader->GetProgramID(), "shadowMatrix");
			colourLocation = glGetUniformLocation(shader->GetProgramID(), "objectColour");
			hasVColLocation = glGetUniformLocation(shader->GetProgramID(), "hasVertexColours");
			hasTexLocation = glGetUniformLocation(shader->GetProgramID(), "hasTexture");
			scaleLocation = glGetUniformLocation(shader->GetProgramID(), "scale");
			useHeightMapLocalLocation = glGetUniformLocation(shader->GetProgramID(), "useHeightMapLocal");

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

			Vector3 camPos = activeCamera->GetPosition();
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
		glUniform1i(widthLocation, (int)maskDims.x);
		glUniform1i(heightLocation, (int)maskDims.y);

		glUniform1f(noiseScaleLocation, noiseScale);
		glUniform1f(noiseOffsetSizeLocation, noiseOffsetSize / 1000.0f * i->GetTransform()->GetScale().Length());
		glUniform1f(noiseNormalStrengthLocation, noiseNormalStrength);
		glUniform1f(noisenormalNoiseMultLocation, noiseNormalNoiseMult);

		glUniform1f(timePassedLocation, timePassed);
		glUniform1f(timeScaleLocation, noiseTimeScale);

		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "toneMap"), toneMap);
		glUniform1f(glGetUniformLocation(shader->GetProgramID(), "exposure"), exposure);

		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "useTriplanarMapping"), i->useTriplanarMapping);

		//glActiveTexture(GL_TEXTURE0);
		//BindTextureToShader((OGLTexture*)(*i).GetDefaultTexture(), "mainTex", 0);

		if (i->isPaintable) {
			glUniform1f(heightMapStrengthLocation, pbrSettings.heightMapStrength);
			glUniform1i(useBumpMapLocation, pbrSettings.useBumpMap);
			glUniform1i(useMetallicMapLocation, pbrSettings.useMetallicMap);
			glUniform1i(useRoughnessMapLocation, pbrSettings.useRoughnessMap);
			glUniform1i(useHeightMapLocation, pbrSettings.useHeightMap);
			glUniform1i(useEmissionMapLocation, pbrSettings.useEmissionMap);
			glUniform1i(useAOMapLocation, pbrSettings.useAOMap);
			glUniform1i(useOpacityMapLocation, pbrSettings.useOpacityMap);
			glUniform1i(useGlossMapLocation, pbrSettings.useGlossMap);

			glUniform1i(useHeightMapLocalLocation, i->useHeightMap);

			glBindImageTexture(0, ((OGLTexture*)i->maskTex)->GetObjectID(), 0, GL_FALSE, NULL, GL_READ_ONLY, GL_R8UI);

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

		if (i->isAnimated) {
			int j = glGetUniformLocation(shader->GetProgramID(), "joints");
			glUniformMatrix4fv(j, (GLsizei)i->frameMatrices.size(), false, (float*)i->frameMatrices.data());
		}
//		glDisable(GL_CULL_FACE);//todo turn back on
		BindMesh((*i).GetMesh());
		int layerCount = (*i).GetMesh()->GetSubMeshCount();
		for (int x = 0; x < layerCount; ++x) {
			if (i->isAnimated) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)i->matTextures[x])->GetObjectID());
				glUniform1i(glGetUniformLocation(shader->GetProgramID(), "diffuseTex"), 0);
			}
			DrawBoundMesh(x);
		}
	}
	glEnable(GL_BLEND);
	glPopDebugGroup();
}

void GameTechRenderer::RenderSkybox() {
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	float screenAspect = (float)renderWidth / (float)renderHeight;
	Matrix4 viewMatrix = activeCamera->BuildViewMatrix();
	Matrix4 projMatrix = activeCamera->BuildProjectionMatrix(screenAspect);

	BindShader(skyboxShader);

	int projLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "viewMatrix");
	int texLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "cubeTex");

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

void GameTechRenderer::RenderFullScreenQuadWithTexture(GLuint texture) {
	glDepthMask(false);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE); //todo reverse winding order
	BindShader(quad->GetShader());

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(((OGLShader*)quad->GetShader())->GetProgramID(), "mainTex"), 0);
	glUniform1i(glGetUniformLocation(((OGLShader*)quad->GetShader())->GetProgramID(), "hasTexture"), true);

	glBindTexture(GL_TEXTURE_2D, texture);
	BindMesh(quad->GetMesh());
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDepthMask(true);
}

void GameTechRenderer::RenderFXAAPass() {
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 4, "fxaa");

	//Run edge detection
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 4, "edge");
	glBindFramebuffer(GL_FRAMEBUFFER, edgesFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(edgesShader);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(edgesShader->GetProgramID(), "depthTex"), 0);
	glUniform1i(glGetUniformLocation(edgesShader->GetProgramID(), "width"), renderWidth);
	glUniform1i(glGetUniformLocation(edgesShader->GetProgramID(), "height"), renderHeight);
	glBindTexture(GL_TEXTURE_2D, sceneColor);

	glUniform1f(glGetUniformLocation(edgesShader->GetProgramID(), "SMAA_THRESHOLD"), fxaaEdgeThreshold);
	BindMesh(quad->GetMesh());
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopDebugGroup();

	//Run FXAA
	BindShader(fxaaShader);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(fxaaShader->GetProgramID(), "mainTex"), 0);
	glBindTexture(GL_TEXTURE_2D, sceneColor);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(fxaaShader->GetProgramID(), "edgesTex"), 1);
	glBindTexture(GL_TEXTURE_2D, edgesTex);

	glUniform1i(glGetUniformLocation(fxaaShader->GetProgramID(), "width"), renderWidth);
	glUniform1i(glGetUniformLocation(fxaaShader->GetProgramID(), "height"), renderHeight);

	glUniform1i(glGetUniformLocation(fxaaShader->GetProgramID(), "edgeDetection"), fxaaEdgeDetection);

	BindMesh(quad->GetMesh());

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopDebugGroup();
}