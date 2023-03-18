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
#include"MeshAnimation.h"
#include"MeshMaterial.h"

namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
	namespace CSC8503 {
		class RenderObject;

		class GameTechRenderer : public OGLRenderer	{
		public:
			GameTechRenderer(GameWorld& world);
			~GameTechRenderer();

			MeshGeometry*	LoadMesh(const string& name, std::vector<MeshGeometry*>* meshes = nullptr);
			TextureBase*	LoadTexture(const string& name);
			ShaderBase*		LoadShader(const string& vertex, const string& fragment);
			ShaderBase*		LoadShader(const string& vertex, const string& fragment, const string& domain, const string& hull);

			//this was me
			RenderObject* quad;
			OGLTexture* rayMarchTexture;
			RenderObject* crosshair;

			struct ImGUIPtrs {
				int* rayMarchMaxSteps;
				float* rayMarchHitDistance;
				float* rayMarchNoHitDistance;
				float* debugValue;
				bool* depthTest;
				Vector3* testSphereCenter;
				float* testSphereRadius;
				//int* currentTeamInt;
				bool* newMethod;
				bool* rayMarchBool;

				
			};
			ImGUIPtrs imguiptrs;

			float noiseScale = 1.8;
			float noiseOffsetSize = 0.009;
			float noiseNormalStrength = 10;
			float noiseNormalNoiseMult = 0.313;

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

			float timePassed = 0;
			float timeScale = 0.419;

			OGLShader* debugShader;
			OGLShader* quadShader;

			bool drawCrosshair = false;

			GLuint sceneFBO;
			GLuint sceneColor;
			GLuint sceneDepth;

			void CreateFBOColorDepth(GLuint& fbo, GLuint& colorTex, GLuint& depthTex);
			void CreateFBOColor(GLuint& fbo, GLuint& colorTex);

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
			float smaaThreshold = 0.05;

			GLuint blendingFBO;
			void WeightCalculation();
			bool renderBlend = true;

			GLuint neighborhoodBlendingFBO;
			OGLShader* neighborhoodBlendingShader;
			GLuint smaaOutput;
			void NeighborhoodBlending();
			bool renderAA = true;

			void SMAA();

			OGLShader* fxaaShader;
			void FXAA();
			bool useFXAA = true;
			bool edgeDetection = true;


			OGLShader* characterShader;

			void LoadPlayerAniamtion();
			void RenderPlayerAnimation();
			OGLMesh* playerMesh;

			NCL::MeshAnimation* playerIdle;
			NCL::MeshAnimation* currentAniamtion;
			MeshMaterial* playerMaterial;
			vector<OGLTexture*> matTextures;
			void SetCurrentAniamtion(NCL::MeshAnimation* anim)
			{
				if (anim != currentAniamtion)
				{
					currentFrame = 0;
					frameTime = 0.0f;
				}
				currentAniamtion = anim;
			}
			int currentFrame;
			float frameTime;
		protected:
			void NewRenderLines();
			void NewRenderText();

			void RenderFrame()	override;

			OGLShader*		defaultShader;

			GameWorld&	gameWorld;

			void BuildObjectList();
			void SortObjectList();
			void RenderShadowMap();
			void RenderCamera(); 
			void RenderSkybox();

			//this was me
			void RenderFullScreenQuadWithTexture(GLuint texture, bool rotated = false);
			void ImGui();

			void LoadSkybox();

			void SetDebugStringBufferSizes(size_t newVertCount);
			void SetDebugLineBufferSizes(size_t newVertCount);

			vector<const RenderObject*> activeObjects;
			


			
			OGLShader*  skyboxShader;
			OGLMesh*	skyboxMesh;
			GLuint		skyboxTex;

			//shadow mapping things
			OGLShader*	shadowShader;
			GLuint		shadowTex;
			GLuint		shadowFBO;
			Matrix4     shadowMatrix;

			

			//Debug data storage things
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

			void DrawCrossHair();

			

		};
	}
}

