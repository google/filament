/*!
\brief Implementation of the Android camera interface.
\file PVRCamera/CameraInterface_Android.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Platform independent camera interface API include file.
*/

#include "PVRCamera/CameraInterface.h"
#include <jni.h>
#include "PVRCore/Log.h"
#include "PVRCore/Errors.h"

//!\cond NO_DOXYGEN

////////////// LIFECYCLE EXPLANATION ///////////////
// There are two separate life cycles, and a few extras going on, and to make matters worse, JNI/Java communication makes things even more involved.
// 1) The first cycle going on is the Android Activity lifecycle (onCreate, on Destroy etc),
//    - This lifecycle will call the JNI methods to provide handles to the java-side CameraInterface object so that the android-side
//    - operations can be performed. Look for the JNIEXPORT/JNICALL functions, such as JNI_OnLoad (plain JNI setup)
//    - cacheJavaObjects and releaseJavaObjects will notify the app that the java-side camera interface is or is not ready, so that they can
//    - start or stop using their functionality
//    - setProjectionMatrix will accept the projection matrix from the Java object
// 2) The second cycle going on is the PVRShell/ Application lifecycle: initApplication, initView, releaseView etc. - the PowerVR SDK state machine
//    - This cycle controls the CameraInterface object - Constructor, Destructor, initialise/destroy session, updateImage.
//    - This object (here, CameraInterfaceImpl) also controls the basic functions, i.e. getRGBTexture etc.
// *) While on first view these two cycles might look almost disjoint, there are two considerations:
//    a) The calling of the java methods must always happen on a JNI thread, hence the attach/detach calls.
//    b) The creation of the Java side Camera object cannot be completed without the id of an OpenGL ES Texture object, and that must happen on
//       a thread where an EGL context is made active, i.e. one controlled by the state machine
//    This "meetup" of the two lifecycles happens lazily, on the "updateImage" function, that checks if the camera is completely created and, if not,
//    checks if the two lifecycles are ready, so as to create it.


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_createJavaObjects(JNIEnv* env, jobject obj);
JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_releaseJavaObjects(JNIEnv* env, jobject obj);
JNIEXPORT jboolean JNICALL Java_com_powervr_PVRCamera_CameraInterface_setTexCoordsProjMatrix(JNIEnv* env, jobject obj, jfloatArray ptr, jint width, jint height);

#ifdef __cplusplus
}
#endif // __cplusplus

namespace pvr {
class CameraInterfaceImpl;
}

static void createCameraObject();

class pvr::CameraInterfaceImpl
{
public:
	CameraInterface* parent;
	GLuint texture;
	glm::mat4 projectionMatrix;
	bool hasProjectionMatrixChanged;
	uint32_t width;
	uint32_t height;
	static pvr::CameraInterfaceImpl* activeSession;
	static jobject jobj;
	static JavaVM* cachedVM;
	static jmethodID updateImageMID;

	CameraInterfaceImpl(pvr::CameraInterface* thisptr) : parent(thisptr), texture(0), hasProjectionMatrixChanged(true) {
		memset(glm::value_ptr(projectionMatrix), 0, sizeof(projectionMatrix));
		parent->_isReady = false;
	}

	void setReady(bool ready)
	{
		parent->_isReady = ready;
	}

	bool isReady()
	{
		return parent->_isReady;
	}

	void initializeSession(HWCamera::Enum eCamera, int width, int height)
	{
		Log(LogLevel::Debug, "PVRCamera::CameraInterfaceImpl::initialiseSession executing");

		if (strstr((const char*)gl::GetString(GL_EXTENSIONS), "OES_EGL_image_external") == 0)
		{ throw InvalidOperationError("CameraInterface - NativeExtension OES_EGL_image_external not found."); }

		activeSession = this;

		// Create an EGLImage External texture
		// Create a http://www.khronos.org/registry/gles/extensions/OES/OES_EGL_image_external.txt
		gl::GenTextures(1, &texture);
		gl::BindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
	}

