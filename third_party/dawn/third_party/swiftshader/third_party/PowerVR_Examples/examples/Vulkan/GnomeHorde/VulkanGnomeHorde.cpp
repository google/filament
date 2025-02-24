/*!
\brief Multi-threaded command buffer generation. Requires the PVRShell.
\file VulkanGnomeHorde.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"
#include "PVRCore/Threading.h"

/*
THE GNOME HORDE - MULTITHREADED RENDERING ON THE VULKAN API USING THE POWERVR
FRAMEWORK.

This examples shows a very efficient multithreaded rendering design using queues
for abstracted inter-thread communication.

The domain of the problem (the "game" world) is divided into a tile grid.
* Each tile has several objects, and will have its own SecondaryCommandBuffer for rendering.
* All tiles that are visible will be gathered and submitted into a primary command
buffer
* Every frame, several threads check tiles for visibility. Since all tiles need to
be checked anyway, this task is subdivided into large chunks of the game world
("Lines" of tiles). This initial work is put into a Producer-Consumer queue.
* If a tile is found to have just became visible, or had its level of detail
changed, it needs to have its command buffer (re?)generated, hence it is entered
into a "Tiles to process" queue and the thread moves to check the next one
* If a tile is found to be visible without change, it is put directly into a
"tiles to Draw" queue thread (bypassing processing entirely)
* Otherwise, it is ignored
* Another group of threads pull items from the "tiles to process" threads and for
each of them generate the command buffers, and enter them into the "tiles to draw"
* The main thread pulls the command buffers and draws them.
*/
const glm::vec4 DirectionToLight = glm::vec4(0.0f, 1.0f, 0.65f, 0.0f);

enum CONSTANTS
{
	MAX_NUMBER_OF_SWAP_IMAGES = 4,
	MAX_NUMBER_OF_THREADS = 16u,
	TILE_SIZE_X = 150,
	TILE_GAP_X = 20,
	TILE_SIZE_Y = 100,
	TILE_SIZE_Z = 150,
	TILE_GAP_Z = 20,
	NUM_TILES_X = 50,
	NUM_TILES_Z = 50,
	NUM_OBJECTS_PER_TILE = 9,
	NUM_UNIQUE_OBJECTS_PER_TILE = 5,
	TOTAL_NUMBER_OF_OBJECTS = NUM_TILES_X * NUM_TILES_Z * NUM_OBJECTS_PER_TILE,
	MAX_GAME_TIME = 10000000
};

// Application logic
struct AppModeParameter
{
	float speedFactor;
	float cameraHeightOffset;
	float cameraForwardOffset;
	float duration;
};

const std::array<AppModeParameter, 4> DemoModes = { { { 2.5f, 100.0f, 5.0f, 10.0f }, { 2.5f, 500.0f, 10.0f, 10.0f }, { 2.5f, 1000.0f, 20.0f, 10.0f }, { 15.0f, 1000.0f, 20.0f, 10.0f } } };

struct TileProcessingResult
{
	int itemsDiscarded;
	glm::ivec2 itemToDraw;
	TileProcessingResult() : itemsDiscarded(0), itemToDraw(-1, -1) {}
	void reset()
	{
		itemToDraw = glm::ivec2(-1, -1);
		itemsDiscarded = 0;
	}
};

class VulkanGnomeHorde;

/// <summary>This queue is to enqueue tasks used for the "determine visibility" producer queues
/// There, our "task" granularity is a "line" of tiles to process.</summary>
typedef pvr::LockedQueue<int32_t> LineTasksQueue;

/// <summary>This queue is used to create command buffers, so its task granularity is a tile.
/// It is Used for the "create command buffers for tile XXX" queues.</summary>
typedef pvr::LockedQueue<TileProcessingResult> TileResultsQueue;

class GnomeHordeWorkerThread
{
public:
	GnomeHordeWorkerThread() : id(static_cast<uint32_t>(-1)), running(false) {}
	std::string myType;
	std::thread thread;
	VulkanGnomeHorde* app;
	volatile uint32_t id;
	volatile bool running;
	void addlog(const std::string& str);
	void run();
	virtual bool doWork() = 0;
};

class GnomeHordeTileThreadData : public GnomeHordeWorkerThread
{
public:
	struct ThreadApiObjects
	{
		std::vector<pvrvk::CommandPool> commandPools;
		std::mutex poolMutex;
		TileResultsQueue::ConsumerToken processQConsumerToken;
		TileResultsQueue::ProducerToken drawQProducerToken;
		uint32_t lastSwapIndex;
		std::array<std::vector<pvrvk::SecondaryCommandBuffer>, MAX_NUMBER_OF_SWAP_IMAGES> preFreeCmdBuffers;
		std::array<std::vector<pvrvk::SecondaryCommandBuffer>, MAX_NUMBER_OF_SWAP_IMAGES> freeCmdBuffers;
		ThreadApiObjects(TileResultsQueue& processQ, TileResultsQueue& drawQ)
			: processQConsumerToken(processQ.getConsumerToken()), drawQProducerToken(drawQ.getProducerToken()), lastSwapIndex(-1)
		{}
	};
	GnomeHordeTileThreadData() { myType = "Tile Thread"; }
	std::unique_ptr<ThreadApiObjects> threadApiObj;

	bool doWork();

	pvrvk::SecondaryCommandBuffer getFreeCommandBuffer(uint32_t _swapchainIndex);

	void garbageCollectPreviousFrameFreeCommandBuffers(uint32_t _swapchainIndex);

	void freeCommandBuffer(const pvrvk::SecondaryCommandBuffer& commandBuff, uint32_t _swapchainIndex);

	void generateTileBuffer(const TileProcessingResult* tiles, uint32_t numTiles);
};

class GnomeHordeVisibilityThreadData : public GnomeHordeWorkerThread
{
public:
	struct DeviceResources
	{
		LineTasksQueue::ConsumerToken linesQConsumerToken;
		TileResultsQueue::ProducerToken processQproducerToken;
		TileResultsQueue::ProducerToken drawQproducerToken;
		DeviceResources(LineTasksQueue& linesQ, TileResultsQueue& processQ, TileResultsQueue& drawQ)
			: linesQConsumerToken(linesQ.getConsumerToken()), processQproducerToken(processQ.getProducerToken()), drawQproducerToken(drawQ.getProducerToken())
		{}
	};
	GnomeHordeVisibilityThreadData() { myType = "Visibility Thread"; }

	std::unique_ptr<DeviceResources> deviceResources;

	bool doWork();

	void determineLineVisibility(const int32_t* lines, uint32_t numLines);
};

pvr::utils::VertexBindings attributeBindings[] = {
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "UV0", 2 },
};

struct MultiBuffering
{
	pvrvk::CommandBuffer cmdBuffers;
	pvrvk::SecondaryCommandBuffer cmdBufferUI;
	pvrvk::DescriptorSet descSetPerFrame;
};
struct Mesh
{
	pvr::assets::MeshHandle mesh;
	pvrvk::Buffer vbo;
	pvrvk::Buffer ibo;
};
typedef std::vector<Mesh> MeshLod;

struct Meshes
{
	MeshLod gnome;
	MeshLod gnomeShadow;
	MeshLod rock;
	MeshLod fern;
	MeshLod fernShadow;
	MeshLod mushroom;
	MeshLod mushroomShadow;
	MeshLod bigMushroom;
	MeshLod bigMushroomShadow;
	void clearAll()
	{
		clearApiMesh(gnome, true);
		clearApiMesh(gnomeShadow, true);
		clearApiMesh(rock, true);
		clearApiMesh(fern, true);
		clearApiMesh(fernShadow, true);
		clearApiMesh(mushroom, true);
		clearApiMesh(mushroomShadow, true);
		clearApiMesh(bigMushroom, true);
		clearApiMesh(bigMushroomShadow, true);
	}

