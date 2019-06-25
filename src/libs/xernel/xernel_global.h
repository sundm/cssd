#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(XERNEL_LIB)
#  define XERNEL_EXPORT Q_DECL_EXPORT
# else
#  define XERNEL_EXPORT Q_DECL_IMPORT
# endif
#else
# define XERNEL_EXPORT
#endif


// PImpl definitions
#define X_DECLARE_PRIVATE(Class) \
	friend class Class##Private; \
	XPrivateImplWrapper<Class, Class##Private> x_d;

#define X_DECLARE_PUBLIC(Class) \
	friend class Class;

#define X_INIT_PRIVATE(QPTR) x_d.set_q(QPTR)
#define X_D(Class) Class##Private * const d = x_d()
#define X_Q(Class) Class * const p = x_q()

template <typename T>
class XPrivateImpl
{
public:
	virtual ~XPrivateImpl() {}
	inline void set_q(T* q) { q_ptr = q; }

protected:
	inline T* x_q() { return q_ptr; }
	inline const T* x_q() const { return q_ptr; }

private:
	T * q_ptr;
};

template <typename T, typename P>
class XPrivateImplWrapper
{
	friend class XPrivateImpl<T>;
public:
	XPrivateImplWrapper() { d_ptr = new P; }
	~XPrivateImplWrapper() { delete d_ptr; }

	inline void set_q(T* q) { d_ptr->set_q(q); }

	inline P* operator()() { return static_cast<P*>(d_ptr); }
	inline const P* operator()() const { return *static_cast<P*>(d_ptr); }

private:
	Q_DISABLE_COPY(XPrivateImplWrapper)
	XPrivateImpl<T>* d_ptr;
};

// a simple version
#define X_DECLARE_SIMPLE_PRIVATE(Class) \
	friend class Class##Private; \
	QScopedPointer<Class##Private> d;

#define X_DECLARE_SIMPLE_PUBLIC(Class) \
	friend class Class;
