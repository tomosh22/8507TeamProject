/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "MeshGeometry.h"
#include "glad\gl.h"

#include <string>

namespace NCL {
	namespace Rendering {
		class OGLMesh : public NCL::MeshGeometry
		{
		public:
			friend class OGLRenderer;
			OGLMesh();
			OGLMesh(const std::string&filename);
			~OGLMesh();

			void RecalculateNormals();

			void UploadToGPU(Rendering::RendererBase* renderer = nullptr) override;
			void UpdateGPUBuffers(unsigned int startVertex, unsigned int vertexCount);

			static OGLMesh* GenerateQuadWithIndices();
			static OGLMesh* GenerateTriangleWithIndices();
			static OGLMesh* GenerateCrossHair();

			GLuint vao;
			GLuint oglType;
			GLuint attributeBuffers[VertexAttribute::MAX_ATTRIBUTES];
			GLuint indexBuffer;

		protected:
			GLuint	GetVAO()			const { return vao;};
			void BindVertexAttribute(int attribSlot, int bufferID, int bindingID, int elementCount, int elementSize, int elementOffset);

			int		subCount;

			
		};
	}
}