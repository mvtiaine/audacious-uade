// SPDX-License-Identifier: CC-PDDC
/* tinynew.cpp

   Overrides operators new and delete
   globally to reduce code size.

   Public domain, use however you wish.
   If you really need a license, consider it MIT:
   http://www.opensource.org/licenses/mit-license.php

   - Eric Agan
     Elegant Invention
 */

// additional overrides added (__cxa*, __throw*, c++14 operator delete, ...), malloc(0) check, etc. -mvtiaine

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>

void* operator new(std::size_t size) {
    /* malloc (0) is unpredictable; avoid it.  */
    if (__builtin_expect (size == 0, false))
        size = 1;

    void *ptr = std::malloc(size);
    if (!ptr)
        abort();

    return ptr;
}

void* operator new[](std::size_t size) {
    return operator new(size);
}

void operator delete(void* ptr) noexcept {
    std::free(ptr);
}

void operator delete[](void* ptr) noexcept {
    operator delete(ptr);
}

/* Optionally you can override the 'nothrow' versions as well.
   This is useful if you want to catch failed allocs with your
   own debug code, or keep track of heap usage for example,
   rather than just eliminate exceptions.
 */

void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
    return operator new(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
    return operator new(size);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
    operator delete(ptr);
}

//eof

// for c++14

void operator delete (void* ptr, std::size_t, const std::nothrow_t&) noexcept {
    operator delete(ptr);
}

void operator delete[] (void* ptr, std::size_t, const std::nothrow_t&) noexcept {
    operator delete(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
    operator delete(ptr);
}

// for c++17

#if defined(__cpp_aligned_new) && !defined(__MINGW32__)  && !defined(__AMIGA__) && !defined(__AROS__) && !defined(__OS2__) && !defined(__QNX__) && !defined(__ORBIS__) && !defined(__sysv5__)

void* operator new(std::size_t size, std::align_val_t al) {
    /* malloc (0) is unpredictable; avoid it.  */
    if (__builtin_expect (size == 0, false))
        size = 1;

    // size must be multiple of alignment
    size_t rounded = (size + static_cast<size_t>(al) - 1) & ~(static_cast<size_t>(al) - 1);
    void *ptr = std::aligned_alloc(static_cast<size_t>(al), rounded);
    if (!ptr)
        abort();

    return ptr;
}

void* operator new(std::size_t size, std::align_val_t al, const std::nothrow_t&) noexcept {
    return operator new(size, al);
}

void* operator new[](std::size_t size, std::align_val_t al) {
    return operator new(size, al);
}

void* operator new[](std::size_t size, std::align_val_t al, const std::nothrow_t&) noexcept {
    return operator new(size, al);
}

void operator delete(void* ptr, std::align_val_t al) noexcept {
    operator delete(ptr);
}

void operator delete(void* ptr, std::align_val_t al, const std::nothrow_t&) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, std::align_val_t al) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, std::align_val_t al, const std::nothrow_t&) noexcept {
    operator delete(ptr);
}

void operator delete(void* ptr, std::size_t size, std::align_val_t al) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, std::size_t size, std::align_val_t al) noexcept {
    operator delete(ptr);
}

#endif // __cpp_aligned_new

// some additional overrides

extern "C" {

void __cxa_pure_virtual() noexcept {
    abort();
}

void __cxa_throw() noexcept {
    abort();
}

void __cxa_rethrow() noexcept {
    abort();
}

void* __cxa_current_primary_exception() noexcept {
    abort();
}

void __cxa_rethrow_primary_exception(void*) noexcept {
    abort();
}

void __cxa_increment_exception_refcount(void*) noexcept {
    abort();
}

void __cxa_decrement_exception_refcount(void*) noexcept {
    abort();
}

void* __cxa_begin_catch() noexcept {
    abort();
}

void __cxa_end_catch() noexcept {
    abort();
}

void* __cxa_allocate_exception(size_t) noexcept {
    abort();
}

void __cxa_free_exception(void *) noexcept {
    abort();
}

void* __cxa_allocate_dependent_exception() noexcept {
    abort();
}

void* __cxa_get_exception_ptr() noexcept {
    abort();
}

void __cxa_call_unexpected (void*) noexcept {
    abort();
}

bool __cxa_uncaught_exception() noexcept {
    abort();
}

int __cxa_uncaught_exceptions() noexcept {
    abort();
}

char* __cxa_demangle(const char*, char*, size_t*, int*) noexcept {
    abort();
}

void* __dynamic_cast(const void*, const void*, const void*, ptrdiff_t) noexcept {
    abort();
}

void __gxx_personality_v0() noexcept {
    abort();
}

typedef uintptr_t _Unwind_Ptr;
typedef uintptr_t _Unwind_Word;
typedef enum {} _Unwind_Reason_Code;
struct _Unwind_Context { int pad; };
struct _Unwind_Exception { int pad; };

void _Unwind_DeleteException(struct _Unwind_Exception*) noexcept {
    abort();
}

_Unwind_Ptr _Unwind_GetIP(struct _Unwind_Context * context) noexcept {
    abort();
}

_Unwind_Ptr _Unwind_GetLanguageSpecificData(struct _Unwind_Context *) noexcept {
    abort();
}

_Unwind_Ptr _Unwind_GetRegionStart(struct _Unwind_Context*) noexcept {
    abort();
}

_Unwind_Reason_Code _Unwind_RaiseException(struct _Unwind_Exception*) noexcept {
    abort();
}

void _Unwind_Resume(struct _Unwind_Exception*) noexcept {
    abort();
}

void _Unwind_SetGR(struct _Unwind_Context *, int, _Unwind_Word) noexcept {
    abort();
}

void _Unwind_SetIP(struct _Unwind_Context*, _Unwind_Word) noexcept {
    abort();
}

} // extern "C"

namespace std {
#if defined(_LIBCPP_VERSION)
#if defined(__ANDROID__)
inline namespace __ndk1 {
#else
inline namespace __1 {
#endif
#endif

void __throw_bad_alloc() {
    abort();
}

void __throw_bad_array_new_length() noexcept {
    abort();
}

void __throw_bad_cast() {
    abort();
}

void __throw_bad_exception() noexcept {
    abort();
}

void __throw_bad_function_call() {
    abort();
}

void __throw_bad_optional_access() noexcept {
    abort();
}

void __throw_bad_typeid() noexcept {
    abort();
}

void __throw_domain_error(const char*) {
    abort();
}

void __throw_invalid_argument(const char*) {
    abort();
}

void __throw_length_error(const char*) {
    abort();
}

void __throw_logic_error(const char*) {
    abort();
}

void __throw_out_of_range(const char*) {
    abort();
}

void __throw_out_of_range_fmt(const char *, ...) noexcept {
    abort();
}

void __throw_overflow_error(const char *) {
    abort();
}

void __throw_range_error(const char*) {
    abort();
}

void __throw_runtime_error(const char*) {
    abort();
}

void __throw_system_error(int) noexcept {
    abort();
}

void __throw_system_error(int, const char*) noexcept {
    abort();
}

void __throw_underflow_error(const char *) {
    abort();
}

#if defined(_LIBCPP_VERSION)
} // namespace __1
#endif
} // namespace std

namespace __gnu_cxx {
void __verbose_terminate_handler() {
    abort();
}
} // namespace _gnu_cxx
