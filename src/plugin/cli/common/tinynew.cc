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

// some additional overrides

extern "C" {

void __cxa_pure_virtual() {
    abort();
}

void __cxa_throw() {
    abort();
}

void __cxa_rethrow() {
    abort();
}

void* __cxa_begin_catch() {
    abort();
}

void __cxa_end_catch() {
    abort();
}

void* __cxa_allocate_exception() {
    abort();
}

void* __cxa_get_exception_ptr() {
    abort();
}

void __cxa_call_unexpected () {
    abort();
}

void __gxx_personality_v0() {
    abort();
}

void _Unwind_Resume() {
    abort();
}

} // extern "C"

namespace std {

void __throw_bad_alloc() {
    abort();
}

#ifndef _LIBCPP_VERSION // llvm libc++
void __throw_bad_array_new_length() {
    abort();
}
#endif

void __throw_bad_cast() {
    abort();
}

void __throw_bad_exception() {
    abort();
}

void __throw_bad_function_call() {
    abort();
}

void __throw_bad_optional_access() {
    abort();
}

void __throw_bad_typeid() {
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

void __throw_out_of_range_fmt(const char *, ...) {
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

void __throw_system_error(int) {
    abort();
}

void __throw_underflow_error(const char *) {
    abort();
}

} // namespace std

namespace __gnu_cxx {
void __verbose_terminate_handler() {
    abort();
}
} // namespace _gnu_cxx
