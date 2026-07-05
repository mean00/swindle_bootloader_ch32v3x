/**
 * @file    stubs.cpp
 * @brief   Minimal C/C++ runtime stubs for the bootloader environment.
 *
 * @details Provides lightweight implementations of common runtime functions
 *          to avoid pulling in the full libc.  Includes:
 *            - @c do_assert / @c Logger_crash — assertion and crash handlers.
 *            - @c xstrlen / @c xstrcpy / @c xstrcat — minimal string operations.
 *            - @c _write / @c _sbrk / @c _putchar / @c printf — stub syscalls
 *              that satisfy the linker without providing full I/O.
 *
 * @note    All I/O syscalls either assert or are no-ops since the bootloader
 *          has no hosted environment.
 *
 * @ingroup common
 */

#include "esprit.h"
#include <stddef.h>

extern "C" void deadEnd(int code);

/**
 * @brief  Assertion failure handler.
 * @param msg  Assertion message string (unused — no output capability).
 */
extern "C" void do_assert(const char *msg)
{
    (void)msg;
    deadEnd(1);
    while (1)
        __asm__("nop");
}

/**
 * @brief  Crash logger stub — called on unrecoverable errors.
 * @param st  Error description string (unused).
 */
extern "C" void Logger_crash(const char *st)
{
    (void)st;
    xAssert(0);
}

/**
 * @brief  Minimal strlen implementation (avoids pulling in libc).
 * @param src  Null-terminated input string.
 * @return int Length of the string (excluding the null terminator).
 */
int xstrlen(const char *src)
{
    int nb = 0;
    while (src[nb])
        nb++;
    return nb;
}

/**
 * @brief  Minimal strcpy implementation.
 * @param tgt  Destination buffer.
 * @param src  Null-terminated source string.
 */
void xstrcpy(char *tgt, const char *src)
{
    while (*src)
    {
        *tgt = *src;
        tgt++;
        src++;
    }
    *tgt = 0;
}

/**
 * @brief  Minimal strcat implementation.
 * @param tgt  Destination buffer (must contain a valid null-terminated string).
 * @param src  Null-terminated string to append.
 */
void xstrcat(char *tgt, const char *src)
{
    tgt += xstrlen(tgt);
    while (*src)
    {
        *tgt = *src;
        tgt++;
        src++;
    }
    *tgt = 0;
}

// ========== Stub syscalls ==========

/** @brief Stub: write system call — asserts if called. */
extern "C" void _write()    { xAssert(0); }
/** @brief Stub: sbrk heap allocator — asserts if called. */
extern "C" void *_sbrk(ptrdiff_t incr) { (void)incr; xAssert(0); return 0; }
/** @brief Stub: putchar — no-op. */
extern "C" void _putchar(char) {}
/** @brief Stub: printf — no-op (returns 0). */
extern "C" int printf(const char *__restrict, ...) { return 0; }
