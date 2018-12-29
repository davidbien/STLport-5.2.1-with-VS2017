/*
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1997
 * Moscow Center for SPARC Technology
 *
 * Copyright (c) 1999
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef _STLP_INTERNAL_ALLOC_H
#define _STLP_INTERNAL_ALLOC_H

#ifndef _STLP_INTERNAL_CSTDDEF
#  include <stl/_cstddef.h>
#endif

#ifndef _STLP_INTERNAL_CSTDLIB
#  include <stl/_cstdlib.h>
#endif

#ifndef _STLP_INTERNAL_CSTRING
#  include <stl/_cstring.h>
#endif

#ifndef _STLP_INTERNAL_ALGOBASE_H
#  include <stl/_algobase.h>
#endif

#ifndef _STLP_INTERNAL_NEW_HEADER
#  include <stl/_new.h>
#endif

#ifndef _STLP_INTERNAL_CONSTRUCT_H
#  include <stl/_construct.h>
#endif

_STLP_BEGIN_NAMESPACE

// Malloc-based allocator.  Typically slower than default alloc below.
// Typically thread-safe and more storage efficient.

#if !defined (_STLP_USE_NO_IOSTREAMS)
typedef void (* __oom_handler_type)();
#endif

class _STLP_CLASS_DECLSPEC __malloc_alloc {
public:
	typedef __malloc_alloc _TyStaticAlloc;
  // this one is needed for proper simple_alloc wrapping
  typedef char value_type;
  typedef size_t size_type;
  static void* _STLP_CALL allocate(size_t __n)
#if !defined (_STLP_USE_NO_IOSTREAMS)
  ;
#else
  {
    void *__result = malloc(__n);
    if (__result == 0) {
      _STLP_THROW_BAD_ALLOC;
    }
    return __result;
  }
#endif

	bool operator == (__malloc_alloc const &) const
	{
		return true;
	}

  static void _STLP_CALL deallocate(void* __p, size_t /* __n */) { free((char*)__p); }
#if !defined (_STLP_USE_NO_IOSTREAMS)
  static __oom_handler_type _STLP_CALL set_malloc_handler(__oom_handler_type __f);
#endif
};

// New-based allocator.  Typically slower than default alloc below.
// Typically thread-safe and more storage efficient.
class _STLP_CLASS_DECLSPEC __new_alloc {
public:
  // this one is needed for proper simple_alloc wrapping
  typedef char value_type;
	typedef size_t size_type;
	static void* _STLP_CALL allocate(size_t __n) { return __stl_new(__n); }
  static void _STLP_CALL deallocate(void* __p, size_t) { __stl_delete(__p); }
};

// Allocator adaptor to check size arguments for debugging.
// Reports errors using assert.  Checking can be disabled with
// NDEBUG, but it's far better to just use the underlying allocator
// instead when no checking is desired.
// There is some evidence that this can confuse Purify.
// This adaptor can only be applied to raw allocators

template <class _Alloc>
class __debug_alloc : public _Alloc {
public:
  typedef _Alloc __allocator_type;
  typedef typename _Alloc::value_type value_type;
  typedef typename _Alloc::size_type size_type;
private:
  struct __alloc_header {
    size_t __magic: 16;
    size_t __type_size:16;
    _STLP_UINT32_T _M_size;
  }; // that is 8 bytes for sure
  // Sunpro CC has bug on enums, so extra_before/after set explicitly
  enum { __pad = 8, __magic = 0xdeba, __deleted_magic = 0xdebd,
         __shred_byte = _STLP_SHRED_BYTE };