	void clearApiObjects()
	{
		clearApiMesh(gnome, false);
		clearApiMesh(gnomeShadow, false);
		clearApiMesh(rock, false);
		clearApiMesh(fern, false);
		clearApiMesh(fernShadow, false);
		clearApiMesh(mushroom, false);
		clearApiMesh(mushroomShadow, false);
		clearApiMesh(bigMushroom, false);
		clearApiMesh(bigMushroomShadow, false);
	}
	void createApiObjects(pvrvk::Device& device, pvrvk::CommandBuffer& uploadCmdBuffer, pvr::utils::vma::Allocator& vmaAllocator)
	{
		createApiMesh(gnome, device, uploadCmdBuffer, vmaAllocator);
		createApiMesh(gnomeShadow, device, uploadCmdBuffer, vmaAllocator);
		createApiMesh(rock, device, uploadCmdBuffer, vmaAllocator);
		createApiMesh(fern, device, uploadCmdBuffer, vmaAllocator);
		createApiMesh(fernShadow, device, uploadCmdBuffer, vmaAllocator);
		createApiMesh(mushroom, device, uploadCmdBuffer, vmaAllocator);
		createApiMesh(mushroomShadow, device, uploadCmdBuffer, vmaAllocator);
		createApiMesh(bigMushroom, device, uploadCmdBuffer, vmaAllocator);
		createApiMesh(bigMushroomShadow, device, uploadCmdBuffer, vmaAllocator);
	}

private:
	static void clearApiMesh(MeshLod& mesh, bool deleteAll)
	{
		for (MeshLod::iterator it = mesh.begin(); it != mesh.end(); ++it)
		{
			it->vbo.reset();
			it->ibo.reset();
			if (deleteAll) { it->mesh.reset(); }
		}
	}
	static void createApiMesh(MeshLod& mesh, pvrvk::Device& device, pvrvk::CommandBuffer& uploadCmdBuffer, pvr::utils::vma::Allocator& vmaAllocator)
	{
		for (MeshLod::iterator it = mesh.begin(); it != mesh.end(); ++it)
		{
			bool requiresCommandBufferSubmission = false;
			pvr::utils::createSingleBuffersFromMesh(device, *it->mesh, it->vbo, it->ibo, uploadCmdBuffer, requiresCommandBufferSubmission, vmaAllocator);
		}
	}
};
struct DescriptorSets
{
	pvrvk::DescriptorSet gnome;
	pvrvk::DescriptorSet gnomeShadow;
	pvrvk::DescriptorSet rock;
	pvrvk::DescriptorSet fern;
	pvrvk::DescriptorSet fernShadow;
	pvrvk::DescriptorSet mushroom;
	pvrvk::DescriptorSet mushroomShadow;
	pvrvk::DescriptorSet bigMushroom;
	pvrvk::DescriptorSet bigMushroomShadow;
};

struct Pipelines
{
	pvrvk::GraphicsPipeline solid;
	pvrvk::GraphicsPipeline shadow;
	pvrvk::GraphicsPipeline alphaPremul;
};
struct TileObject
{
	MeshLod* mesh;
	pvrvk::DescriptorSet set;
	pvrvk::GraphicsPipeline pipeline;
};

struct TileInfo
{
	// Per tile info
	std::array<TileObject, NUM_OBJECTS_PER_TILE> objects;
	std::array<std::pair<pvrvk::SecondaryCommandBuffer, std::mutex*>, MAX_NUMBER_OF_SWAP_IMAGES> cbs;
	pvr::math::AxisAlignedBox aabb;
	uint32_t threadId;
	uint8_t lod;
	uint8_t oldLod;
	bool visibility;
	bool oldVisibility;
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::Swapchain swapchain;
	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;
	pvrvk::Queue queue;
	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;
	pvr::Multi<pvrvk::ImageView> depthStencilImages;
	pvr::utils::StructuredBufferView uboPerObjectBufferView;
	pvrvk::Buffer uboPerObject;
	pvrvk::PipelineLayout pipeLayout;

	pvrvk::Sampler trilinear;
	pvrvk::Sampler nonMipmapped;

	pvrvk::DescriptorSet descSetScene;
	pvrvk::DescriptorSet descSetAllObjects;
	DescriptorSets descSets;
	Pipelines pipelines;

	std::array<GnomeHordeTileThreadData, MAX_NUMBER_OF_THREADS> tileThreadData;
	std::array<GnomeHordeVisibilityThreadData, MAX_NUMBER_OF_THREADS> visibilityThreadData;

	std::array<std::array<TileInfo, NUM_TILES_X>, NUM_TILES_Z> tileInfos;
	MultiBuffering multiBuffering[MAX_NUMBER_OF_SWAP_IMAGES];

	pvr::utils::StructuredBufferView uboBufferView;
	pvrvk::Buffer ubo;

	pvr::utils::StructuredBufferView sceneUboBufferView;
	pvrvk::Buffer sceneUbo;

	std::array<std::thread, 16> threads;
	LineTasksQueue::ProducerToken lineQproducerToken;
	TileResultsQueue::ConsumerToken drawQconsumerToken;

	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;

	pvrvk::PipelineCache pipelineCache;

	DeviceResources(LineTasksQueue& lineQ, TileResultsQueue& drawQ) : lineQproducerToken(lineQ.getProducerToken()), drawQconsumerToken(drawQ.getConsumerToken()) {}
	~DeviceResources()
	{
		if (device)
		{
			device->waitIdle();
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
			}
		}
	}
};

class VulkanGnomeHorde : public pvr::Shell
{
public:
	std::deque<std::string> _multiThreadLog;
	std::mutex _logMutex;
	Meshes _meshes;
	std::unique_ptr<DeviceResources> _deviceResources;

	LineTasksQueue _linesToProcessQ;
	TileResultsQueue _tilesToProcessQ;
	TileResultsQueue _tilesToDrawQ;

	uint32_t _allLines[NUM_TILES_Z]; // Stores the line #. Used to kick all work in the visibility threads as each thread will be processing one line

	volatile glm::vec3 _cameraPosition;
	volatile pvr::math::ViewingFrustum _frustum;
	volatile uint32_t _swapchainIndex;
	uint32_t _frameId;
	bool _isPaused;
	uint32_t _numVisibilityThreads;
	uint32_t _numTileThreads;

	// Projection and Model View matrices
	glm::mat4 _projMtx;
	glm::mat4 _viewMtx;

	VulkanGnomeHorde() : _isPaused(false), _numVisibilityThreads(0), _numTileThreads(0)
	{
		for (uint32_t i = 0; i < NUM_TILES_Z; ++i) { _allLines[i] = i; }
	}

	pvr::Result initApplication();
	pvr::Result initView();
	pvr::Result releaseView();
	pvr::Result quitApplication();
	pvr::Result renderFrame();

	void setupUI();

	pvrvk::DescriptorSet createDescriptorSetUtil(const pvrvk::DescriptorSetLayout&, const pvr::StringHash& texture, const pvrvk::Sampler& mipMapped,
		const pvrvk::Sampler& nonMipMapped, pvrvk::CommandBuffer& uploadCmdBuffer);

	//// HELPERS ////
	MeshLod loadLodMesh(const pvr::StringHash& filename, const pvr::StringHash& mesh, uint32_t max_lods);

	AppModeParameter calcAnimationParameters();
	void initUboStructuredObjects();

	void createDescSetsAndTiles(const pvrvk::DescriptorSetLayout& layoutImage, const pvrvk::DescriptorSetLayout& layoutScene, const pvrvk::DescriptorSetLayout& layoutPerObject,
		const pvrvk::DescriptorSetLayout& layoutPerFrameUbo, pvrvk::CommandBuffer& uploadCmdBuffer);

	void updateCameraUbo(const glm::mat4& matrix);

	pvrvk::Device& getDevice() { return _deviceResources->device; }

	pvrvk::Queue& getQueue() { return _deviceResources->queue; }

	void printLog()
	{
		std::unique_lock<std::mutex> lock(_logMutex);
		while (!_multiThreadLog.empty())
		{
			Log(LogLevel::Information, _multiThreadLog.front().c_str());
			_multiThreadLog.pop_front();
		}
	}

