#pragma once

#include "OGLRenderer.h"
#include "OGLShader.h"
#include "OGLTexture.h"
#include "OGLMesh.h"
#include "OGLComputeShader.h"

#include "GameWorld.h"

//this was me
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "AreaTex.h"
#include "SearchTex.h"

//animation
#include "MeshAnimation.h"
#include "MeshMaterial.h"

namespace NCL::CSC8503 {

	class RenderObject;

	class GameTechRenderer : public OGLRenderer
	{
	public:
		struct RayMarchedSphere
		{
			Vector3 position;
			Vector3 color;
			float radius;
		};
	private:
		OGLShader* quadShader;
		RenderObject* quad;

		int renderWidth;
		int renderHeight;

		//Ray marching stuff
		GLuint rayMarchSphereSSBO;
		GLuint rayMarchTexture;
		OGLComputeShader* rayMarchComputeShader;

		std::vector<RayMarchedSphere> marchedSpheresToDraw;
	public:
		struct RayMarchingSettings
		{
			bool enabled = true;
			int maxSteps;
			float hitDistance;
			float noHitDistance;
			float debugValue;
			bool rayMarchDepthTest;
		} rayMarchingSettings;

		struct DebugData
		{
			vector<Vector3> debugLineData;

			vector<Vector3> debugTextPos;
			vector<Vector4> debugTextColours;
			vector<Vector2> debugTextUVs;

			GLuint lineVAO;
			GLuint lineVertVBO;
			size_t lineCount;

			GLuint textVAO;
			GLuint textVertVBO;
			GLuint textColourVBO;
			GLuint textTexVBO;
			size_t textCount;
		} debugData;
	private:
		void CreateViewDependent();
		void DestroyViewDependent();

		void InitializeImGui();

		void LoadSkybox();
		void UnloadSkybox();

		void CreateShadowFbo();
		void DestroyShadowFbo();

		void CreateDebugData();
		void DestroyDebugData();

		void NewRenderLines();
		void NewRenderText();
		void SetDebugStringBufferSizes(size_t newVertCount);
		void SetDebugLineBufferSizes(size_t newVertCount);

		void SendRayMarchData();
		void ExecuteRayMarching();
	public:
		GameTechRenderer(Window* window);
		~GameTechRenderer();

		MeshGeometry* LoadMesh(const string& name, std::vector<MeshGeometry*>* meshes = nullptr);
		TextureBase* LoadTexture(const string& name);
		ShaderBase* LoadShader(const string& vertex, const string& fragment);
		ShaderBase* LoadShader(const string& vertex, const string& fragment, const string& domain, const string& hull);

		void BeginImGui();
		void EndImGui();

		void OnWindowResize(int w, int h) override;
		void BeginFrame() override;
		void RenderFrame() override;
		void EndFrame() override;

		inline void SubmitRayMarchedSphere(const RayMarchedSphere& sphere) { marchedSpheresToDraw.push_back(sphere); }
		inline void SubmitRenderObject(const RenderObject* renderObj) { activeObjects.push_back(renderObj); }

		inline void SetActiveCamera(Camera* camera) { this->activeCamera = camera; }

		/********************************** Seal of approval **********************************/

		//this was me
		RenderObject* crosshair;

		float noiseScale = 1.8f;
		float noiseOffsetSize = 0.009f;
		float noiseNormalStrength = 10.0f;
		float noiseNormalNoiseMult = 0.313f;

		bool newMethod = true;

		bool renderFullScreenQuad = true;

		Vector4		lightColour;
		float		lightRadius;
		Vector3		lightPosition;

		float heightMapStrength = 1;
		bool useBumpMap = true;
		bool useMetallicMap = true;
		bool useRoughnessMap = true;
		bool useHeightMap = true;
		bool useEmissionMap = true;
		bool useAOMap = true;
		bool useOpacityMap = true;
		bool useGlossMap = true;

		float timePassed = 0.0f;
		float timeScale = 0.419f;

		OGLShader* debugShader;

		bool drawCrosshair = false;

		GLuint sceneFBO;
		GLuint sceneColor;
		GLuint sceneDepth;

		void CreateFBOColorDepth(GLuint& fbo, GLuint& colorTex, GLuint& depthTex, GLenum colorFormat = GL_RGBA8);
		void CreateFBOColor(GLuint& fbo, GLuint& colorTex, GLenum colorFormat = GL_RGBA8);

		GLuint edgesFBO;
		GLuint edgesTex;
		OGLShader* edgesShader;

		GLuint weightCalcFBO;
		GLuint blendTex;
		GLuint areaTex;
		GLuint searchTex;
		OGLShader* weightCalcShader;

		void EdgeDetection();
		bool renderEdges = false;
		float smaaThreshold = 0.05f;

		GLuint blendingFBO;
		void WeightCalculation();
		bool renderBlend = true;

		GLuint neighborhoodBlendingFBO;
		OGLShader* neighborhoodBlendingShader;
		GLuint smaaOutput;
		void NeighborhoodBlending();
		bool renderAA = true;

		void SMAA();

		GLuint fxaaFBO;
		OGLShader* fxaaShader;
		void FXAA();
		bool useFXAA = true;
		bool edgeDetection = true;

		GLuint hdrFBO;
		OGLShader* hdrShader;
		GLuint tonemappedTexture;
		bool toneMap = true;
		float exposure = 1;
	protected:

		void RenderShadowMap();
		void RenderCamera(); 
		void RenderSkybox();

		//this was me
		void RenderFullScreenQuadWithTexture(GLuint texture);


		std::vector<const RenderObject*> activeObjects;
		std::vector<RenderObject*> animatedObjects;

		Camera* activeCamera = nullptr;

		OGLShader* defaultShader;
		OGLShader*  skyboxShader;
		OGLMesh*	skyboxMesh;
		GLuint		skyboxTex;

		//shadow mapping things
		OGLShader*	shadowShader;
		GLuint		shadowTex;
		GLuint		shadowFBO;
		Matrix4     shadowMatrix;

		void DrawCrossHair();
	};

}