  enum { __extra_before = 16, __extra_after = 8 };
  // Size of space used to store size.  Note
  // that this must be large enough to preserve
  // alignment.
  static size_t _STLP_CALL __extra_before_chunk() {
    return (long)__extra_before / sizeof(value_type) +
      (size_t)((long)__extra_before % sizeof(value_type) > 0);
  }
  static size_t _STLP_CALL __extra_after_chunk() {
    return (long)__extra_after / sizeof(value_type) +
      (size_t)((long)__extra_after % sizeof(value_type) > 0);
  }
public:
  __debug_alloc() {}
  ~__debug_alloc() {}
  static void* _STLP_CALL allocate(size_t);
  static void _STLP_CALL deallocate(void *, size_t);
};

#  if defined (__OS400__)
// dums 02/05/2007: is it really necessary ?
enum { _MAX_BYTES = 256 };
#  else
enum { _MAX_BYTES = 32 * sizeof(void*) };
#  endif

#if !defined (_STLP_USE_NO_IOSTREAMS)
// Default node allocator.
// With a reasonable compiler, this should be roughly as fast as the
// original STL class-specific allocators, but with less fragmentation.
class _STLP_CLASS_DECLSPEC __node_alloc {
  static void * _STLP_CALL _M_allocate(size_t& __n);
  /* __p may not be 0 */
  static void _STLP_CALL _M_deallocate(void *__p, size_t __n);

public:
  // this one is needed for proper simple_alloc wrapping
	typedef size_t size_type;
	typedef char value_type;
  /* __n must be > 0      */
  static void* _STLP_CALL allocate(size_t& __n)
  { return (__n > (size_t)_MAX_BYTES) ? __stl_new(__n) : _M_allocate(__n); }
  /* __p may not be 0 */
  static void _STLP_CALL deallocate(void *__p, size_t __n)
  { if (__n > (size_t)_MAX_BYTES) __stl_delete(__p); else _M_deallocate(__p, __n); }
};

#  if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE_CLASS __debug_alloc<__node_alloc>;
#  endif

#endif

#if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE_CLASS __debug_alloc<__new_alloc>;
_STLP_EXPORT_TEMPLATE_CLASS __debug_alloc<__malloc_alloc>;
#endif

/* macro to convert the allocator for initialization
 * not using MEMBER_TEMPLATE_CLASSES as it should work given template constructor  */
#if defined (_STLP_MEMBER_TEMPLATES) || ! defined (_STLP_CLASS_PARTIAL_SPECIALIZATION)
/* if _STLP_NO_TEMPLATE_CONVERSIONS is set, the member template constructor is
 * not used implicitly to convert allocator parameter, so let us do it explicitly */
#  if defined (_STLP_MEMBER_TEMPLATE_CLASSES) && defined (_STLP_NO_TEMPLATE_CONVERSIONS)
#    define _STLP_CONVERT_ALLOCATOR(__a, _Tp) __stl_alloc_create(__a,(_Tp*)0)
#  else
#    define _STLP_CONVERT_ALLOCATOR(__a, _Tp) __a
#  endif
/* else convert, but only if partial specialization works, since else
 * Container::allocator_type won't be different */
#else
#  define _STLP_CONVERT_ALLOCATOR(__a, _Tp) __stl_alloc_create(__a,(_Tp*)0)
#endif

// Another allocator adaptor: _Alloc_traits.  This serves two
// purposes.  First, make it possible to write containers that can use
// either SGI-style allocators or standard-conforming allocator.

// The fully general version.
template <class _Tp, class _Allocator>
struct _Alloc_traits {
  typedef _Allocator _Orig;
#if !defined (_STLP_DONT_SUPPORT_REBIND_MEMBER_TEMPLATE)
  typedef typename _Allocator::_STLP_TEMPLATE rebind<_Tp> _Rebind_type;
  typedef typename _Rebind_type::other  allocator_type;
  static allocator_type create_allocator(const _Orig& __a)
  { return allocator_type(_STLP_CONVERT_ALLOCATOR(__a, _Tp)); }
	static const bool _S_instanceless = true;
#else
  // this is not actually true, used only to pass this type through
  // to dynamic overload selection in _STLP_alloc_proxy methods
  typedef _Allocator allocator_type;
#endif
};