	struct DemoDetails
	{
		// Time tracking
		float logicTime; //!< Total time that has elapsed for the application (Conceptual: Clock at start - Clock time now - Paused time)
		float gameTime; //!< Time that has elapsed for the application (Conceptual: Integration of logicTime * the demo's speed factor at each point)
		bool isManual;
		uint32_t currentMode;
		uint32_t previousMode;
		float modeSwitchTime;
		DemoDetails() : logicTime(0), gameTime(0), isManual(false), currentMode(0), previousMode(0), modeSwitchTime(0.f) {}
	} _animDetails;
};

void GnomeHordeWorkerThread::addlog(const std::string& str)
{
	std::unique_lock<std::mutex> lock(app->_logMutex);
	app->_multiThreadLog.push_back(str);
}

void GnomeHordeWorkerThread::run()
{
	addlog(pvr::strings::createFormatted("=== [%s] [%d] ===            Starting", myType.c_str(), id));
	running = true;
	while (doWork()) { continue; } // grabs a piece of work as long as the queue is not empty.
	running = false;
	addlog(pvr::strings::createFormatted("=== [%s] [%d] ===            Exiting", myType.c_str(), id));
}

void GnomeHordeTileThreadData::garbageCollectPreviousFrameFreeCommandBuffers(uint32_t _swapchainIndex)
{
	auto& freeCmd = threadApiObj->freeCmdBuffers[_swapchainIndex];
	auto& prefreeCmd = threadApiObj->preFreeCmdBuffers[_swapchainIndex];

	std::move(prefreeCmd.begin(), prefreeCmd.end(), std::back_inserter(freeCmd));
	prefreeCmd.clear();
	if (freeCmd.size() > 10)
	{
		for (auto it = freeCmd.begin(); it != freeCmd.end(); ++it)
		{
			if (it->use_count() > 1) { return; }
		}
		freeCmd.clear();
	}
}

pvrvk::SecondaryCommandBuffer GnomeHordeTileThreadData::getFreeCommandBuffer(uint32_t _swapchainIndex)
{
	if (threadApiObj->lastSwapIndex != app->_swapchainIndex)
	{
		threadApiObj->lastSwapIndex = app->_swapchainIndex;
		std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
		garbageCollectPreviousFrameFreeCommandBuffers(app->_swapchainIndex);
	}

	pvrvk::SecondaryCommandBuffer retval;
	{
		std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
		if (!threadApiObj->freeCmdBuffers[_swapchainIndex].empty())
		{
			retval = threadApiObj->freeCmdBuffers[_swapchainIndex].back();
			threadApiObj->freeCmdBuffers[_swapchainIndex].pop_back();
		}
	}
	if (!retval)
	{
		if (threadApiObj->commandPools.size() == 0)
		{
			threadApiObj->commandPools.push_back(
				app->getDevice()->createCommandPool(pvrvk::CommandPoolCreateInfo(app->getQueue()->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT)));
			addlog(pvr::strings::createFormatted("Created command pool %llu on thread %llu", threadApiObj->commandPools.back()->getVkHandle(), std::this_thread::get_id()));
		}
		{
			std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
			retval = threadApiObj->commandPools.back()->allocateSecondaryCommandBuffer();
		}
		if (!retval)
		{
			Log(LogLevel::Error,
				"[THREAD %ull] Command buffer allocation failed, "
				". Trying to create additional command buffer pool.",
				id);

			threadApiObj->commandPools.push_back(
				app->getDevice()->createCommandPool(pvrvk::CommandPoolCreateInfo(app->getQueue()->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT)));

			addlog(pvr::strings::createFormatted("Created command pool %d on thread %d", threadApiObj->commandPools.back()->getVkHandle(), std::this_thread::get_id()));
			{
				std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
				retval = threadApiObj->commandPools.back()->allocateSecondaryCommandBuffer();
			}
			if (!retval) { Log(LogLevel::Critical, "COMMAND BUFFER ALLOCATION FAILED ON FRESH COMMAND POOL."); }
		}
	}
	return retval;
}

void GnomeHordeTileThreadData::freeCommandBuffer(const pvrvk::SecondaryCommandBuffer& commandBuff, uint32_t _swapchainIndex)
{
	std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
	if (threadApiObj->lastSwapIndex != app->_swapchainIndex)
	{
		threadApiObj->lastSwapIndex = app->_swapchainIndex;
		garbageCollectPreviousFrameFreeCommandBuffers(app->_swapchainIndex);
	}
	threadApiObj->preFreeCmdBuffers[_swapchainIndex].push_back(commandBuff);
}

///// UTILS (local) /////
inline glm::vec3 getTrackPosition(float time, const glm::vec3& world_size)
{
	const float angle = time * 0.02f;
	const glm::vec3 centre = world_size * 0.5f;
	const glm::vec3 radius = world_size * 0.2f;
	// Main circle
	float a1 = time * 0.07f;
	float a2 = time * 0.1f;
	float a3 = angle;

	const float h = glm::sin(a1) * 15.f + 100.f;
	const float radius_factor = 0.95f + 0.1f * glm::sin(a2);
	const glm::vec3 circle(glm::sin(a3) * radius.x * radius_factor, h, glm::cos(a3) * radius.z * radius_factor);

	return centre + circle;
}

