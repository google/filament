/*!
\brief A smart pointer implementation very close to the spec of std::shared_ptr but with some differences and
tweaks to make it more suitable for the PowerVR Framework.
Note that RefCounted.h has now been made deprecated and is unused throughout the PowerVR SDK.
\file PVRCore/RefCounted.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <type_traits>
#include <stdexcept>

#ifdef debug_assertion
#define DEBUG_ASSERTION_DEFINED = 1
#else
#ifdef DEBUG
/// <summary>Provides a debug build only std::runtime_error when a given condition occurs with a particular message.</summary>
#define debug_assertion(condition, message) \
	do \
	{ \
		if (!(condition)) { throw std::runtime_error(message); } \
	} while (false);
#else
/// <summary>Provides a debug build only std::runtime_error when a given condition occurs with a particular message.</summary>
#define debug_assertion(condition, message)
#endif
#endif

namespace pvr {
template<typename>
class EmbeddedRefCountedResource;
template<typename>
class RefCountedResource;
template<typename>
class RefCountedWeakReference;
template<typename>
class EmbeddedRefCount;

/// <summary>An interface that represents a block of memory that will be doing the bookkeeping for an object that
/// will be Reference Counted. This bit of memory holds the reference counts.</summary>
struct IRefCountEntry
{
	template<typename>
	friend class RefCountedResource;
	template<typename>
	friend class EmbeddedRefCountedResource;
	template<typename>
	friend class RefCountedWeakReference;
	template<typename>
	friend class EmbeddedRefCount;

private:
	mutable volatile std::atomic<int32_t> count_; //!< Number of total references for this object
	mutable volatile std::atomic<int32_t> weakcount_; //!< Number of weak references for this object
	std::mutex mylock;

public:
	int32_t count() { return count_; } //!< Number of total references for this object
	int32_t weakcount() { return weakcount_; } //!< Number of weak references for this object

protected:
	/// <summary>Increment strong references by one. It is an error to call on a deleted object.</summary>
	void increment_count()
	{
		if (std::atomic_fetch_add(&count_, 1) == 0)
		{
			std::lock_guard<std::mutex> lockguard(mylock);
			if (count_.load() == 0) { throw std::runtime_error("RefCounted::increment_count:  Tried to increment the count of an object but it had already been destroyed!"); }
		}
	}
	/// <summary>Increment weak references by one. It is an error to call on a deleted object.</summary>
	void increment_weakcount()
	{
		if (std::atomic_fetch_add(&weakcount_, 1) == 0)
		{
			std::lock_guard<std::mutex> lockguard(mylock);
			if (weakcount_.load() == 0) { throw std::runtime_error("RefCounted::increment_count:  Tried to increment the count of an object but it had already been destroyed!"); }
		}
	}

	/// <summary>Decrement strong reference count by one.Will destroy the object when the strong reference count reaches zero, and
	/// additionally the bookkeeping entry if the weak reference count also reaches zero. It is an error to call on a deleted object.</summary>
	void decrement_count()
	{
		bool deleter = false; // Track, if it was THIS object that was the one to do the deletion!
		int32_t cnt = 0;
		{
			if ((cnt = std::atomic_fetch_add(&count_, -1)) == 1)
			{
				std::lock_guard<std::mutex> lockguard(mylock);
				destroyObject();
				if (weakcount() == 0) { deleter = true; }
			}
		}
		if (deleter) { deleteEntry(); }
	}

	/// <summary>Decrement weak references by one. If it reaches zero and the strong reference count is also zero, will destroy the bookkeeping entry.
	/// It is an error to call on a deleted object.</summary>
	void decrement_weakcount()
	{
		bool deleter = false; // DO NOT DESTROY THE OBJECT WHILE THE MUTEX IS USED!
		if (((std::atomic_fetch_add(&weakcount_, -1)) == 1))
		{
			std::lock_guard<std::mutex> lockguard(mylock);
			if (count_.load() == 0) { deleter = true; }
		}
		if (deleter) { deleteEntry(); }
	} //!< Number of weak references for this object

	virtual void deleteEntry() = 0; //!< Will be overriden with the actual code required to delete the bookkeeping entry
	virtual void destroyObject() = 0; //!< Will be overriden with the actual code required to delete the object
	virtual ~IRefCountEntry() {} //!< Will be overriden with the actual code required to delete the object
	IRefCountEntry() : count_(1), weakcount_(0) {}
};

/// <summary>DO NOT USE DIRECTLY. The RefCountedResource uses this class when required. An Intrusive Refcount
/// entry stores the object on the same block of memory as the reference counts.</summary>
/// <typeparam name="MyClass_">The type of reference counted object.</typeparam>
/// <remarks>This class is used to store the generated object on the same block of memory as the reference counts
/// bookkeeping giving much better memory coherency. It is created when the "construct" method is used to create a
/// new object inside a smart pointer. This is a much better solution than creating the object yourself and then
/// just wrapping the pointer you have with a RefCountEntry, as if you use the intrusive class you save one level
/// of indirection. This class is used when the user does RefCountEntry<MyClass> entry; entry.construct(...);</remarks>
template<typename MyClass_>
struct RefCountEntryIntrusive : public IRefCountEntry
{
	union
	{
		double _alignment_; //!< Unused, value irrelevent. Used to 4-char align the following char array.
		char entry[sizeof(MyClass_)]; //!< Represents enough bytes of memory to store a MyClass_ object.
	};
	/// <summary>Construct a new RefCountEntryIntrusive and the contained object. One Parameter constructor.</summary>
	/// <param name="pointee">After returning, pointee will contain the address of the newly constructed object</param>
	/// <param name="args">The parameters that will be passed as MyClass_'s constructor arguments</param>
	/// <typeparam name="Args">The types of the arguments required for MyClass_'s constructor.
	/// </typeparam>
	/// <remarks>This constructor is called when the user calls construct(). Forwards to any of MyClass_'s
	/// constructor based on the arguments. Creates the class in place on our block of memory.</remarks>
	template<typename... Args>
	RefCountEntryIntrusive(MyClass_*& pointee, Args&&... args)
	{
		pointee = new (entry) MyClass_(std::forward<Args>(args)...);
	}
	/// <summary>Destroys the RefcountEntryIntrusive object (entry and counters). Called when all references (count + weak
	/// count) are 0. It assumes the object has already been destroyed - so it will NOT destroy the object properly -
	/// only free its memory! destroyObject must be explicitly called otherwise undefined behaviour occurs by freeing
	/// the memory of a live object!</summary>
	void deleteEntry() { delete this; }

	/// <summary>Destroys the the held object. Called when all strong references are dead (count is zero).</summary>
	void destroyObject() { reinterpret_cast<MyClass_*>(entry)->~MyClass_(); }
};

/// <summary>DO NOT USE DIRECTLY. The RefCountedResource uses this class when required. A "simple" Refcount entry
/// keeps a user-provided pointer to the object together with the reference counts.</summary>
/// <typeparam name="MyClass_">The type of the pointer kept.</typeparam>
/// <remarks>This class is used to store a user-provided pointer with the reference counts bookkeeping. This class
/// is used when instead of calling "RefCountedResource::construct", the user provides a pointer to his object
/// directly. This is worse than construct because it has worse memory coherency (the object lives "far" from the
/// refcount information). The best solution is to use "construct", which creates a new object on a
/// RefCountEntryIntrusive. On destruction, calls "delete" on the pointer.</remarks>
template<typename MyClass_>
struct RefCountEntry : public IRefCountEntry
{
	MyClass_* ptr; //!< Pointer to the object
	RefCountEntry() : ptr(NULL) {}
	RefCountEntry(MyClass_* ptr) : ptr(ptr) {}
	void deleteEntry() { delete this; }
	void destroyObject() { delete ptr; }
};

/// <summary>DO NOT USE DIRECTLY. The RefCountedResource follows this class.</summary>
/// <typeparam name="MyClass_">The type of the pointer kept.</typeparam>
/// <remarks>This class stores a pointer alias to the object to avoid the double-indirection of the RefCountEntry.
/// Broken off to make specialising for the MyClass_ = void easier.</remarks>
template<typename MyClass_>
class Dereferenceable
{
protected:
	MyClass_* _pointee;
	Dereferenceable(const MyClass_* pointee) { this->_pointee = const_cast<MyClass_*>(pointee); }

public:
	/// <summary>Dereferencing operator. Returns the contained object.</summary>
	/// <returns>The contained object.</remarks>
	MyClass_& operator*()
	{
		debug_assertion(_pointee != 0, "Dereferencing NULL pointer");
		return *_pointee;
	}
	/// <summary>Dereferencing operator. Returns the contained object.</summary>
	/// <returns>The contained object.</remarks>
	const MyClass_& operator*() const
	{
		debug_assertion(_pointee != 0, "Dereferencing NULL pointer");
		return *_pointee;
	}

	/// <summary>"Arrow" dereferencing operator. Returns the pointer to the contained object and dereferences it.</summary>
	/// <returns>The contained object.</remarks>
	MyClass_* operator->()
	{
		debug_assertion(_pointee != 0, "Dereferencing NULL pointer");
		return _pointee;
	}
	/// <summary>"Arrow" dereferencing operator. Returns the pointer to the contained object and dereferences it.</summary>
	/// <returns>The contained object.</remarks>
	const MyClass_* operator->() const
	{
		debug_assertion(_pointee != 0, "Dereferencing NULL pointer");
		return _pointee;
	}

	/// <summary>Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the two RefCountedResources contain equal pointers, otherwise false</param>
	bool operator==(const Dereferenceable& rhs) const { return _pointee == rhs._pointee; }

	/// <summary>Tests inequality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the two RefCountedResources contain unequal pointers, otherwise false</param>
	bool operator!=(const Dereferenceable& rhs) const { return _pointee != rhs._pointee; }

	/// <summary>Tests inequality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the pointer to left-hand-side is less than the pointer of right-hand-side, otherwise false</param>
	bool operator<(const Dereferenceable& rhs) const { return _pointee < rhs._pointee; }

	/// <summary>Tests inequality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the pointer to left-hand-side is greater than the pointer of right-hand-side, otherwise false</param>
	bool operator>(const Dereferenceable& rhs) const { return _pointee > rhs._pointee; }

	/// <summary>Tests inequality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the pointer to left-hand-side is less than or equal the pointer of right-hand-side, otherwise false</param>
	bool operator<=(const Dereferenceable& rhs) const { return _pointee <= rhs._pointee; }

	/// <summary>Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the pointer to left-hand-side is greater than or equal the pointer of right-hand-side, otherwise false</param>
	bool operator>=(const Dereferenceable& rhs) const { return _pointee >= rhs._pointee; }
};
//!\cond NO_DOXYGEN
template<>
class Dereferenceable<void>
{
protected:
	Dereferenceable(void* pointee) : _pointee(pointee) {}
	void* _pointee;

public:
	/// <summary>Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	bool operator==(const Dereferenceable& rhs) const { return _pointee == rhs._pointee; }

	/// <summary>Tests inequality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the pointer to left-hand-side is not equal to the pointer of right-hand-side, otherwise false</param>
	bool operator!=(const Dereferenceable& rhs) const { return _pointee != rhs._pointee; }

	/// <summary>Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the pointer to left-hand-side is less than the pointer of right-hand-side, otherwise false</param>
	bool operator<(const Dereferenceable& rhs) const { return _pointee < rhs._pointee; }

	/// <summary>Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the pointer to left-hand-side is greater than the pointer of right-hand-side, otherwise false</param>
	bool operator>(const Dereferenceable& rhs) const { return _pointee > rhs._pointee; }

	/// <summary>Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the pointer to left-hand-side is less than or equal to the pointer of right-hand-side, otherwise false</param>
	bool operator<=(const Dereferenceable& rhs) const { return _pointee <= rhs._pointee; }

	/// <summary>Tests equality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	/// <param name="rhs">Right hand side of the operator</param>
	/// <returns>True if the pointer to left-hand-side is greater than or equal to the pointer of right-hand-side, otherwise false</param>
	bool operator>=(const Dereferenceable& rhs) const { return _pointee >= rhs._pointee; }
};

template<typename MyClass_>
struct RefcountEntryHolder
{
	template<typename>
	friend class RefCountedResource;
	template<typename>
	friend class EmbeddedRefCountedResource;

protected:
	// Holding the actual entry (pointers, objects and everything). Use directly in the subclass.
	IRefCountEntry* refCountEntry;

	// Create a new intrusive refcount entry, and on it, construct a new object with its constructor.
	template<typename... Args>
	void construct(MyClass_*& pointee, Args&&... args)
	{
		RefCountEntryIntrusive<MyClass_>* ptr = new RefCountEntryIntrusive<MyClass_>(pointee, std::forward<Args>(args)...);
		refCountEntry = ptr;
	}

	// Create this entry with an already constructed reference-counted entry.
	RefcountEntryHolder(IRefCountEntry* refCountEntry) : refCountEntry(refCountEntry) {}
};

template<>
struct RefcountEntryHolder<void>
{
protected:
	IRefCountEntry* refCountEntry;
	RefcountEntryHolder(IRefCountEntry* refCountEntry) : refCountEntry(refCountEntry) {}
};
//!\endcond

/// <summary>"Embedded" subset of the Reference counted smart pointer. See RefCountedResource. It can be used very
/// similarly to the C++11 shared_ptr, but with some differences, especially as far as conversions go. The
/// EmbeddedRefCountedResource is used when the user must not have access to the .construct(...) functions of the
/// RefCountedResource (especially, when a class is designed around reference counting and cannot stand widthout
/// it). A RefCountedResource is-a EmbeddedRefCountedResource and is safe to "slice" to it as well (see details).</summary>
/// <typeparam name="MyClass_">The type of object held. If a createNew function returning an EmbeddedRefCounted is
/// called, this must be the type of the object that will be created.</typeparam>
/// <remarks>In order for objects of a class to be used as EmbeddedRefCountedResource, it will need to inherit
/// from EmbeddedRefCounted<Class>, and then implement destroyObject (a function that conceptually "clears" an
/// object by freeing its resources, without freeing the memory of the object itself - like a custom destructor),
/// and MyClass::createNew() static function (or other factory function that forwards arguments to the correct
/// EmbeddedRefCount::createNew overloads, one for each "constructor" that needs to be exposed). Also see
/// RefCountedResource.</remarks>
template<typename MyClass_>
class EmbeddedRefCountedResource : public Dereferenceable<MyClass_>, public RefcountEntryHolder<MyClass_>
{
	template<typename>
	friend class EmbeddedRefCount;
	template<typename>
	friend class RefCountedResource;
	template<typename>
	friend class EmbeddedRefCountedResource;
	template<typename>
	friend class RefCountedWeakReference;

public:
	/// <summary>Type of the object that this object contains. Can be a superclass of the actual type of the object.</summary>
	typedef MyClass_ ElementType;

	/// <summary>Check if the object contains a reference to a non-null object (i.e. is safely dereferenceable). Will
	/// return false for both unallocated strong or weak references, or weak references that point to objects without
	/// strong references (destroyed). Equivalent to !isNull().</summary>
	/// <returns>True if this object contains a reference to a non-null object (is safely dereferenceable).</returns>
	bool isValid() const { return RefcountEntryHolder<MyClass_>::refCountEntry != NULL && RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0; }

	/// <summary>Check if the object does not contains a reference to a non-null object (i.e. is safely
	/// dereferenceable). Will return true for both unallocated strong or weak references, or weak references that
	/// point to objects without strong references (destroyed). Equivalent to !isValid().</summary>
	/// <returns>True if this object does not contain a reference to a non-null object (is not safely dereferenceable).</returns>
	bool isNull() const { return RefcountEntryHolder<MyClass_>::refCountEntry == NULL || RefcountEntryHolder<MyClass_>::refCountEntry->count() == 0; }

	/// <summary>Get a raw pointer to the pointed-to object.</summary>
	/// <returns>A raw pointer to the pointed-to object.</returns>
	MyClass_* get() { return Dereferenceable<MyClass_>::_pointee; }

	/// <summary>Get a const raw pointer to the pointed-to object.</summary>
	/// <returns>A const raw pointer to the pointed-to object.</returns>
	const MyClass_* get() const { return Dereferenceable<MyClass_>::_pointee; }

	/// <summary>Swap the contents of this RefCountedResource object with another RefCountedResource object of the same
	/// type.</summary>
	/// <param name="rhs">The object to swap the contents with.</param>
	void swap(EmbeddedRefCountedResource& rhs)
	{
		IRefCountEntry* tmpRefCountEntry = RefcountEntryHolder<MyClass_>::refCountEntry;
		RefcountEntryHolder<MyClass_>::refCountEntry = rhs.refCountEntry;
		rhs.refCountEntry = tmpRefCountEntry;
		MyClass_* tmpPointee = Dereferenceable<MyClass_>::_pointee;
		Dereferenceable<MyClass_>::_pointee = rhs._pointee;
		rhs._pointee = tmpPointee;
	}

	/// <summary>Default constructor. Points to NULL.</summary>
	EmbeddedRefCountedResource() : Dereferenceable<MyClass_>(NULL), RefcountEntryHolder<MyClass_>(0) {}

	/// <summary>Copy constructor (points to the same object, increments reference count).</summary>
	/// <param name="rhs">The object to copy the reference from.</param>
	/// <remarks>The copy constructor and copy assignment operators are the main point of any smart resource class.
	/// The copy constructor-created object references the same object, and increments the reference-counting by one.</remarks>
	EmbeddedRefCountedResource(const EmbeddedRefCountedResource& rhs) : Dereferenceable<MyClass_>(rhs), RefcountEntryHolder<MyClass_>(rhs)
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_count();
			debug_assertion((RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0) && (RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0), "BUG - Count was negative.");
		}
	}

	/// <summary>Wrap an already existing object with a RefCountedResource. Will call delete on a MyPointer_ type when
	/// the reference count reaches zero.</summary>
	/// <param name="ref">The pointer to wrap into the RefCountedResource. Will be deleted when refcount reaches zero.</param>
	/// <typeparam name="MyPointer_">the type of the class pointed to by the pointer. May be a subclass of MyClass_,
	/// but be careful as delete will be called on this type, so it either needs to be the real type, or the base
	/// class needs a virtual destructor.</typeparam>
	template<typename MyPointer_>
	explicit EmbeddedRefCountedResource(MyPointer_* ref) : Dereferenceable<MyClass_>(ref), RefcountEntryHolder<MyClass_>(new RefCountEntry<MyPointer_>(ref))
	{
		RefcountEntryHolder<MyClass_>::refCountEntry->count_ = 1;
	}

	/// <summary>Copy Assignment operator. Increments reference count.</summary>
	/// <param name="rhs">The right hand side of the assignment. After the operation, the objects on both sides will
	/// be pointing to the same object and the reference counting will be increased.</param>
	/// <remarks>"this" (the left-hand-side of the operator) object will be released (if not NULL, reference count will
	/// be decreased as normal and, if zero, the object will be deleted). Reference count of the right-hand-side
	/// object will be increased and this RefCountedResource will be pointing to it. Utilises copy-and-swap, so
	/// self-assign is safe.</remarks>
	EmbeddedRefCountedResource& operator=(EmbeddedRefCountedResource rhs)
	{
		swap(rhs);
		debug_assertion(!RefcountEntryHolder<MyClass_>::refCountEntry ||
				(RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0 && RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0),
			"BUG - Count was negative.");
		return *this;
	}

	/// <summary>Implicit Copy Conversion constructor.</summary>
	/// <param name="rhs">The object to convert</param>
	/// <remarks>A RefCountedResource is implicitly convertible to any type that its current ElementType is
	/// implicitly convertible to. For example, a pointer to a subclass is implicitly convertible to a pointer to
	/// superclass. Reference counting keeps working normally on the original object regardless of the type of the
	/// pointer.</remarks>
	/// <remarks>The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and
	/// is not used.</remarks>
	template<class OldType_>
	EmbeddedRefCountedResource(const EmbeddedRefCountedResource<OldType_>& rhs, typename std::enable_if<std::is_convertible<OldType_*, MyClass_*>::value, void*>::type* = 0)
		: Dereferenceable<MyClass_>(rhs._pointee), RefcountEntryHolder<MyClass_>(rhs.refCountEntry)
	{
		// Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<OldType_*>(Dereferenceable<MyClass_>::_pointee);
		(void)checkMe;
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			debug_assertion((RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0) && (RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0), "BUG - Count was negative.");
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_count();
			debug_assertion((RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0) && (RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0), "BUG - Count was negative.");
		}
	}

	/// <summary>Explicit Copy Conversion constructor.</summary>
	/// <param name="rhs">The object to convert</param>
	/// <remarks>A RefCountedResource is explicitly convertible to any type that its current ElementType is
	/// explicitly convertible to. For example, a pointer to void is explicitly convertible to a pointer to any class.
	/// Conversion used is static_cast. Reference counting keeps working normally on the original object regardless of
	/// the type of the pointer.</remarks>
	/// <remarks>The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and
	/// is not used.</remarks>
	template<class OldType_>
	explicit EmbeddedRefCountedResource(const EmbeddedRefCountedResource<OldType_>& rhs, typename std::enable_if<!std::is_convertible<OldType_*, MyClass_*>::value, void*>::type* = 0)
		: Dereferenceable<MyClass_>(static_cast<MyClass_*>(rhs._pointee)), RefcountEntryHolder<MyClass_>(rhs.refCountEntry)
	{
		// Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<MyClass_*>(Dereferenceable<MyClass_>::_pointee);
		(void)checkMe;
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			debug_assertion((RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0) && (RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0), "BUG - Count was negative.");
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_count();
			debug_assertion((RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0) && (RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0), "BUG - Count was negative.");
		}
	}

	/// <summary>Destructor. Releases the object (decreases reference count and deletes it if zero).</summary>
	virtual ~EmbeddedRefCountedResource() { reset(); }

	/// <summary>Tests inequality of the pointers of two compatible RefCountedResource. NULL tests equal to NULL.</summary>
	bool operator!=(const EmbeddedRefCountedResource& rhs) const { return get() != rhs.get(); }

	/// <summary>Decrements reference count. If it is the last pointer, destroys the pointed-to object. Else, abandons it.
	/// Then, wraps the provided pointer and points to it. Equivalent to (but more efficient than) destroying the
	/// original smart pointer and creating a new one.</summary>
	/// <param name="ref">The new pointer to wrap.</param>
	/// <typeparam name="MyPointer_">The type of the new pointer to wrap. Must be implicitly convertible to MyClass_.
	/// </typeparam>
	template<typename MyPointer_>
	void reset(MyPointer_* ref)
	{
		reset();
		RefcountEntryHolder<MyClass_>::refCountEntry = new RefCountEntry<MyPointer_>(ref);
		Dereferenceable<MyClass_>::_pointee = ref;
	}

	/// <summary>Decrements reference count. If it is the last pointer, destroys the pointed-to object. Else, abandons it.
	/// Then, resets to NULL.</summary>
	void reset() { releaseOne(); }

protected:
	template<typename OriginalType>
	EmbeddedRefCountedResource(IRefCountEntry* entry, OriginalType* pointee) : Dereferenceable<MyClass_>(static_cast<MyClass_*>(pointee)), RefcountEntryHolder<MyClass_>(entry)
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			debug_assertion((RefcountEntryHolder<MyClass_>::refCountEntry->count() >= 0) && (RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0), "BUG - Count was negative.");
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_count();
			debug_assertion((RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0) && (RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0), "BUG - Count was negative.");
		}
	}

private:
	void releaseOne()
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			IRefCountEntry* tmp = RefcountEntryHolder<MyClass_>::refCountEntry;
			RefcountEntryHolder<MyClass_>::refCountEntry = 0;
			Dereferenceable<MyClass_>::_pointee = 0;
			tmp->decrement_count();
		}
	}
};

/// <summary>Reference counted smart pointer. It can be used very similarly to the C++11 shared_ptr, but with some
/// differences.</summary>
/// <typeparam name="MyClass_">The type of object held. If .construct(...) is called, this is the type of the
/// object that will be created.</typeparam>
/// <remarks>This reference counted smart resource will keep track of how many references to a specific object
/// exist, and release it when this is zero. Use its .construct(...) method overloads to efficiently construct an
/// object together with its ref-counting bookkeeping information. If this is not practical, you can also assign an
/// already created pointer to it, as long as it is safe to call "delete" on it. Copy construction, move and
/// assignment all work as expected and can freely be used. It can very easily be used polymorphically. It is
/// intended that this class will mainly be used with the .construct() method. If .construct() is used to create
/// the object held (as opposed to wrapping a pre-existing pointer)</remarks>
template<typename MyClass_>
class RefCountedResource : public EmbeddedRefCountedResource<MyClass_>
{
	template<typename>
	friend class EmbeddedRefCount;
	template<typename>
	friend class RefCountedResource;
	template<typename>
	friend class EmbeddedRefCountedResource;
	template<typename>
	friend class RefCountedWeakReference;

public:
	typedef typename EmbeddedRefCountedResource<MyClass_>::ElementType ElementType;

	/// <summary>Swap the contents of this RefCountedResource object with another RefCountedResource object of the same
	/// type.</summary>
	void swap(RefCountedResource& rhs)
	{
		IRefCountEntry* tmpRefCountEntry = RefcountEntryHolder<MyClass_>::refCountEntry;
		RefcountEntryHolder<MyClass_>::refCountEntry = rhs.refCountEntry;
		rhs.refCountEntry = tmpRefCountEntry;
		MyClass_* tmpPointee = Dereferenceable<MyClass_>::_pointee;
		Dereferenceable<MyClass_>::_pointee = rhs._pointee;
		rhs._pointee = tmpPointee;
	}

	/// <summary>Default constructor. Points to NULL.</summary>
	RefCountedResource() {}

	/// <summary>Copy constructor. Increments reference count.</summary>
	RefCountedResource(const RefCountedResource& rhs) : EmbeddedRefCountedResource<MyClass_>(rhs) {}

	/// <summary>Wrap an already existing object with a RefCountedResource.</summary>
	/// <summary>Wrap an already existing object with a RefCountedResource. Will call delete on a MyPointer_ type when
	/// the reference count reaches zero. MyPointer_ may be MyClass_ or a subclass of MyClass_. delete MyPointer_ will
	/// happen, not MyClass_.</summary>
	/// <param name="ref">The pointer to wrap into the RefCountedResource. Will be deleted when refcount reaches zero.</param>
	/// <typeparam name="MyPointer_">the type of the class pointed to by the pointer. May be a subclass of MyClass_.
	/// </typeparam>
	template<typename MyPointer_>
	explicit RefCountedResource(MyPointer_* ref) : EmbeddedRefCountedResource<MyClass_>(ref)
	{
		RefcountEntryHolder<MyClass_>::refCountEntry->count_ = 1;
	}

	/// <summary>Copy Assignment operator. Increments reference count.</summary>
	/// <remarks>"this" (the left-hand-side of the operator) object will be released (if not NULL, reference count will
	/// be decreased as normal and, if zero, the object will be deleted). Reference count of the right-hand-side
	/// object will be increased and this RefCountedResource will be pointing to it.</remarks>
	RefCountedResource& operator=(RefCountedResource rhs)
	{
		swap(rhs);
		return *this;
	}

	/// <summary>Implicit Copy Conversion constructor.</summary>
	/// <param name="rhs">The object to convert</param>
	/// <remarks>A RefCountedResource is implicitly convertible to any type that its current ElementType is
	/// implicitly convertible to. For example, a pointer to a subclass is implicitly convertible to a pointer to
	/// superclass. Reference counting keeps working normally on the original object regardless of the type of the
	/// pointer.</remarks>
	/// <remarks>The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and
	/// is not used.</remarks>
	template<class OldType_>
	RefCountedResource(const RefCountedResource<OldType_>& rhs, typename std::enable_if<std::is_convertible<OldType_*, MyClass_*>::value, void*>::type* t = 0)
		: EmbeddedRefCountedResource<MyClass_>(rhs, t)
	{
		// Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<OldType_*>(Dereferenceable<MyClass_>::_pointee);
		(void)checkMe;
	}

	/// <summary>Explicit Copy Conversion constructor.</summary>
	/// <param name="rhs">The object to convert</param>
	/// <remarks>A RefCountedResource is explicitly convertible to any type that its current ElementType is
	/// explicitly convertible to. For example, a pointer to void is explicitly convertible to a pointer to any class.
	/// Conversion used is static_cast. Reference counting keeps working normally on the original object regardless of
	/// the type of the pointer.</remarks>
	/// <remarks>The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and
	/// is not used.</remarks>
	template<class OldType_>
	explicit RefCountedResource(const RefCountedResource<OldType_>& rhs, typename std::enable_if<!std::is_convertible<OldType_*, MyClass_*>::value, void*>::type* t = 0)
		: EmbeddedRefCountedResource<MyClass_>(rhs, t)
	{
		// Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<MyClass_*>(Dereferenceable<MyClass_>::_pointee);
		(void)checkMe;
	}

	/// <summary>Destructor. Releases the object (decreases reference count and deletes it if zero).</summary>
	virtual ~RefCountedResource()
	{ /*release called from EmbeddedRefCountedResource*/
	}

	/// <summary>Decrements reference count. If it is the last pointer, destroys the pointed-to object. Else, abandons it.
	/// Then, wraps the provided pointer and points to it. Equivalent to destroying the original smart pointer and
	/// creating a new one.</summary>
	/// <param name="ref">The new pointer to wrap.</param>
	/// <typeparam name="MyPointer_">The type of the new pointer to wrap. Must be implicitly convertible to MyClass_.
	/// </typeparam>
	template<typename MyPointer_>
	void reset(MyPointer_* ref)
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
		RefcountEntryHolder<MyClass_>::refCountEntry = new RefCountEntry<MyPointer_>(ref);
		Dereferenceable<MyClass_>::_pointee = ref;
	}

	/// <summary>Decrements reference count. If it is the last pointer, destroys the pointed-to object. Else, abandons it.</summary>
	void reset() { EmbeddedRefCountedResource<MyClass_>::reset(); }

	/// <summary>-- Use this method to construct a new instance of ElementType using its one-parameter constructor,
	/// and use reference counting with it. --- ONE ARGUMENT CONSTRUCTOR OVERLOAD</summary>
	/// <param name="params">The arguments that will be forwarded to MyClass_'s constructor</param>
	/// <typeparam name="ParamType">The types of the arguments accepted by MyClass_'s constructor</typeparam>
	/// <remarks>Use this method or its overloads to create a new object in the most efficient way possible and wrap
	/// it in a RefCountedResource object. Use this whenever possible instead of wrapping a user-provided pointer in
	/// order to get the best memory locality, as this method will create both the object and the necessary reference
	/// counters in one block of memory. If an object is already owned by this object, it will first be properly
	/// released before constructing the new one.</remarks>
	template<typename... Params>
	void construct(Params&&... params)
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
		RefcountEntryHolder<MyClass_>::construct(Dereferenceable<MyClass_>::_pointee, std::forward<Params>(params)...);
	}

	/// <summary>Use this function to share the refcounting between two unrelated classes that share lifetime (for
	/// example, the node of a read-only list). This function will use the "parent" objects' entry for the child
	/// object, so that the reference count to the child object will keep the parent alive.</summary>
	/// <remarks>WARNING. This function does NOT cause lifetime dependencies, it only EXPRESSES them if they already exist.
	/// i.e. The main purpose of this method is to allow the user to express hierarchical lifetime relationships
	/// between objects. For example, if we had a RefCountedResource<std::vector<pvr::assets::Model>> (that is never
	/// modified), this method would allow us to get smart resources to the Mesh objects of the models, with shared
	/// refcounting to the vector object, and not keep the original RefCountedResource at all, allowing the vector to
	/// be destroyed automatically when no references to its Meshes exist anymore - the Meshes would all be destroyed
	/// together as normal when the vector was destroyed, in turn deleting the Models, in turn deleting the Meshes.
	/// WARNING: All that this method does is make sure that as long as pointers to children exist, the parent object
	/// stays alive. If the parent objects are in fact unrelated (For example, have a Mesh share refcounting to a
	/// std::vector<Mesh> that does NOT contain it, then a) The object will not be properly destroyed when its
	/// reference is released, and b1) Potentially result in a memory leak if the object actually needed to be
	/// released an/or b2) Cause undefined behaviour when the object was released by its "normal" lifetime, since the
	/// shared resource was NOT really keeping it alive.</remarks>
	template<typename OriginalType>
	void shareRefCountFrom(const RefCountedResource<OriginalType>& resource, typename EmbeddedRefCountedResource<MyClass_>::ElementType* pointee)
	{
		shareRefCountingFrom(RefcountEntryHolder<OriginalType>(resource).refCountEntry, pointee);
	}