template < class _Tp >
struct _Alloc_traits< _Tp, __malloc_alloc >
{
	typedef __malloc_alloc _Orig;
	static const bool _S_instanceless = true;
};

template < class _Tp >
struct _Alloc_traits< _Tp, __new_alloc >
{
	typedef __new_alloc _Orig;
	static const bool _S_instanceless = true;
};

template < class _Tp >
struct _Alloc_traits< _Tp, __node_alloc >
{
	typedef __node_alloc _Orig;
	static const bool _S_instanceless = true;
};

template < class _Tp, class t_TyAlloc >
struct _Alloc_traits< _Tp, __debug_alloc< t_TyAlloc > >
{
	typedef __debug_alloc< t_TyAlloc > _Orig;
	static const bool _S_instanceless = true;
};


#if defined (_STLP_USE_PERTHREAD_ALLOC)

_STLP_END_NAMESPACE
// include additional header here
#  include <stl/_pthread_alloc.h>
_STLP_BEGIN_NAMESPACE

typedef __pthread_alloc __alloc_type;
#elif defined (_STLP_USE_NEWALLOC)
typedef __new_alloc __alloc_type;
#elif defined (_STLP_USE_MALLOC)
typedef __malloc_alloc __alloc_type;
#else
typedef __node_alloc __alloc_type;
#endif

#if defined (_STLP_DEBUG_ALLOC)
typedef __debug_alloc<__alloc_type> __sgi_alloc;
#else
typedef __alloc_type __sgi_alloc;
#endif

#if !defined (_STLP_NO_ANACHRONISMS)
typedef __sgi_alloc __single_client_alloc;
typedef __sgi_alloc __multithreaded_alloc;
#endif

// This implements allocators as specified in the C++ standard.
//
// Note that standard-conforming allocators use many language features
// that are not yet widely implemented.  In particular, they rely on
// member templates, partial specialization, partial ordering of function
// templates, the typename keyword, and the use of the template keyword
// to refer to a template member of a dependent type.

/*
template <class _Tp>
struct _AllocatorAux {
  typedef _Tp*       pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp&       reference;
  typedef const _Tp& const_reference;

  pointer address(reference __x) const {return &__x;}
  const_pointer address(const_reference __x) const { return &__x; }
};

template <class _Tp>
struct _AllocatorAux<const _Tp> {
  typedef _Tp*       pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp&       reference;
  typedef const _Tp& const_reference;

  const_pointer address(const_reference __x) const { return &__x; }
};
*/