	void destroySession()
	{
#pragma warning TODO - SHOULDNT WE TELL ANYONE?

		Log(LogLevel::Debug, "PVRCamera::CameraInterfaceImpl::destroy executing");

		activeSession = NULL;

		if(texture){
			gl::BindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
			gl::DeleteTextures(1, &texture);
			texture = 0;
		}
	}

	/****************************************************************************/

	bool updateImage()
	{
		bool result = 0;
		if (!isReady())
		{
			if (cachedVM && jobj)
			{
				Log(LogLevel::Debug, "pvr::CameraInterfaceImpl::updateImage: Attempting to create camera.");
				if (cachedVM && jobj) { createCameraObject(); }
			}else
			{
				static int framedelay = 0;
				if (!(framedelay++%60))
				{
					Log(LogLevel::Debug, "pvr::CameraInterfaceImpl::updateImage: Warning - Camera not created yet.");
				}

			}
		}
		if (isReady())
		{
			JNIEnv* env = 0;
			jint res = cachedVM->AttachCurrentThread(&env, 0);
			if (res || !env)
			{
				Log(LogLevel::Error, "pvr::CameraInterfaceImpl::updateImage - fNativeAttachCurrentThreadailed.");
				throw pvr::InvalidOperationError("pvr::CameraInterfaceImpl::updateImage - NativeAttachCurrentThread failed.");
			}
			result = env->CallBooleanMethod(jobj, updateImageMID);
			cachedVM->DetachCurrentThread();
		}
		return (bool)result;
	}

	bool getCameraResolution(uint32_t& x, uint32_t& y) {
		if (!jobj) return false;

		x = width;
		y = height;
		return true;
	}

	void please_dont_strip_jni_functions()
	{
		Java_com_powervr_PVRCamera_CameraInterface_createJavaObjects(0, 0);
		Java_com_powervr_PVRCamera_CameraInterface_releaseJavaObjects(0, 0);
		Java_com_powervr_PVRCamera_CameraInterface_setTexCoordsProjMatrix(0, 0, 0, 0, 0);
		JNI_OnLoad(0, 0);
	}


	void createCameraObject()
	{
		Log(LogLevel::Debug, "PVRCamera::createCameraObject - enter.");
		JNIEnv* env;
		jint res = cachedVM->AttachCurrentThread(&env, 0);

		if ((res != 0) || (env == 0))
		{
			Log(LogLevel::Error, "PVRCamera::createCameraObject - NativeAttachCurrentThread failed.");
			throw pvr::InvalidOperationError("PVRCamera::createCameraObject - NativeAttachCurrentThread failed.");
		}

		jclass clazz = 0;
		clazz = env->GetObjectClass(pvr::CameraInterfaceImpl::jobj);
		if (clazz == 0)
		{
			Log(LogLevel::Error, "PVRCamera::createCameraObject - NativeGetObjectClass failed.");
			cachedVM->DetachCurrentThread();
			throw pvr::InvalidOperationError("PVRCamera::createCameraObject - NativeGetObjectClass failed.");
		}

		jmethodID createCameraMethod = env->GetMethodID(clazz, "createCamera", "(I)I");
		if (createCameraMethod == 0)
		{
			Log(LogLevel::Error, "PVRCamera::createCameraObject - NativeGetObjectClass failed.");
			cachedVM->DetachCurrentThread();
			throw pvr::InvalidOperationError("PVRCamera::createCameraObject - NativeGetObjectClass failed.");
		}

		Log(LogLevel::Debug, "PVRCamera::createCameraObject: Executing Java function createCamera.");

		int result = env->CallIntMethod(jobj, createCameraMethod, pvr::CameraInterfaceImpl::activeSession->texture);
		if (!result)
		{
			Log(LogLevel::Debug, "PVRCamera::createCameraObject: createCamera FAILED!");
			cachedVM->DetachCurrentThread();
			throw pvr::InvalidOperationError("PVRCamera::createCameraObject: createCamera FAILED!");
		}

		// Get and cache the method ID.
		updateImageMID = env->GetMethodID(clazz, "updateImage", "()Z");
		if (pvr::CameraInterfaceImpl::updateImageMID == 0)
		{
			pvr::CameraInterfaceImpl::cachedVM->DetachCurrentThread();
			throw pvr::InvalidOperationError("CameraInterface - updateImage::NativeGetMethodID failed");
		}

		env->DeleteLocalRef(clazz);
		cachedVM->DetachCurrentThread();
		setReady(true);
	}
};

