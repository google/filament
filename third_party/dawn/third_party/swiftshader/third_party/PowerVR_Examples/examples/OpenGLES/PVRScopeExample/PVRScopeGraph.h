/*!*********************************************************************************************************************
\File         PVRScopeGraph.h
\Title
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description
***********************************************************************************************************************/
#pragma once

#include "PVRScopeStats.h"
#include "PVRUtils/OpenGLES/BindingsGles.h"
#include "PVRUtils/PVRUtilsGles.h"

struct PVRGraphCounter
{
	std::vector<float> valueCB; // Circular buffer of counter values
	uint32_t writePosCB; // Current write position of circular buffer
	bool showGraph; // Show the graph
	uint32_t colorLutIdx; // colour lookup table index
	float maximum;

	PVRGraphCounter() : writePosCB(0), showGraph(), maximum(0.0f) {}
};

namespace Configuration {
enum
{
	VertexArrayBinding = 0,
	NumVerticesGraphBorder = 6,
	MaxSwapChains = 8
};
}

class PVRScopeGraph
{
protected:
	std::vector<glm::vec2> verticesGraphContent;
	glm::vec2 verticesGraphBorder[Configuration::NumVerticesGraphBorder];

	SPVRScopeCounterReading reading;

	uint32_t numCounter;
	SPVRScopeImplData* scopeData;
	SPVRScopeCounterDef* counters;
	uint32_t activeGroup; // most recent group seen
	uint32_t activeGroupSelect; // users desired group
	bool isActiveGroupChanged;

	uint32_t sizeCB;

	struct ActiveCounter
	{
		GLuint vbo;
		uint32_t bufferSize;
		pvr::ui::Text legendLabel;
		pvr::ui::Text legendValue;
		ActiveCounter() : vbo(0), bufferSize(0) {}
	};

	std::vector<PVRGraphCounter> graphCounters;
	std::vector<ActiveCounter> activeCounters;
	std::vector<uint32_t> activeCounterIds;

	float x, y, pixelW, graphH;

	uint32_t updateInterval, updateIntervalCounter;

	uint32_t idxFPS;
	uint32_t idx2D;
	uint32_t idx3D;
	uint32_t idxTA;
	uint32_t idxCompute;
	uint32_t idxShaderPixel;
	uint32_t idxShaderVertex;
	uint32_t idxShaderCompute;

	GLuint _program;
	GLuint _vertexBufferGraphBorder;
	GLuint _indexBuffer;
	pvr::ui::UIRenderer* _uiRenderer;
	pvr::IAssetProvider* _assetProvider;
	uint32_t _esShaderColorId;
	bool _isInitialzed;

public:
	PVRScopeGraph();
	~PVRScopeGraph();

	// Disallow copying
	PVRScopeGraph(const PVRScopeGraph&); // deleted
	PVRScopeGraph& operator=(const PVRScopeGraph&); // deleted
	void executeCommands();
	void executeUICommands();
	void ping(float dt_millis);
	void showCounter(uint32_t nCounter, bool bShow);
	bool isCounterShown(const uint32_t nCounter) const;
	bool isCounterBeingDrawn(uint32_t nCounter) const;
	bool isCounterPercentage(const uint32_t nCounter) const;
	bool setActiveGroup(const uint32_t nActiveGroup);
	uint32_t getActiveGroup() const { return activeGroup; }
	float getMaximumOfData(uint32_t nCounter);
	float getMaximum(uint32_t nCounter);
	void setMaximum(uint32_t nCounter, float fMaximum);
	float getStandardFPS() const;
	int32_t getStandardFPSIndex() const;
	float getStandard2D() const;
	int32_t getStandard2DIndex() const;
	float getStandard3D() const;
	int32_t getStandard3DIndex() const;
	float getStandardTA() const;
	int32_t getStandardTAIndex() const;
	float getStandardCompute() const;
	int32_t getStandardComputeIndex() const;
	float getStandardShaderPixel() const;
	int32_t getStandardShaderPixelIndex() const;
	float getStandardShaderVertex() const;
	int32_t getStandardShaderVertexIndex() const;
	float getStandardShaderCompute() const;
	int32_t getStandardShaderComputeIndex() const;
	uint32_t getCounterNum() const { return numCounter; }
	const char* getCounterName(const uint32_t i) const;
	int getCounterGroup(const uint32_t i) const;

	void position(const uint32_t nViewportW, const uint32_t nViewportH, pvr::Rectanglei const& graph);

	void setUpdateInterval(const uint32_t nUpdateInverval);
	bool isInitialized() const { return _isInitialzed; }

	bool init(pvr::EglContext& context, pvr::IAssetProvider& assetProvider, pvr::ui::UIRenderer& uiRenderer);

protected:
	void updateCounters();
	void updateBufferLines();
	void update(float dt_millis);
	bool createProgram(pvr::EglContext& context);
	void setGlCommonStates();
};