private:
	void shareRefCountingFrom(IRefCountEntry* entry, ElementType* pointee)
	{
		EmbeddedRefCountedResource<MyClass_>::reset();
		RefcountEntryHolder<ElementType>::refCountEntry = entry;
		entry->increment_count();
		debug_assertion((RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0) && (RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() >= 0), "BUG - Count was negative.");
		Dereferenceable<ElementType>::_pointee = pointee;
	}

	template<typename OriginalType>
	RefCountedResource(const IRefCountEntry* entry, OriginalType* pointee) : EmbeddedRefCountedResource<MyClass_>(entry, pointee)
	{}
};

/// <summary>A RefCountedWeakReference is a "Weak Reference" to a Reference counted object.</summary>
/// <typeparam name="MyClass_">The type of the element that this RefCountedWeakReference points to. Can be
/// polymorphic.</typeparam>
/// <remarks>Weak references are the same as a normal RefCountedResource with a few key differences: 1) They cannot
/// keep the object alive. If an object only has weak references pointing to it, it is destroyed. 2) Weak reference
/// can still be safely queried to see if their object is "alive" (i.e. has strong references pointing to it) 3)
/// You cannot .construct() an object on a weak reference, only strong references. Weak references are used to
/// avoid Cyclic Dependencies which would sometimes make objects undeleteable and hanging even when no application
/// references exist to them (If "A" holds a reference to "B" and "B" holds a reference to "A" then even when the
/// user abandons all references to A and B the objects will be dangling in memory, keeping each other alive). In
/// those cases one object must hold a weak reference to the other ("A" holds a reference to "B" but "B" holds a
/// weak reference to "A", then when the last reference to "A" is abandoned from the application, "A" will be
/// deleted, freeing the reference to "B" and allowing it to also be deleted. Can only be created from an already
/// existing RefCountedResource or another RefCountedWeakReference.</remarks>
template<typename MyClass_>
class RefCountedWeakReference : public Dereferenceable<MyClass_>, public RefcountEntryHolder<MyClass_>
{
	template<typename>
	friend class EmbeddedRefCount;
	template<typename>
	friend class RefCountedResource;
	template<typename>
	friend class EmbeddedRefCountedResource;
	template<typename>
	friend class RefCountedWeakReference;

public:
	typedef MyClass_ ElementType; //!< The type of the element pointed to. Can be polymorphic.