template <class _Tp>
class allocator //: public _AllocatorAux<_Tp>
/* A small helper struct to recognize STLport allocator implementation
 * from any user specialization one.
 */
                : public __stlport_class<allocator<_Tp> >
{
public:
  typedef _Tp        value_type;
  typedef _Tp*       pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp&       reference;
  typedef const _Tp& const_reference;
  typedef size_t     size_type;
  typedef ptrdiff_t  difference_type;
#if defined (_STLP_MEMBER_TEMPLATE_CLASSES)
  template <class _Tp1> struct rebind {
    typedef allocator<_Tp1> other;
  };
#endif
  allocator() _STLP_NOTHROW {}
#if defined (_STLP_MEMBER_TEMPLATES)
  template <class _Tp1> allocator(const allocator<_Tp1>&) _STLP_NOTHROW {}
#endif
  allocator(const allocator<_Tp>&) _STLP_NOTHROW {}
#if !defined (_STLP_NO_MOVE_SEMANTIC)
  allocator(__move_source<allocator<_Tp> > src) _STLP_NOTHROW {}
#endif
  ~allocator() _STLP_NOTHROW {}
  pointer address(reference __x) const {return &__x;}
  const_pointer address(const_reference __x) const { return &__x; }
  // __n is permitted to be 0.  The C++ standard says nothing about what the return value is when __n == 0.
  _Tp* allocate(size_type __n, const void* = 0) {
    if (__n > max_size()) {
      _STLP_THROW_BAD_ALLOC;
    }
    if (__n != 0) {
      size_type __buf_size = __n * sizeof(value_type);
      _Tp* __ret = __REINTERPRET_CAST(_Tp*, __sgi_alloc::allocate(__buf_size));
#if defined (_STLP_DEBUG_UNINITIALIZED) && !defined (_STLP_DEBUG_ALLOC)
      memset((char*)__ret, _STLP_SHRED_BYTE, __buf_size);
#endif
      return __ret;
    }

    return 0;
  }
  // __p is permitted to be a null pointer, only if n==0.
  void deallocate(pointer __p, size_type __n) {
    _STLP_ASSERT( (__p == 0) == (__n == 0) )
    if (__p != 0) {
#if defined (_STLP_DEBUG_UNINITIALIZED) && !defined (_STLP_DEBUG_ALLOC)
      memset((char*)__p, _STLP_SHRED_BYTE, __n * sizeof(value_type));
#endif
      __sgi_alloc::deallocate((void*)__p, __n * sizeof(value_type));
    }
  }
#if !defined (_STLP_NO_ANACHRONISMS)
  // backwards compatibility
  void deallocate(pointer __p) const {  if (__p != 0) __sgi_alloc::deallocate((void*)__p, sizeof(value_type)); }
#endif
  size_type max_size() const _STLP_NOTHROW  { return size_t(-1) / sizeof(value_type); }
  void construct(pointer __p, const_reference __val) { _STLP_STD::_Copy_Construct(__p, __val); }
  void destroy(pointer __p) { _STLP_STD::_Destroy(__p); }

#if defined (_STLP_NO_EXTENSIONS)
  /* STLport extension giving rounded size of an allocated memory buffer
   * This method do not have to be part of a user defined allocator implementation
   * and won't even be called if such a function was granted.
   */
protected:
#endif
  _Tp* _M_allocate(size_type __n, size_type& __allocated_n) {
    if (__n > max_size()) {
      _STLP_THROW_BAD_ALLOC;
    }

    if (__n != 0) {
      size_type __buf_size = __n * sizeof(value_type);
      _Tp* __ret = __REINTERPRET_CAST(_Tp*, __sgi_alloc::allocate(__buf_size));
#if defined (_STLP_DEBUG_UNINITIALIZED) && !defined (_STLP_DEBUG_ALLOC)
      memset((char*)__ret, _STLP_SHRED_BYTE, __buf_size);
#endif
      __allocated_n = __buf_size / sizeof(value_type);
      return __ret;
    }

    return 0;
  }
#if defined (_STLP_USE_PARTIAL_SPEC_WORKAROUND) && !defined (_STLP_FUNCTION_TMPL_PARTIAL_ORDER)
  void _M_swap_workaround(allocator<_Tp>& __other) {}
#endif
};

_STLP_TEMPLATE_NULL
class _STLP_CLASS_DECLSPEC allocator<void> {
public:
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;
  typedef void*       pointer;
  typedef const void* const_pointer;
#if defined (_STLP_CLASS_PARTIAL_SPECIALIZATION)
  typedef void        value_type;
#endif
#if defined (_STLP_MEMBER_TEMPLATE_CLASSES)
  template <class _Tp1> struct rebind {
    typedef allocator<_Tp1> other;
  };
#endif
};

template <class _T1, class _T2>
inline bool _STLP_CALL operator==(const allocator<_T1>&, const allocator<_T2>&) _STLP_NOTHROW
{ return true; }
template <class _T1, class _T2>
inline bool _STLP_CALL operator!=(const allocator<_T1>&, const allocator<_T2>&) _STLP_NOTHROW
{ return false; }

#if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE_CLASS allocator<char>;
#  if defined (_STLP_HAS_WCHAR_T)
_STLP_EXPORT_TEMPLATE_CLASS allocator<wchar_t>;
#  endif
#  if defined (_STLP_USE_PTR_SPECIALIZATIONS)
_STLP_EXPORT_TEMPLATE_CLASS allocator<void*>;
#  endif
#endif

