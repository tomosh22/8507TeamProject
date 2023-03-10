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

			float noiseScale= 0.38f;
			float noiseOffsetSize = 0.002f;
			float noiseNormalStrength= 0.6;
			float noiseNormalNoiseMult = 1.27;

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
			float timeScale = 0.5;

			OGLShader* debugShader;
			OGLShader* quadShader;

			bool drawCrosshair = false;

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
			void RenderFullScreenQuad();
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