	/// <summary>Test if this reference points to a valid object.</summary>
	/// <returns>True if this reference points to a valid object.</returns>
	bool isValid() const { return RefcountEntryHolder<MyClass_>::refCountEntry != NULL && RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0; }

	/// <summary>Test if this reference does not point to a valid object.</summary>
	/// <returns>True if this reference does not points to a valid object.</returns>
	bool isNull() const { return RefcountEntryHolder<MyClass_>::refCountEntry == NULL || RefcountEntryHolder<MyClass_>::refCountEntry->count() == 0; }

	/// <summary>Get a raw pointer to the pointed-to object.</summary>
	/// <returns>Get a raw pointer of MyClass_* type to the pointed-to object.</returns>
	MyClass_* get() { return Dereferenceable<MyClass_>::_pointee; }

	/// <summary>Get a const raw pointer to the pointed-to object.</summary>
	/// <returns>Get a const raw pointer of MyClass_* type to the pointed-to object.</returns>
	const MyClass_* get() const { return Dereferenceable<MyClass_>::_pointee; }

	void swap(RefCountedWeakReference& rhs)
	{
		IRefCountEntry* tmpRefCountEntry = RefcountEntryHolder<MyClass_>::refCountEntry;
		RefcountEntryHolder<MyClass_>::refCountEntry = rhs.refCountEntry;
		rhs.refCountEntry = tmpRefCountEntry;
		MyClass_* tmpPointee = Dereferenceable<MyClass_>::_pointee;
		Dereferenceable<MyClass_>::_pointee = rhs._pointee;
		rhs._pointee = tmpPointee;
	}

