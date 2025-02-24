/*!
\brief Contains classes and functions to load textures on a worker thread. Only contains functionality for loading into CPU-side memory (not API textures)
\file PVRCore/texture/TextureLoadAsync.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCore/texture/TextureLoad.h"
#include "PVRCore/Threading.h"
#include "PVRCore/IAssetProvider.h"

namespace pvr {
namespace async {

/// <summary>A ref counted pointer to a Texture object  Used to return and pass a dynamically allocated textures</summary>
typedef std::shared_ptr<Texture> TexturePtr;

/// <summary>A class wrapping the operations necessary to retrieve an asynchronously loaded texture, (e.g. querying
/// if the load is complete, or blocking-wait get the result. It is a shared reference counted resource</summary>
struct TextureLoadFuture_ : public IFrameworkAsyncResult<TexturePtr>, public std::enable_shared_from_this<TextureLoadFuture_>
{
public:
	typedef IFrameworkAsyncResult<TexturePtr> MyBase; ///< Base class
	typedef MyBase::Callback CallbackType; ///< The type of function that can be used as a completion callback

	TextureLoadFuture_() {}

	Semaphore* workSemaphore; ///< A pointer to an externally used semaphore (normally the one used by the queue)
	std::string filename; ///< The filename from which the texture is loaded
	IAssetProvider* loader; ///< The AssetProvider to use to load the texture
	TextureFileFormat format; ///< The format of the texture
	mutable SemaphorePtr resultSemaphore; ///< The semaphore that is used to wait for the result
	/// <summary>The result of the operation will be stored here</summary>
	TexturePtr result;
	/// <summary>A pointer to an exception to throw</summary>
	std::exception_ptr exception;

	/// <summary>Load the texture synchronously and signal the result semaphore. Normally called by the worker thread</summary>
	void loadNow()
	{
		std::unique_ptr<Stream> stream = loader->getAssetStream(filename);
		_successful = false;
		try
		{
			*result = textureLoad(*stream, format);
			_successful = true;
		}
		catch (...)
		{
			exception = std::current_exception();
			_successful = false;
		}

		resultSemaphore->signal();
		executeCallBack(shared_from_this());
	}
	/// <summary>Set a function to be called when the texture loading has been finished.</summary>
	/// <param name="callback">Set a function to be called when the texture loading has been finished.</param>
	void setCallBack(CallbackType callback) { setTheCallback(callback); }

private:
	TexturePtr get_() const
	{
		if (!_inCallback)
		{
			resultSemaphore->wait();
			resultSemaphore->signal();
		}
		return result;
	}
	bool isComplete_() const
	{
		if (resultSemaphore->tryWait())
		{
			resultSemaphore->signal();
			return true;
		}
		return false;
	}
	void cleanup_() {}
	void destroyObject() {}
};

/// <summary>A reference counted handle to a TextureLoadFuture_. A TextureLoadFuture_ can only
/// be handled using this class</summary>
typedef std::shared_ptr<TextureLoadFuture_> TextureLoadFuture;

namespace impl {
/// <summary>internal</summary>
/// <param name="future">The Texture load future to kick work for</param>
inline void textureLoadAsyncWorker(TextureLoadFuture future) { future->loadNow(); }
} // namespace impl

/// <summary>A class that loads Textures in a (single) different thread and provides futures to them.
/// Create an instance of it, and then just call loadTextureAsync foreach texture to load. When each texture
/// has completed loading, a callback may be called, otherwise you can use all the typical functionality
/// of futures, such as querying if loading is comlete, or using a blocking wait to get the result</summary>
class TextureAsyncLoader : public AsyncScheduler<TexturePtr, TextureLoadFuture, &impl::textureLoadAsyncWorker>
{
public:
	TextureAsyncLoader() { _myInfo = "TextureAsyncLoader"; }
	/// <summary>This function enqueues a "load texture" on a background thread, and returns an object
	/// that can be used to query and wait for the result.</summary>
	/// <param name="filename">The filename of the texture to load</param>
	/// <param name="loader">A class that provides a "getAssetStream" function to get a Stream from the filename (usually, the application class itself)</param>
	/// <param name="format">The texture format as which to load the texture.</param>
	/// <param name="callback">An optional callback to call immediately after texture loading is complete.</param>
	/// <returns> A future to a texture : TextureLoadFuture</returns>
	AsyncResult loadTextureAsync(const std::string& filename, IAssetProvider* loader, TextureFileFormat format, AsyncResult::element_type::Callback callback = NULL)
	{
		auto future = std::make_shared<TextureLoadFuture_>();
		auto& params = *future;
		params.filename = filename;
		params.format = format;
		params.loader = loader;
		params.result = std::make_shared<Texture>();
		params.resultSemaphore = std::make_shared<Semaphore>();
		params.workSemaphore = &_workSemaphore;
		params.setCallBack(callback);
		_queueSemaphore.wait();
		_queue.emplace_back(future);
		_queueSemaphore.signal();
		_workSemaphore.signal();
		return future;
	}
};
} // namespace async
} // namespace pvr
