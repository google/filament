/*!
\brief Contains a futures system for asynchronous loading of resources into Vulkan.
\file PVRUtils/Vulkan/AsynchronousVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/Threading.h"
#include "PVRCore/texture/Texture.h"
#include "PVRCore/texture/TextureLoadAsync.h"
#include "PVRVk/ImageVk.h"
#include "PVRUtils/Vulkan/HelperVk.h"
namespace pvr {
namespace utils {

/// <summary>Provides a reference counted pointer to a pvr::Texture which will be used for loading API agnostic texture data to disk.</summary>
typedef std::shared_ptr<Texture> TexturePtr;

/// <summary>Provides a reference counted pointer to a IFrameworkAsyncResult specialised by a TexturePtr which will be used
/// as the main interface for using API agnostic asynchronous textures.</summary>
typedef std::shared_ptr<async::IFrameworkAsyncResult<TexturePtr>> AsyncTexture;

/// <summary>Provides a reference counted pointer to a IFrameworkAsyncResult specialised by an ImageView which will be used
/// as the main interface for using API specific asynchronous textures.</summary>
typedef std::shared_ptr<async::IFrameworkAsyncResult<pvrvk::ImageView>> AsyncApiTexture;

/// <summary>This class provides a wrapper for a Image upload future</summary>
struct ImageUploadFuture_ : public async::IFrameworkAsyncResult<pvrvk::ImageView>, public std::enable_shared_from_this<ImageUploadFuture_>
{
public:
	/// <summary>The type of the optional callback that is called at the end of the operation</summary>
	typedef IFrameworkAsyncResult<pvrvk::ImageView>::Callback CallbackType;

	/// <summary>Default constructor for a pvr::utils::ImageUploadFuture_.</summary>
	ImageUploadFuture_() {}

	/// <summary>A queue to be used to submit image upload operations.</summary>
	pvrvk::Queue _queue;

	/// <summary>A _device to be used for creating temporary resources required for uploading an image.</summary>
	pvrvk::Device _device;

	/// <summary>A pvr::Texture to asynchronously upload to the Gpu.</summary>
	AsyncTexture _texture;

	/// <summary>A command pool from which comand buffers will be allocated to record image upload operations.</summary>
	pvrvk::CommandPool _cmdPool;

	/// <summary>A semaphore used to guard access to submitting to the CommandQueue.
	async::Mutex* _cmdQueueMutex;

	/// <summary>Specifies whether the uploaded texture can be decompressed as it is uploaded.</summary>
	bool _allowDecompress;

	/// <summary>Specifies a semaphore which will be signalled at the point the upload of the texture is finished.</summary>
	mutable async::SemaphorePtr _resultSemaphore;

	/// <summary>Specifies whether the callback should be called prior to signalling the completion of the image upload.</summary>
	bool _callbackBeforeSignal = false;

	/// <summary>Sets a callback which will be called after the image upload has completed.</summary>
	/// <param name="callback">Specifies a callback to call when the image upload has completed.</param>
	void setCallBack(CallbackType callback) { setTheCallback(callback); }

	/// <summary>Initiates the asynchronous image upload.</summary>
	void loadNow()
	{
		_result = customUploadImage();

		_successful = _result != nullptr;
		if (_callbackBeforeSignal)
		{
			callBack();
			_resultSemaphore->signal();
		}
		else
		{
			_resultSemaphore->signal();
			callBack();
		}
	}

	/// <summary>Returns the result of the asynchronous image upload.</summary>
	/// <returns>The result of the asynchronous image upload which is an ImageView.</returns>
	const pvrvk::ImageView& getResult() { return _result; }

private:
	pvrvk::ImageView customUploadImage()
	{
		Texture& assetTexture = *_texture->get();
		pvrvk::CommandBuffer cmdBuffer = _cmdPool->allocateCommandBuffer();
		cmdBuffer->begin();
		pvrvk::ImageView results = uploadImageAndView(_device, assetTexture, _allowDecompress, cmdBuffer);
		cmdBuffer->end();

		pvrvk::SubmitInfo submitInfo;
		submitInfo.commandBuffers = &cmdBuffer;
		submitInfo.numCommandBuffers = 1;
		pvrvk::Fence fence = _device->createFence();
		if (_cmdQueueMutex != nullptr)
		{
			std::lock_guard<pvr::async::Mutex> lock(*_cmdQueueMutex);
			_queue->submit(&submitInfo, 1, fence);
		}
		else
		{
			_queue->submit(&submitInfo, 1, fence);
		}
		fence->wait();

		return results;
	}

	void callBack() { executeCallBack(shared_from_this()); }
	pvrvk::ImageView _result;
	pvrvk::ImageView get_() const
	{
		if (!_inCallback)
		{
			_resultSemaphore->wait();
			_resultSemaphore->signal();
		}
		return _result;
	}

	pvrvk::ImageView getNoWait() const { return _result; }

	bool isComplete_() const
	{
		if (_resultSemaphore->tryWait())
		{
			_resultSemaphore->signal();
			return true;
		}
		return false;
	}
	void cleanup_() {}
	void destroyObject() {}
};

/// <summary>A ref-counted pointer to a Future of an Image Upload: A class that wraps the texture that
/// "is being uploaded on a separate thread", together with functions to "query if the upload is yet complete"
/// and to "block until the upload is complete, if necessary, and return the result"</summary>
typedef std::shared_ptr<ImageUploadFuture_> ImageUploadFuture;

/// <summary>Provides a mechanism for kicking an asynchronous image upload worker</summary>
/// <param name="uploadFuture">An image upload future to be uploaded on a separate thread.</param>
inline void imageUploadAsyncWorker(ImageUploadFuture uploadFuture) { uploadFuture->loadNow(); }

/// <summary>This class wraps a worker thread that uploads texture to the GPU asynchronously and returns
/// futures to them. This class would normally be used with Texture Futures as well, in order to do both
/// of the operations asynchronously.</summary>
class ImageApiAsyncUploader : public async::AsyncScheduler<pvrvk::ImageView, ImageUploadFuture, imageUploadAsyncWorker>
{
private:
	pvrvk::Device _device;
	pvrvk::Queue _queueVk;
	pvrvk::CommandPool _cmdPool;
	async::Mutex* _cmdQueueMutex;

public:
	ImageApiAsyncUploader() : _cmdQueueMutex(nullptr) { _myInfo = "ImageApiAsyncUploader"; }
	/// <summary>The type of the optional callback that is called at the end of the operation</summary>
	typedef async::IFrameworkAsyncResult<pvrvk::ImageView>::Callback CallbackType;

	/// <summary>Initialize this AsyncUploader. Do not use the queue and pool unguarded aftewards, as
	/// they will be accessed from an indeterminate thread at indeterminate times. It is ideal that
	/// the queue is only used by this Uploader, but if it is not, a (CPU) Semaphore must be passed
	/// to guard access to the queue.</summary>
	/// <param name="device">A Vulkan _device that will be used to create the textures. A Command pool
	/// will be created on this device for the queue family of the queue</param>
	/// <param name="queue">A Vulkan command queue that will be used to upload the textures</param>
	/// <param name="queueSemaphore">Use the Semaphore as a mutex: Initial count 1, call wait() before
	/// all accesses to the Vulkan queue, then signal() when finished accessing. If the queue does
	/// not need external synchronization (i.e. it is only used by this object), leave the
	/// queueSemaphore at its default value of NULL</param>
	void init(pvrvk::Device& device, pvrvk::Queue& queue, async::Mutex* queueSemaphore = nullptr)
	{
		_device = device;
		_queueVk = queue;
		_cmdPool = device->createCommandPool(pvrvk::CommandPoolCreateInfo(queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
		_cmdQueueMutex = queueSemaphore;
	}

	/// <summary>Begin a texture uploading task and return the future to the Vulkan Texture. Use the returned
	/// future to query completion and get the result. Takes asynchronous textures as input so that the loading
	/// and uploading tasks can be chained and done on different threads.</summary>
	/// <param name="texture">A PVR Texture future (can be gotten from the Async Texture Loader or another async source)</param>
	/// <param name="allowDecompress">If the texture is compressed to an unsupported format, allow it to be decompressed to
	/// RGBA8 if a software decompressor is available. Default true</param>
	/// <param name="callback">An optional function pointer that will be invoked when the uploading is complete. IF you use the
	/// callback AND the you set the flag "callbackBeforeSignal" to TRUE, do NOT call the "get" function of the future in the
	/// callback. Default NULL.</param>
	/// <param name="callbackBeforeSignal">A flag signifying if the callback should be called BEFORE or AFTER the semaphore
	/// of the Result future (the Texture future) is signalled as complete. Defaults to false, so as to avoid the deadlock that
	/// will happen if the user attempts to call "get" on the future while the signal will happen just after return of the callback.
	/// Set to "true" if you want to do something WITHOUT calling "get" on the future, but before the texture is  used.</param>
	/// <returns> A texture upload Future which you can use to query or get the uploaded texture</returns>
	AsyncApiTexture uploadTextureAsync(const AsyncTexture& texture, bool allowDecompress = true, CallbackType callback = nullptr, bool callbackBeforeSignal = false)
	{
		assertion(_queueVk != nullptr, "Queue has not been initialized");
		auto future = std::make_shared<ImageUploadFuture_>();
		auto& params = *future;
		params._allowDecompress = allowDecompress;
		params._queue = _queueVk;
		params._device = _device;
		params._texture = texture;
		params._resultSemaphore = std::make_shared<async::Semaphore>();
		params._cmdPool = _cmdPool;
		params.setCallBack(callback);
		params._callbackBeforeSignal = callbackBeforeSignal;
		params._cmdQueueMutex = _cmdQueueMutex;
		_queueSemaphore.wait();
		_queue.emplace_back(future);
		_queueSemaphore.signal();
		_workSemaphore.signal();
		return future;
	}
};
} // namespace utils
} // namespace pvr