jobject pvr::CameraInterfaceImpl::jobj = 0;
JavaVM* pvr::CameraInterfaceImpl::cachedVM = 0;
jmethodID pvr::CameraInterfaceImpl::updateImageMID = 0;
pvr::CameraInterfaceImpl* pvr::CameraInterfaceImpl::activeSession = NULL;

static void destroyCameraObject(JNIEnv* env){
	if (pvr::CameraInterfaceImpl::jobj)
	{
		env->DeleteGlobalRef(pvr::CameraInterfaceImpl::jobj);
		pvr::CameraInterfaceImpl::jobj = nullptr;
	}
	pvr::CameraInterfaceImpl::updateImageMID = nullptr;
	if (pvr::CameraInterfaceImpl::activeSession)
	{
		pvr::CameraInterfaceImpl::activeSession->setReady(false);
	}
}

////////////////////// CameraInterface implementation //////////////////////

void pvr::CameraInterface::initializeSession(HWCamera::Enum camera, int width, int height)
{
	Log(LogLevel::Debug, "pvr::CameraInterface::initialiseSession executing");
	static_cast<pvr::CameraInterfaceImpl*>(pImpl)->initializeSession(camera, width, height);
}

void pvr::CameraInterface::destroySession()
{
	Log(LogLevel::Debug, "pvr::CameraInterface::destroySession executing");
	static_cast<pvr::CameraInterfaceImpl*>(pImpl)->destroySession();
}

pvr::CameraInterface::CameraInterface()
{
	Log(LogLevel::Debug, "pvr::CameraInterface::Ctor executing");
	pImpl = static_cast<void*>(new CameraInterfaceImpl(this));
}
pvr::CameraInterface::~CameraInterface()
{
	Log(LogLevel::Debug, "pvr::CameraInterface::Dtor executing");
	delete static_cast<pvr::CameraInterfaceImpl*>(pImpl);
	pvr::CameraInterfaceImpl::activeSession = NULL;
}

bool pvr::CameraInterface::hasProjectionMatrixChanged() { return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->hasProjectionMatrixChanged; }

bool pvr::CameraInterface::getCameraResolution(uint32_t& x, uint32_t& y) { return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->getCameraResolution(x, y); }

GLuint pvr::CameraInterface::getRgbTexture() { return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->texture; }

GLuint pvr::CameraInterface::getLuminanceTexture() { return 0; }
GLuint pvr::CameraInterface::getChrominanceTexture() { return 0; }
bool pvr::CameraInterface::hasRgbTexture() { return true; }
bool pvr::CameraInterface::hasLumaChromaTextures() { return false; }
bool pvr::CameraInterface::updateImage() { return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->updateImage(); }

const glm::mat4& pvr::CameraInterface::getProjectionMatrix()
{
	static_cast<pvr::CameraInterfaceImpl*>(pImpl)->hasProjectionMatrixChanged = false;
	return static_cast<pvr::CameraInterfaceImpl*>(pImpl)->projectionMatrix;
}

//////////////////// JNI Functions and communication with Java ////////////////////

JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_createJavaObjects(JNIEnv* env, jobject obj) {
	Log(LogLevel::Debug, "com.powervr.PVRCamera::Java_com_powervr_PVRCamera_CameraInterface_createJavaObjects enter.");
	if (pvr::CameraInterfaceImpl::jobj != obj){
		if (pvr::CameraInterfaceImpl::jobj)
		{
			Log(LogLevel::Debug, "com.powervr.PVRCamera::Java_com_powervr_PVRCamera_CameraInterface_createJavaObjects freeing 'jobj' as it was not null");
			env->DeleteGlobalRef(pvr::CameraInterfaceImpl::jobj);
			pvr::CameraInterfaceImpl::jobj = nullptr;
		}
		Log(LogLevel::Debug, "com.powervr.PVRCamera::Java_com_powervr_PVRCamera_CameraInterface_createJavaObjects caching jobj");
		pvr::CameraInterfaceImpl::jobj = env->NewGlobalRef(obj);
	} else {
		Log(LogLevel::Warning, "com.powervr.PVRCamera::Java_com_powervr_PVRCamera_CameraInterface_createJavaObjects jobj already cached.");
	}

	// Cache the VM
	env->GetJavaVM(&pvr::CameraInterfaceImpl::cachedVM);
}