	/// <summary>Default constructor. Constructed object points to NULL.</summary>
	RefCountedWeakReference() : Dereferenceable<MyClass_>(NULL), RefcountEntryHolder<MyClass_>(0) {}

	/// <summary>Copy constructor. Implements normal reference counting (increases Weak reference count).</summary>
	RefCountedWeakReference(const RefCountedWeakReference& rhs) : Dereferenceable<MyClass_>(rhs), RefcountEntryHolder<MyClass_>(rhs)
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "BUG - Reference count was nonpositive.");
		}
	}

	/// <summary>Copy conversion constructor from Normal (non-weak) reference. Implements normal reference counting
	/// (increases Weak reference count).</summary>
	RefCountedWeakReference(const EmbeddedRefCountedResource<MyClass_>& rhs) : Dereferenceable<MyClass_>(rhs), RefcountEntryHolder<MyClass_>(rhs)
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}

	/// <summary>Copy assignment operator. Implements normal reference counting (increases Weak reference count).</summary>
	RefCountedWeakReference& operator=(RefCountedWeakReference rhs)
	{
		swap(rhs);
		return *this;
	}

	/// <summary>Implicit Copy Conversion operator. Converts to this type any RefCountedWeakReference to a type that is
	/// implicitly convertible to MyClass_.</summary>
	/// <param name="rhs">The object to convert</param>
	/// <typeparam name="OldType_">The type that the right hand side wraps. Must be implicitly convertible to MyType_
	/// </typeparam>
	/// <remarks>The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and
	/// is not used.</remarks>
	template<class OldType_>
	RefCountedWeakReference(const EmbeddedRefCountedResource<OldType_>& rhs, typename std::enable_if<std::is_convertible<OldType_*, MyClass_*>::value, void*>::type* = 0)
		: Dereferenceable<MyClass_>(rhs._pointee), RefcountEntryHolder<MyClass_>(rhs.refCountEntry)
	{
		// Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<OldType_*>(Dereferenceable<MyClass_>::_pointee);
		(void)checkMe;
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}

	/// <summary>Explicit Copy Conversion operator. Will converts to this type any RefCountedWeakReference to a type that
	/// is explicitly convertible to MyClass_.</summary>
	/// <param name="rhs">The object to convert</param>
	/// <typeparam name="OldType_">The type that the right hand side wraps. Must be explicitly convertible
	/// (static_cast) to MyType_</typeparam>
	/// <remarks>The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and
	/// is not used.</remarks>
	template<class OldType_>
	explicit RefCountedWeakReference(const EmbeddedRefCountedResource<OldType_>& rhs, typename std::enable_if<!std::is_convertible<OldType_*, MyClass_*>::value, void*>::type* = 0)
		: Dereferenceable<MyClass_>(static_cast<MyClass_*>(rhs._pointee)), RefcountEntryHolder<MyClass_>(rhs.refCountEntry)
	{
		// Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<MyClass_*>(Dereferenceable<MyClass_>::_pointee);
		(void)checkMe;
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}

	/// <summary>Implicit Copy Conversion operator. Converts to this type any RefCountedWeakReference to a type that is
	/// implicitly convertible to MyClass_.</summary>
	/// <param name="rhs">The object to convert</param>
	/// <typeparam name="OldType_">The type that the right hand side wraps. Must be implicitly convertible to MyType_
	/// </typeparam>
	/// <remarks>The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and
	/// is not used.</remarks>
	template<class OldType_>
	RefCountedWeakReference(const RefCountedWeakReference<OldType_>& rhs, typename std::enable_if<std::is_convertible<OldType_*, MyClass_*>::value, void*>::type* = 0)
		: Dereferenceable<MyClass_>(rhs._pointee), RefcountEntryHolder<MyClass_>(rhs.refCountEntry)
	{
		// Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<OldType_*>(Dereferenceable<MyClass_>::_pointee);
		(void)checkMe;
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}

	/// <summary>Explicit Copy Conversion operator. Will converts to this type any RefCountedWeakReference to a type that
	/// is explicitly convertible to MyClass_.</summary>
	/// <param name="rhs">The object to convert</param>
	/// <typeparam name="OldType_">The type that the right hand side wraps. Must be explicitly convertible
	/// (static_cast) to MyType_</typeparam>
	/// <remarks>The last (unnamed) parameter of this function is required for template argument matching (SFINAE) and
	/// is not used.</remarks>
	template<class OldType_>
	explicit RefCountedWeakReference(const RefCountedWeakReference<OldType_>& rhs, typename std::enable_if<!std::is_convertible<OldType_*, MyClass_*>::value, void*>::type* = 0)
		: Dereferenceable<MyClass_>(static_cast<MyClass_*>(rhs._pointee)), RefcountEntryHolder<MyClass_>(rhs.refCountEntry)
	{
		// Compile-time check: are these types compatible?
		OldType_* checkMe = static_cast<MyClass_*>(Dereferenceable<MyClass_>::_pointee);
		(void)checkMe;
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}

	/// <summary>Destructor. Reduces weak references by one, freeing the bookkeeping block if total references reach 0.</summary>
	virtual ~RefCountedWeakReference() { release(); }

	/// <summary>Decrements weak reference count. If it is the last pointer, the object must have already been destroyed, but
	/// it destroys the reference counts block. Then, points to NULL.</summary>
	void release()
	{
		releaseOne();
		RefcountEntryHolder<MyClass_>::refCountEntry = 0;
		Dereferenceable<MyClass_>::_pointee = 0;
	}

	/// <summary>Decrements weak reference count. If it is the last pointer, the object must have already been destroyed, but
	/// it destroys the reference counts block. Then, points to NULL.</summary>
	void reset() { release(); }