// REVIEW:<dbien>: This moved from an old version of STLPort so that I could continue to use it in my codebase.
// Allocator adaptor to turn an SGI-style allocator (e.g. alloc, __malloc_alloc)
// into a standard-conforming allocator.   Note that this adaptor does
// *not* assume that all objects of the underlying alloc class are
// identical, nor does it assume that all of the underlying alloc's
// member functions are static member functions.  Note, also, that 
// _stlallocator<_Tp, alloc> is essentially the same thing as allocator<_Tp>.

// _stlallocator_base: The instanced version.
template < class _Tp, class _Alloc, const bool t_fInstancelessAllocator >
class _stlallocator_base
{
public:
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef _Tp*       pointer;
	typedef const _Tp* const_pointer;
	typedef _Tp&       reference;
	typedef const _Tp& const_reference;
	typedef _Tp        value_type;

	_Alloc __underlying_alloc;

	_stlallocator_base() _STLP_NOTHROW {}
	_stlallocator_base(const _stlallocator_base& __a) _STLP_NOTHROW
		: __underlying_alloc(__a.__underlying_alloc) {}
	template <class _Tp1>
	_stlallocator_base(const _stlallocator_base<_Tp1, _Alloc, t_fInstancelessAllocator>& __a) _STLP_NOTHROW
		: __underlying_alloc(__a.__underlying_alloc) {}
	~_stlallocator_base() _STLP_NOTHROW {}

	static pointer address(reference __x) { return &__x; }
	static const_pointer address(const_reference __x) { return &__x; }

	// __n is permitted to be 0.
	_Tp* allocate(size_type __n, const void* = 0) 
	{
		return __n != 0
			? static_cast<_Tp*>(__underlying_alloc.allocate(__n * sizeof(_Tp)))
			: 0;
	}

	// __p is not permitted to be a null pointer.
	void deallocate(pointer __p, size_type __n)
	{
		__underlying_alloc.deallocate(__p, __n * sizeof(_Tp));
	}

	static size_type max_size() _STLP_NOTHROW
	{
		return size_t(-1) / sizeof(_Tp);
	}

	static void construct(pointer __p, const _Tp& __val) { new(__p) _Tp(__val); }
	static void destroy(pointer __p) { __p->~_Tp(); }

	bool operator == (_stlallocator_base const & _r) const
	{
		return __underlying_alloc == _r.__underlying_alloc;
	}
	bool operator != (_stlallocator_base const & _r) const
	{
		return __underlying_alloc != _r.__underlying_alloc;
	}
};

// _stlallocator_base: The instanceless version.
template < class _Tp, class _Alloc >
class _stlallocator_base< _Tp, _Alloc, true >
{
public:
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef _Tp*       pointer;
	typedef const _Tp* const_pointer;
	typedef _Tp&       reference;
	typedef const _Tp& const_reference;
	typedef _Tp        value_type;

	_stlallocator_base() _STLP_NOTHROW {}
	_stlallocator_base(const _stlallocator_base& __a) _STLP_NOTHROW {}
	template <class _Tp1>
	_stlallocator_base(const _stlallocator_base<_Tp1, _Alloc, true>& __a) _STLP_NOTHROW {}
	~_stlallocator_base() _STLP_NOTHROW {}

	static pointer address(reference __x) { return &__x; }
	static const_pointer address(const_reference __x) { return &__x; }

	// __n is permitted to be 0.
	static _Tp* allocate(size_type __n, const void* = 0) 
	{
		return __n != 0 ? static_cast<_Tp*>(_Alloc::allocate(__n * sizeof(_Tp))) : 0;
	}

	// __p is not permitted to be a null pointer.
	static void deallocate(pointer __p, size_type __n)
	{
		_Alloc::deallocate( __p, __n * sizeof(_Tp) );
	}