JNIEXPORT void JNICALL Java_com_powervr_PVRCamera_CameraInterface_releaseJavaObjects(JNIEnv* env, jobject obj) {
	Log(LogLevel::Debug, "com.powervr.PVRCamera::Java_com_powervr_PVRCamera_CameraInterface_releaseJavaObjects Executing");
	destroyCameraObject(env);
}


JNIEXPORT jboolean JNICALL Java_com_powervr_PVRCamera_CameraInterface_setTexCoordsProjMatrix(JNIEnv* env, jobject obj, jfloatArray ptr, jint width, jint height)
{
	Log(LogLevel::Debug, "PVRCamera::Java_com_powervr_PVRCamera_CameraInterface_setTexCoordsProjMatrix executing");
	if (!pvr::CameraInterfaceImpl::activeSession) {
		return false;
	}

	jfloat* flt1 = env->GetFloatArrayElements(ptr, 0);

	memcpy(glm::value_ptr(pvr::CameraInterfaceImpl::activeSession->projectionMatrix), flt1, 16 * sizeof(float));
	Log(LogLevel::Debug,
		"CameraInterface - Projection matrix changed. Projection matrix is now\n"
		"%4.3f %4.3f %4.3f %4.3f\n%4.3f %4.3f %4.3f %4.3f\n%4.3f %4.3f %4.3f %4.3f\n%4.3f %4.3f %4.3f %4.3f",
	pvr::CameraInterfaceImpl::activeSession->projectionMatrix[0][0], pvr::CameraInterfaceImpl::activeSession->projectionMatrix[0][1],
	pvr::CameraInterfaceImpl::activeSession->projectionMatrix[0][2], pvr::CameraInterfaceImpl::activeSession->projectionMatrix[0][3],
	pvr::CameraInterfaceImpl::activeSession->projectionMatrix[1][0], pvr::CameraInterfaceImpl::activeSession->projectionMatrix[1][1],
	pvr::CameraInterfaceImpl::activeSession->projectionMatrix[1][2], pvr::CameraInterfaceImpl::activeSession->projectionMatrix[1][3],
	pvr::CameraInterfaceImpl::activeSession->projectionMatrix[2][0], pvr::CameraInterfaceImpl::activeSession->projectionMatrix[2][1],
	pvr::CameraInterfaceImpl::activeSession->projectionMatrix[2][2], pvr::CameraInterfaceImpl::activeSession->projectionMatrix[2][3],
	pvr::CameraInterfaceImpl::activeSession->projectionMatrix[3][0], pvr::CameraInterfaceImpl::activeSession->projectionMatrix[3][1],
	pvr::CameraInterfaceImpl::activeSession->projectionMatrix[3][2], pvr::CameraInterfaceImpl::activeSession->projectionMatrix[3][3]);

	pvr::CameraInterfaceImpl::activeSession->hasProjectionMatrixChanged = true;

	pvr::CameraInterfaceImpl::activeSession->width = width;
	pvr::CameraInterfaceImpl::activeSession->height= height;

	env->ReleaseFloatArrayElements(ptr, flt1, 0);

	return true;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	Log(LogLevel::Debug, "PVRCamera::JNI_OnLoad executing");
	JNIEnv* env = 0;

	if ((vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) || (env == 0))
	{
		Log(LogLevel::Debug, "CameraInterface - NativeGetEnv failed");
		return -1;
	}

	return JNI_VERSION_1_6;
}



//!\endcond