inline void generatePositions(glm::vec3* points, glm::vec3 minBound, glm::vec3 maxBound)
{
	// Jittered Grid - each object is placed on a normal (custom) grid square, and then moved randomly around.
	static const float deviation = .2f;

	static const glm::vec3 normalGridPositions[NUM_UNIQUE_OBJECTS_PER_TILE] = {
		{ .25f, 0.f, .25f },
		{ .25f, 0.f, .75f },
		{ .75f, 0.f, .25f },
		{ .75f, 0.f, .75f },
		{ .50f, 0.f, .50f },
	};

	const int randomObj = rand() % NUM_UNIQUE_OBJECTS_PER_TILE;

	for (uint32_t i = 0; i < NUM_UNIQUE_OBJECTS_PER_TILE; ++i)
	{
		uint32_t obj = (i + randomObj) % NUM_UNIQUE_OBJECTS_PER_TILE;
		glm::vec3 pos = normalGridPositions[obj] + deviation * glm::vec3(pvr::randomrange(-1.f, 1.f), 0.f, pvr::randomrange(-1.f, 1.f));
		points[i] = glm::mix(minBound, maxBound, pos);
	}
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGnomeHorde::initApplication()
{
	uint32_t num_cores = std::thread::hardware_concurrency();
	uint32_t THREAD_FACTOR_RELAXATION = 1;

	uint32_t thread_factor = std::max(num_cores - THREAD_FACTOR_RELAXATION, 1u);

	_numVisibilityThreads = std::min(thread_factor, static_cast<uint32_t>(MAX_NUMBER_OF_THREADS));
	_numTileThreads = std::min(thread_factor, static_cast<uint32_t>(MAX_NUMBER_OF_THREADS));
	Log(LogLevel::Information, "Hardware concurrency reported: %u cores. Enabling %u visibility threads plus %u tile processing threads\n", num_cores, _numVisibilityThreads,
		_numTileThreads);

	_frameId = 0;

	// Meshes
	_meshes.gnome = loadLodMesh(pvr::StringHash("gnome"), "body", 7);
	_meshes.gnomeShadow = loadLodMesh("gnome_shadow", "Plane001", 1);
	_meshes.fern = loadLodMesh("fern", "Plane006", 1);
	_meshes.fernShadow = loadLodMesh("fern_shadow", "Plane001", 1);
	_meshes.mushroom = loadLodMesh("mushroom", "Mushroom1", 2);
	_meshes.mushroomShadow = loadLodMesh("mushroom_shadow", "Plane001", 1);
	_meshes.bigMushroom = loadLodMesh("bigMushroom", "Mushroom1", 1);
	_meshes.bigMushroomShadow = loadLodMesh("bigMushroom_shadow", "Plane001", 1);
	_meshes.rock = loadLodMesh("rocks", "rock5", 1);

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGnomeHorde::quitApplication()
{
	_meshes.clearAll();
	return pvr::Result::Success;
}

void VulkanGnomeHorde::setupUI()
{
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("GnomeHorde");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->setText("Multithreaded command buffer generation and rendering");
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->multiBuffering[i].cmdBufferUI = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		// UIRenderer - the easy stuff first, but we still must create one command buffer per frame.
		_deviceResources->multiBuffering[i].cmdBufferUI->begin(_deviceResources->onScreenFramebuffer[i], 0);
		_deviceResources->uiRenderer.beginRendering(_deviceResources->multiBuffering[i].cmdBufferUI);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->multiBuffering[i].cmdBufferUI->end();
	}
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGnomeHorde::initView()
{
	_deviceResources = std::make_unique<DeviceResources>(_linesToProcessQ, _tilesToDrawQ);

	// Create instance and retrieve compatible physical devices
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName());

	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}

	// Create the surface
	pvrvk::Surface surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	// create the device and the queue
	// look for  a queue that supports graphics, transfer and presentation
	pvr::utils::QueuePopulateInfo queuePopulateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;

	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo);
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	// Create memory allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // create the swapchain

	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	//--------------------
	// Create the command pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	//--------------------
	// Create the DescriptorPool
	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 32)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 32)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 32)
																						  .setMaxDescriptorSets(32));

	setupUI();

	//--------------------
	// create per swapchain resources
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->multiBuffering[i].cmdBuffers = _deviceResources->commandPool->allocateCommandBuffer();
	}

	initUboStructuredObjects();

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// Create Descriptor set layouts
	pvrvk::DescriptorSetLayoutCreateInfo imageDescParam;
	imageDescParam.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

	pvrvk::DescriptorSetLayout descLayoutImage = _deviceResources->device->createDescriptorSetLayout(imageDescParam);

	pvrvk::DescriptorSetLayoutCreateInfo dynamicUboDescParam;
	dynamicUboDescParam.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

	pvrvk::DescriptorSetLayout descLayoutUboDynamic = _deviceResources->device->createDescriptorSetLayout(dynamicUboDescParam);

	pvrvk::DescriptorSetLayoutCreateInfo uboDescParam;
	uboDescParam.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

	pvrvk::DescriptorSetLayout descLayoutUboStatic = _deviceResources->device->createDescriptorSetLayout(uboDescParam);

	pvrvk::DescriptorSetLayoutCreateInfo sceneUboDescParam;
	sceneUboDescParam.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

	pvrvk::DescriptorSetLayout descLayoutUboScene = _deviceResources->device->createDescriptorSetLayout(sceneUboDescParam);

	// Create Pipelines
	{
		_deviceResources->pipeLayout = _deviceResources->device->createPipelineLayout(
			pvrvk::PipelineLayoutCreateInfo().setDescSetLayout(0, descLayoutImage).setDescSetLayout(1, descLayoutUboScene).setDescSetLayout(2, descLayoutUboDynamic).setDescSetLayout(3, descLayoutUboStatic));

		// Must not assume the cache will always work

		pvrvk::ShaderModule objectVsh = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("Object.vsh.spv")->readToEnd<uint32_t>()));
		pvrvk::ShaderModule shadowVsh = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("Shadow.vsh.spv")->readToEnd<uint32_t>()));
		pvrvk::ShaderModule solidFsh = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("Solid.fsh.spv")->readToEnd<uint32_t>()));
		pvrvk::ShaderModule shadowFsh = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("Shadow.fsh.spv")->readToEnd<uint32_t>()));
		pvrvk::ShaderModule premulFsh = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream("Plant.fsh.spv")->readToEnd<uint32_t>()));

		pvrvk::GraphicsPipelineCreateInfo pipeCreate;
		pvrvk::PipelineColorBlendAttachmentState cbStateNoBlend(false);

		pvrvk::PipelineColorBlendAttachmentState cbStateBlend(true, pvrvk::BlendFactor::e_ONE_MINUS_SRC_ALPHA, pvrvk::BlendFactor::e_SRC_ALPHA, pvrvk::BlendOp::e_ADD,
			pvrvk::BlendFactor::e_ZERO, pvrvk::BlendFactor::e_ONE, pvrvk::BlendOp::e_ADD);

		pvrvk::PipelineColorBlendAttachmentState cbStatePremulAlpha(true, pvrvk::BlendFactor::e_ONE, pvrvk::BlendFactor::e_ONE_MINUS_SRC_ALPHA, pvrvk::BlendOp::e_ADD,
			pvrvk::BlendFactor::e_ZERO, pvrvk::BlendFactor::e_ONE, pvrvk::BlendOp::e_ADD);

		pvr::utils::populateInputAssemblyFromMesh(*_meshes.gnome[0].mesh, &attributeBindings[0], 3, pipeCreate.vertexInput, pipeCreate.inputAssembler);

		pipeCreate.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);
		pipeCreate.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
		pipeCreate.depthStencil.enableDepthTest(true);
		pipeCreate.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
		pipeCreate.depthStencil.enableDepthWrite(true);
		pipeCreate.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		pipeCreate.pipelineLayout = _deviceResources->pipeLayout;

		pipeCreate.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(
				0.f, 0.f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()), static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
			pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));

		// create the solid pipeline
		pipeCreate.vertexShader = objectVsh;
		pipeCreate.fragmentShader = solidFsh;
		pipeCreate.colorBlend.setAttachmentState(0, cbStateNoBlend);

		_deviceResources->pipelines.solid = _deviceResources->device->createGraphicsPipeline(pipeCreate, _deviceResources->pipelineCache);

		pipeCreate.depthStencil.enableDepthWrite(false);
		// create the alpha pre-multiply pipeline
		pipeCreate.vertexShader = objectVsh;
		pipeCreate.fragmentShader = premulFsh;
		pipeCreate.colorBlend.setAttachmentState(0, cbStatePremulAlpha);
		_deviceResources->pipelines.alphaPremul = _deviceResources->device->createGraphicsPipeline(pipeCreate, _deviceResources->pipelineCache);

		// create the shadow pipeline
		pipeCreate.colorBlend.setAttachmentState(0, cbStateBlend);
		pipeCreate.vertexShader = shadowVsh;
		pipeCreate.fragmentShader = shadowFsh;

		_deviceResources->pipelines.shadow = _deviceResources->device->createGraphicsPipeline(pipeCreate, _deviceResources->pipelineCache);
	}

	pvrvk::CommandBuffer cb = _deviceResources->commandPool->allocateCommandBuffer();
	cb->begin();

	createDescSetsAndTiles(descLayoutImage, descLayoutUboScene, descLayoutUboDynamic, descLayoutUboStatic, cb);

	//--------------------
	// submit the initial commandbuffer
	cb->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &cb;
	submitInfo.numCommandBuffers = 1;

	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle(); // make sure the queue is finished

	_animDetails.logicTime = 0.f;
	_animDetails.gameTime = 0.f;
	{
		for (uint32_t i = 0; i < _numVisibilityThreads; ++i)
		{
			_deviceResources->visibilityThreadData[i].id = i;
			_deviceResources->visibilityThreadData[i].app = this;
			_deviceResources->visibilityThreadData[i].deviceResources =
				std::make_unique<GnomeHordeVisibilityThreadData::DeviceResources>(_linesToProcessQ, _tilesToProcessQ, _tilesToDrawQ);

			_deviceResources->visibilityThreadData[i].thread =
				std::thread(&GnomeHordeVisibilityThreadData::run, (GnomeHordeVisibilityThreadData*)&_deviceResources->visibilityThreadData[i]);
		}
		for (uint32_t i = 0; i < _numTileThreads; ++i)
		{
			_deviceResources->tileThreadData[i].id = i;
			_deviceResources->tileThreadData[i].app = this;
			_deviceResources->tileThreadData[i].threadApiObj = std::make_unique<GnomeHordeTileThreadData::ThreadApiObjects>(_tilesToProcessQ, _tilesToDrawQ);

			_deviceResources->tileThreadData[i].thread = std::thread(&GnomeHordeTileThreadData::run, (GnomeHordeTileThreadData*)&_deviceResources->tileThreadData[i]);
		}
	}
	printLog();

	_deviceResources->sceneUboBufferView.getElementByName("directionToLight").setValue(glm::normalize(DirectionToLight));

	// Calculates the projection matrix
	bool isRotated = this->isScreenRotated();
	if (isRotated)
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, 1.0f, static_cast<float>(this->getHeight()) / static_cast<float>(this->getWidth()), 10.0f, 5000.f, glm::pi<float>() * 0.5f);
	}
	else
	{
		_projMtx = pvr::math::perspective(pvr::Api::Vulkan, 1.0f, static_cast<float>(this->getWidth()) / static_cast<float>(this->getHeight()), 10.0f, 5000.f);
	}

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGnomeHorde::releaseView()
{
	Log(LogLevel::Information, "Signalling all worker threads: Signal drain empty queues...");
	// Done will allow the queue to finish its work if it has any, but then immediately
	// afterwards it will free any and all threads waiting. Any threads attempting to
	// dequeue work from the queue will immediately return "false".
	_linesToProcessQ.done();
	_tilesToProcessQ.done();
	_tilesToDrawQ.done();

	// waitIdle is being called to make sure the command buffers we will be destroying
	// are not being referenced.
	if (_deviceResources.get() && _deviceResources->device) { _deviceResources->device->waitIdle(); }

	Log(LogLevel::Information, "Joining all worker threads...");

	// Finally, tear down everything.
	for (uint32_t i = 0; i < _numVisibilityThreads; ++i)
	{
		try
		{
			_deviceResources->visibilityThreadData[i].thread.join();
		}
		catch (const std::runtime_error& err) // nothing to do but attempt...
		{
			Log(LogLevel::Error,
				"Runtime error thrown while joining visibility threads. This is expected if the application failed before initialisation was complete. Message: [%s]", err.what());
		}
	}
	for (uint32_t i = 0; i < _numTileThreads; ++i)
	{
		try
		{
			_deviceResources->tileThreadData[i].thread.join();
		}
		catch (const std::runtime_error& err) // nothing to do but attempt...
		{
			Log(LogLevel::Error, "Runtime error thrown while joining tile threads. This is expected if the application failed before initialisation was complete. Message: [%s]",
				err.what());
		}
	}

	// Clear all objects. This will also free the command buffers that were allocated
	// from the worker thread's command pools, but are currently only held by the
	// tiles.
	_meshes.clearApiObjects();
	_deviceResources.reset();
	Log(LogLevel::Information, "All worker threads done!");
	return pvr::Result::Success;
}