	static size_type max_size() _STLP_NOTHROW
	{
		return size_t(-1) / sizeof(_Tp);
	}

	static void construct(pointer __p, const _Tp& __val) { new(__p) _Tp(__val); }
	static void destroy(pointer __p) { __p->~_Tp(); }

	bool operator == (_stlallocator_base const & _r) const
	{
		return true;
	}
	bool operator != (_stlallocator_base const & _r) const
	{
		return false;
	}
};

template <class _Tp, class _Alloc>
struct _stlallocator 
	: public _stlallocator_base< _Tp, _Alloc, _Alloc_traits< _Tp, _Alloc >::_S_instanceless >
{
private:
	typedef _stlallocator_base< _Tp, _Alloc, _Alloc_traits< _Tp, _Alloc >::_S_instanceless > _TyBase;
public:
  _stlallocator() _STLP_NOTHROW {}
  _stlallocator(const _stlallocator& __a) _STLP_NOTHROW
    : _TyBase(__a) {}
  template <class _Tp1> 
  _stlallocator( const _stlallocator<_Tp1, _Alloc>& __a ) _STLP_NOTHROW
    : _TyBase( __a ) {}

	template <class _Tp1> struct rebind {
		typedef _stlallocator<_Tp1, _Alloc> other;
	};

	bool operator == (_stlallocator const & _r) const
	{
		return _TyBase::operator == (_r);
	}
	bool operator != (_stlallocator const & _r) const
	{
		return _TyBase::operator != (_r);
	}
};

template < class t_tyAlloc >
class _stlallocator< void, t_tyAlloc > 
{
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;
  typedef void*       pointer;
  typedef const void* const_pointer;
  typedef void        value_type;

  template <class _Tp1> struct rebind 
  {
    typedef _stlallocator<_Tp1, t_tyAlloc> other;
  };
};

template<class _Tp, class _Alloc>
class simple_alloc {
public:
    // Get to the actual static allocator for this allocation:
    typedef typename _Alloc::_TyStaticAlloc _TyStaticAlloc;
    typedef typename _Alloc::size_type size_type;

    static _Tp* allocate(size_t __n)
      { return 0 == __n ? 0 : (_Tp*) _TyStaticAlloc::allocate(__n * sizeof (_Tp)); }
    static _Tp* allocate(void)
      { return (_Tp*) _TyStaticAlloc::allocate(sizeof (_Tp)); }
    static void deallocate(_Tp* __p, size_t __n)
      { if (0 != __n) _TyStaticAlloc::deallocate(__p, __n * sizeof (_Tp)); }
    static void deallocate(_Tp* __p)
      { _TyStaticAlloc::deallocate(__p, sizeof (_Tp)); }
};

_STLP_MOVE_TO_PRIV_NAMESPACE

template <class _Tp>
struct __alloc_type_traits {
#if !defined (__BORLANDC__)
  typedef typename _IsSTLportClass<allocator<_Tp> >::_Ret _STLportAlloc;
#else
  enum { _Is = _IsSTLportClass<allocator<_Tp> >::_Is };
  typedef typename __bool2type<_Is>::_Ret _STLportAlloc;
#endif
  //The default allocator implementation which is recognize thanks to the
  //__stlport_class inheritance is a stateless object so:
  typedef _STLportAlloc has_trivial_default_constructor;
  typedef _STLportAlloc has_trivial_copy_constructor;
  typedef _STLportAlloc has_trivial_assignment_operator;
  typedef _STLportAlloc has_trivial_destructor;
  typedef _STLportAlloc is_POD_type;
};

_STLP_MOVE_TO_STD_NAMESPACE