private:
	void retainOne()
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}

	void releaseOne()
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			IRefCountEntry* tmp = RefcountEntryHolder<MyClass_>::refCountEntry;
			RefcountEntryHolder<MyClass_>::refCountEntry = 0;
			Dereferenceable<MyClass_>::_pointee = 0;
			tmp->decrement_weakcount();
		}
	}

	template<typename OriginalType>
	RefCountedWeakReference(IRefCountEntry* entry, OriginalType* pointee) : Dereferenceable<MyClass_>(static_cast<MyClass_*>(pointee)), RefcountEntryHolder<MyClass_>(entry)
	{
		if (RefcountEntryHolder<MyClass_>::refCountEntry)
		{
			RefcountEntryHolder<MyClass_>::refCountEntry->increment_weakcount();
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->weakcount() > 0, "Reference count was not positive");
			debug_assertion(RefcountEntryHolder<MyClass_>::refCountEntry->count() > 0, "Reference count was not positive");
		}
	}
};

/// <summary>EmbeddedRefCount is the class that must be inherited from in order to have a class that automatically has
/// refcounted members, with awareness and access to their ref counting by keeping their ref-counting bookkeeping
/// entries embedded in the main class</summary>
/// <typeparam name="MyClass_">The type of the class that this EmbeddedRefCount belongs to. CRTP parameter. MUST
/// be the actual class that inherits from EmbeddedRefCount (as in class MyClass: public
/// EmbeddedRefCount<MyClass>{}, even if the pointer returned to the user is ultimately of another kind.
/// </typeparam>
/// <remarks>EmbeddedRefCount has two uses: First, it is a performance optimization, for all intents and purposes identical to
/// using .construct</remarks>
template<typename MyClass_>
class EmbeddedRefCount : public IRefCountEntry
{
public:
	typedef EmbeddedRefCount<MyClass_> MyEmbeddedType;
	typedef EmbeddedRefCountedResource<MyClass_> StrongReferenceType;
	typedef RefCountedWeakReference<MyClass_> WeakReferenceType;
	WeakReferenceType getWeakReference() { return WeakReferenceType(static_cast<MyClass_*>(this), static_cast<MyClass_*>(this)); }
	StrongReferenceType getReference() { return StrongReferenceType(static_cast<MyClass_*>(this), static_cast<MyClass_*>(this)); }

protected:
	EmbeddedRefCount() {}

	template<typename... Params>
	static StrongReferenceType createNew(Params&&... params)
	{
		MyClass_* item = new MyClass_(std::forward<Params>(params)...);
		StrongReferenceType ptr(item, item);
		item->decrement_count();
		return ptr;
	}

	void deleteEntry() { delete static_cast<MyClass_*>(this); }
};

//!\cond NO_DOXYGEN
template<typename T>
struct IsRefCountedType
{
	template<typename U, void (U::*)()>
	struct SFINAE
	{};
	template<typename U>
	static char Test(SFINAE<U, &U::construct>*);
	template<typename U>
	static int Test(...);
	enum
	{
		value = (sizeof(Test<T>(0)) == sizeof(char))
	};
};
} // namespace pvr
//!\endcond
#ifdef DEBUG_ASSERTION_DEFINED
#undef DEBUG_ASSERTION_DEFINED
#else
#undef debug_assertion
#endif