/// <summary>Consumes the items in the tiles queue and generates Tile buffers</summary>
/// <returns>Returns true if items were processed correctly and false if queue not or queue was empty.</return>
bool GnomeHordeTileThreadData::doWork()
{
	TileProcessingResult workItem[4];
	int32_t result;
	if ((result = app->_tilesToProcessQ.consume(threadApiObj->processQConsumerToken, workItem[0]))) { generateTileBuffer(workItem, result); }
	return result != 0;
}
/// <summary>Updates resources for the Tile and Re-records the command buffers.</summary>
void GnomeHordeTileThreadData::generateTileBuffer(const TileProcessingResult* tileIdxs, uint32_t numTiles)
{
	// Lambda to create the tiles secondary command buffers
	pvr::utils::StructuredBufferView& uboAllObj = app->_deviceResources->uboPerObjectBufferView;
	const pvrvk::DescriptorSet& descSetAllObj = app->_deviceResources->descSetAllObjects;
	pvr::utils::StructuredBufferView& uboCamera = app->_deviceResources->uboBufferView;

	for (uint32_t tilenum = 0; tilenum < numTiles; ++tilenum)
	{
		const TileProcessingResult& tileInfo = tileIdxs[tilenum];
		glm::ivec2 tileId2d = tileInfo.itemToDraw;
		if (tileId2d != glm::ivec2(-1, -1))
		{
			uint32_t x = tileId2d.x, y = tileId2d.y;
			uint32_t tileIdx = y * NUM_TILES_X + x;

			TileInfo& tile = app->_deviceResources->tileInfos[y][x];

			// Recreate the command buffer
			for (uint32_t swapIdx = 0; swapIdx < app->_deviceResources->swapchain->getSwapchainLength(); ++swapIdx)
			{
				MultiBuffering& multi = app->_deviceResources->multiBuffering[swapIdx];
				tile.cbs[swapIdx].first = getFreeCommandBuffer(swapIdx);
				tile.cbs[swapIdx].second = &threadApiObj->poolMutex;
				tile.threadId = id;

				auto& cb = tile.cbs[swapIdx];
				std::unique_lock<std::mutex> lock(threadApiObj->poolMutex);
				cb.first->begin(app->_deviceResources->onScreenFramebuffer[swapIdx], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

				for (uint32_t objId = 0; objId < NUM_OBJECTS_PER_TILE; ++objId)
				{
					TileObject& obj = tile.objects[objId];
					uint32_t lod = std::min<uint32_t>(static_cast<uint32_t>(obj.mesh->size()) - 1, tile.lod);

					// Can it NOT be different than before? - Not in this demo.
					cb.first->bindPipeline(obj.pipeline);

					Mesh& mesh = (*obj.mesh)[lod];

					uint32_t offset = uboAllObj.getDynamicSliceOffset(tileIdx * NUM_OBJECTS_PER_TILE + objId);
					uint32_t uboCameraOffset = uboCamera.getDynamicSliceOffset(swapIdx);

					// Use the right texture and position - TEXTURES PER OBJECT (Can optimize to object type)
					cb.first->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, app->_deviceResources->pipeLayout, 0, obj.set);

					cb.first->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, app->_deviceResources->pipeLayout, 1, app->_deviceResources->descSetScene);

					cb.first->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, app->_deviceResources->pipeLayout, 2, descSetAllObj, &offset, 1);

					cb.first->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, app->_deviceResources->pipeLayout, 3, multi.descSetPerFrame, &uboCameraOffset, 1);

					// If different than before?
					cb.first->bindVertexBuffer(mesh.vbo, 0, 0);
					cb.first->bindIndexBuffer(mesh.ibo, 0, pvr::utils::convertToPVRVk(mesh.mesh->getFaces().getDataType()));

					// Offset in the per-object transformation matrices UBO - these do not change frame-to-frame
					// getArrayOffset, will return the actual byte offset of item #(first param) that is in an
					// array of items, at array index #(second param).
					cb.first->drawIndexed(0, mesh.mesh->getNumIndices());
				}
				cb.first->end();
			}
		}
		app->_tilesToDrawQ.produce(threadApiObj->drawQProducerToken, tileInfo);
	}
}

/// <summary>Consumes the items in the lines queue and checks for line visibility.</summary>
/// <returns>Returns true if items were processed correctly and false if queue not or queue was empty.</return>
bool GnomeHordeVisibilityThreadData::doWork()
{
	int32_t workItem[4];
	int32_t result;
	if ((result = app->_linesToProcessQ.consume(deviceResources->linesQConsumerToken, workItem[0]))) { determineLineVisibility(workItem, result); }
	return result != 0;
}