#if defined (_STLP_CLASS_PARTIAL_SPECIALIZATION)
template <class _Tp>
struct __type_traits<allocator<_Tp> > : _STLP_PRIV __alloc_type_traits<_Tp> {};
#else
_STLP_TEMPLATE_NULL
struct __type_traits<allocator<char> > : _STLP_PRIV __alloc_type_traits<char> {};
#  if defined (_STLP_HAS_WCHAR_T)
_STLP_TEMPLATE_NULL
struct __type_traits<allocator<wchar_t> > : _STLP_PRIV __alloc_type_traits<wchar_t> {};
#  endif
#  if defined (_STLP_USE_PTR_SPECIALIZATIONS)
_STLP_TEMPLATE_NULL
struct __type_traits<allocator<void*> > : _STLP_PRIV __alloc_type_traits<void*> {};
#  endif
#endif


#if !defined (_STLP_FORCE_ALLOCATORS)
#  define _STLP_FORCE_ALLOCATORS(a,y)
#endif

#if defined (_STLP_CLASS_PARTIAL_SPECIALIZATION) && !defined (_STLP_MEMBER_TEMPLATE_CLASSES)
// The version for the default allocator, for rare occasion when we have partial spec w/o member template classes
template <class _Tp, class _Tp1>
struct _Alloc_traits<_Tp, allocator<_Tp1> > {
  typedef allocator<_Tp1> _Orig;
  typedef allocator<_Tp> allocator_type;
  static allocator_type create_allocator(const allocator<_Tp1 >& __a)
  { return allocator_type(_STLP_CONVERT_ALLOCATOR(__a, _Tp)); }
};
#endif

#if !defined (_STLP_DONT_SUPPORT_REBIND_MEMBER_TEMPLATE) && defined (_STLP_MEMBER_TEMPLATES)
template <class _Tp, class _Alloc>
inline _STLP_TYPENAME_ON_RETURN_TYPE _Alloc_traits<_Tp, _Alloc>::allocator_type  _STLP_CALL
__stl_alloc_create(const _Alloc& __a, const _Tp*) {
  typedef typename _Alloc::_STLP_TEMPLATE rebind<_Tp>::other _Rebound_type;
  return _Rebound_type(__a);
}
#else
// If custom allocators are being used without member template classes support :
// user (on purpose) is forced to define rebind/get operations !!!
template <class _Tp1, class _Tp2>
inline allocator<_Tp2>& _STLP_CALL
__stl_alloc_rebind(allocator<_Tp1>& __a, const _Tp2*) {  return (allocator<_Tp2>&)(__a); }
template <class _Tp1, class _Tp2>
inline allocator<_Tp2> _STLP_CALL
__stl_alloc_create(const allocator<_Tp1>&, const _Tp2*) { return allocator<_Tp2>(); }
#endif

_STLP_MOVE_TO_PRIV_NAMESPACE

// inheritance is being used for EBO optimization
template <class _Value, class _Tp, class _MaybeReboundAlloc>
class _STLP_alloc_proxy : public _MaybeReboundAlloc {
private:
  typedef _MaybeReboundAlloc _Base;
  typedef typename _Base::size_type size_type;
  typedef _STLP_alloc_proxy<_Value, _Tp, _MaybeReboundAlloc> _Self;
public:
  _Value _M_data;

  _STLP_alloc_proxy (const _MaybeReboundAlloc& __a, _Value __p) :
    _MaybeReboundAlloc(__a), _M_data(__p) {}

#if !defined (_STLP_NO_MOVE_SEMANTIC)
  _STLP_alloc_proxy (__move_source<_Self> src) :
    _Base(_STLP_PRIV _AsMoveSource(src.get()._M_base())),
    _M_data(_STLP_PRIV _AsMoveSource(src.get()._M_data)) {}

  _Base& _M_base()
  { return *this; }
#endif

private:
  /* Following are helper methods to detect stateless allocators and avoid
   * swap in this case. For some compilers (VC6) it is a workaround for a
   * compiler bug in the Empty Base class Optimization feature, for others
   * it is a small optimization or nothing if no EBO. */
  void _M_swap_alloc(_Self&, const __true_type& /*_IsStateless*/)
  {}

