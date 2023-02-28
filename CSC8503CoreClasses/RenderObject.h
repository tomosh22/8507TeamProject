#pragma once
#include "TextureBase.h"
#include "ShaderBase.h"
#include <array>
#define MAX_TRIS 10000

namespace NCL {
	using namespace NCL::Rendering;

	class MeshGeometry;
	namespace CSC8503 {
		class Transform;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(Transform* parentTransform, MeshGeometry* mesh, TextureBase* tex, ShaderBase* shader);
			~RenderObject();

			void SetDefaultTexture(TextureBase* t) {
				texture = t;
			}

			TextureBase* GetDefaultTexture() const {
				return texture;
			}

			MeshGeometry*	GetMesh() const {
				return mesh;
			}

			Transform*		GetTransform() const {
				return transform;
			}

			ShaderBase*		GetShader() const {
				return shader;
			}

			void SetColour(const Vector4& c) {
				colour = c;
			}

			Vector4 GetColour() const {
				return colour;
			}

			//this was me
			//unsigned int texID;

			TextureBase* triDataTex = nullptr;
			TextureBase* maskTex;
			TextureBase* baseTex;
			TextureBase* bumpTex;
			bool isPaintable = false;

		protected:
			MeshGeometry*	mesh;
			TextureBase*	texture;
			ShaderBase*		shader;
			Transform*		transform;
			Vector4			colour;

			
			
		};
	}
}