/// <summary>Determines whether the line is visible on screen or not.</summary>
void GnomeHordeVisibilityThreadData::determineLineVisibility(const int32_t* lineIdxs, uint32_t numLines)
{
	auto& tileInfos = app->_deviceResources->tileInfos;
	// Local temporaries of the "global volatile" visibility variables. memcpy used as they are declared "volatile"
	// It is perfectly fine to use memcopy for these volatiles AT THIS TIME because we know certainly that MAIN thread
	// has finished writing to them some time now and no race condition is possible (the calculations happen before the threads)
	pvr::math::ViewingFrustum _frustum;
	pvr::utils::memCopyFromVolatile(_frustum, app->_frustum);
	glm::vec3 camPos;
	pvr::utils::memCopyFromVolatile(camPos, app->_cameraPosition);

	TileResultsQueue& processQ = app->_tilesToProcessQ;
	TileResultsQueue& drawQ = app->_tilesToDrawQ;

	TileProcessingResult retval;
	const float MIN_LOD_DISTANCE = 400.f;
	const float MAX_LOD_DISTANCE = 2000.f; // Controls the falloff of the LOD
	const float MAX_LOD = 7; // Cap the maximum lod level by this number

	int numItems = 0;
	int numItemsProcessed = 0;
	int numItemsDrawn = 0;
	int numItemsDiscarded = 0;
	for (uint32_t line = 0; line < numLines; ++line)
	{
		glm::ivec2 id2d(0, lineIdxs[line]);
		for (id2d.x = 0; id2d.x < NUM_TILES_X; ++id2d.x)
		{
			tileInfos[id2d.y][id2d.x].visibility = pvr::math::aabbInFrustum(tileInfos[id2d.y][id2d.x].aabb, _frustum);

			TileInfo& tile = tileInfos[id2d.y][id2d.x];

			// Compute tile lod
			float dist = glm::distance(tile.aabb.center(), camPos); // Distance of the tile to the camera
			float flod = glm::max((dist - MIN_LOD_DISTANCE) / (MAX_LOD_DISTANCE - MIN_LOD_DISTANCE), 0.0f) * MAX_LOD; // Cap the minimum to zero
			flod = glm::min<float>(flod, MAX_LOD);

			tile.lod = static_cast<uint8_t>(flod);

			if (tile.visibility != tile.oldVisibility || tile.lod != tile.oldLod) // The tile has some change. Will need to do something.
			{
				for (uint32_t i = 0; i < app->_deviceResources->swapchain->getSwapchainLength(); ++i) // First, free its pre-existing command buffers (just mark free)
				{
					if (tile.cbs[i].first)
					{
						app->_deviceResources->tileThreadData[tile.threadId].freeCommandBuffer(tile.cbs[i].first, i);
						tile.cbs[i].first.reset();
					}
				}
				if (tile.visibility) // Item is visible, so must be recreated and drawn
				{
					retval.itemToDraw = id2d;
					processQ.produce(deviceResources->processQproducerToken, retval);
					retval.reset();
					numItems++;
					numItemsProcessed++;
				} // Otherwise, see below
			}
			else if (tile.visibility) // Tile had no change, but was visible - just add it to the drawing queue.
			{
				retval.itemToDraw = id2d;
				drawQ.produce(deviceResources->drawQproducerToken, retval);
				retval.reset();
				numItemsDrawn++;
				numItems++;
			}
			if (!tile.visibility) // THIS IS NOT AN ELSE. All invisible items end up here
			{
				++retval.itemsDiscarded;
				numItems++;
				numItemsDiscarded++;
			}

			tile.oldVisibility = tile.visibility;
			tile.oldLod = tile.lod;
		}
	}
	if (retval.itemsDiscarded != 0)
	{
		drawQ.produce(deviceResources->drawQproducerToken, retval);
		retval.reset();
	}
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred</returns>
pvr::Result VulkanGnomeHorde::renderFrame()
{
	const pvrvk::ClearValue clearVals[] = { pvrvk::ClearValue(0.0f, 0.128f, 0.0f, 1.0f), pvrvk::ClearValue(1.0f, 0u) };

	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	_swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[_swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[_swapchainIndex]->reset();

	float dt = getFrameTime() * 0.001f;
	_animDetails.logicTime += dt;
	if (_animDetails.logicTime > 10000000) { _animDetails.logicTime = 0; }

	// Get the next free swapchain image
	// We have implemented the application so that command buffers[0,1,2] points to swapchain frame buffers[0,1,2]
	// so we must submit the command buffers that this index points to.
	// Applications that generate command buffers on the fly may not need to do this.

	// Interpolate frame parameters
	AppModeParameter parameters = calcAnimationParameters();

	_animDetails.gameTime += dt * parameters.speedFactor;
	if (_animDetails.gameTime > MAX_GAME_TIME) { _animDetails.gameTime = 0; }

	const glm::vec3 worldSize = glm::vec3(TILE_SIZE_X + TILE_GAP_X, TILE_SIZE_Y, TILE_SIZE_Z + TILE_GAP_Z) * glm::vec3(NUM_TILES_X, 1, NUM_TILES_Z);
	glm::vec3 camPos = getTrackPosition(_animDetails.gameTime, worldSize);
	// _cameraPosition is also used by the visibility threads. The "volatile"
	// variable is to make sure it is visible to the threads
	// we will be starting in a bit. For the moment, NO concurrent access happens
	// (as the worker threads are inactive).
	pvr::utils::memCopyToVolatile(_cameraPosition, camPos);
	glm::vec3 camTarget = getTrackPosition(_animDetails.gameTime + parameters.cameraForwardOffset, worldSize) + glm::vec3(10.0f, 10.0f, 10.0f);
	camTarget.y = 0.0f;
	camPos.y += parameters.cameraHeightOffset;

	glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

	_viewMtx = glm::lookAt(camPos, camTarget, camUp);

	glm::mat4 cameraMat = _projMtx * _viewMtx;
	updateCameraUbo(cameraMat);

	pvr::math::ViewingFrustum frustumTmp;
	pvr::math::getFrustumPlanes(pvr::Api::Vulkan, cameraMat, frustumTmp);
	pvr::utils::memCopyToVolatile(_frustum, frustumTmp);
	_linesToProcessQ.produceMultiple(_deviceResources->lineQproducerToken, _allLines, NUM_TILES_Z);

	auto& cb = _deviceResources->multiBuffering[_swapchainIndex].cmdBuffers;
	cb->begin();

	cb->beginRenderPass(_deviceResources->onScreenFramebuffer[_swapchainIndex], false, clearVals, ARRAY_SIZE(clearVals));

	// Check culling and push the secondary command buffers on to the primary command buffer
	enum ItemsTotal
	{
		itemsTotal = NUM_TILES_X * NUM_TILES_Z
	};
	// MAIN RENDER LOOP - Collect the work (tiles) as it is processed
	{
		// Consume extra command buffers as they become ready
		uint32_t numItems;
		TileProcessingResult results[256];

		// We need some rather complex safeguards to make sure this thread does not wait forever.
		// First - we must (using atomics) make sure that when we say we are done, i.e. no items are unaccounted for.
		// Second, for the case where the main thread is waiting, but all remaining items are not visible, the last thread
		// to process an item will trigger an additional "unblock" to the main thread.

		uint32_t numItemsToDraw = itemsTotal;

		while (numItemsToDraw > 0)
		{
			numItems = static_cast<uint32_t>(_tilesToDrawQ.consumeMultiple(_deviceResources->drawQconsumerToken, results, 256));
			for (uint32_t i = 0; i < numItems; ++i)
			{
				numItemsToDraw -= results[i].itemsDiscarded;

				glm::ivec2 tileId = results[i].itemToDraw;
				if (tileId != glm::ivec2(-1, -1))
				{
					--numItemsToDraw;
					std::unique_lock<std::mutex> lock(*_deviceResources->tileInfos[tileId.y][tileId.x].cbs[_swapchainIndex].second);
					cb->executeCommands(&(_deviceResources->tileInfos[tileId.y][tileId.x].cbs[_swapchainIndex].first), 1);
				}
			}
		}
	}

	// The uirenderer should always be drawn last as it has (and checks) no depth
	cb->executeCommands(_deviceResources->multiBuffering[_swapchainIndex].cmdBufferUI);

	assertion(_linesToProcessQ.isEmpty(), "Initial Line Processing Queue was not empty after work done!");
	assertion(_tilesToProcessQ.isEmpty(), "Worker Tile Processing Queue was not empty after work done!");

	// We do not need any additional syncing - we know all dispatched work is done.

	cb->endRenderPass();
	cb->end();
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags waitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &cb;
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &waitStage;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[_swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, _swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	uint32_t swapIdx = _swapchainIndex;
	//--------------------
	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &swapIdx;
	presentInfo.numWaitSemaphores = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	_deviceResources->queue->present(presentInfo);
	printLog();
	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();
	return pvr::Result::Success;
}

void VulkanGnomeHorde::updateCameraUbo(const glm::mat4& matrix)
{
	_deviceResources->uboBufferView.getElement(0, 0, _swapchainIndex).setValue(matrix);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->ubo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->ubo->getDeviceMemory()->flushRange(
			_deviceResources->uboBufferView.getDynamicSliceOffset(_swapchainIndex), _deviceResources->uboBufferView.getDynamicSliceSize());
	}
}

void VulkanGnomeHorde::createDescSetsAndTiles(const pvrvk::DescriptorSetLayout& layoutImage, const pvrvk::DescriptorSetLayout& layoutScene,
	const pvrvk::DescriptorSetLayout& layoutPerObject, const pvrvk::DescriptorSetLayout& layoutPerFrameUbo, pvrvk::CommandBuffer& uploadCmdBuffer)
{
	pvrvk::Device& device = _deviceResources->device;
	{
		// The objects could have been completely different -
		// the fact that there are only a handful of different
		// objects is coincidental and does not affect the demo.
		auto& trilinear = _deviceResources->trilinear =
			device->createSampler(pvrvk::SamplerCreateInfo(pvrvk::Filter::e_LINEAR, pvrvk::Filter::e_LINEAR, pvrvk::SamplerMipmapMode::e_LINEAR));

		auto& nonMipmapped = _deviceResources->nonMipmapped =
			device->createSampler(pvrvk::SamplerCreateInfo(pvrvk::Filter::e_LINEAR, pvrvk::Filter::e_LINEAR, pvrvk::SamplerMipmapMode::e_NEAREST));

		_deviceResources->descSets.gnome = createDescriptorSetUtil(layoutImage, "gnome_texture.pvr", trilinear, nonMipmapped, uploadCmdBuffer);

		_deviceResources->descSets.gnomeShadow = createDescriptorSetUtil(layoutImage, "gnome_shadow.pvr", trilinear, nonMipmapped, uploadCmdBuffer);

		_deviceResources->descSets.rock = createDescriptorSetUtil(layoutImage, "rocks.pvr", trilinear, nonMipmapped, uploadCmdBuffer);

		_deviceResources->descSets.fern = createDescriptorSetUtil(layoutImage, "fern.pvr", trilinear, nonMipmapped, uploadCmdBuffer);

		_deviceResources->descSets.fernShadow = createDescriptorSetUtil(layoutImage, "fern_shadow.pvr", trilinear, nonMipmapped, uploadCmdBuffer);

		_deviceResources->descSets.mushroom = createDescriptorSetUtil(layoutImage, "mushroom_texture.pvr", trilinear, nonMipmapped, uploadCmdBuffer);

		_deviceResources->descSets.mushroomShadow = createDescriptorSetUtil(layoutImage, "mushroom_shadow.pvr", trilinear, nonMipmapped, uploadCmdBuffer);

		_deviceResources->descSets.bigMushroom = createDescriptorSetUtil(layoutImage, "bigMushroom_texture.pvr", trilinear, nonMipmapped, uploadCmdBuffer);

		_deviceResources->descSets.bigMushroomShadow = createDescriptorSetUtil(layoutImage, "bigMushroom_shadow.pvr", trilinear, nonMipmapped, uploadCmdBuffer);
	}

	// The ::pvr::utils::StructuredMemoryView is a simple class that allows us easy access to update members of a buffer - it keeps track of offsets,
	// data types and sizes of items in the buffer, allowing us to update them very easily.

	// The uboPerObject is one huge DynamicUniformBuffer, whose data is STATIC, and contains the object Model->World matrices
	// A different bit of this buffer is bound for each and every object.

	// CAUTION: The Range of the Buffer View for a Dynamic Uniform Buffer must be the BINDING size, not the TOTAL size, i.e. the size
	// of the part of the buffer that will be bound each time, not the total size. That is why we cannot do a one-step creation (...createBufferAndView) like
	// for static UBOs.
	pvrvk::WriteDescriptorSet descSetWrites[pvrvk::FrameworkCaps::MaxSwapChains + 2];

	_deviceResources->descSetScene = _deviceResources->descriptorPool->allocateDescriptorSet(layoutScene);

	descSetWrites[0]
		.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->descSetScene, 0)
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->sceneUbo, 0, _deviceResources->sceneUboBufferView.getDynamicSliceSize()));

	_deviceResources->descSetAllObjects = _deviceResources->descriptorPool->allocateDescriptorSet(layoutPerObject);

	descSetWrites[1]
		.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->descSetAllObjects, 0)
		.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->uboPerObject, 0, _deviceResources->uboPerObjectBufferView.getDynamicSliceSize()));

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// The uboPerFrame is a small UniformBuffer that contains the camera (World->Projection) matrix.
		// Since it is updated every frame, it is multi-buffered to avoid stalling the GPU
		auto& current = _deviceResources->multiBuffering[i];
		current.descSetPerFrame = _deviceResources->descriptorPool->allocateDescriptorSet(layoutPerFrameUbo);

		descSetWrites[i + 2]
			.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, current.descSetPerFrame, 0)
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->ubo, 0, _deviceResources->uboBufferView.getDynamicSliceSize()));
	}
	_deviceResources->device->updateDescriptorSets(descSetWrites, _deviceResources->swapchain->getSwapchainLength() + 2, nullptr, 0);
	// Create the UBOs/VBOs for the main objects. This automatically creates the VBOs.
	_meshes.createApiObjects(device, uploadCmdBuffer, _deviceResources->vmaAllocator);

	// Using the StructuredMemoryView to update the objects
	pvr::utils::StructuredBufferView& perObj = _deviceResources->uboPerObjectBufferView;
	pvrvk::Buffer& perObjBuffer = _deviceResources->uboPerObject;
	uint32_t mIndex = perObj.getIndex("modelMatrix");
	uint32_t mITIndex = perObj.getIndex("modelMatrixIT");

	for (uint32_t y = 0; y < NUM_TILES_Z; ++y)
	{
		for (uint32_t x = 0; x < NUM_TILES_X; ++x)
		{
			TileInfo& thisTile = _deviceResources->tileInfos[y][x];

			glm::vec3 tileBL(x * (TILE_SIZE_X + TILE_GAP_Z), 0, y * (TILE_SIZE_Z + TILE_GAP_Z));
			glm::vec3 tileTR = tileBL + glm::vec3(TILE_SIZE_X, TILE_SIZE_Y, TILE_SIZE_Z);

			thisTile.visibility = false;
			thisTile.lod = uint8_t(0xFFu);
			thisTile.oldVisibility = false;
			thisTile.oldLod = (uint8_t)0xFFu - (uint8_t)1;

			thisTile.objects[0].mesh = &_meshes.gnome;
			thisTile.objects[0].set = _deviceResources->descSets.gnome;
			thisTile.objects[0].pipeline = _deviceResources->pipelines.solid;

			thisTile.objects[1].mesh = &_meshes.gnomeShadow;
			thisTile.objects[1].set = _deviceResources->descSets.gnomeShadow;
			thisTile.objects[1].pipeline = _deviceResources->pipelines.shadow;

			thisTile.objects[2].mesh = &_meshes.mushroom;
			thisTile.objects[2].set = _deviceResources->descSets.mushroom;
			thisTile.objects[2].pipeline = _deviceResources->pipelines.solid;

			thisTile.objects[3].mesh = &_meshes.mushroomShadow;
			thisTile.objects[3].set = _deviceResources->descSets.mushroomShadow;
			thisTile.objects[3].pipeline = _deviceResources->pipelines.shadow;

			thisTile.objects[4].mesh = &_meshes.bigMushroom;
			thisTile.objects[4].set = _deviceResources->descSets.bigMushroom;
			thisTile.objects[4].pipeline = _deviceResources->pipelines.solid;

			thisTile.objects[5].mesh = &_meshes.bigMushroomShadow;
			thisTile.objects[5].set = _deviceResources->descSets.bigMushroomShadow;
			thisTile.objects[5].pipeline = _deviceResources->pipelines.shadow;

			thisTile.objects[7].mesh = &_meshes.fernShadow;
			thisTile.objects[7].set = _deviceResources->descSets.fernShadow;
			thisTile.objects[7].pipeline = _deviceResources->pipelines.shadow;

			thisTile.objects[6].mesh = &_meshes.fern;
			thisTile.objects[6].set = _deviceResources->descSets.fern;
			thisTile.objects[6].pipeline = _deviceResources->pipelines.alphaPremul;

			thisTile.objects[8].mesh = &_meshes.rock;
			thisTile.objects[8].set = _deviceResources->descSets.rock;
			thisTile.objects[8].pipeline = _deviceResources->pipelines.solid;

			std::array<glm::vec3, NUM_UNIQUE_OBJECTS_PER_TILE> points;
			generatePositions(points.data(), tileBL, tileTR);
			uint32_t tileBaseIndex = (y * NUM_TILES_X + x) * NUM_OBJECTS_PER_TILE;

			for (uint32_t halfobj = 0; halfobj < NUM_UNIQUE_OBJECTS_PER_TILE; ++halfobj)
			{
				uint32_t obj = halfobj * 2;
				uint32_t objShadow = obj + 1;
				// Note: do not put these in-line with the function call ..\..use it seems that the nexus player
				// swaps around the order that the parameters are evaluated compared to desktop
				float rot = pvr::randomrange(-glm::pi<float>(), glm::pi<float>());
				float s = pvr::randomrange(.8f, 1.3f);

				glm::vec3 position = points[halfobj];
				glm::mat4 rotation = glm::rotate(rot, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 scale = glm::scale(glm::vec3(s));
				glm::mat4 xform = glm::translate(position) * rotation * scale;
				glm::mat4 xformIT = glm::transpose(glm::inverse(xform));

				const auto& mesh = *thisTile.objects[obj].mesh->back().mesh;
				auto positions = mesh.getVertexAttributeByName("POSITION");
				auto numVertices = mesh.getNumVertices();
				const uint8_t* data = (const uint8_t*)mesh.getData(positions->getDataIndex()); // Use a byte-sized pointer so that offsets work properly.
				uint32_t stride = mesh.getStride(positions->getDataIndex());
				uint32_t offset = positions->getOffset();

				for (uint32_t i = 0; i < numVertices; ++i)
				{
					glm::vec3 pos_tmp;
					memcpy(&pos_tmp, data + offset + i * stride, sizeof(float) * 3);

					glm::vec3 pos = glm::vec3(xform * glm::vec4(pos_tmp, 1.f));

					if (halfobj == 0 && i == 0) { thisTile.aabb.setMinMax(pos, pos); }

					thisTile.aabb.add(pos);
				}

				perObj.getElement(mIndex, 0, tileBaseIndex + obj).setValue(xform);
				perObj.getElement(mITIndex, 0, tileBaseIndex + obj).setValue(xformIT);

				// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
				if (static_cast<uint32_t>(perObjBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
				{ perObjBuffer->getDeviceMemory()->flushRange(perObj.getDynamicSliceOffset(tileBaseIndex + obj), perObj.getDynamicSliceSize()); }
				if (objShadow != 9)
				{
					perObj.getElement(mIndex, 0, tileBaseIndex + objShadow).setValue(xform);
					perObj.getElement(mITIndex, 0, tileBaseIndex + objShadow).setValue(xformIT);

					// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
					if (static_cast<uint32_t>(perObjBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
					{ perObjBuffer->getDeviceMemory()->flushRange(perObj.getDynamicSliceOffset(tileBaseIndex + objShadow), perObj.getDynamicSliceSize()); }
				}
			}
		}
	}
}

MeshLod VulkanGnomeHorde::loadLodMesh(const pvr::StringHash& filename, const pvr::StringHash& mesh, uint32_t num_lods)
{
	MeshLod meshLod;
	meshLod.resize(num_lods);

	for (uint32_t i = 0; i < num_lods; ++i)
	{
		std::stringstream ss;
		ss << i;
		ss << ".pod";

		std::string path = filename.str() + ss.str();
		Log(LogLevel::Information, "Loading model:%s mesh:%s", path.c_str(), mesh.c_str());
		pvr::assets::ModelHandle model = pvr::assets::loadModel(*this, path);

		if (model == nullptr) { assertion(false, pvr::strings::createFormatted("Failed to load model file %s", path.c_str()).c_str()); }
		for (uint32_t j = 0; j < model->getNumMeshNodes(); ++j)
		{
			if (model->getMeshNode(j).getName() == mesh)
			{
				uint32_t meshId = model->getMeshNode(j).getObjectId();
				meshLod[i].mesh = pvr::assets::getMeshHandle(model, meshId);
				break;
			}
			if (j == model->getNumMeshNodes()) { assertion(false, pvr::strings::createFormatted("Could not find mesh %s in model file %s", mesh.c_str(), path.c_str()).c_str()); }
		}
	}
	return meshLod;
}

pvrvk::DescriptorSet VulkanGnomeHorde::createDescriptorSetUtil(const pvrvk::DescriptorSetLayout& layout, const pvr::StringHash& texture, const pvrvk::Sampler& mipMapped,
	const pvrvk::Sampler& nonMipMapped, pvrvk::CommandBuffer& uploadCmdBuffer)
{
	//--------------------
	// Load the texture

	pvrvk::ImageView tex = pvr::utils::loadAndUploadImageAndView(_deviceResources->device, texture.c_str(), true, uploadCmdBuffer, *this, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
		pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	pvrvk::DescriptorSet tmp = _deviceResources->descriptorPool->allocateDescriptorSet(layout);
	bool hasMipmaps = tex->getImage()->getNumMipLevels() > 1;
	pvrvk::WriteDescriptorSet write(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, tmp, 0);
	write.setImageInfo(0, pvrvk::DescriptorImageInfo(tex, hasMipmaps ? mipMapped : nonMipMapped, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	_deviceResources->device->updateDescriptorSets(&write, 1, nullptr, 0);
	return tmp;
}

void VulkanGnomeHorde::initUboStructuredObjects()
{
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("directionToLight", pvr::GpuDatatypes::vec4);

		_deviceResources->sceneUboBufferView.init(desc);

		_deviceResources->sceneUbo = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->sceneUboBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->sceneUboBufferView.pointToMappedMemory(_deviceResources->sceneUbo->getDeviceMemory()->getMappedData());
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("viewProjectionMat", pvr::GpuDatatypes::mat4x4);

		_deviceResources->uboBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

		_deviceResources->ubo = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->uboBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->uboBufferView.pointToMappedMemory(_deviceResources->ubo->getDeviceMemory()->getMappedData());
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("modelMatrix", pvr::GpuDatatypes::mat4x4);
		desc.addElement("modelMatrixIT", pvr::GpuDatatypes::mat4x4);

		_deviceResources->uboPerObjectBufferView.initDynamic(desc, TOTAL_NUMBER_OF_OBJECTS, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

		_deviceResources->uboPerObject = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->uboPerObjectBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->uboPerObjectBufferView.pointToMappedMemory(_deviceResources->uboPerObject->getDeviceMemory()->getMappedData());
	}
}

AppModeParameter VulkanGnomeHorde::calcAnimationParameters()
{
	bool needsTransition = false;
	if (!_animDetails.isManual)
	{
		if (_animDetails.logicTime > _animDetails.modeSwitchTime + DemoModes[_animDetails.currentMode].duration)
		{
			_animDetails.previousMode = _animDetails.currentMode;
			_animDetails.currentMode = (_animDetails.currentMode + 1) % DemoModes.size();
			Log(LogLevel::Information, "Switching to mode: [%d]", _animDetails.currentMode);
			needsTransition = true;
		}
	}
	if (needsTransition)
	{
		_animDetails.modeSwitchTime = _animDetails.logicTime;
		needsTransition = false;
	}

	// Generate camera position
	float iterp = glm::clamp((_animDetails.logicTime - _animDetails.modeSwitchTime) * 1.25f, 0.0f, 1.0f);
	float factor = (1.0f - glm::cos(iterp * 3.14159f)) / 2.0f;
	const AppModeParameter& current = DemoModes[_animDetails.currentMode];
	const AppModeParameter& prev = DemoModes[_animDetails.previousMode];

	// Interpolate
	AppModeParameter result;
	result.cameraForwardOffset = glm::mix(prev.cameraForwardOffset, current.cameraForwardOffset, factor);
	result.cameraHeightOffset = glm::mix(prev.cameraHeightOffset, current.cameraHeightOffset, factor);
	result.speedFactor = glm::mix(prev.speedFactor, current.speedFactor, factor);
	return result;
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanGnomeHorde>(); }