  void _M_swap_alloc(_Self& __x, const __false_type& /*_IsStateless*/) {
    _MaybeReboundAlloc &__base_this = *this;
    _MaybeReboundAlloc &__base_x = __x;
    _STLP_STD::swap(__base_this, __base_x);
  }

public:
  void _M_swap_alloc(_Self& __x) {
#if !defined (__BORLANDC__)
    typedef typename _IsStateless<_MaybeReboundAlloc>::_Ret _StatelessAlloc;
#else
    typedef typename __bool2type<_IsStateless<_MaybeReboundAlloc>::_Is>::_Ret _StatelessAlloc;
#endif
    _M_swap_alloc(__x, _StatelessAlloc());
  }

  /* We need to define the following swap implementation for allocator with state
   * as those allocators might have implement a special swap function to correctly
   * move datas from an instance to the oher, _STLP_alloc_proxy should not break
   * this mecanism. */
  void swap(_Self& __x) {
    _M_swap_alloc(__x);
    _STLP_STD::swap(_M_data, __x._M_data);
  }

  _Tp* allocate(size_type __n, size_type& __allocated_n) {
#if !defined (__BORLANDC__)
    typedef typename _IsSTLportClass<_MaybeReboundAlloc>::_Ret _STLportAlloc;
#else
    typedef typename __bool2type<_IsSTLportClass<_MaybeReboundAlloc>::_Is>::_Ret _STLportAlloc;
#endif
    return allocate(__n, __allocated_n, _STLportAlloc());
  }

  // Unified interface to perform allocate()/deallocate() with limited
  // language support
#if defined (_STLP_DONT_SUPPORT_REBIND_MEMBER_TEMPLATE)
  // else it is rebound already, and allocate() member is accessible
  _Tp* allocate(size_type __n)
  { return __stl_alloc_rebind(__STATIC_CAST(_Base&, *this), __STATIC_CAST(_Tp*, 0)).allocate(__n, 0); }
  void deallocate(_Tp* __p, size_type __n)
  { __stl_alloc_rebind(__STATIC_CAST(_Base&, *this), __STATIC_CAST(_Tp*, 0)).deallocate(__p, __n); }
private:
  _Tp* allocate(size_type __n, size_type& __allocated_n, const __true_type& /*STLport allocator*/)
  { return __stl_alloc_rebind(__STATIC_CAST(_Base&, *this), __STATIC_CAST(_Tp*, 0))._M_allocate(__n, __allocated_n); }
#else
  //Expose Standard allocate overload (using expression do not work for some compilers (Borland))
  _Tp* allocate(size_type __n)
  { return _Base::allocate(__n); }
private:
  _Tp* allocate(size_type __n, size_type& __allocated_n, const __true_type& /*STLport allocator*/)
  { return _Base::_M_allocate(__n, __allocated_n); }
#endif

  _Tp* allocate(size_type __n, size_type& __allocated_n, const __false_type& /*STLport allocator*/)
  { __allocated_n = __n; return allocate(__n); }
};

#if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE_CLASS _STLP_alloc_proxy<char*, char, allocator<char> >;
#  if defined (_STLP_HAS_WCHAR_T)
_STLP_EXPORT_TEMPLATE_CLASS _STLP_alloc_proxy<wchar_t*, wchar_t, allocator<wchar_t> >;
#  endif
#  if defined (_STLP_USE_PTR_SPECIALIZATIONS)
_STLP_EXPORT_TEMPLATE_CLASS _STLP_alloc_proxy<void**, void*, allocator<void*> >;
#  endif
#endif

_STLP_MOVE_TO_STD_NAMESPACE
_STLP_END_NAMESPACE

#if defined (_STLP_EXPOSE_GLOBALS_IMPLEMENTATION) && !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/_alloc.c>
#endif

#endif /* _STLP_INTERNAL_ALLOC_H */

// Local Variables:
// mode:C++
// End:

