/* Amalgamated libdeflate source files */


/*** Start of inlined file: adler32.c ***/

/*** Start of inlined file: lib_common.h ***/
#ifndef LIB_LIB_COMMON_H
#define LIB_LIB_COMMON_H

#ifdef LIBDEFLATE_H
 /*
  * When building the library, LIBDEFLATEAPI needs to be defined properly before
  * including libdeflate.h.
  */
#  error "lib_common.h must always be included before libdeflate.h"
#endif

#if defined(LIBDEFLATE_DLL) && (defined(_WIN32) || defined(__CYGWIN__))
#  define LIBDEFLATE_EXPORT_SYM  __declspec(dllexport)
#elif defined(__GNUC__)
#  define LIBDEFLATE_EXPORT_SYM  __attribute__((visibility("default")))
#else
#  define LIBDEFLATE_EXPORT_SYM
#endif

/*
 * On i386, gcc assumes that the stack is 16-byte aligned at function entry.
 * However, some compilers (e.g. MSVC) and programming languages (e.g. Delphi)
 * only guarantee 4-byte alignment when calling functions.  This is mainly an
 * issue on Windows, but it has been seen on Linux too.  Work around this ABI
 * incompatibility by realigning the stack pointer when entering libdeflate.
 * This prevents crashes in SSE/AVX code.
 */
#if defined(__GNUC__) && defined(__i386__)
#  define LIBDEFLATE_ALIGN_STACK  __attribute__((force_align_arg_pointer))
#else
#  define LIBDEFLATE_ALIGN_STACK
#endif

#define LIBDEFLATEAPI	LIBDEFLATE_EXPORT_SYM LIBDEFLATE_ALIGN_STACK


/*** Start of inlined file: common_defs.h ***/
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H


/*** Start of inlined file: libdeflate.h ***/
#ifndef LIBDEFLATE_H
#define LIBDEFLATE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LIBDEFLATE_VERSION_MAJOR	1
#define LIBDEFLATE_VERSION_MINOR	25
#define LIBDEFLATE_VERSION_STRING	"1.25"

/*
 * Users of libdeflate.dll on Windows can define LIBDEFLATE_DLL to cause
 * __declspec(dllimport) to be used.  This should be done when it's easy to do.
 * Otherwise it's fine to skip it, since it is a very minor performance
 * optimization that is irrelevant for most use cases of libdeflate.
 */
#ifndef LIBDEFLATEAPI
#  if defined(LIBDEFLATE_DLL) && (defined(_WIN32) || defined(__CYGWIN__))
#    define LIBDEFLATEAPI	__declspec(dllimport)
#  else
#    define LIBDEFLATEAPI
#  endif
#endif

/* ========================================================================== */
/*                             Compression                                    */
/* ========================================================================== */

struct libdeflate_compressor;
struct libdeflate_options;

/*
 * libdeflate_alloc_compressor() allocates a new compressor that supports
 * DEFLATE, zlib, and gzip compression.  'compression_level' is the compression
 * level on a zlib-like scale but with a higher maximum value (1 = fastest, 6 =
 * medium/default, 9 = slow, 12 = slowest).  Level 0 is also supported and means
 * "no compression", specifically "create a valid stream, but only emit
 * uncompressed blocks" (this will expand the data slightly).
 *
 * The return value is a pointer to the new compressor, or NULL if out of memory
 * or if the compression level is invalid (i.e. outside the range [0, 12]).
 *
 * Note: for compression, the sliding window size is defined at compilation time
 * to 32768, the largest size permissible in the DEFLATE format.  It cannot be
 * changed at runtime.
 *
 * A single compressor is not safe to use by multiple threads concurrently.
 * However, different threads may use different compressors concurrently.
 */
LIBDEFLATEAPI struct libdeflate_compressor *
libdeflate_alloc_compressor(int compression_level);

/*
 * Like libdeflate_alloc_compressor(), but adds the 'options' argument.
 */
LIBDEFLATEAPI struct libdeflate_compressor *
libdeflate_alloc_compressor_ex(int compression_level,
			       const struct libdeflate_options *options);

/*
 * libdeflate_deflate_compress() performs raw DEFLATE compression on a buffer of
 * data.  It attempts to compress 'in_nbytes' bytes of data located at 'in' and
 * write the result to 'out', which has space for 'out_nbytes_avail' bytes.  The
 * return value is the compressed size in bytes, or 0 if the data could not be
 * compressed to 'out_nbytes_avail' bytes or fewer.
 *
 * If compression is successful, then the output data is guaranteed to be a
 * valid DEFLATE stream that decompresses to the input data.  No other
 * guarantees are made about the output data.  Notably, different versions of
 * libdeflate can produce different compressed data for the same uncompressed
 * data, even at the same compression level.  Do ***NOT*** do things like
 * writing tests that compare compressed data to a golden output, as this can
 * break when libdeflate is updated.  (This property isn't specific to
 * libdeflate; the same is true for zlib and other compression libraries too.)
 */
LIBDEFLATEAPI size_t
libdeflate_deflate_compress(struct libdeflate_compressor *compressor,
			    const void *in, size_t in_nbytes,
			    void *out, size_t out_nbytes_avail);

/*
 * libdeflate_deflate_compress_bound() returns a worst-case upper bound on the
 * number of bytes of compressed data that may be produced by compressing any
 * buffer of length less than or equal to 'in_nbytes' using
 * libdeflate_deflate_compress() with the specified compressor.  This bound will
 * necessarily be a number greater than or equal to 'in_nbytes'.  It may be an
 * overestimate of the true upper bound.  The return value is guaranteed to be
 * the same for all invocations with the same compressor and same 'in_nbytes'.
 *
 * As a special case, 'compressor' may be NULL.  This causes the bound to be
 * taken across *any* libdeflate_compressor that could ever be allocated with
 * this build of the library, with any options.
 *
 * Note that this function is not necessary in many applications.  With
 * block-based compression, it is usually preferable to separately store the
 * uncompressed size of each block and to store any blocks that did not compress
 * to less than their original size uncompressed.  In that scenario, there is no
 * need to know the worst-case compressed size, since the maximum number of
 * bytes of compressed data that may be used would always be one less than the
 * input length.  You can just pass a buffer of that size to
 * libdeflate_deflate_compress() and store the data uncompressed if
 * libdeflate_deflate_compress() returns 0, indicating that the compressed data
 * did not fit into the provided output buffer.
 */
LIBDEFLATEAPI size_t
libdeflate_deflate_compress_bound(struct libdeflate_compressor *compressor,
				  size_t in_nbytes);

/*
 * Like libdeflate_deflate_compress(), but uses the zlib wrapper format instead
 * of raw DEFLATE.
 */
LIBDEFLATEAPI size_t
libdeflate_zlib_compress(struct libdeflate_compressor *compressor,
			 const void *in, size_t in_nbytes,
			 void *out, size_t out_nbytes_avail);

/*
 * Like libdeflate_deflate_compress_bound(), but assumes the data will be
 * compressed with libdeflate_zlib_compress() rather than with
 * libdeflate_deflate_compress().
 */
LIBDEFLATEAPI size_t
libdeflate_zlib_compress_bound(struct libdeflate_compressor *compressor,
			       size_t in_nbytes);

/*
 * Like libdeflate_deflate_compress(), but uses the gzip wrapper format instead
 * of raw DEFLATE.
 */
LIBDEFLATEAPI size_t
libdeflate_gzip_compress(struct libdeflate_compressor *compressor,
			 const void *in, size_t in_nbytes,
			 void *out, size_t out_nbytes_avail);

/*
 * Like libdeflate_deflate_compress_bound(), but assumes the data will be
 * compressed with libdeflate_gzip_compress() rather than with
 * libdeflate_deflate_compress().
 */
LIBDEFLATEAPI size_t
libdeflate_gzip_compress_bound(struct libdeflate_compressor *compressor,
			       size_t in_nbytes);

/*
 * libdeflate_free_compressor() frees a compressor that was allocated with
 * libdeflate_alloc_compressor().  If a NULL pointer is passed in, no action is
 * taken.
 */
LIBDEFLATEAPI void
libdeflate_free_compressor(struct libdeflate_compressor *compressor);

/* ========================================================================== */
/*                             Decompression                                  */
/* ========================================================================== */

struct libdeflate_decompressor;
struct libdeflate_options;

/*
 * libdeflate_alloc_decompressor() allocates a new decompressor that can be used
 * for DEFLATE, zlib, and gzip decompression.  The return value is a pointer to
 * the new decompressor, or NULL if out of memory.
 *
 * This function takes no parameters, and the returned decompressor is valid for
 * decompressing data that was compressed at any compression level and with any
 * sliding window size.
 *
 * A single decompressor is not safe to use by multiple threads concurrently.
 * However, different threads may use different decompressors concurrently.
 */
LIBDEFLATEAPI struct libdeflate_decompressor *
libdeflate_alloc_decompressor(void);

/*
 * Like libdeflate_alloc_decompressor(), but adds the 'options' argument.
 */
LIBDEFLATEAPI struct libdeflate_decompressor *
libdeflate_alloc_decompressor_ex(const struct libdeflate_options *options);

/*
 * Result of a call to libdeflate_deflate_decompress(),
 * libdeflate_zlib_decompress(), or libdeflate_gzip_decompress().
 */
enum libdeflate_result {
	/* Decompression was successful.  */
	LIBDEFLATE_SUCCESS = 0,

	/* Decompression failed because the compressed data was invalid,
	 * corrupt, or otherwise unsupported.  */
	LIBDEFLATE_BAD_DATA = 1,

	/* A NULL 'actual_out_nbytes_ret' was provided, but the data would have
	 * decompressed to fewer than 'out_nbytes_avail' bytes.  */
	LIBDEFLATE_SHORT_OUTPUT = 2,

	/* The data would have decompressed to more than 'out_nbytes_avail'
	 * bytes.  */
	LIBDEFLATE_INSUFFICIENT_SPACE = 3,
};

/*
 * libdeflate_deflate_decompress() decompresses a DEFLATE stream from the buffer
 * 'in' with compressed size up to 'in_nbytes' bytes.  The uncompressed data is
 * written to 'out', a buffer with size 'out_nbytes_avail' bytes.  If
 * decompression succeeds, then 0 (LIBDEFLATE_SUCCESS) is returned.  Otherwise,
 * a nonzero result code such as LIBDEFLATE_BAD_DATA is returned, and the
 * contents of the output buffer are undefined.
 *
 * Decompression stops at the end of the DEFLATE stream (as indicated by the
 * BFINAL flag), even if it is actually shorter than 'in_nbytes' bytes.
 *
 * libdeflate_deflate_decompress() can be used in cases where the actual
 * uncompressed size is known (recommended) or unknown (not recommended):
 *
 *   - If the actual uncompressed size is known, then pass the actual
 *     uncompressed size as 'out_nbytes_avail' and pass NULL for
 *     'actual_out_nbytes_ret'.  This makes libdeflate_deflate_decompress() fail
 *     with LIBDEFLATE_SHORT_OUTPUT if the data decompressed to fewer than the
 *     specified number of bytes.
 *
 *   - If the actual uncompressed size is unknown, then provide a non-NULL
 *     'actual_out_nbytes_ret' and provide a buffer with some size
 *     'out_nbytes_avail' that you think is large enough to hold all the
 *     uncompressed data.  In this case, if the data decompresses to less than
 *     or equal to 'out_nbytes_avail' bytes, then
 *     libdeflate_deflate_decompress() will write the actual uncompressed size
 *     to *actual_out_nbytes_ret and return 0 (LIBDEFLATE_SUCCESS).  Otherwise,
 *     it will return LIBDEFLATE_INSUFFICIENT_SPACE if the provided buffer was
 *     not large enough but no other problems were encountered, or another
 *     nonzero result code if decompression failed for another reason.
 */
LIBDEFLATEAPI enum libdeflate_result
libdeflate_deflate_decompress(struct libdeflate_decompressor *decompressor,
			      const void *in, size_t in_nbytes,
			      void *out, size_t out_nbytes_avail,
			      size_t *actual_out_nbytes_ret);

/*
 * Like libdeflate_deflate_decompress(), but adds the 'actual_in_nbytes_ret'
 * argument.  If decompression succeeds and 'actual_in_nbytes_ret' is not NULL,
 * then the actual compressed size of the DEFLATE stream (aligned to the next
 * byte boundary) is written to *actual_in_nbytes_ret.
 */
LIBDEFLATEAPI enum libdeflate_result
libdeflate_deflate_decompress_ex(struct libdeflate_decompressor *decompressor,
				 const void *in, size_t in_nbytes,
				 void *out, size_t out_nbytes_avail,
				 size_t *actual_in_nbytes_ret,
				 size_t *actual_out_nbytes_ret);

/*
 * Like libdeflate_deflate_decompress(), but assumes the zlib wrapper format
 * instead of raw DEFLATE.
 *
 * Decompression will stop at the end of the zlib stream, even if it is shorter
 * than 'in_nbytes'.  If you need to know exactly where the zlib stream ended,
 * use libdeflate_zlib_decompress_ex().
 */
LIBDEFLATEAPI enum libdeflate_result
libdeflate_zlib_decompress(struct libdeflate_decompressor *decompressor,
			   const void *in, size_t in_nbytes,
			   void *out, size_t out_nbytes_avail,
			   size_t *actual_out_nbytes_ret);

/*
 * Like libdeflate_zlib_decompress(), but adds the 'actual_in_nbytes_ret'
 * argument.  If 'actual_in_nbytes_ret' is not NULL and the decompression
 * succeeds (indicating that the first zlib-compressed stream in the input
 * buffer was decompressed), then the actual number of input bytes consumed is
 * written to *actual_in_nbytes_ret.
 */
LIBDEFLATEAPI enum libdeflate_result
libdeflate_zlib_decompress_ex(struct libdeflate_decompressor *decompressor,
			      const void *in, size_t in_nbytes,
			      void *out, size_t out_nbytes_avail,
			      size_t *actual_in_nbytes_ret,
			      size_t *actual_out_nbytes_ret);

/*
 * Like libdeflate_deflate_decompress(), but assumes the gzip wrapper format
 * instead of raw DEFLATE.
 *
 * If multiple gzip-compressed members are concatenated, then only the first
 * will be decompressed.  Use libdeflate_gzip_decompress_ex() if you need
 * multi-member support.
 */
LIBDEFLATEAPI enum libdeflate_result
libdeflate_gzip_decompress(struct libdeflate_decompressor *decompressor,
			   const void *in, size_t in_nbytes,
			   void *out, size_t out_nbytes_avail,
			   size_t *actual_out_nbytes_ret);

/*
 * Like libdeflate_gzip_decompress(), but adds the 'actual_in_nbytes_ret'
 * argument.  If 'actual_in_nbytes_ret' is not NULL and the decompression
 * succeeds (indicating that the first gzip-compressed member in the input
 * buffer was decompressed), then the actual number of input bytes consumed is
 * written to *actual_in_nbytes_ret.
 */
LIBDEFLATEAPI enum libdeflate_result
libdeflate_gzip_decompress_ex(struct libdeflate_decompressor *decompressor,
			      const void *in, size_t in_nbytes,
			      void *out, size_t out_nbytes_avail,
			      size_t *actual_in_nbytes_ret,
			      size_t *actual_out_nbytes_ret);

/*
 * libdeflate_free_decompressor() frees a decompressor that was allocated with
 * libdeflate_alloc_decompressor().  If a NULL pointer is passed in, no action
 * is taken.
 */
LIBDEFLATEAPI void
libdeflate_free_decompressor(struct libdeflate_decompressor *decompressor);

/* ========================================================================== */
/*                                Checksums                                   */
/* ========================================================================== */

/*
 * libdeflate_adler32() updates a running Adler-32 checksum with 'len' bytes of
 * data and returns the updated checksum.  When starting a new checksum, the
 * required initial value for 'adler' is 1.  This value is also returned when
 * 'buffer' is specified as NULL.
 */
LIBDEFLATEAPI uint32_t
libdeflate_adler32(uint32_t adler, const void *buffer, size_t len);

/*
 * libdeflate_crc32() updates a running CRC-32 checksum with 'len' bytes of data
 * and returns the updated checksum.  When starting a new checksum, the required
 * initial value for 'crc' is 0.  This value is also returned when 'buffer' is
 * specified as NULL.
 */
LIBDEFLATEAPI uint32_t
libdeflate_crc32(uint32_t crc, const void *buffer, size_t len);

/* ========================================================================== */
/*                           Custom memory allocator                          */
/* ========================================================================== */

/*
 * Install a custom memory allocator which libdeflate will use for all memory
 * allocations by default.  'malloc_func' is a function that must behave like
 * malloc(), and 'free_func' is a function that must behave like free().
 *
 * The per-(de)compressor custom memory allocator that can be specified in
 * 'struct libdeflate_options' takes priority over this.
 *
 * This doesn't affect the free() function that will be used to free
 * (de)compressors that were already in existence when this is called.
 */
LIBDEFLATEAPI void
libdeflate_set_memory_allocator(void *(*malloc_func)(size_t),
				void (*free_func)(void *));

/*
 * Advanced options.  This is the options structure that
 * libdeflate_alloc_compressor_ex() and libdeflate_alloc_decompressor_ex()
 * require.  Most users won't need this and should just use the non-"_ex"
 * functions instead.  If you do need this, it should be initialized like this:
 *
 *	struct libdeflate_options options;
 *
 *	memset(&options, 0, sizeof(options));
 *	options.sizeof_options = sizeof(options);
 *	// Then set the fields that you need to override the defaults for.
 */
struct libdeflate_options {

	/*
	 * This field must be set to the struct size.  This field exists for
	 * extensibility, so that fields can be appended to this struct in
	 * future versions of libdeflate while still supporting old binaries.
	 */
	size_t sizeof_options;

	/*
	 * An optional custom memory allocator to use for this (de)compressor.
	 * 'malloc_func' must be a function that behaves like malloc(), and
	 * 'free_func' must be a function that behaves like free().
	 *
	 * This is useful in cases where a process might have multiple users of
	 * libdeflate who want to use different memory allocators.  For example,
	 * a library might want to use libdeflate with a custom memory allocator
	 * without interfering with user code that might use libdeflate too.
	 *
	 * This takes priority over the "global" memory allocator (which by
	 * default is malloc() and free(), but can be changed by
	 * libdeflate_set_memory_allocator()).  Moreover, libdeflate will never
	 * call the "global" memory allocator if a per-(de)compressor custom
	 * allocator is always given.
	 */
	void *(*malloc_func)(size_t);
	void (*free_func)(void *);
};

#ifdef __cplusplus
}
#endif

#endif /* LIBDEFLATE_H */

/*** End of inlined file: libdeflate.h ***/

#include <stdbool.h>
#include <stddef.h>	/* for size_t */
#include <stdint.h>
#ifdef _MSC_VER
#  include <intrin.h>	/* for _BitScan*() and other intrinsics */
#  include <stdlib.h>	/* for _byteswap_*() */
   /* Disable MSVC warnings that are expected. */
   /* /W2 */
#  pragma warning(disable : 4146) /* unary minus on unsigned type */
   /* /W3 */
#  pragma warning(disable : 4018) /* signed/unsigned mismatch */
#  pragma warning(disable : 4244) /* possible loss of data */
#  pragma warning(disable : 4267) /* possible loss of precision */
#  pragma warning(disable : 4310) /* cast truncates constant value */
   /* /W4 */
#  pragma warning(disable : 4100) /* unreferenced formal parameter */
#  pragma warning(disable : 4127) /* conditional expression is constant */
#  pragma warning(disable : 4189) /* local variable initialized but not referenced */
#  pragma warning(disable : 4232) /* nonstandard extension used */
#  pragma warning(disable : 4245) /* conversion from 'int' to 'unsigned int' */
#  pragma warning(disable : 4295) /* array too small to include terminating null */
#endif
#ifndef FREESTANDING
#  include <string.h>	/* for memcpy() */
#endif

/* ========================================================================== */
/*                             Target architecture                            */
/* ========================================================================== */

/* If possible, define a compiler-independent ARCH_* macro. */
#undef ARCH_X86_64
#undef ARCH_X86_32
#undef ARCH_ARM64
#undef ARCH_ARM32
#undef ARCH_RISCV
#ifdef _MSC_VER
   /* Way too many things are broken in ARM64EC to pretend that it is x86_64. */
#  if defined(_M_X64) && !defined(_M_ARM64EC)
#    define ARCH_X86_64
#  elif defined(_M_IX86)
#    define ARCH_X86_32
#  elif defined(_M_ARM64)
#    define ARCH_ARM64
#  elif defined(_M_ARM)
#    define ARCH_ARM32
#  endif
#else
#  if defined(__x86_64__)
#    define ARCH_X86_64
#  elif defined(__i386__)
#    define ARCH_X86_32
#  elif defined(__aarch64__)
#    define ARCH_ARM64
#  elif defined(__arm__)
#    define ARCH_ARM32
#  elif defined(__riscv)
#    define ARCH_RISCV
#  endif
#endif

/* ========================================================================== */
/*                              Type definitions                              */
/* ========================================================================== */

/* Fixed-width integer types */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

/* ssize_t, if not available in <sys/types.h> */
#ifdef _MSC_VER
#  ifdef _WIN64
     typedef long long ssize_t;
#  else
     typedef long ssize_t;
#  endif
#endif

/*
 * Word type of the target architecture.  Use 'size_t' instead of
 * 'unsigned long' to account for platforms such as Windows that use 32-bit
 * 'unsigned long' on 64-bit architectures.
 */
typedef size_t machine_word_t;

/* Number of bytes in a word */
#define WORDBYTES	((int)sizeof(machine_word_t))

/* Number of bits in a word */
#define WORDBITS	(8 * WORDBYTES)

/* ========================================================================== */
/*                         Optional compiler features                         */
/* ========================================================================== */

/* Compiler version checks.  Only use when absolutely necessary. */
#if defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#  define GCC_PREREQ(major, minor)		\
	(__GNUC__ > (major) ||			\
	 (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))
#  if !GCC_PREREQ(4, 9)
#    error "gcc versions older than 4.9 are no longer supported"
#  endif
#else
#  define GCC_PREREQ(major, minor)	0
#endif
#ifdef __clang__
#  ifdef __apple_build_version__
#    define CLANG_PREREQ(major, minor, apple_version)	\
	(__apple_build_version__ >= (apple_version))
#  else
#    define CLANG_PREREQ(major, minor, apple_version)	\
	(__clang_major__ > (major) ||			\
	 (__clang_major__ == (major) && __clang_minor__ >= (minor)))
#  endif
#  if !CLANG_PREREQ(3, 9, 8000000)
#    error "clang versions older than 3.9 are no longer supported"
#  endif
#else
#  define CLANG_PREREQ(major, minor, apple_version)	0
#endif
#ifdef _MSC_VER
#  define MSVC_PREREQ(version)	(_MSC_VER >= (version))
#  if !MSVC_PREREQ(1900)
#    error "MSVC versions older than Visual Studio 2015 are no longer supported"
#  endif
#else
#  define MSVC_PREREQ(version)	0
#endif

/*
 * __has_attribute(attribute) - check whether the compiler supports the given
 * attribute (and also supports doing the check in the first place).  Mostly
 * useful just for clang, since gcc didn't add this macro until gcc 5.
 */
#ifndef __has_attribute
#  define __has_attribute(attribute)	0
#endif

/*
 * __has_builtin(builtin) - check whether the compiler supports the given
 * builtin (and also supports doing the check in the first place).  Mostly
 * useful just for clang, since gcc didn't add this macro until gcc 10.
 */
#ifndef __has_builtin
#  define __has_builtin(builtin)	0
#endif

/* inline - suggest that a function be inlined */
#ifdef _MSC_VER
#  define inline		__inline
#endif /* else assume 'inline' is usable as-is */

/* forceinline - force a function to be inlined, if possible */
#if defined(__GNUC__) || __has_attribute(always_inline)
#  define forceinline		inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#  define forceinline		__forceinline
#else
#  define forceinline		inline
#endif

/* MAYBE_UNUSED - mark a function or variable as maybe unused */
#if defined(__GNUC__) || __has_attribute(unused)
#  define MAYBE_UNUSED		__attribute__((unused))
#else
#  define MAYBE_UNUSED
#endif

/* NORETURN - mark a function as never returning, e.g. due to calling abort() */
#if defined(__GNUC__) || __has_attribute(noreturn)
#  define NORETURN		__attribute__((noreturn))
#else
#  define NORETURN
#endif

/*
 * restrict - hint that writes only occur through the given pointer.
 *
 * Don't use MSVC's __restrict, since it has nonstandard behavior.
 * Standard restrict is okay, if it is supported.
 */
#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 201112L)
#  if defined(__GNUC__) || defined(__clang__)
#    define restrict		__restrict__
#  else
#    define restrict
#  endif
#endif /* else assume 'restrict' is usable as-is */

/* likely(expr) - hint that an expression is usually true */
#if defined(__GNUC__) || __has_builtin(__builtin_expect)
#  define likely(expr)		__builtin_expect(!!(expr), 1)
#else
#  define likely(expr)		(expr)
#endif

/* unlikely(expr) - hint that an expression is usually false */
#if defined(__GNUC__) || __has_builtin(__builtin_expect)
#  define unlikely(expr)	__builtin_expect(!!(expr), 0)
#else
#  define unlikely(expr)	(expr)
#endif

/* prefetchr(addr) - prefetch into L1 cache for read */
#undef prefetchr
#if defined(__GNUC__) || __has_builtin(__builtin_prefetch)
#  define prefetchr(addr)	__builtin_prefetch((addr), 0)
#elif defined(_MSC_VER)
#  if defined(ARCH_X86_32) || defined(ARCH_X86_64)
#    define prefetchr(addr)	_mm_prefetch((addr), _MM_HINT_T0)
#  elif defined(ARCH_ARM64)
#    define prefetchr(addr)	__prefetch2((addr), 0x00 /* prfop=PLDL1KEEP */)
#  elif defined(ARCH_ARM32)
#    define prefetchr(addr)	__prefetch(addr)
#  endif
#endif
#ifndef prefetchr
#  define prefetchr(addr)
#endif

/* prefetchw(addr) - prefetch into L1 cache for write */
#undef prefetchw
#if defined(__GNUC__) || __has_builtin(__builtin_prefetch)
#  define prefetchw(addr)	__builtin_prefetch((addr), 1)
#elif defined(_MSC_VER)
#  if defined(ARCH_X86_32) || defined(ARCH_X86_64)
#    define prefetchw(addr)	_m_prefetchw(addr)
#  elif defined(ARCH_ARM64)
#    define prefetchw(addr)	__prefetch2((addr), 0x10 /* prfop=PSTL1KEEP */)
#  elif defined(ARCH_ARM32)
#    define prefetchw(addr)	__prefetchw(addr)
#  endif
#endif
#ifndef prefetchw
#  define prefetchw(addr)
#endif

/*
 * _aligned_attribute(n) - declare that the annotated variable, or variables of
 * the annotated type, must be aligned on n-byte boundaries.
 */
#undef _aligned_attribute
#if defined(__GNUC__) || __has_attribute(aligned)
#  define _aligned_attribute(n)	__attribute__((aligned(n)))
#elif defined(_MSC_VER)
#  define _aligned_attribute(n)	__declspec(align(n))
#endif

/*
 * _target_attribute(attrs) - override the compilation target for a function.
 *
 * This accepts one or more comma-separated suffixes to the -m prefix jointly
 * forming the name of a machine-dependent option.  On gcc-like compilers, this
 * enables codegen for the given targets, including arbitrary compiler-generated
 * code as well as the corresponding intrinsics.  On other compilers this macro
 * expands to nothing, though MSVC allows intrinsics to be used anywhere anyway.
 */
#if defined(__GNUC__) || __has_attribute(target)
#  define _target_attribute(attrs)	__attribute__((target(attrs)))
#else
#  define _target_attribute(attrs)
#endif

/* ========================================================================== */
/*                          Miscellaneous macros                              */
/* ========================================================================== */

#define ARRAY_LEN(A)		(sizeof(A) / sizeof((A)[0]))
#define MIN(a, b)		((a) <= (b) ? (a) : (b))
#define MAX(a, b)		((a) >= (b) ? (a) : (b))
#define DIV_ROUND_UP(n, d)	(((n) + (d) - 1) / (d))
#define STATIC_ASSERT(expr)	((void)sizeof(char[1 - 2 * !(expr)]))
#define ALIGN(n, a)		(((n) + (a) - 1) & ~((a) - 1))
#define ROUND_UP(n, d)		((d) * DIV_ROUND_UP((n), (d)))

/* ========================================================================== */
/*                           Endianness handling                              */
/* ========================================================================== */

/*
 * CPU_IS_LITTLE_ENDIAN() - 1 if the CPU is little endian, or 0 if it is big
 * endian.  When possible this is a compile-time macro that can be used in
 * preprocessor conditionals.  As a fallback, a generic method is used that
 * can't be used in preprocessor conditionals but should still be optimized out.
 */
#if defined(__BYTE_ORDER__) /* gcc v4.6+ and clang */
#  define CPU_IS_LITTLE_ENDIAN()  (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#elif defined(_MSC_VER)
#  define CPU_IS_LITTLE_ENDIAN()  true
#else
static forceinline bool CPU_IS_LITTLE_ENDIAN(void)
{
	union {
		u32 w;
		u8 b;
	} u;

	u.w = 1;
	return u.b;
}
#endif

/* bswap16(v) - swap the bytes of a 16-bit integer */
static forceinline u16 bswap16(u16 v)
{
#if defined(__GNUC__) || __has_builtin(__builtin_bswap16)
	return __builtin_bswap16(v);
#elif defined(_MSC_VER)
	return _byteswap_ushort(v);
#else
	return (v << 8) | (v >> 8);
#endif
}

/* bswap32(v) - swap the bytes of a 32-bit integer */
static forceinline u32 bswap32(u32 v)
{
#if defined(__GNUC__) || __has_builtin(__builtin_bswap32)
	return __builtin_bswap32(v);
#elif defined(_MSC_VER)
	return _byteswap_ulong(v);
#else
	return ((v & 0x000000FF) << 24) |
	       ((v & 0x0000FF00) << 8) |
	       ((v & 0x00FF0000) >> 8) |
	       ((v & 0xFF000000) >> 24);
#endif
}

/* bswap64(v) - swap the bytes of a 64-bit integer */
static forceinline u64 bswap64(u64 v)
{
#if defined(__GNUC__) || __has_builtin(__builtin_bswap64)
	return __builtin_bswap64(v);
#elif defined(_MSC_VER)
	return _byteswap_uint64(v);
#else
	return ((v & 0x00000000000000FF) << 56) |
	       ((v & 0x000000000000FF00) << 40) |
	       ((v & 0x0000000000FF0000) << 24) |
	       ((v & 0x00000000FF000000) << 8) |
	       ((v & 0x000000FF00000000) >> 8) |
	       ((v & 0x0000FF0000000000) >> 24) |
	       ((v & 0x00FF000000000000) >> 40) |
	       ((v & 0xFF00000000000000) >> 56);
#endif
}

#define le16_bswap(v) (CPU_IS_LITTLE_ENDIAN() ? (v) : bswap16(v))
#define le32_bswap(v) (CPU_IS_LITTLE_ENDIAN() ? (v) : bswap32(v))
#define le64_bswap(v) (CPU_IS_LITTLE_ENDIAN() ? (v) : bswap64(v))
#define be16_bswap(v) (CPU_IS_LITTLE_ENDIAN() ? bswap16(v) : (v))
#define be32_bswap(v) (CPU_IS_LITTLE_ENDIAN() ? bswap32(v) : (v))
#define be64_bswap(v) (CPU_IS_LITTLE_ENDIAN() ? bswap64(v) : (v))

/* ========================================================================== */
/*                          Unaligned memory accesses                         */
/* ========================================================================== */

/*
 * UNALIGNED_ACCESS_IS_FAST() - 1 if unaligned memory accesses can be performed
 * efficiently on the target platform, otherwise 0.
 */
#if (defined(__GNUC__) || defined(__clang__)) && \
	(defined(ARCH_X86_64) || defined(ARCH_X86_32) || \
	 defined(__ARM_FEATURE_UNALIGNED) || defined(__powerpc64__) || \
	 defined(__riscv_misaligned_fast) || \
	 /*
	  * For all compilation purposes, WebAssembly behaves like any other CPU
	  * instruction set. Even though WebAssembly engine might be running on
	  * top of different actual CPU architectures, the WebAssembly spec
	  * itself permits unaligned access and it will be fast on most of those
	  * platforms, and simulated at the engine level on others, so it's
	  * worth treating it as a CPU architecture with fast unaligned access.
	  */ defined(__wasm__))
#  define UNALIGNED_ACCESS_IS_FAST	1
#elif defined(_MSC_VER)
#  define UNALIGNED_ACCESS_IS_FAST	1
#else
#  define UNALIGNED_ACCESS_IS_FAST	0
#endif

/*
 * Implementing unaligned memory accesses using memcpy() is portable, and it
 * usually gets optimized appropriately by modern compilers.  I.e., each
 * memcpy() of 1, 2, 4, or WORDBYTES bytes gets compiled to a load or store
 * instruction, not to an actual function call.
 *
 * We no longer use the "packed struct" approach to unaligned accesses, as that
 * is nonstandard, has unclear semantics, and doesn't receive enough testing
 * (see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=94994).
 *
 * arm32 with __ARM_FEATURE_UNALIGNED in gcc 5 and earlier is a known exception
 * where memcpy() generates inefficient code
 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67366).  However, we no longer
 * consider that one case important enough to maintain different code for.
 * If you run into it, please just use a newer version of gcc (or use clang).
 */

#ifdef FREESTANDING
#  define MEMCOPY	__builtin_memcpy
#else
#  define MEMCOPY	memcpy
#endif

/* Unaligned loads and stores without endianness conversion */

#define DEFINE_UNALIGNED_TYPE(type)				\
static forceinline type						\
load_##type##_unaligned(const void *p)				\
{								\
	type v;							\
								\
	MEMCOPY(&v, p, sizeof(v));				\
	return v;						\
}								\
								\
static forceinline void						\
store_##type##_unaligned(type v, void *p)			\
{								\
	MEMCOPY(p, &v, sizeof(v));				\
}

DEFINE_UNALIGNED_TYPE(u16)
DEFINE_UNALIGNED_TYPE(u32)
DEFINE_UNALIGNED_TYPE(u64)
DEFINE_UNALIGNED_TYPE(machine_word_t)

#undef MEMCOPY

#define load_word_unaligned	load_machine_word_t_unaligned
#define store_word_unaligned	store_machine_word_t_unaligned

/* Unaligned loads with endianness conversion */

static forceinline u16
get_unaligned_le16(const u8 *p)
{
	if (UNALIGNED_ACCESS_IS_FAST)
		return le16_bswap(load_u16_unaligned(p));
	else
		return ((u16)p[1] << 8) | p[0];
}

static forceinline u16
get_unaligned_be16(const u8 *p)
{
	if (UNALIGNED_ACCESS_IS_FAST)
		return be16_bswap(load_u16_unaligned(p));
	else
		return ((u16)p[0] << 8) | p[1];
}

static forceinline u32
get_unaligned_le32(const u8 *p)
{
	if (UNALIGNED_ACCESS_IS_FAST)
		return le32_bswap(load_u32_unaligned(p));
	else
		return ((u32)p[3] << 24) | ((u32)p[2] << 16) |
			((u32)p[1] << 8) | p[0];
}

static forceinline u32
get_unaligned_be32(const u8 *p)
{
	if (UNALIGNED_ACCESS_IS_FAST)
		return be32_bswap(load_u32_unaligned(p));
	else
		return ((u32)p[0] << 24) | ((u32)p[1] << 16) |
			((u32)p[2] << 8) | p[3];
}

static forceinline u64
get_unaligned_le64(const u8 *p)
{
	if (UNALIGNED_ACCESS_IS_FAST)
		return le64_bswap(load_u64_unaligned(p));
	else
		return ((u64)p[7] << 56) | ((u64)p[6] << 48) |
			((u64)p[5] << 40) | ((u64)p[4] << 32) |
			((u64)p[3] << 24) | ((u64)p[2] << 16) |
			((u64)p[1] << 8) | p[0];
}

static forceinline machine_word_t
get_unaligned_leword(const u8 *p)
{
	STATIC_ASSERT(WORDBITS == 32 || WORDBITS == 64);
	if (WORDBITS == 32)
		return get_unaligned_le32(p);
	else
		return get_unaligned_le64(p);
}

/* Unaligned stores with endianness conversion */

static forceinline void
put_unaligned_le16(u16 v, u8 *p)
{
	if (UNALIGNED_ACCESS_IS_FAST) {
		store_u16_unaligned(le16_bswap(v), p);
	} else {
		p[0] = (u8)(v >> 0);
		p[1] = (u8)(v >> 8);
	}
}

static forceinline void
put_unaligned_be16(u16 v, u8 *p)
{
	if (UNALIGNED_ACCESS_IS_FAST) {
		store_u16_unaligned(be16_bswap(v), p);
	} else {
		p[0] = (u8)(v >> 8);
		p[1] = (u8)(v >> 0);
	}
}

static forceinline void
put_unaligned_le32(u32 v, u8 *p)
{
	if (UNALIGNED_ACCESS_IS_FAST) {
		store_u32_unaligned(le32_bswap(v), p);
	} else {
		p[0] = (u8)(v >> 0);
		p[1] = (u8)(v >> 8);
		p[2] = (u8)(v >> 16);
		p[3] = (u8)(v >> 24);
	}
}

static forceinline void
put_unaligned_be32(u32 v, u8 *p)
{
	if (UNALIGNED_ACCESS_IS_FAST) {
		store_u32_unaligned(be32_bswap(v), p);
	} else {
		p[0] = (u8)(v >> 24);
		p[1] = (u8)(v >> 16);
		p[2] = (u8)(v >> 8);
		p[3] = (u8)(v >> 0);
	}
}

static forceinline void
put_unaligned_le64(u64 v, u8 *p)
{
	if (UNALIGNED_ACCESS_IS_FAST) {
		store_u64_unaligned(le64_bswap(v), p);
	} else {
		p[0] = (u8)(v >> 0);
		p[1] = (u8)(v >> 8);
		p[2] = (u8)(v >> 16);
		p[3] = (u8)(v >> 24);
		p[4] = (u8)(v >> 32);
		p[5] = (u8)(v >> 40);
		p[6] = (u8)(v >> 48);
		p[7] = (u8)(v >> 56);
	}
}

static forceinline void
put_unaligned_leword(machine_word_t v, u8 *p)
{
	STATIC_ASSERT(WORDBITS == 32 || WORDBITS == 64);
	if (WORDBITS == 32)
		put_unaligned_le32(v, p);
	else
		put_unaligned_le64(v, p);
}

/* ========================================================================== */
/*                         Bit manipulation functions                         */
/* ========================================================================== */

/*
 * Bit Scan Reverse (BSR) - find the 0-based index (relative to the least
 * significant end) of the *most* significant 1 bit in the input value.  The
 * input value must be nonzero!
 */

static forceinline unsigned
bsr32(u32 v)
{
#if defined(__GNUC__) || __has_builtin(__builtin_clz)
	return 31 - __builtin_clz(v);
#elif defined(_MSC_VER)
	unsigned long i;

	_BitScanReverse(&i, v);
	return i;
#else
	unsigned i = 0;

	while ((v >>= 1) != 0)
		i++;
	return i;
#endif
}

static forceinline unsigned
bsr64(u64 v)
{
#if defined(__GNUC__) || __has_builtin(__builtin_clzll)
	return 63 - __builtin_clzll(v);
#elif defined(_MSC_VER) && defined(_WIN64)
	unsigned long i;

	_BitScanReverse64(&i, v);
	return i;
#else
	unsigned i = 0;

	while ((v >>= 1) != 0)
		i++;
	return i;
#endif
}

static forceinline unsigned
bsrw(machine_word_t v)
{
	STATIC_ASSERT(WORDBITS == 32 || WORDBITS == 64);
	if (WORDBITS == 32)
		return bsr32(v);
	else
		return bsr64(v);
}

/*
 * Bit Scan Forward (BSF) - find the 0-based index (relative to the least
 * significant end) of the *least* significant 1 bit in the input value.  The
 * input value must be nonzero!
 */

static forceinline unsigned
bsf32(u32 v)
{
#if defined(__GNUC__) || __has_builtin(__builtin_ctz)
	return __builtin_ctz(v);
#elif defined(_MSC_VER)
	unsigned long i;

	_BitScanForward(&i, v);
	return i;
#else
	unsigned i = 0;

	for (; (v & 1) == 0; v >>= 1)
		i++;
	return i;
#endif
}

static forceinline unsigned
bsf64(u64 v)
{
#if defined(__GNUC__) || __has_builtin(__builtin_ctzll)
	return __builtin_ctzll(v);
#elif defined(_MSC_VER) && defined(_WIN64)
	unsigned long i;

	_BitScanForward64(&i, v);
	return i;
#else
	unsigned i = 0;

	for (; (v & 1) == 0; v >>= 1)
		i++;
	return i;
#endif
}

static forceinline unsigned
bsfw(machine_word_t v)
{
	STATIC_ASSERT(WORDBITS == 32 || WORDBITS == 64);
	if (WORDBITS == 32)
		return bsf32(v);
	else
		return bsf64(v);
}

/*
 * rbit32(v): reverse the bits in a 32-bit integer.  This doesn't have a
 * fallback implementation; use '#ifdef rbit32' to check if this is available.
 */
#undef rbit32
#if (defined(__GNUC__) || defined(__clang__)) && defined(ARCH_ARM32) && \
	(__ARM_ARCH >= 7 || (__ARM_ARCH == 6 && defined(__ARM_ARCH_6T2__)))
static forceinline u32
rbit32(u32 v)
{
	__asm__("rbit %0, %1" : "=r" (v) : "r" (v));
	return v;
}
#define rbit32 rbit32
#elif (defined(__GNUC__) || defined(__clang__)) && defined(ARCH_ARM64)
static forceinline u32
rbit32(u32 v)
{
	__asm__("rbit %w0, %w1" : "=r" (v) : "r" (v));
	return v;
}
#define rbit32 rbit32
#endif

#endif /* COMMON_DEFS_H */

/*** End of inlined file: common_defs.h ***/

typedef void *(*malloc_func_t)(size_t);
typedef void (*free_func_t)(void *);

extern malloc_func_t libdeflate_default_malloc_func;
extern free_func_t libdeflate_default_free_func;

void *libdeflate_aligned_malloc(malloc_func_t malloc_func,
				size_t alignment, size_t size);
void libdeflate_aligned_free(free_func_t free_func, void *ptr);

#ifdef FREESTANDING
/*
 * With -ffreestanding, <string.h> may be missing, and we must provide
 * implementations of memset(), memcpy(), memmove(), and memcmp().
 * See https://gcc.gnu.org/onlinedocs/gcc/Standards.html
 *
 * Also, -ffreestanding disables interpreting calls to these functions as
 * built-ins.  E.g., calling memcpy(&v, p, WORDBYTES) will make a function call,
 * not be optimized to a single load instruction.  For performance reasons we
 * don't want that.  So, declare these functions as macros that expand to the
 * corresponding built-ins.  This approach is recommended in the gcc man page.
 * We still need the actual function definitions in case gcc calls them.
 */
void *memset(void *s, int c, size_t n);
#define memset(s, c, n)		__builtin_memset((s), (c), (n))

void *memcpy(void *dest, const void *src, size_t n);
#define memcpy(dest, src, n)	__builtin_memcpy((dest), (src), (n))

void *memmove(void *dest, const void *src, size_t n);
#define memmove(dest, src, n)	__builtin_memmove((dest), (src), (n))

int memcmp(const void *s1, const void *s2, size_t n);
#define memcmp(s1, s2, n)	__builtin_memcmp((s1), (s2), (n))

#undef LIBDEFLATE_ENABLE_ASSERTIONS
#else
#  include <string.h>
   /*
    * To prevent false positive static analyzer warnings, ensure that assertions
    * are visible to the static analyzer.
    */
#  ifdef __clang_analyzer__
#    define LIBDEFLATE_ENABLE_ASSERTIONS
#  endif
#endif

/*
 * Runtime assertion support.  Don't enable this in production builds; it may
 * hurt performance significantly.
 */
#ifdef LIBDEFLATE_ENABLE_ASSERTIONS
NORETURN void
libdeflate_assertion_failed(const char *expr, const char *file, int line);
#define ASSERT(expr) { if (unlikely(!(expr))) \
	libdeflate_assertion_failed(#expr, __FILE__, __LINE__); }
#else
#define ASSERT(expr) (void)(expr)
#endif

#define CONCAT_IMPL(a, b)	a##b
#define CONCAT(a, b)		CONCAT_IMPL(a, b)
#define ADD_SUFFIX(name)	CONCAT(name, SUFFIX)

#endif /* LIB_LIB_COMMON_H */

/*** End of inlined file: lib_common.h ***/

/* The Adler-32 divisor, or "base", value */
#define DIVISOR 65521

/*
 * MAX_CHUNK_LEN is the most bytes that can be processed without the possibility
 * of s2 overflowing when it is represented as an unsigned 32-bit integer.  This
 * value was computed using the following Python script:
 *
 *	divisor = 65521
 *	count = 0
 *	s1 = divisor - 1
 *	s2 = divisor - 1
 *	while True:
 *		s1 += 0xFF
 *		s2 += s1
 *		if s2 > 0xFFFFFFFF:
 *			break
 *		count += 1
 *	print(count)
 *
 * Note that to get the correct worst-case value, we must assume that every byte
 * has value 0xFF and that s1 and s2 started with the highest possible values
 * modulo the divisor.
 */
#define MAX_CHUNK_LEN	5552

/*
 * Update the Adler-32 values s1 and s2 using n bytes from p, update p to p + n,
 * update n to 0, and reduce s1 and s2 mod DIVISOR.  It is assumed that neither
 * s1 nor s2 can overflow before the reduction at the end, i.e. n plus any bytes
 * already processed after the last reduction must not exceed MAX_CHUNK_LEN.
 *
 * This uses only portable C code.  This is used as a fallback when a vectorized
 * implementation of Adler-32 (e.g. AVX2) is unavailable on the platform.
 *
 * Some of the vectorized implementations also use this to handle the end of the
 * data when the data isn't evenly divisible by the length the vectorized code
 * works on.  To avoid compiler errors about target-specific option mismatches
 * when this is used in that way, this is a macro rather than a function.
 *
 * Although this is unvectorized, this does include an optimization where the
 * main loop processes four bytes at a time using a strategy similar to that
 * used by vectorized implementations.  This provides increased instruction-
 * level parallelism compared to the traditional 's1 += *p++; s2 += s1;'.
 */
#define ADLER32_CHUNK(s1, s2, p, n)					\
do {									\
	if (n >= 4) {							\
		u32 s1_sum = 0;						\
		u32 byte_0_sum = 0;					\
		u32 byte_1_sum = 0;					\
		u32 byte_2_sum = 0;					\
		u32 byte_3_sum = 0;					\
									\
		do {							\
			s1_sum += s1;					\
			s1 += p[0] + p[1] + p[2] + p[3];		\
			byte_0_sum += p[0];				\
			byte_1_sum += p[1];				\
			byte_2_sum += p[2];				\
			byte_3_sum += p[3];				\
			p += 4;						\
			n -= 4;						\
		} while (n >= 4);					\
		s2 += (4 * (s1_sum + byte_0_sum)) + (3 * byte_1_sum) +	\
		      (2 * byte_2_sum) + byte_3_sum;			\
	}								\
	for (; n; n--, p++) {						\
		s1 += *p;						\
		s2 += s1;						\
	}								\
	s1 %= DIVISOR;							\
	s2 %= DIVISOR;							\
} while (0)

static u32 MAYBE_UNUSED
adler32_generic(u32 adler, const u8 *p, size_t len)
{
	u32 s1 = adler & 0xFFFF;
	u32 s2 = adler >> 16;

	while (len) {
		size_t n = MIN(len, MAX_CHUNK_LEN & ~3);

		len -= n;
		ADLER32_CHUNK(s1, s2, p, n);
	}

	return (s2 << 16) | s1;
}

/* Include architecture-specific implementation(s) if available. */
#undef DEFAULT_IMPL
#undef arch_select_adler32_func
typedef u32 (*adler32_func_t)(u32 adler, const u8 *p, size_t len);
#if defined(ARCH_ARM32) || defined(ARCH_ARM64)

/*** Start of inlined file: adler32_impl.h ***/
#ifndef LIB_ARM_ADLER32_IMPL_H
#define LIB_ARM_ADLER32_IMPL_H


/*** Start of inlined file: cpu_features.h ***/
#ifndef LIB_ARM_CPU_FEATURES_H
#define LIB_ARM_CPU_FEATURES_H

#if defined(ARCH_ARM32) || defined(ARCH_ARM64)

#define ARM_CPU_FEATURE_NEON		(1 << 0)
#define ARM_CPU_FEATURE_PMULL		(1 << 1)
/*
 * PREFER_PMULL indicates that the CPU has very high pmull throughput, and so
 * the 12x wide pmull-based CRC-32 implementation is likely to be faster than an
 * implementation based on the crc32 instructions.
 */
#define ARM_CPU_FEATURE_PREFER_PMULL	(1 << 2)
#define ARM_CPU_FEATURE_CRC32		(1 << 3)
#define ARM_CPU_FEATURE_SHA3		(1 << 4)
#define ARM_CPU_FEATURE_DOTPROD		(1 << 5)

#if !defined(FREESTANDING) && \
    (defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)) && \
    (defined(__linux__) || \
     (defined(__APPLE__) && defined(ARCH_ARM64)) || \
     (defined(_WIN32) && defined(ARCH_ARM64)))
/* Runtime ARM CPU feature detection is supported. */
#  define ARM_CPU_FEATURES_KNOWN	(1U << 31)
extern volatile u32 libdeflate_arm_cpu_features;

void libdeflate_init_arm_cpu_features(void);

static inline u32 get_arm_cpu_features(void)
{
	if (libdeflate_arm_cpu_features == 0)
		libdeflate_init_arm_cpu_features();
	return libdeflate_arm_cpu_features;
}
#else
static inline u32 get_arm_cpu_features(void) { return 0; }
#endif

/* NEON */
#if defined(__ARM_NEON) || (defined(_MSC_VER) && defined(ARCH_ARM64))
#  define HAVE_NEON(features)	1
#  define HAVE_NEON_NATIVE	1
#else
#  define HAVE_NEON(features)	((features) & ARM_CPU_FEATURE_NEON)
#  define HAVE_NEON_NATIVE	0
#endif
/*
 * With both gcc and clang, NEON intrinsics require that the main target has
 * NEON enabled already.  Exception: with gcc 6.1 and later (r230411 for arm32,
 * r226563 for arm64), hardware floating point support is sufficient.
 */
#if (defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)) && \
	(HAVE_NEON_NATIVE || (GCC_PREREQ(6, 1) && defined(__ARM_FP)))
#  define HAVE_NEON_INTRIN	1
#  include <arm_neon.h>
#else
#  define HAVE_NEON_INTRIN	0
#endif

/* PMULL */
#ifdef __ARM_FEATURE_CRYPTO
#  define HAVE_PMULL(features)	1
#else
#  define HAVE_PMULL(features)	((features) & ARM_CPU_FEATURE_PMULL)
#endif
#if defined(ARCH_ARM64) && HAVE_NEON_INTRIN && \
	(GCC_PREREQ(7, 1) || defined(__clang__) || defined(_MSC_VER)) && \
	CPU_IS_LITTLE_ENDIAN() /* untested on big endian */
#  define HAVE_PMULL_INTRIN	1
   /* Work around MSVC's vmull_p64() taking poly64x1_t instead of poly64_t */
#  ifdef _MSC_VER
#    define compat_vmull_p64(a, b)  vmull_p64(vcreate_p64(a), vcreate_p64(b))
#  else
#    define compat_vmull_p64(a, b)  vmull_p64((a), (b))
#  endif
#else
#  define HAVE_PMULL_INTRIN	0
#endif

/* CRC32 */
#ifdef __ARM_FEATURE_CRC32
#  define HAVE_CRC32(features)	1
#else
#  define HAVE_CRC32(features)	((features) & ARM_CPU_FEATURE_CRC32)
#endif
#if defined(ARCH_ARM64) && \
	(defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER))
#  define HAVE_CRC32_INTRIN	1
#  if defined(__GNUC__) || defined(__clang__)
#    include <arm_acle.h>
#  endif
   /*
    * Use an inline assembly fallback for clang 15 and earlier, which only
    * defined the crc32 intrinsics when crc32 is enabled in the main target.
    */
#  if defined(__clang__) && !CLANG_PREREQ(16, 0, 16000000) && \
	!defined(__ARM_FEATURE_CRC32)
#    undef __crc32b
#    define __crc32b(a, b)					\
	({ uint32_t res;					\
	   __asm__("crc32b %w0, %w1, %w2"			\
		   : "=r" (res) : "r" (a), "r" (b));		\
	   res; })
#    undef __crc32h
#    define __crc32h(a, b)					\
	({ uint32_t res;					\
	   __asm__("crc32h %w0, %w1, %w2"			\
		   : "=r" (res) : "r" (a), "r" (b));		\
	   res; })
#    undef __crc32w
#    define __crc32w(a, b)					\
	({ uint32_t res;					\
	   __asm__("crc32w %w0, %w1, %w2"			\
		   : "=r" (res) : "r" (a), "r" (b));		\
	   res; })
#    undef __crc32d
#    define __crc32d(a, b)					\
	({ uint32_t res;					\
	   __asm__("crc32x %w0, %w1, %2"			\
		   : "=r" (res) : "r" (a), "r" (b));		\
	   res; })
#    pragma clang diagnostic ignored "-Wgnu-statement-expression"
#  endif
#else
#  define HAVE_CRC32_INTRIN	0
#endif

/* SHA3 (needed for the eor3 instruction) */
#ifdef __ARM_FEATURE_SHA3
#  define HAVE_SHA3(features)	1
#else
#  define HAVE_SHA3(features)	((features) & ARM_CPU_FEATURE_SHA3)
#endif
#if defined(ARCH_ARM64) && HAVE_NEON_INTRIN && \
	(GCC_PREREQ(9, 1) /* r268049 */ || \
	 CLANG_PREREQ(7, 0, 10010463) /* r338010 */)
#  define HAVE_SHA3_INTRIN	1
   /*
    * Use an inline assembly fallback for clang 15 and earlier, which only
    * defined the sha3 intrinsics when sha3 is enabled in the main target.
    */
#  if defined(__clang__) && !CLANG_PREREQ(16, 0, 16000000) && \
	!defined(__ARM_FEATURE_SHA3)
#    undef veor3q_u8
#    define veor3q_u8(a, b, c)					\
	({ uint8x16_t res;					\
	   __asm__("eor3 %0.16b, %1.16b, %2.16b, %3.16b"	\
		   : "=w" (res) : "w" (a), "w" (b), "w" (c));	\
	   res; })
#    pragma clang diagnostic ignored "-Wgnu-statement-expression"
#  endif
#else
#  define HAVE_SHA3_INTRIN	0
#endif

/* dotprod */
#ifdef __ARM_FEATURE_DOTPROD
#  define HAVE_DOTPROD(features)	1
#else
#  define HAVE_DOTPROD(features)	((features) & ARM_CPU_FEATURE_DOTPROD)
#endif
#if defined(ARCH_ARM64) && HAVE_NEON_INTRIN && \
	(GCC_PREREQ(8, 1) || CLANG_PREREQ(7, 0, 10010000) || defined(_MSC_VER))
#  define HAVE_DOTPROD_INTRIN	1
   /*
    * Use an inline assembly fallback for clang 15 and earlier, which only
    * defined the dotprod intrinsics when dotprod is enabled in the main target.
    */
#  if defined(__clang__) && !CLANG_PREREQ(16, 0, 16000000) && \
	!defined(__ARM_FEATURE_DOTPROD)
#    undef vdotq_u32
#    define vdotq_u32(a, b, c)					\
	({ uint32x4_t res = (a);				\
	   __asm__("udot %0.4s, %1.16b, %2.16b"			\
		   : "+w" (res) : "w" (b), "w" (c));		\
	   res; })
#    pragma clang diagnostic ignored "-Wgnu-statement-expression"
#  endif
#else
#  define HAVE_DOTPROD_INTRIN	0
#endif

#endif /* ARCH_ARM32 || ARCH_ARM64 */

#endif /* LIB_ARM_CPU_FEATURES_H */

/*** End of inlined file: cpu_features.h ***/

/* Regular NEON implementation */
#if HAVE_NEON_INTRIN && CPU_IS_LITTLE_ENDIAN()
#  define adler32_arm_neon	adler32_arm_neon
#  if HAVE_NEON_NATIVE
     /*
      * Use no attributes if none are needed, to support old versions of clang
      * that don't accept the simd target attribute.
      */
#    define ATTRIBUTES
#  elif defined(ARCH_ARM32)
#    define ATTRIBUTES	_target_attribute("fpu=neon")
#  elif defined(__clang__)
#    define ATTRIBUTES	_target_attribute("simd")
#  else
#    define ATTRIBUTES	_target_attribute("+simd")
#  endif
static ATTRIBUTES MAYBE_UNUSED u32
adler32_arm_neon(u32 adler, const u8 *p, size_t len)
{
	static const u16 _aligned_attribute(16) mults[64] = {
		64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
		48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
		32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
		16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
	};
	const uint16x8_t mults_a = vld1q_u16(&mults[0]);
	const uint16x8_t mults_b = vld1q_u16(&mults[8]);
	const uint16x8_t mults_c = vld1q_u16(&mults[16]);
	const uint16x8_t mults_d = vld1q_u16(&mults[24]);
	const uint16x8_t mults_e = vld1q_u16(&mults[32]);
	const uint16x8_t mults_f = vld1q_u16(&mults[40]);
	const uint16x8_t mults_g = vld1q_u16(&mults[48]);
	const uint16x8_t mults_h = vld1q_u16(&mults[56]);
	u32 s1 = adler & 0xFFFF;
	u32 s2 = adler >> 16;

	/*
	 * If the length is large and the pointer is misaligned, align it.
	 * For smaller lengths, just take the misaligned load penalty.
	 */
	if (unlikely(len > 32768 && ((uintptr_t)p & 15))) {
		do {
			s1 += *p++;
			s2 += s1;
			len--;
		} while ((uintptr_t)p & 15);
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}

	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX.
		 */
		size_t n = MIN(len, MAX_CHUNK_LEN & ~63);

		len -= n;

		if (n >= 64) {
			uint32x4_t v_s1 = vdupq_n_u32(0);
			uint32x4_t v_s2 = vdupq_n_u32(0);
			/*
			 * v_byte_sums_* contain the sum of the bytes at index i
			 * across all 64-byte segments, for each index 0..63.
			 */
			uint16x8_t v_byte_sums_a = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_b = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_c = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_d = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_e = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_f = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_g = vdupq_n_u16(0);
			uint16x8_t v_byte_sums_h = vdupq_n_u16(0);

			s2 += s1 * (n & ~63);

			do {
				/* Load the next 64 data bytes. */
				const uint8x16_t data_a = vld1q_u8(p + 0);
				const uint8x16_t data_b = vld1q_u8(p + 16);
				const uint8x16_t data_c = vld1q_u8(p + 32);
				const uint8x16_t data_d = vld1q_u8(p + 48);
				uint16x8_t tmp;

				/*
				 * Accumulate the previous s1 counters into the
				 * s2 counters.  The needed multiplication by 64
				 * is delayed to later.
				 */
				v_s2 = vaddq_u32(v_s2, v_s1);

				/*
				 * Add the 64 data bytes to their v_byte_sums
				 * counters, while also accumulating the sums of
				 * each adjacent set of 4 bytes into v_s1.
				 */
				tmp = vpaddlq_u8(data_a);
				v_byte_sums_a = vaddw_u8(v_byte_sums_a,
							 vget_low_u8(data_a));
				v_byte_sums_b = vaddw_u8(v_byte_sums_b,
							 vget_high_u8(data_a));
				tmp = vpadalq_u8(tmp, data_b);
				v_byte_sums_c = vaddw_u8(v_byte_sums_c,
							 vget_low_u8(data_b));
				v_byte_sums_d = vaddw_u8(v_byte_sums_d,
							 vget_high_u8(data_b));
				tmp = vpadalq_u8(tmp, data_c);
				v_byte_sums_e = vaddw_u8(v_byte_sums_e,
							 vget_low_u8(data_c));
				v_byte_sums_f = vaddw_u8(v_byte_sums_f,
							 vget_high_u8(data_c));
				tmp = vpadalq_u8(tmp, data_d);
				v_byte_sums_g = vaddw_u8(v_byte_sums_g,
							 vget_low_u8(data_d));
				v_byte_sums_h = vaddw_u8(v_byte_sums_h,
							 vget_high_u8(data_d));
				v_s1 = vpadalq_u16(v_s1, tmp);

				p += 64;
				n -= 64;
			} while (n >= 64);

			/* s2 = 64*s2 + (64*bytesum0 + 63*bytesum1 + ... + 1*bytesum63) */
		#ifdef ARCH_ARM32
		#  define umlal2(a, b, c)  vmlal_u16((a), vget_high_u16(b), vget_high_u16(c))
		#else
		#  define umlal2	   vmlal_high_u16
		#endif
			v_s2 = vqshlq_n_u32(v_s2, 6);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_a),
					 vget_low_u16(mults_a));
			v_s2 = umlal2(v_s2, v_byte_sums_a, mults_a);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_b),
					 vget_low_u16(mults_b));
			v_s2 = umlal2(v_s2, v_byte_sums_b, mults_b);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_c),
					 vget_low_u16(mults_c));
			v_s2 = umlal2(v_s2, v_byte_sums_c, mults_c);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_d),
					 vget_low_u16(mults_d));
			v_s2 = umlal2(v_s2, v_byte_sums_d, mults_d);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_e),
					 vget_low_u16(mults_e));
			v_s2 = umlal2(v_s2, v_byte_sums_e, mults_e);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_f),
					 vget_low_u16(mults_f));
			v_s2 = umlal2(v_s2, v_byte_sums_f, mults_f);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_g),
					 vget_low_u16(mults_g));
			v_s2 = umlal2(v_s2, v_byte_sums_g, mults_g);
			v_s2 = vmlal_u16(v_s2, vget_low_u16(v_byte_sums_h),
					 vget_low_u16(mults_h));
			v_s2 = umlal2(v_s2, v_byte_sums_h, mults_h);
		#undef umlal2

			/* Horizontal sum to finish up */
		#ifdef ARCH_ARM32
			s1 += vgetq_lane_u32(v_s1, 0) + vgetq_lane_u32(v_s1, 1) +
			      vgetq_lane_u32(v_s1, 2) + vgetq_lane_u32(v_s1, 3);
			s2 += vgetq_lane_u32(v_s2, 0) + vgetq_lane_u32(v_s2, 1) +
			      vgetq_lane_u32(v_s2, 2) + vgetq_lane_u32(v_s2, 3);
		#else
			s1 += vaddvq_u32(v_s1);
			s2 += vaddvq_u32(v_s2);
		#endif
		}
		/*
		 * Process the last 0 <= n < 64 bytes of the chunk using
		 * scalar instructions and reduce s1 and s2 mod DIVISOR.
		 */
		ADLER32_CHUNK(s1, s2, p, n);
	}
	return (s2 << 16) | s1;
}
#undef ATTRIBUTES
#endif /* Regular NEON implementation */

/* NEON+dotprod implementation */
#if HAVE_DOTPROD_INTRIN && CPU_IS_LITTLE_ENDIAN() && \
	!defined(LIBDEFLATE_ASSEMBLER_DOES_NOT_SUPPORT_DOTPROD)
#  define adler32_arm_neon_dotprod	adler32_arm_neon_dotprod
#  ifdef __clang__
#    define ATTRIBUTES	_target_attribute("dotprod")
   /*
    * Both gcc and binutils originally considered dotprod to depend on
    * arch=armv8.2-a or later.  This was fixed in gcc 13.2 by commit
    * 9aac37ab8a7b ("aarch64: Remove architecture dependencies from intrinsics")
    * and in binutils 2.41 by commit 205e4380c800 ("aarch64: Remove version
    * dependencies from features").  Unfortunately, always using arch=armv8.2-a
    * causes build errors with some compiler options because it may reduce the
    * arch rather than increase it.  Therefore we try to omit the arch whenever
    * possible.  If gcc is 14 or later, then both gcc and binutils are probably
    * fixed, so we omit the arch.  We also omit the arch if a feature that
    * depends on armv8.2-a or later (in gcc 13.1 and earlier) is present.
    */
#  elif GCC_PREREQ(14, 0) || defined(__ARM_FEATURE_JCVT) \
			  || defined(__ARM_FEATURE_DOTPROD)
#    define ATTRIBUTES	_target_attribute("+dotprod")
#  else
#    define ATTRIBUTES	_target_attribute("arch=armv8.2-a+dotprod")
#  endif
static ATTRIBUTES u32
adler32_arm_neon_dotprod(u32 adler, const u8 *p, size_t len)
{
	static const u8 _aligned_attribute(16) mults[64] = {
		64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
		48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
		32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
		16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
	};
	const uint8x16_t mults_a = vld1q_u8(&mults[0]);
	const uint8x16_t mults_b = vld1q_u8(&mults[16]);
	const uint8x16_t mults_c = vld1q_u8(&mults[32]);
	const uint8x16_t mults_d = vld1q_u8(&mults[48]);
	const uint8x16_t ones = vdupq_n_u8(1);
	u32 s1 = adler & 0xFFFF;
	u32 s2 = adler >> 16;

	/*
	 * If the length is large and the pointer is misaligned, align it.
	 * For smaller lengths, just take the misaligned load penalty.
	 */
	if (unlikely(len > 32768 && ((uintptr_t)p & 15))) {
		do {
			s1 += *p++;
			s2 += s1;
			len--;
		} while ((uintptr_t)p & 15);
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}

	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX.
		 */
		size_t n = MIN(len, MAX_CHUNK_LEN & ~63);

		len -= n;

		if (n >= 64) {
			uint32x4_t v_s1_a = vdupq_n_u32(0);
			uint32x4_t v_s1_b = vdupq_n_u32(0);
			uint32x4_t v_s1_c = vdupq_n_u32(0);
			uint32x4_t v_s1_d = vdupq_n_u32(0);
			uint32x4_t v_s2_a = vdupq_n_u32(0);
			uint32x4_t v_s2_b = vdupq_n_u32(0);
			uint32x4_t v_s2_c = vdupq_n_u32(0);
			uint32x4_t v_s2_d = vdupq_n_u32(0);
			uint32x4_t v_s1_sums_a = vdupq_n_u32(0);
			uint32x4_t v_s1_sums_b = vdupq_n_u32(0);
			uint32x4_t v_s1_sums_c = vdupq_n_u32(0);
			uint32x4_t v_s1_sums_d = vdupq_n_u32(0);
			uint32x4_t v_s1;
			uint32x4_t v_s2;
			uint32x4_t v_s1_sums;

			s2 += s1 * (n & ~63);

			do {
				uint8x16_t data_a = vld1q_u8(p + 0);
				uint8x16_t data_b = vld1q_u8(p + 16);
				uint8x16_t data_c = vld1q_u8(p + 32);
				uint8x16_t data_d = vld1q_u8(p + 48);

				v_s1_sums_a = vaddq_u32(v_s1_sums_a, v_s1_a);
				v_s1_a = vdotq_u32(v_s1_a, data_a, ones);
				v_s2_a = vdotq_u32(v_s2_a, data_a, mults_a);

				v_s1_sums_b = vaddq_u32(v_s1_sums_b, v_s1_b);
				v_s1_b = vdotq_u32(v_s1_b, data_b, ones);
				v_s2_b = vdotq_u32(v_s2_b, data_b, mults_b);

				v_s1_sums_c = vaddq_u32(v_s1_sums_c, v_s1_c);
				v_s1_c = vdotq_u32(v_s1_c, data_c, ones);
				v_s2_c = vdotq_u32(v_s2_c, data_c, mults_c);

				v_s1_sums_d = vaddq_u32(v_s1_sums_d, v_s1_d);
				v_s1_d = vdotq_u32(v_s1_d, data_d, ones);
				v_s2_d = vdotq_u32(v_s2_d, data_d, mults_d);

				p += 64;
				n -= 64;
			} while (n >= 64);

			v_s1 = vaddq_u32(vaddq_u32(v_s1_a, v_s1_b),
					 vaddq_u32(v_s1_c, v_s1_d));
			v_s2 = vaddq_u32(vaddq_u32(v_s2_a, v_s2_b),
					 vaddq_u32(v_s2_c, v_s2_d));
			v_s1_sums = vaddq_u32(vaddq_u32(v_s1_sums_a,
							v_s1_sums_b),
					      vaddq_u32(v_s1_sums_c,
							v_s1_sums_d));
			v_s2 = vaddq_u32(v_s2, vqshlq_n_u32(v_s1_sums, 6));

			s1 += vaddvq_u32(v_s1);
			s2 += vaddvq_u32(v_s2);
		}
		/*
		 * Process the last 0 <= n < 64 bytes of the chunk using
		 * scalar instructions and reduce s1 and s2 mod DIVISOR.
		 */
		ADLER32_CHUNK(s1, s2, p, n);
	}
	return (s2 << 16) | s1;
}
#undef ATTRIBUTES
#endif /* NEON+dotprod implementation */

#if defined(adler32_arm_neon_dotprod) && defined(__ARM_FEATURE_DOTPROD)
#define DEFAULT_IMPL	adler32_arm_neon_dotprod
#else
static inline adler32_func_t
arch_select_adler32_func(void)
{
	const u32 features MAYBE_UNUSED = get_arm_cpu_features();

#ifdef adler32_arm_neon_dotprod
	if (HAVE_NEON(features) && HAVE_DOTPROD(features))
		return adler32_arm_neon_dotprod;
#endif
#ifdef adler32_arm_neon
	if (HAVE_NEON(features))
		return adler32_arm_neon;
#endif
	return NULL;
}
#define arch_select_adler32_func	arch_select_adler32_func
#endif

#endif /* LIB_ARM_ADLER32_IMPL_H */

/*** End of inlined file: adler32_impl.h ***/


#elif defined(ARCH_X86_32) || defined(ARCH_X86_64)

/*** Start of inlined file: adler32_impl.h ***/
#ifndef LIB_X86_ADLER32_IMPL_H
#define LIB_X86_ADLER32_IMPL_H


/*** Start of inlined file: cpu_features.h ***/
#ifndef LIB_X86_CPU_FEATURES_H
#define LIB_X86_CPU_FEATURES_H

#if defined(ARCH_X86_32) || defined(ARCH_X86_64)

#define X86_CPU_FEATURE_SSE2		(1 << 0)
#define X86_CPU_FEATURE_PCLMULQDQ	(1 << 1)
#define X86_CPU_FEATURE_AVX		(1 << 2)
#define X86_CPU_FEATURE_AVX2		(1 << 3)
#define X86_CPU_FEATURE_BMI2		(1 << 4)
/*
 * ZMM indicates whether 512-bit vectors (zmm registers) should be used.  On
 * some CPUs, to avoid downclocking issues we don't set ZMM even if the CPU and
 * operating system support AVX-512.  On these CPUs, we may still use AVX-512
 * instructions, but only with xmm and ymm registers.
 */
#define X86_CPU_FEATURE_ZMM		(1 << 5)
#define X86_CPU_FEATURE_AVX512BW	(1 << 6)
#define X86_CPU_FEATURE_AVX512VL	(1 << 7)
#define X86_CPU_FEATURE_VPCLMULQDQ	(1 << 8)
#define X86_CPU_FEATURE_AVX512VNNI	(1 << 9)
#define X86_CPU_FEATURE_AVXVNNI		(1 << 10)

#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
/* Runtime x86 CPU feature detection is supported. */
#  define X86_CPU_FEATURES_KNOWN	(1U << 31)
extern volatile u32 libdeflate_x86_cpu_features;

void libdeflate_init_x86_cpu_features(void);

static inline u32 get_x86_cpu_features(void)
{
	if (libdeflate_x86_cpu_features == 0)
		libdeflate_init_x86_cpu_features();
	return libdeflate_x86_cpu_features;
}
/*
 * x86 intrinsics are also supported.  Include the headers needed to use them.
 * Normally just immintrin.h suffices.  With clang in MSVC compatibility mode,
 * immintrin.h incorrectly skips including sub-headers, so include those too.
 */
#  include <immintrin.h>
#  if defined(_MSC_VER) && defined(__clang__)
#    include <tmmintrin.h>
#    include <smmintrin.h>
#    include <wmmintrin.h>
#    include <avxintrin.h>
#    include <avx2intrin.h>
#    include <avx512fintrin.h>
#    include <avx512bwintrin.h>
#    include <avx512vlintrin.h>
#    if __has_include(<avx512vlbwintrin.h>)
#      include <avx512vlbwintrin.h>
#    endif
#    if __has_include(<vpclmulqdqintrin.h>)
#      include <vpclmulqdqintrin.h>
#    endif
#    if __has_include(<avx512vnniintrin.h>)
#      include <avx512vnniintrin.h>
#    endif
#    if __has_include(<avx512vlvnniintrin.h>)
#      include <avx512vlvnniintrin.h>
#    endif
#    if __has_include(<avxvnniintrin.h>)
#      include <avxvnniintrin.h>
#    endif
#  endif
#else
static inline u32 get_x86_cpu_features(void) { return 0; }
#endif

#if defined(__SSE2__) || \
	(defined(_MSC_VER) && \
	 (defined(ARCH_X86_64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)))
#  define HAVE_SSE2(features)		1
#  define HAVE_SSE2_NATIVE		1
#else
#  define HAVE_SSE2(features)		((features) & X86_CPU_FEATURE_SSE2)
#  define HAVE_SSE2_NATIVE		0
#endif

#if (defined(__PCLMUL__) && defined(__SSE4_1__)) || \
	(defined(_MSC_VER) && defined(__AVX2__))
#  define HAVE_PCLMULQDQ(features)	1
#else
#  define HAVE_PCLMULQDQ(features)	((features) & X86_CPU_FEATURE_PCLMULQDQ)
#endif

#ifdef __AVX__
#  define HAVE_AVX(features)		1
#else
#  define HAVE_AVX(features)		((features) & X86_CPU_FEATURE_AVX)
#endif

#ifdef __AVX2__
#  define HAVE_AVX2(features)		1
#else
#  define HAVE_AVX2(features)		((features) & X86_CPU_FEATURE_AVX2)
#endif

#if defined(__BMI2__) || (defined(_MSC_VER) && defined(__AVX2__))
#  define HAVE_BMI2(features)		1
#  define HAVE_BMI2_NATIVE		1
#else
#  define HAVE_BMI2(features)		((features) & X86_CPU_FEATURE_BMI2)
#  define HAVE_BMI2_NATIVE		0
#endif

#ifdef __AVX512BW__
#  define HAVE_AVX512BW(features)	1
#else
#  define HAVE_AVX512BW(features)	((features) & X86_CPU_FEATURE_AVX512BW)
#endif

#ifdef __AVX512VL__
#  define HAVE_AVX512VL(features)	1
#else
#  define HAVE_AVX512VL(features)	((features) & X86_CPU_FEATURE_AVX512VL)
#endif

#ifdef __VPCLMULQDQ__
#  define HAVE_VPCLMULQDQ(features)	1
#else
#  define HAVE_VPCLMULQDQ(features)	((features) & X86_CPU_FEATURE_VPCLMULQDQ)
#endif

#ifdef __AVX512VNNI__
#  define HAVE_AVX512VNNI(features)	1
#else
#  define HAVE_AVX512VNNI(features)	((features) & X86_CPU_FEATURE_AVX512VNNI)
#endif

#ifdef __AVXVNNI__
#  define HAVE_AVXVNNI(features)	1
#else
#  define HAVE_AVXVNNI(features)	((features) & X86_CPU_FEATURE_AVXVNNI)
#endif

#endif /* ARCH_X86_32 || ARCH_X86_64 */

#endif /* LIB_X86_CPU_FEATURES_H */

/*** End of inlined file: cpu_features.h ***/

/* SSE2 and AVX2 implementations.  Used on older CPUs. */
#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
#  define adler32_x86_sse2	adler32_x86_sse2
#  define SUFFIX			   _sse2
#  define ATTRIBUTES		_target_attribute("sse2")
#  define VL			16
#  define USE_VNNI		0
#  define USE_AVX512		0

/*** Start of inlined file: adler32_template.h ***/
/*
 * This file is a "template" for instantiating Adler-32 functions for x86.
 * The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_VNNI=0 && USE_AVX512=0: at least sse2
 *	   VL=32 && USE_VNNI=0 && USE_AVX512=0: at least avx2
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=0: at least avx2,avxvnni
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vl,avx512vnni
 *	   VL=64 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vnni
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_VNNI:
 *	If 1, use the VNNI dot product based algorithm.
 *	If 0, use the legacy SSE2 and AVX2 compatible algorithm.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking.  This doesn't
 *	enable the use of 512-bit vectors; the vector length is controlled by
 *	VL.  If 0, assume that the CPU might not support AVX-512.
 */

#if VL == 16
#  define vec_t			__m128i
#  define mask_t		u16
#  define LOG2_VL		4
#  define VADD8(a, b)		_mm_add_epi8((a), (b))
#  define VADD16(a, b)		_mm_add_epi16((a), (b))
#  define VADD32(a, b)		_mm_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm_load_si128((const void *)(p))
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VMADD16(a, b)		_mm_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm_set1_epi8(a)
#  define VSET1_32(a)		_mm_set1_epi32(a)
#  define VSETZERO()		_mm_setzero_si128()
#  define VSLL32(a, b)		_mm_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm_unpackhi_epi8((a), (b))
#elif VL == 32
#  define vec_t			__m256i
#  define mask_t		u32
#  define LOG2_VL		5
#  define VADD8(a, b)		_mm256_add_epi8((a), (b))
#  define VADD16(a, b)		_mm256_add_epi16((a), (b))
#  define VADD32(a, b)		_mm256_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm256_load_si256((const void *)(p))
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VMADD16(a, b)		_mm256_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm256_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm256_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm256_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm256_set1_epi8(a)
#  define VSET1_32(a)		_mm256_set1_epi32(a)
#  define VSETZERO()		_mm256_setzero_si256()
#  define VSLL32(a, b)		_mm256_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm256_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm256_unpackhi_epi8((a), (b))
#elif VL == 64
#  define vec_t			__m512i
#  define mask_t		u64
#  define LOG2_VL		6
#  define VADD8(a, b)		_mm512_add_epi8((a), (b))
#  define VADD16(a, b)		_mm512_add_epi16((a), (b))
#  define VADD32(a, b)		_mm512_add_epi32((a), (b))
#  define VDPBUSD(a, b, c)	_mm512_dpbusd_epi32((a), (b), (c))
#  define VLOAD(p)		_mm512_load_si512((const void *)(p))
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VMADD16(a, b)		_mm512_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm512_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm512_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm512_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm512_set1_epi8(a)
#  define VSET1_32(a)		_mm512_set1_epi32(a)
#  define VSETZERO()		_mm512_setzero_si512()
#  define VSLL32(a, b)		_mm512_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm512_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm512_unpackhi_epi8((a), (b))
#else
#  error "unsupported vector length"
#endif

#define VADD32_3X(a, b, c)	VADD32(VADD32((a), (b)), (c))
#define VADD32_4X(a, b, c, d)	VADD32(VADD32((a), (b)), VADD32((c), (d)))
#define VADD32_5X(a, b, c, d, e) VADD32((a), VADD32_4X((b), (c), (d), (e)))
#define VADD32_7X(a, b, c, d, e, f, g)	\
	VADD32(VADD32_3X((a), (b), (c)), VADD32_4X((d), (e), (f), (g)))

/* Sum the 32-bit elements of v_s1 and add them to s1, and likewise for s2. */
#undef reduce_to_32bits
static forceinline ATTRIBUTES void
ADD_SUFFIX(reduce_to_32bits)(vec_t v_s1, vec_t v_s2, u32 *s1_p, u32 *s2_p)
{
	__m128i v_s1_128, v_s2_128;
#if VL == 16
	{
		v_s1_128 = v_s1;
		v_s2_128 = v_s2;
	}
#else
	{
		__m256i v_s1_256, v_s2_256;
	#if VL == 32
		v_s1_256 = v_s1;
		v_s2_256 = v_s2;
	#else
		/* Reduce 512 bits to 256 bits. */
		v_s1_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s1, 0),
					    _mm512_extracti64x4_epi64(v_s1, 1));
		v_s2_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s2, 0),
					    _mm512_extracti64x4_epi64(v_s2, 1));
	#endif
		/* Reduce 256 bits to 128 bits. */
		v_s1_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s1_256, 0),
					 _mm256_extracti128_si256(v_s1_256, 1));
		v_s2_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s2_256, 0),
					 _mm256_extracti128_si256(v_s2_256, 1));
	}
#endif

	/*
	 * Reduce 128 bits to 32 bits.
	 *
	 * If the bytes were summed into v_s1 using psadbw + paddd, then ignore
	 * the odd-indexed elements of v_s1_128 since they are zero.
	 */
#if USE_VNNI
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x31));
#endif
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x31));
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x02));
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x02));

	*s1_p += (u32)_mm_cvtsi128_si32(v_s1_128);
	*s2_p += (u32)_mm_cvtsi128_si32(v_s2_128);
}
#define reduce_to_32bits	ADD_SUFFIX(reduce_to_32bits)

static ATTRIBUTES u32
ADD_SUFFIX(adler32_x86)(u32 adler, const u8 *p, size_t len)
{
#if USE_VNNI
	/* This contains the bytes [VL, VL-1, VL-2, ..., 1]. */
	static const u8 _aligned_attribute(VL) raw_mults[VL] = {
	#if VL == 64
		64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
		48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
	#endif
	#if VL >= 32
		32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
	#endif
		16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
	};
	const vec_t ones = VSET1_8(1);
#else
	/*
	 * This contains the 16-bit values [2*VL, 2*VL - 1, 2*VL - 2, ..., 1].
	 * For VL==32 the ordering is weird because it has to match the way that
	 * vpunpcklbw and vpunpckhbw work on 128-bit lanes separately.
	 */
	static const u16 _aligned_attribute(VL) raw_mults[4][VL / 2] = {
	#if VL == 16
		{ 32, 31, 30, 29, 28, 27, 26, 25 },
		{ 24, 23, 22, 21, 20, 19, 18, 17 },
		{ 16, 15, 14, 13, 12, 11, 10, 9  },
		{ 8,  7,  6,  5,  4,  3,  2,  1  },
	#elif VL == 32
		{ 64, 63, 62, 61, 60, 59, 58, 57, 48, 47, 46, 45, 44, 43, 42, 41 },
		{ 56, 55, 54, 53, 52, 51, 50, 49, 40, 39, 38, 37, 36, 35, 34, 33 },
		{ 32, 31, 30, 29, 28, 27, 26, 25, 16, 15, 14, 13, 12, 11, 10,  9 },
		{ 24, 23, 22, 21, 20, 19, 18, 17,  8,  7,  6,  5,  4,  3,  2,  1 },
	#else
	#  error "unsupported parameters"
	#endif
	};
	const vec_t mults_a = VLOAD(raw_mults[0]);
	const vec_t mults_b = VLOAD(raw_mults[1]);
	const vec_t mults_c = VLOAD(raw_mults[2]);
	const vec_t mults_d = VLOAD(raw_mults[3]);
#endif
	const vec_t zeroes = VSETZERO();
	u32 s1 = adler & 0xFFFF;
	u32 s2 = adler >> 16;

	/*
	 * If the length is large and the pointer is misaligned, align it.
	 * For smaller lengths, just take the misaligned load penalty.
	 */
	if (unlikely(len > 65536 && ((uintptr_t)p & (VL-1)))) {
		do {
			s1 += *p++;
			s2 += s1;
			len--;
		} while ((uintptr_t)p & (VL-1));
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}

#if USE_VNNI
	/*
	 * This is Adler-32 using the vpdpbusd instruction from AVX512VNNI or
	 * AVX-VNNI.  vpdpbusd multiplies the unsigned bytes of one vector by
	 * the signed bytes of another vector and adds the sums in groups of 4
	 * to the 32-bit elements of a third vector.  We use it in two ways:
	 * multiplying the data bytes by a sequence like 64,63,62,...,1 for
	 * calculating part of s2, and multiplying the data bytes by an all-ones
	 * sequence 1,1,1,...,1 for calculating s1 and part of s2.  The all-ones
	 * trick seems to be faster than the alternative of vpsadbw + vpaddd.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX.
		 */
		size_t n = MIN(len, MAX_CHUNK_LEN & ~(4*VL - 1));
		vec_t mults = VLOAD(raw_mults);
		vec_t v_s1 = zeroes;
		vec_t v_s2 = zeroes;

		s2 += s1 * n;
		len -= n;

		if (n >= 4*VL) {
			vec_t v_s1_b = zeroes;
			vec_t v_s1_c = zeroes;
			vec_t v_s1_d = zeroes;
			vec_t v_s2_b = zeroes;
			vec_t v_s2_c = zeroes;
			vec_t v_s2_d = zeroes;
			vec_t v_s1_sums   = zeroes;
			vec_t v_s1_sums_b = zeroes;
			vec_t v_s1_sums_c = zeroes;
			vec_t v_s1_sums_d = zeroes;
			vec_t tmp0, tmp1;

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);
				vec_t data_c = VLOADU(p + 2*VL);
				vec_t data_d = VLOADU(p + 3*VL);

				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+v" (data_a), "+v" (data_b),
					     "+v" (data_c), "+v" (data_d));
			#endif

				v_s2   = VDPBUSD(v_s2,   data_a, mults);
				v_s2_b = VDPBUSD(v_s2_b, data_b, mults);
				v_s2_c = VDPBUSD(v_s2_c, data_c, mults);
				v_s2_d = VDPBUSD(v_s2_d, data_d, mults);

				v_s1_sums   = VADD32(v_s1_sums,   v_s1);
				v_s1_sums_b = VADD32(v_s1_sums_b, v_s1_b);
				v_s1_sums_c = VADD32(v_s1_sums_c, v_s1_c);
				v_s1_sums_d = VADD32(v_s1_sums_d, v_s1_d);

				v_s1   = VDPBUSD(v_s1,   data_a, ones);
				v_s1_b = VDPBUSD(v_s1_b, data_b, ones);
				v_s1_c = VDPBUSD(v_s1_c, data_c, ones);
				v_s1_d = VDPBUSD(v_s1_d, data_d, ones);

				/* Same gcc bug workaround.  See above */
			#if GCC_PREREQ(1, 0) && !defined(ARCH_X86_32)
				__asm__("" : "+v" (v_s2), "+v" (v_s2_b),
					     "+v" (v_s2_c), "+v" (v_s2_d),
					     "+v" (v_s1_sums),
					     "+v" (v_s1_sums_b),
					     "+v" (v_s1_sums_c),
					     "+v" (v_s1_sums_d),
					     "+v" (v_s1), "+v" (v_s1_b),
					     "+v" (v_s1_c), "+v" (v_s1_d));
			#endif
				p += 4*VL;
				n -= 4*VL;
			} while (n >= 4*VL);

			/*
			 * Reduce into v_s1 and v_s2 as follows:
			 *
			 * v_s2 = v_s2 + v_s2_b + v_s2_c + v_s2_d +
			 *	  (4*VL)*(v_s1_sums   + v_s1_sums_b +
			 *		  v_s1_sums_c + v_s1_sums_d) +
			 *	  (3*VL)*v_s1 + (2*VL)*v_s1_b + VL*v_s1_c
			 * v_s1 = v_s1 + v_s1_b + v_s1_c + v_s1_d
			 */
			tmp0 = VADD32(v_s1, v_s1_b);
			tmp1 = VADD32(v_s1, v_s1_c);
			v_s1_sums = VADD32_4X(v_s1_sums, v_s1_sums_b,
					      v_s1_sums_c, v_s1_sums_d);
			v_s1 = VADD32_3X(tmp0, v_s1_c, v_s1_d);
			v_s2 = VADD32_7X(VSLL32(v_s1_sums, LOG2_VL + 2),
					 VSLL32(tmp0, LOG2_VL + 1),
					 VSLL32(tmp1, LOG2_VL),
					 v_s2, v_s2_b, v_s2_c, v_s2_d);
		}

		/* Process the last 0 <= n < 4*VL bytes of the chunk. */
		if (n >= 2*VL) {
			const vec_t data_a = VLOADU(p + 0*VL);
			const vec_t data_b = VLOADU(p + 1*VL);

			v_s2 = VADD32(v_s2, VSLL32(v_s1, LOG2_VL + 1));
			v_s1 = VDPBUSD(v_s1, data_a, ones);
			v_s1 = VDPBUSD(v_s1, data_b, ones);
			v_s2 = VDPBUSD(v_s2, data_a, VSET1_8(VL));
			v_s2 = VDPBUSD(v_s2, data_a, mults);
			v_s2 = VDPBUSD(v_s2, data_b, mults);
			p += 2*VL;
			n -= 2*VL;
		}
		if (n) {
			/* Process the last 0 < n < 2*VL bytes of the chunk. */
			vec_t data;

			v_s2 = VADD32(v_s2, VMULLO32(v_s1, VSET1_32(n)));

			mults = VADD8(mults, VSET1_8((int)n - VL));
			if (n > VL) {
				data = VLOADU(p);
				v_s1 = VDPBUSD(v_s1, data, ones);
				v_s2 = VDPBUSD(v_s2, data, mults);
				p += VL;
				n -= VL;
				mults = VADD8(mults, VSET1_8(-VL));
			}
			/*
			 * Process the last 0 < n <= VL bytes of the chunk.
			 * Utilize a masked load if it's available.
			 */
		#if USE_AVX512
			data = VMASKZ_LOADU((mask_t)-1 >> (VL - n), p);
		#else
			data = zeroes;
			memcpy(&data, p, n);
		#endif
			v_s1 = VDPBUSD(v_s1, data, ones);
			v_s2 = VDPBUSD(v_s2, data, mults);
			p += n;
		}

		reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}
#else /* USE_VNNI */
	/*
	 * This is Adler-32 for SSE2 and AVX2.
	 *
	 * To horizontally sum bytes, use psadbw + paddd, where one of the
	 * arguments to psadbw is all-zeroes.
	 *
	 * For the s2 contribution from (2*VL - i)*data[i] for each of the 2*VL
	 * bytes of each iteration of the inner loop, use punpck{l,h}bw + paddw
	 * to sum, for each i across iterations, byte i into a corresponding
	 * 16-bit counter in v_byte_sums_*.  After the inner loop, use pmaddwd
	 * to multiply each counter by (2*VL - i), then add the products to s2.
	 *
	 * An alternative implementation would use pmaddubsw and pmaddwd in the
	 * inner loop to do (2*VL - i)*data[i] directly and add the products in
	 * groups of 4 to 32-bit counters.  However, on average that approach
	 * seems to be slower than the current approach which delays the
	 * multiplications.  Also, pmaddubsw requires SSSE3; the current
	 * approach keeps the implementation aligned between SSE2 and AVX2.
	 *
	 * The inner loop processes 2*VL bytes per iteration.  Increasing this
	 * to 4*VL doesn't seem to be helpful here.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX, and every
		 * v_byte_sums_* counter is guaranteed to not exceed INT16_MAX.
		 * It's INT16_MAX, not UINT16_MAX, because v_byte_sums_* are
		 * used with pmaddwd which does signed multiplication.  In the
		 * SSE2 case this limits chunks to 4096 bytes instead of 5536.
		 */
		size_t n = MIN(len, MIN(2 * VL * (INT16_MAX / UINT8_MAX),
					MAX_CHUNK_LEN) & ~(2*VL - 1));
		len -= n;

		if (n >= 2*VL) {
			vec_t v_s1 = zeroes;
			vec_t v_s1_sums = zeroes;
			vec_t v_byte_sums_a = zeroes;
			vec_t v_byte_sums_b = zeroes;
			vec_t v_byte_sums_c = zeroes;
			vec_t v_byte_sums_d = zeroes;
			vec_t v_s2;

			s2 += s1 * (n & ~(2*VL - 1));

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);

				v_s1_sums = VADD32(v_s1_sums, v_s1);
				v_byte_sums_a = VADD16(v_byte_sums_a,
						       VUNPACKLO8(data_a, zeroes));
				v_byte_sums_b = VADD16(v_byte_sums_b,
						       VUNPACKHI8(data_a, zeroes));
				v_byte_sums_c = VADD16(v_byte_sums_c,
						       VUNPACKLO8(data_b, zeroes));
				v_byte_sums_d = VADD16(v_byte_sums_d,
						       VUNPACKHI8(data_b, zeroes));
				v_s1 = VADD32(v_s1,
					      VADD32(VSAD8(data_a, zeroes),
						     VSAD8(data_b, zeroes)));
				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+x" (v_s1), "+x" (v_s1_sums),
					     "+x" (v_byte_sums_a),
					     "+x" (v_byte_sums_b),
					     "+x" (v_byte_sums_c),
					     "+x" (v_byte_sums_d));
			#endif
				p += 2*VL;
				n -= 2*VL;
			} while (n >= 2*VL);

			/*
			 * Calculate v_s2 as (2*VL)*v_s1_sums +
			 * [2*VL, 2*VL - 1, 2*VL - 2, ..., 1] * v_byte_sums.
			 * Then update s1 and s2 from v_s1 and v_s2.
			 */
			v_s2 = VADD32_5X(VSLL32(v_s1_sums, LOG2_VL + 1),
					 VMADD16(v_byte_sums_a, mults_a),
					 VMADD16(v_byte_sums_b, mults_b),
					 VMADD16(v_byte_sums_c, mults_c),
					 VMADD16(v_byte_sums_d, mults_d));
			reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		}
		/*
		 * Process the last 0 <= n < 2*VL bytes of the chunk using
		 * scalar instructions and reduce s1 and s2 mod DIVISOR.
		 */
		ADLER32_CHUNK(s1, s2, p, n);
	}
#endif /* !USE_VNNI */
	return (s2 << 16) | s1;
}

#undef vec_t
#undef mask_t
#undef LOG2_VL
#undef VADD8
#undef VADD16
#undef VADD32
#undef VDPBUSD
#undef VLOAD
#undef VLOADU
#undef VMADD16
#undef VMASKZ_LOADU
#undef VMULLO32
#undef VSAD8
#undef VSET1_8
#undef VSET1_32
#undef VSETZERO
#undef VSLL32
#undef VUNPACKLO8
#undef VUNPACKHI8

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_VNNI
#undef USE_AVX512

/*** End of inlined file: adler32_template.h ***/


#  define adler32_x86_avx2	adler32_x86_avx2
#  define SUFFIX			   _avx2
#  define ATTRIBUTES		_target_attribute("avx2")
#  define VL			32
#  define USE_VNNI		0
#  define USE_AVX512		0

/*** Start of inlined file: adler32_template.h ***/
/*
 * This file is a "template" for instantiating Adler-32 functions for x86.
 * The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_VNNI=0 && USE_AVX512=0: at least sse2
 *	   VL=32 && USE_VNNI=0 && USE_AVX512=0: at least avx2
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=0: at least avx2,avxvnni
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vl,avx512vnni
 *	   VL=64 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vnni
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_VNNI:
 *	If 1, use the VNNI dot product based algorithm.
 *	If 0, use the legacy SSE2 and AVX2 compatible algorithm.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking.  This doesn't
 *	enable the use of 512-bit vectors; the vector length is controlled by
 *	VL.  If 0, assume that the CPU might not support AVX-512.
 */

#if VL == 16
#  define vec_t			__m128i
#  define mask_t		u16
#  define LOG2_VL		4
#  define VADD8(a, b)		_mm_add_epi8((a), (b))
#  define VADD16(a, b)		_mm_add_epi16((a), (b))
#  define VADD32(a, b)		_mm_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm_load_si128((const void *)(p))
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VMADD16(a, b)		_mm_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm_set1_epi8(a)
#  define VSET1_32(a)		_mm_set1_epi32(a)
#  define VSETZERO()		_mm_setzero_si128()
#  define VSLL32(a, b)		_mm_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm_unpackhi_epi8((a), (b))
#elif VL == 32
#  define vec_t			__m256i
#  define mask_t		u32
#  define LOG2_VL		5
#  define VADD8(a, b)		_mm256_add_epi8((a), (b))
#  define VADD16(a, b)		_mm256_add_epi16((a), (b))
#  define VADD32(a, b)		_mm256_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm256_load_si256((const void *)(p))
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VMADD16(a, b)		_mm256_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm256_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm256_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm256_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm256_set1_epi8(a)
#  define VSET1_32(a)		_mm256_set1_epi32(a)
#  define VSETZERO()		_mm256_setzero_si256()
#  define VSLL32(a, b)		_mm256_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm256_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm256_unpackhi_epi8((a), (b))
#elif VL == 64
#  define vec_t			__m512i
#  define mask_t		u64
#  define LOG2_VL		6
#  define VADD8(a, b)		_mm512_add_epi8((a), (b))
#  define VADD16(a, b)		_mm512_add_epi16((a), (b))
#  define VADD32(a, b)		_mm512_add_epi32((a), (b))
#  define VDPBUSD(a, b, c)	_mm512_dpbusd_epi32((a), (b), (c))
#  define VLOAD(p)		_mm512_load_si512((const void *)(p))
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VMADD16(a, b)		_mm512_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm512_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm512_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm512_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm512_set1_epi8(a)
#  define VSET1_32(a)		_mm512_set1_epi32(a)
#  define VSETZERO()		_mm512_setzero_si512()
#  define VSLL32(a, b)		_mm512_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm512_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm512_unpackhi_epi8((a), (b))
#else
#  error "unsupported vector length"
#endif

#define VADD32_3X(a, b, c)	VADD32(VADD32((a), (b)), (c))
#define VADD32_4X(a, b, c, d)	VADD32(VADD32((a), (b)), VADD32((c), (d)))
#define VADD32_5X(a, b, c, d, e) VADD32((a), VADD32_4X((b), (c), (d), (e)))
#define VADD32_7X(a, b, c, d, e, f, g)	\
	VADD32(VADD32_3X((a), (b), (c)), VADD32_4X((d), (e), (f), (g)))

/* Sum the 32-bit elements of v_s1 and add them to s1, and likewise for s2. */
#undef reduce_to_32bits
static forceinline ATTRIBUTES void
ADD_SUFFIX(reduce_to_32bits)(vec_t v_s1, vec_t v_s2, u32 *s1_p, u32 *s2_p)
{
	__m128i v_s1_128, v_s2_128;
#if VL == 16
	{
		v_s1_128 = v_s1;
		v_s2_128 = v_s2;
	}
#else
	{
		__m256i v_s1_256, v_s2_256;
	#if VL == 32
		v_s1_256 = v_s1;
		v_s2_256 = v_s2;
	#else
		/* Reduce 512 bits to 256 bits. */
		v_s1_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s1, 0),
					    _mm512_extracti64x4_epi64(v_s1, 1));
		v_s2_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s2, 0),
					    _mm512_extracti64x4_epi64(v_s2, 1));
	#endif
		/* Reduce 256 bits to 128 bits. */
		v_s1_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s1_256, 0),
					 _mm256_extracti128_si256(v_s1_256, 1));
		v_s2_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s2_256, 0),
					 _mm256_extracti128_si256(v_s2_256, 1));
	}
#endif

	/*
	 * Reduce 128 bits to 32 bits.
	 *
	 * If the bytes were summed into v_s1 using psadbw + paddd, then ignore
	 * the odd-indexed elements of v_s1_128 since they are zero.
	 */
#if USE_VNNI
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x31));
#endif
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x31));
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x02));
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x02));

	*s1_p += (u32)_mm_cvtsi128_si32(v_s1_128);
	*s2_p += (u32)_mm_cvtsi128_si32(v_s2_128);
}
#define reduce_to_32bits	ADD_SUFFIX(reduce_to_32bits)

static ATTRIBUTES u32
ADD_SUFFIX(adler32_x86)(u32 adler, const u8 *p, size_t len)
{
#if USE_VNNI
	/* This contains the bytes [VL, VL-1, VL-2, ..., 1]. */
	static const u8 _aligned_attribute(VL) raw_mults[VL] = {
	#if VL == 64
		64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
		48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
	#endif
	#if VL >= 32
		32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
	#endif
		16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
	};
	const vec_t ones = VSET1_8(1);
#else
	/*
	 * This contains the 16-bit values [2*VL, 2*VL - 1, 2*VL - 2, ..., 1].
	 * For VL==32 the ordering is weird because it has to match the way that
	 * vpunpcklbw and vpunpckhbw work on 128-bit lanes separately.
	 */
	static const u16 _aligned_attribute(VL) raw_mults[4][VL / 2] = {
	#if VL == 16
		{ 32, 31, 30, 29, 28, 27, 26, 25 },
		{ 24, 23, 22, 21, 20, 19, 18, 17 },
		{ 16, 15, 14, 13, 12, 11, 10, 9  },
		{ 8,  7,  6,  5,  4,  3,  2,  1  },
	#elif VL == 32
		{ 64, 63, 62, 61, 60, 59, 58, 57, 48, 47, 46, 45, 44, 43, 42, 41 },
		{ 56, 55, 54, 53, 52, 51, 50, 49, 40, 39, 38, 37, 36, 35, 34, 33 },
		{ 32, 31, 30, 29, 28, 27, 26, 25, 16, 15, 14, 13, 12, 11, 10,  9 },
		{ 24, 23, 22, 21, 20, 19, 18, 17,  8,  7,  6,  5,  4,  3,  2,  1 },
	#else
	#  error "unsupported parameters"
	#endif
	};
	const vec_t mults_a = VLOAD(raw_mults[0]);
	const vec_t mults_b = VLOAD(raw_mults[1]);
	const vec_t mults_c = VLOAD(raw_mults[2]);
	const vec_t mults_d = VLOAD(raw_mults[3]);
#endif
	const vec_t zeroes = VSETZERO();
	u32 s1 = adler & 0xFFFF;
	u32 s2 = adler >> 16;

	/*
	 * If the length is large and the pointer is misaligned, align it.
	 * For smaller lengths, just take the misaligned load penalty.
	 */
	if (unlikely(len > 65536 && ((uintptr_t)p & (VL-1)))) {
		do {
			s1 += *p++;
			s2 += s1;
			len--;
		} while ((uintptr_t)p & (VL-1));
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}

#if USE_VNNI
	/*
	 * This is Adler-32 using the vpdpbusd instruction from AVX512VNNI or
	 * AVX-VNNI.  vpdpbusd multiplies the unsigned bytes of one vector by
	 * the signed bytes of another vector and adds the sums in groups of 4
	 * to the 32-bit elements of a third vector.  We use it in two ways:
	 * multiplying the data bytes by a sequence like 64,63,62,...,1 for
	 * calculating part of s2, and multiplying the data bytes by an all-ones
	 * sequence 1,1,1,...,1 for calculating s1 and part of s2.  The all-ones
	 * trick seems to be faster than the alternative of vpsadbw + vpaddd.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX.
		 */
		size_t n = MIN(len, MAX_CHUNK_LEN & ~(4*VL - 1));
		vec_t mults = VLOAD(raw_mults);
		vec_t v_s1 = zeroes;
		vec_t v_s2 = zeroes;

		s2 += s1 * n;
		len -= n;

		if (n >= 4*VL) {
			vec_t v_s1_b = zeroes;
			vec_t v_s1_c = zeroes;
			vec_t v_s1_d = zeroes;
			vec_t v_s2_b = zeroes;
			vec_t v_s2_c = zeroes;
			vec_t v_s2_d = zeroes;
			vec_t v_s1_sums   = zeroes;
			vec_t v_s1_sums_b = zeroes;
			vec_t v_s1_sums_c = zeroes;
			vec_t v_s1_sums_d = zeroes;
			vec_t tmp0, tmp1;

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);
				vec_t data_c = VLOADU(p + 2*VL);
				vec_t data_d = VLOADU(p + 3*VL);

				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+v" (data_a), "+v" (data_b),
					     "+v" (data_c), "+v" (data_d));
			#endif

				v_s2   = VDPBUSD(v_s2,   data_a, mults);
				v_s2_b = VDPBUSD(v_s2_b, data_b, mults);
				v_s2_c = VDPBUSD(v_s2_c, data_c, mults);
				v_s2_d = VDPBUSD(v_s2_d, data_d, mults);

				v_s1_sums   = VADD32(v_s1_sums,   v_s1);
				v_s1_sums_b = VADD32(v_s1_sums_b, v_s1_b);
				v_s1_sums_c = VADD32(v_s1_sums_c, v_s1_c);
				v_s1_sums_d = VADD32(v_s1_sums_d, v_s1_d);

				v_s1   = VDPBUSD(v_s1,   data_a, ones);
				v_s1_b = VDPBUSD(v_s1_b, data_b, ones);
				v_s1_c = VDPBUSD(v_s1_c, data_c, ones);
				v_s1_d = VDPBUSD(v_s1_d, data_d, ones);

				/* Same gcc bug workaround.  See above */
			#if GCC_PREREQ(1, 0) && !defined(ARCH_X86_32)
				__asm__("" : "+v" (v_s2), "+v" (v_s2_b),
					     "+v" (v_s2_c), "+v" (v_s2_d),
					     "+v" (v_s1_sums),
					     "+v" (v_s1_sums_b),
					     "+v" (v_s1_sums_c),
					     "+v" (v_s1_sums_d),
					     "+v" (v_s1), "+v" (v_s1_b),
					     "+v" (v_s1_c), "+v" (v_s1_d));
			#endif
				p += 4*VL;
				n -= 4*VL;
			} while (n >= 4*VL);

			/*
			 * Reduce into v_s1 and v_s2 as follows:
			 *
			 * v_s2 = v_s2 + v_s2_b + v_s2_c + v_s2_d +
			 *	  (4*VL)*(v_s1_sums   + v_s1_sums_b +
			 *		  v_s1_sums_c + v_s1_sums_d) +
			 *	  (3*VL)*v_s1 + (2*VL)*v_s1_b + VL*v_s1_c
			 * v_s1 = v_s1 + v_s1_b + v_s1_c + v_s1_d
			 */
			tmp0 = VADD32(v_s1, v_s1_b);
			tmp1 = VADD32(v_s1, v_s1_c);
			v_s1_sums = VADD32_4X(v_s1_sums, v_s1_sums_b,
					      v_s1_sums_c, v_s1_sums_d);
			v_s1 = VADD32_3X(tmp0, v_s1_c, v_s1_d);
			v_s2 = VADD32_7X(VSLL32(v_s1_sums, LOG2_VL + 2),
					 VSLL32(tmp0, LOG2_VL + 1),
					 VSLL32(tmp1, LOG2_VL),
					 v_s2, v_s2_b, v_s2_c, v_s2_d);
		}

		/* Process the last 0 <= n < 4*VL bytes of the chunk. */
		if (n >= 2*VL) {
			const vec_t data_a = VLOADU(p + 0*VL);
			const vec_t data_b = VLOADU(p + 1*VL);

			v_s2 = VADD32(v_s2, VSLL32(v_s1, LOG2_VL + 1));
			v_s1 = VDPBUSD(v_s1, data_a, ones);
			v_s1 = VDPBUSD(v_s1, data_b, ones);
			v_s2 = VDPBUSD(v_s2, data_a, VSET1_8(VL));
			v_s2 = VDPBUSD(v_s2, data_a, mults);
			v_s2 = VDPBUSD(v_s2, data_b, mults);
			p += 2*VL;
			n -= 2*VL;
		}
		if (n) {
			/* Process the last 0 < n < 2*VL bytes of the chunk. */
			vec_t data;

			v_s2 = VADD32(v_s2, VMULLO32(v_s1, VSET1_32(n)));

			mults = VADD8(mults, VSET1_8((int)n - VL));
			if (n > VL) {
				data = VLOADU(p);
				v_s1 = VDPBUSD(v_s1, data, ones);
				v_s2 = VDPBUSD(v_s2, data, mults);
				p += VL;
				n -= VL;
				mults = VADD8(mults, VSET1_8(-VL));
			}
			/*
			 * Process the last 0 < n <= VL bytes of the chunk.
			 * Utilize a masked load if it's available.
			 */
		#if USE_AVX512
			data = VMASKZ_LOADU((mask_t)-1 >> (VL - n), p);
		#else
			data = zeroes;
			memcpy(&data, p, n);
		#endif
			v_s1 = VDPBUSD(v_s1, data, ones);
			v_s2 = VDPBUSD(v_s2, data, mults);
			p += n;
		}

		reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}
#else /* USE_VNNI */
	/*
	 * This is Adler-32 for SSE2 and AVX2.
	 *
	 * To horizontally sum bytes, use psadbw + paddd, where one of the
	 * arguments to psadbw is all-zeroes.
	 *
	 * For the s2 contribution from (2*VL - i)*data[i] for each of the 2*VL
	 * bytes of each iteration of the inner loop, use punpck{l,h}bw + paddw
	 * to sum, for each i across iterations, byte i into a corresponding
	 * 16-bit counter in v_byte_sums_*.  After the inner loop, use pmaddwd
	 * to multiply each counter by (2*VL - i), then add the products to s2.
	 *
	 * An alternative implementation would use pmaddubsw and pmaddwd in the
	 * inner loop to do (2*VL - i)*data[i] directly and add the products in
	 * groups of 4 to 32-bit counters.  However, on average that approach
	 * seems to be slower than the current approach which delays the
	 * multiplications.  Also, pmaddubsw requires SSSE3; the current
	 * approach keeps the implementation aligned between SSE2 and AVX2.
	 *
	 * The inner loop processes 2*VL bytes per iteration.  Increasing this
	 * to 4*VL doesn't seem to be helpful here.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX, and every
		 * v_byte_sums_* counter is guaranteed to not exceed INT16_MAX.
		 * It's INT16_MAX, not UINT16_MAX, because v_byte_sums_* are
		 * used with pmaddwd which does signed multiplication.  In the
		 * SSE2 case this limits chunks to 4096 bytes instead of 5536.
		 */
		size_t n = MIN(len, MIN(2 * VL * (INT16_MAX / UINT8_MAX),
					MAX_CHUNK_LEN) & ~(2*VL - 1));
		len -= n;

		if (n >= 2*VL) {
			vec_t v_s1 = zeroes;
			vec_t v_s1_sums = zeroes;
			vec_t v_byte_sums_a = zeroes;
			vec_t v_byte_sums_b = zeroes;
			vec_t v_byte_sums_c = zeroes;
			vec_t v_byte_sums_d = zeroes;
			vec_t v_s2;

			s2 += s1 * (n & ~(2*VL - 1));

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);

				v_s1_sums = VADD32(v_s1_sums, v_s1);
				v_byte_sums_a = VADD16(v_byte_sums_a,
						       VUNPACKLO8(data_a, zeroes));
				v_byte_sums_b = VADD16(v_byte_sums_b,
						       VUNPACKHI8(data_a, zeroes));
				v_byte_sums_c = VADD16(v_byte_sums_c,
						       VUNPACKLO8(data_b, zeroes));
				v_byte_sums_d = VADD16(v_byte_sums_d,
						       VUNPACKHI8(data_b, zeroes));
				v_s1 = VADD32(v_s1,
					      VADD32(VSAD8(data_a, zeroes),
						     VSAD8(data_b, zeroes)));
				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+x" (v_s1), "+x" (v_s1_sums),
					     "+x" (v_byte_sums_a),
					     "+x" (v_byte_sums_b),
					     "+x" (v_byte_sums_c),
					     "+x" (v_byte_sums_d));
			#endif
				p += 2*VL;
				n -= 2*VL;
			} while (n >= 2*VL);

			/*
			 * Calculate v_s2 as (2*VL)*v_s1_sums +
			 * [2*VL, 2*VL - 1, 2*VL - 2, ..., 1] * v_byte_sums.
			 * Then update s1 and s2 from v_s1 and v_s2.
			 */
			v_s2 = VADD32_5X(VSLL32(v_s1_sums, LOG2_VL + 1),
					 VMADD16(v_byte_sums_a, mults_a),
					 VMADD16(v_byte_sums_b, mults_b),
					 VMADD16(v_byte_sums_c, mults_c),
					 VMADD16(v_byte_sums_d, mults_d));
			reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		}
		/*
		 * Process the last 0 <= n < 2*VL bytes of the chunk using
		 * scalar instructions and reduce s1 and s2 mod DIVISOR.
		 */
		ADLER32_CHUNK(s1, s2, p, n);
	}
#endif /* !USE_VNNI */
	return (s2 << 16) | s1;
}

#undef vec_t
#undef mask_t
#undef LOG2_VL
#undef VADD8
#undef VADD16
#undef VADD32
#undef VDPBUSD
#undef VLOAD
#undef VLOADU
#undef VMADD16
#undef VMASKZ_LOADU
#undef VMULLO32
#undef VSAD8
#undef VSET1_8
#undef VSET1_32
#undef VSETZERO
#undef VSLL32
#undef VUNPACKLO8
#undef VUNPACKHI8

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_VNNI
#undef USE_AVX512

/*** End of inlined file: adler32_template.h ***/


#endif

/*
 * AVX-VNNI implementation.  This is used on CPUs that have AVX2 and AVX-VNNI
 * but don't have AVX-512, for example Intel Alder Lake.
 *
 * Unusually for a new CPU feature, gcc added support for the AVX-VNNI
 * intrinsics (in gcc 11.1) slightly before binutils added support for
 * assembling AVX-VNNI instructions (in binutils 2.36).  Distros can reasonably
 * have gcc 11 with binutils 2.35.  Because of this issue, we check for gcc 12
 * instead of gcc 11.  (libdeflate supports direct compilation without a
 * configure step, so checking the binutils version is not always an option.)
 */
#if (GCC_PREREQ(12, 1) || CLANG_PREREQ(12, 0, 13000000) || MSVC_PREREQ(1930)) && \
	!defined(LIBDEFLATE_ASSEMBLER_DOES_NOT_SUPPORT_AVX_VNNI)
#  define adler32_x86_avx2_vnni	adler32_x86_avx2_vnni
#  define SUFFIX			   _avx2_vnni
#  define ATTRIBUTES		_target_attribute("avx2,avxvnni")
#  define VL			32
#  define USE_VNNI		1
#  define USE_AVX512		0

/*** Start of inlined file: adler32_template.h ***/
/*
 * This file is a "template" for instantiating Adler-32 functions for x86.
 * The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_VNNI=0 && USE_AVX512=0: at least sse2
 *	   VL=32 && USE_VNNI=0 && USE_AVX512=0: at least avx2
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=0: at least avx2,avxvnni
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vl,avx512vnni
 *	   VL=64 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vnni
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_VNNI:
 *	If 1, use the VNNI dot product based algorithm.
 *	If 0, use the legacy SSE2 and AVX2 compatible algorithm.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking.  This doesn't
 *	enable the use of 512-bit vectors; the vector length is controlled by
 *	VL.  If 0, assume that the CPU might not support AVX-512.
 */

#if VL == 16
#  define vec_t			__m128i
#  define mask_t		u16
#  define LOG2_VL		4
#  define VADD8(a, b)		_mm_add_epi8((a), (b))
#  define VADD16(a, b)		_mm_add_epi16((a), (b))
#  define VADD32(a, b)		_mm_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm_load_si128((const void *)(p))
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VMADD16(a, b)		_mm_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm_set1_epi8(a)
#  define VSET1_32(a)		_mm_set1_epi32(a)
#  define VSETZERO()		_mm_setzero_si128()
#  define VSLL32(a, b)		_mm_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm_unpackhi_epi8((a), (b))
#elif VL == 32
#  define vec_t			__m256i
#  define mask_t		u32
#  define LOG2_VL		5
#  define VADD8(a, b)		_mm256_add_epi8((a), (b))
#  define VADD16(a, b)		_mm256_add_epi16((a), (b))
#  define VADD32(a, b)		_mm256_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm256_load_si256((const void *)(p))
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VMADD16(a, b)		_mm256_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm256_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm256_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm256_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm256_set1_epi8(a)
#  define VSET1_32(a)		_mm256_set1_epi32(a)
#  define VSETZERO()		_mm256_setzero_si256()
#  define VSLL32(a, b)		_mm256_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm256_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm256_unpackhi_epi8((a), (b))
#elif VL == 64
#  define vec_t			__m512i
#  define mask_t		u64
#  define LOG2_VL		6
#  define VADD8(a, b)		_mm512_add_epi8((a), (b))
#  define VADD16(a, b)		_mm512_add_epi16((a), (b))
#  define VADD32(a, b)		_mm512_add_epi32((a), (b))
#  define VDPBUSD(a, b, c)	_mm512_dpbusd_epi32((a), (b), (c))
#  define VLOAD(p)		_mm512_load_si512((const void *)(p))
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VMADD16(a, b)		_mm512_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm512_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm512_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm512_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm512_set1_epi8(a)
#  define VSET1_32(a)		_mm512_set1_epi32(a)
#  define VSETZERO()		_mm512_setzero_si512()
#  define VSLL32(a, b)		_mm512_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm512_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm512_unpackhi_epi8((a), (b))
#else
#  error "unsupported vector length"
#endif

#define VADD32_3X(a, b, c)	VADD32(VADD32((a), (b)), (c))
#define VADD32_4X(a, b, c, d)	VADD32(VADD32((a), (b)), VADD32((c), (d)))
#define VADD32_5X(a, b, c, d, e) VADD32((a), VADD32_4X((b), (c), (d), (e)))
#define VADD32_7X(a, b, c, d, e, f, g)	\
	VADD32(VADD32_3X((a), (b), (c)), VADD32_4X((d), (e), (f), (g)))

/* Sum the 32-bit elements of v_s1 and add them to s1, and likewise for s2. */
#undef reduce_to_32bits
static forceinline ATTRIBUTES void
ADD_SUFFIX(reduce_to_32bits)(vec_t v_s1, vec_t v_s2, u32 *s1_p, u32 *s2_p)
{
	__m128i v_s1_128, v_s2_128;
#if VL == 16
	{
		v_s1_128 = v_s1;
		v_s2_128 = v_s2;
	}
#else
	{
		__m256i v_s1_256, v_s2_256;
	#if VL == 32
		v_s1_256 = v_s1;
		v_s2_256 = v_s2;
	#else
		/* Reduce 512 bits to 256 bits. */
		v_s1_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s1, 0),
					    _mm512_extracti64x4_epi64(v_s1, 1));
		v_s2_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s2, 0),
					    _mm512_extracti64x4_epi64(v_s2, 1));
	#endif
		/* Reduce 256 bits to 128 bits. */
		v_s1_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s1_256, 0),
					 _mm256_extracti128_si256(v_s1_256, 1));
		v_s2_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s2_256, 0),
					 _mm256_extracti128_si256(v_s2_256, 1));
	}
#endif

	/*
	 * Reduce 128 bits to 32 bits.
	 *
	 * If the bytes were summed into v_s1 using psadbw + paddd, then ignore
	 * the odd-indexed elements of v_s1_128 since they are zero.
	 */
#if USE_VNNI
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x31));
#endif
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x31));
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x02));
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x02));

	*s1_p += (u32)_mm_cvtsi128_si32(v_s1_128);
	*s2_p += (u32)_mm_cvtsi128_si32(v_s2_128);
}
#define reduce_to_32bits	ADD_SUFFIX(reduce_to_32bits)

static ATTRIBUTES u32
ADD_SUFFIX(adler32_x86)(u32 adler, const u8 *p, size_t len)
{
#if USE_VNNI
	/* This contains the bytes [VL, VL-1, VL-2, ..., 1]. */
	static const u8 _aligned_attribute(VL) raw_mults[VL] = {
	#if VL == 64
		64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
		48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
	#endif
	#if VL >= 32
		32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
	#endif
		16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
	};
	const vec_t ones = VSET1_8(1);
#else
	/*
	 * This contains the 16-bit values [2*VL, 2*VL - 1, 2*VL - 2, ..., 1].
	 * For VL==32 the ordering is weird because it has to match the way that
	 * vpunpcklbw and vpunpckhbw work on 128-bit lanes separately.
	 */
	static const u16 _aligned_attribute(VL) raw_mults[4][VL / 2] = {
	#if VL == 16
		{ 32, 31, 30, 29, 28, 27, 26, 25 },
		{ 24, 23, 22, 21, 20, 19, 18, 17 },
		{ 16, 15, 14, 13, 12, 11, 10, 9  },
		{ 8,  7,  6,  5,  4,  3,  2,  1  },
	#elif VL == 32
		{ 64, 63, 62, 61, 60, 59, 58, 57, 48, 47, 46, 45, 44, 43, 42, 41 },
		{ 56, 55, 54, 53, 52, 51, 50, 49, 40, 39, 38, 37, 36, 35, 34, 33 },
		{ 32, 31, 30, 29, 28, 27, 26, 25, 16, 15, 14, 13, 12, 11, 10,  9 },
		{ 24, 23, 22, 21, 20, 19, 18, 17,  8,  7,  6,  5,  4,  3,  2,  1 },
	#else
	#  error "unsupported parameters"
	#endif
	};
	const vec_t mults_a = VLOAD(raw_mults[0]);
	const vec_t mults_b = VLOAD(raw_mults[1]);
	const vec_t mults_c = VLOAD(raw_mults[2]);
	const vec_t mults_d = VLOAD(raw_mults[3]);
#endif
	const vec_t zeroes = VSETZERO();
	u32 s1 = adler & 0xFFFF;
	u32 s2 = adler >> 16;

	/*
	 * If the length is large and the pointer is misaligned, align it.
	 * For smaller lengths, just take the misaligned load penalty.
	 */
	if (unlikely(len > 65536 && ((uintptr_t)p & (VL-1)))) {
		do {
			s1 += *p++;
			s2 += s1;
			len--;
		} while ((uintptr_t)p & (VL-1));
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}

#if USE_VNNI
	/*
	 * This is Adler-32 using the vpdpbusd instruction from AVX512VNNI or
	 * AVX-VNNI.  vpdpbusd multiplies the unsigned bytes of one vector by
	 * the signed bytes of another vector and adds the sums in groups of 4
	 * to the 32-bit elements of a third vector.  We use it in two ways:
	 * multiplying the data bytes by a sequence like 64,63,62,...,1 for
	 * calculating part of s2, and multiplying the data bytes by an all-ones
	 * sequence 1,1,1,...,1 for calculating s1 and part of s2.  The all-ones
	 * trick seems to be faster than the alternative of vpsadbw + vpaddd.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX.
		 */
		size_t n = MIN(len, MAX_CHUNK_LEN & ~(4*VL - 1));
		vec_t mults = VLOAD(raw_mults);
		vec_t v_s1 = zeroes;
		vec_t v_s2 = zeroes;

		s2 += s1 * n;
		len -= n;

		if (n >= 4*VL) {
			vec_t v_s1_b = zeroes;
			vec_t v_s1_c = zeroes;
			vec_t v_s1_d = zeroes;
			vec_t v_s2_b = zeroes;
			vec_t v_s2_c = zeroes;
			vec_t v_s2_d = zeroes;
			vec_t v_s1_sums   = zeroes;
			vec_t v_s1_sums_b = zeroes;
			vec_t v_s1_sums_c = zeroes;
			vec_t v_s1_sums_d = zeroes;
			vec_t tmp0, tmp1;

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);
				vec_t data_c = VLOADU(p + 2*VL);
				vec_t data_d = VLOADU(p + 3*VL);

				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+v" (data_a), "+v" (data_b),
					     "+v" (data_c), "+v" (data_d));
			#endif

				v_s2   = VDPBUSD(v_s2,   data_a, mults);
				v_s2_b = VDPBUSD(v_s2_b, data_b, mults);
				v_s2_c = VDPBUSD(v_s2_c, data_c, mults);
				v_s2_d = VDPBUSD(v_s2_d, data_d, mults);

				v_s1_sums   = VADD32(v_s1_sums,   v_s1);
				v_s1_sums_b = VADD32(v_s1_sums_b, v_s1_b);
				v_s1_sums_c = VADD32(v_s1_sums_c, v_s1_c);
				v_s1_sums_d = VADD32(v_s1_sums_d, v_s1_d);

				v_s1   = VDPBUSD(v_s1,   data_a, ones);
				v_s1_b = VDPBUSD(v_s1_b, data_b, ones);
				v_s1_c = VDPBUSD(v_s1_c, data_c, ones);
				v_s1_d = VDPBUSD(v_s1_d, data_d, ones);

				/* Same gcc bug workaround.  See above */
			#if GCC_PREREQ(1, 0) && !defined(ARCH_X86_32)
				__asm__("" : "+v" (v_s2), "+v" (v_s2_b),
					     "+v" (v_s2_c), "+v" (v_s2_d),
					     "+v" (v_s1_sums),
					     "+v" (v_s1_sums_b),
					     "+v" (v_s1_sums_c),
					     "+v" (v_s1_sums_d),
					     "+v" (v_s1), "+v" (v_s1_b),
					     "+v" (v_s1_c), "+v" (v_s1_d));
			#endif
				p += 4*VL;
				n -= 4*VL;
			} while (n >= 4*VL);

			/*
			 * Reduce into v_s1 and v_s2 as follows:
			 *
			 * v_s2 = v_s2 + v_s2_b + v_s2_c + v_s2_d +
			 *	  (4*VL)*(v_s1_sums   + v_s1_sums_b +
			 *		  v_s1_sums_c + v_s1_sums_d) +
			 *	  (3*VL)*v_s1 + (2*VL)*v_s1_b + VL*v_s1_c
			 * v_s1 = v_s1 + v_s1_b + v_s1_c + v_s1_d
			 */
			tmp0 = VADD32(v_s1, v_s1_b);
			tmp1 = VADD32(v_s1, v_s1_c);
			v_s1_sums = VADD32_4X(v_s1_sums, v_s1_sums_b,
					      v_s1_sums_c, v_s1_sums_d);
			v_s1 = VADD32_3X(tmp0, v_s1_c, v_s1_d);
			v_s2 = VADD32_7X(VSLL32(v_s1_sums, LOG2_VL + 2),
					 VSLL32(tmp0, LOG2_VL + 1),
					 VSLL32(tmp1, LOG2_VL),
					 v_s2, v_s2_b, v_s2_c, v_s2_d);
		}

		/* Process the last 0 <= n < 4*VL bytes of the chunk. */
		if (n >= 2*VL) {
			const vec_t data_a = VLOADU(p + 0*VL);
			const vec_t data_b = VLOADU(p + 1*VL);

			v_s2 = VADD32(v_s2, VSLL32(v_s1, LOG2_VL + 1));
			v_s1 = VDPBUSD(v_s1, data_a, ones);
			v_s1 = VDPBUSD(v_s1, data_b, ones);
			v_s2 = VDPBUSD(v_s2, data_a, VSET1_8(VL));
			v_s2 = VDPBUSD(v_s2, data_a, mults);
			v_s2 = VDPBUSD(v_s2, data_b, mults);
			p += 2*VL;
			n -= 2*VL;
		}
		if (n) {
			/* Process the last 0 < n < 2*VL bytes of the chunk. */
			vec_t data;

			v_s2 = VADD32(v_s2, VMULLO32(v_s1, VSET1_32(n)));

			mults = VADD8(mults, VSET1_8((int)n - VL));
			if (n > VL) {
				data = VLOADU(p);
				v_s1 = VDPBUSD(v_s1, data, ones);
				v_s2 = VDPBUSD(v_s2, data, mults);
				p += VL;
				n -= VL;
				mults = VADD8(mults, VSET1_8(-VL));
			}
			/*
			 * Process the last 0 < n <= VL bytes of the chunk.
			 * Utilize a masked load if it's available.
			 */
		#if USE_AVX512
			data = VMASKZ_LOADU((mask_t)-1 >> (VL - n), p);
		#else
			data = zeroes;
			memcpy(&data, p, n);
		#endif
			v_s1 = VDPBUSD(v_s1, data, ones);
			v_s2 = VDPBUSD(v_s2, data, mults);
			p += n;
		}

		reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}
#else /* USE_VNNI */
	/*
	 * This is Adler-32 for SSE2 and AVX2.
	 *
	 * To horizontally sum bytes, use psadbw + paddd, where one of the
	 * arguments to psadbw is all-zeroes.
	 *
	 * For the s2 contribution from (2*VL - i)*data[i] for each of the 2*VL
	 * bytes of each iteration of the inner loop, use punpck{l,h}bw + paddw
	 * to sum, for each i across iterations, byte i into a corresponding
	 * 16-bit counter in v_byte_sums_*.  After the inner loop, use pmaddwd
	 * to multiply each counter by (2*VL - i), then add the products to s2.
	 *
	 * An alternative implementation would use pmaddubsw and pmaddwd in the
	 * inner loop to do (2*VL - i)*data[i] directly and add the products in
	 * groups of 4 to 32-bit counters.  However, on average that approach
	 * seems to be slower than the current approach which delays the
	 * multiplications.  Also, pmaddubsw requires SSSE3; the current
	 * approach keeps the implementation aligned between SSE2 and AVX2.
	 *
	 * The inner loop processes 2*VL bytes per iteration.  Increasing this
	 * to 4*VL doesn't seem to be helpful here.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX, and every
		 * v_byte_sums_* counter is guaranteed to not exceed INT16_MAX.
		 * It's INT16_MAX, not UINT16_MAX, because v_byte_sums_* are
		 * used with pmaddwd which does signed multiplication.  In the
		 * SSE2 case this limits chunks to 4096 bytes instead of 5536.
		 */
		size_t n = MIN(len, MIN(2 * VL * (INT16_MAX / UINT8_MAX),
					MAX_CHUNK_LEN) & ~(2*VL - 1));
		len -= n;

		if (n >= 2*VL) {
			vec_t v_s1 = zeroes;
			vec_t v_s1_sums = zeroes;
			vec_t v_byte_sums_a = zeroes;
			vec_t v_byte_sums_b = zeroes;
			vec_t v_byte_sums_c = zeroes;
			vec_t v_byte_sums_d = zeroes;
			vec_t v_s2;

			s2 += s1 * (n & ~(2*VL - 1));

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);

				v_s1_sums = VADD32(v_s1_sums, v_s1);
				v_byte_sums_a = VADD16(v_byte_sums_a,
						       VUNPACKLO8(data_a, zeroes));
				v_byte_sums_b = VADD16(v_byte_sums_b,
						       VUNPACKHI8(data_a, zeroes));
				v_byte_sums_c = VADD16(v_byte_sums_c,
						       VUNPACKLO8(data_b, zeroes));
				v_byte_sums_d = VADD16(v_byte_sums_d,
						       VUNPACKHI8(data_b, zeroes));
				v_s1 = VADD32(v_s1,
					      VADD32(VSAD8(data_a, zeroes),
						     VSAD8(data_b, zeroes)));
				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+x" (v_s1), "+x" (v_s1_sums),
					     "+x" (v_byte_sums_a),
					     "+x" (v_byte_sums_b),
					     "+x" (v_byte_sums_c),
					     "+x" (v_byte_sums_d));
			#endif
				p += 2*VL;
				n -= 2*VL;
			} while (n >= 2*VL);

			/*
			 * Calculate v_s2 as (2*VL)*v_s1_sums +
			 * [2*VL, 2*VL - 1, 2*VL - 2, ..., 1] * v_byte_sums.
			 * Then update s1 and s2 from v_s1 and v_s2.
			 */
			v_s2 = VADD32_5X(VSLL32(v_s1_sums, LOG2_VL + 1),
					 VMADD16(v_byte_sums_a, mults_a),
					 VMADD16(v_byte_sums_b, mults_b),
					 VMADD16(v_byte_sums_c, mults_c),
					 VMADD16(v_byte_sums_d, mults_d));
			reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		}
		/*
		 * Process the last 0 <= n < 2*VL bytes of the chunk using
		 * scalar instructions and reduce s1 and s2 mod DIVISOR.
		 */
		ADLER32_CHUNK(s1, s2, p, n);
	}
#endif /* !USE_VNNI */
	return (s2 << 16) | s1;
}

#undef vec_t
#undef mask_t
#undef LOG2_VL
#undef VADD8
#undef VADD16
#undef VADD32
#undef VDPBUSD
#undef VLOAD
#undef VLOADU
#undef VMADD16
#undef VMASKZ_LOADU
#undef VMULLO32
#undef VSAD8
#undef VSET1_8
#undef VSET1_32
#undef VSETZERO
#undef VSLL32
#undef VUNPACKLO8
#undef VUNPACKHI8

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_VNNI
#undef USE_AVX512

/*** End of inlined file: adler32_template.h ***/


#endif

#if (GCC_PREREQ(8, 1) || CLANG_PREREQ(6, 0, 10000000) || MSVC_PREREQ(1920)) && \
	!defined(LIBDEFLATE_ASSEMBLER_DOES_NOT_SUPPORT_AVX512VNNI)
/*
 * AVX512VNNI implementation using 256-bit vectors.  This is very similar to the
 * AVX-VNNI implementation but takes advantage of masking and more registers.
 * This is used on certain older Intel CPUs, specifically Ice Lake and Tiger
 * Lake, which support AVX512VNNI but downclock a bit too eagerly when ZMM
 * registers are used.
 */
#  define adler32_x86_avx512_vl256_vnni	adler32_x86_avx512_vl256_vnni
#  define SUFFIX				   _avx512_vl256_vnni
#  define ATTRIBUTES		_target_attribute("avx512bw,avx512vl,avx512vnni")
#  define VL			32
#  define USE_VNNI		1
#  define USE_AVX512		1

/*** Start of inlined file: adler32_template.h ***/
/*
 * This file is a "template" for instantiating Adler-32 functions for x86.
 * The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_VNNI=0 && USE_AVX512=0: at least sse2
 *	   VL=32 && USE_VNNI=0 && USE_AVX512=0: at least avx2
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=0: at least avx2,avxvnni
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vl,avx512vnni
 *	   VL=64 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vnni
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_VNNI:
 *	If 1, use the VNNI dot product based algorithm.
 *	If 0, use the legacy SSE2 and AVX2 compatible algorithm.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking.  This doesn't
 *	enable the use of 512-bit vectors; the vector length is controlled by
 *	VL.  If 0, assume that the CPU might not support AVX-512.
 */

#if VL == 16
#  define vec_t			__m128i
#  define mask_t		u16
#  define LOG2_VL		4
#  define VADD8(a, b)		_mm_add_epi8((a), (b))
#  define VADD16(a, b)		_mm_add_epi16((a), (b))
#  define VADD32(a, b)		_mm_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm_load_si128((const void *)(p))
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VMADD16(a, b)		_mm_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm_set1_epi8(a)
#  define VSET1_32(a)		_mm_set1_epi32(a)
#  define VSETZERO()		_mm_setzero_si128()
#  define VSLL32(a, b)		_mm_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm_unpackhi_epi8((a), (b))
#elif VL == 32
#  define vec_t			__m256i
#  define mask_t		u32
#  define LOG2_VL		5
#  define VADD8(a, b)		_mm256_add_epi8((a), (b))
#  define VADD16(a, b)		_mm256_add_epi16((a), (b))
#  define VADD32(a, b)		_mm256_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm256_load_si256((const void *)(p))
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VMADD16(a, b)		_mm256_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm256_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm256_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm256_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm256_set1_epi8(a)
#  define VSET1_32(a)		_mm256_set1_epi32(a)
#  define VSETZERO()		_mm256_setzero_si256()
#  define VSLL32(a, b)		_mm256_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm256_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm256_unpackhi_epi8((a), (b))
#elif VL == 64
#  define vec_t			__m512i
#  define mask_t		u64
#  define LOG2_VL		6
#  define VADD8(a, b)		_mm512_add_epi8((a), (b))
#  define VADD16(a, b)		_mm512_add_epi16((a), (b))
#  define VADD32(a, b)		_mm512_add_epi32((a), (b))
#  define VDPBUSD(a, b, c)	_mm512_dpbusd_epi32((a), (b), (c))
#  define VLOAD(p)		_mm512_load_si512((const void *)(p))
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VMADD16(a, b)		_mm512_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm512_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm512_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm512_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm512_set1_epi8(a)
#  define VSET1_32(a)		_mm512_set1_epi32(a)
#  define VSETZERO()		_mm512_setzero_si512()
#  define VSLL32(a, b)		_mm512_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm512_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm512_unpackhi_epi8((a), (b))
#else
#  error "unsupported vector length"
#endif

#define VADD32_3X(a, b, c)	VADD32(VADD32((a), (b)), (c))
#define VADD32_4X(a, b, c, d)	VADD32(VADD32((a), (b)), VADD32((c), (d)))
#define VADD32_5X(a, b, c, d, e) VADD32((a), VADD32_4X((b), (c), (d), (e)))
#define VADD32_7X(a, b, c, d, e, f, g)	\
	VADD32(VADD32_3X((a), (b), (c)), VADD32_4X((d), (e), (f), (g)))

/* Sum the 32-bit elements of v_s1 and add them to s1, and likewise for s2. */
#undef reduce_to_32bits
static forceinline ATTRIBUTES void
ADD_SUFFIX(reduce_to_32bits)(vec_t v_s1, vec_t v_s2, u32 *s1_p, u32 *s2_p)
{
	__m128i v_s1_128, v_s2_128;
#if VL == 16
	{
		v_s1_128 = v_s1;
		v_s2_128 = v_s2;
	}
#else
	{
		__m256i v_s1_256, v_s2_256;
	#if VL == 32
		v_s1_256 = v_s1;
		v_s2_256 = v_s2;
	#else
		/* Reduce 512 bits to 256 bits. */
		v_s1_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s1, 0),
					    _mm512_extracti64x4_epi64(v_s1, 1));
		v_s2_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s2, 0),
					    _mm512_extracti64x4_epi64(v_s2, 1));
	#endif
		/* Reduce 256 bits to 128 bits. */
		v_s1_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s1_256, 0),
					 _mm256_extracti128_si256(v_s1_256, 1));
		v_s2_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s2_256, 0),
					 _mm256_extracti128_si256(v_s2_256, 1));
	}
#endif

	/*
	 * Reduce 128 bits to 32 bits.
	 *
	 * If the bytes were summed into v_s1 using psadbw + paddd, then ignore
	 * the odd-indexed elements of v_s1_128 since they are zero.
	 */
#if USE_VNNI
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x31));
#endif
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x31));
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x02));
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x02));

	*s1_p += (u32)_mm_cvtsi128_si32(v_s1_128);
	*s2_p += (u32)_mm_cvtsi128_si32(v_s2_128);
}
#define reduce_to_32bits	ADD_SUFFIX(reduce_to_32bits)

static ATTRIBUTES u32
ADD_SUFFIX(adler32_x86)(u32 adler, const u8 *p, size_t len)
{
#if USE_VNNI
	/* This contains the bytes [VL, VL-1, VL-2, ..., 1]. */
	static const u8 _aligned_attribute(VL) raw_mults[VL] = {
	#if VL == 64
		64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
		48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
	#endif
	#if VL >= 32
		32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
	#endif
		16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
	};
	const vec_t ones = VSET1_8(1);
#else
	/*
	 * This contains the 16-bit values [2*VL, 2*VL - 1, 2*VL - 2, ..., 1].
	 * For VL==32 the ordering is weird because it has to match the way that
	 * vpunpcklbw and vpunpckhbw work on 128-bit lanes separately.
	 */
	static const u16 _aligned_attribute(VL) raw_mults[4][VL / 2] = {
	#if VL == 16
		{ 32, 31, 30, 29, 28, 27, 26, 25 },
		{ 24, 23, 22, 21, 20, 19, 18, 17 },
		{ 16, 15, 14, 13, 12, 11, 10, 9  },
		{ 8,  7,  6,  5,  4,  3,  2,  1  },
	#elif VL == 32
		{ 64, 63, 62, 61, 60, 59, 58, 57, 48, 47, 46, 45, 44, 43, 42, 41 },
		{ 56, 55, 54, 53, 52, 51, 50, 49, 40, 39, 38, 37, 36, 35, 34, 33 },
		{ 32, 31, 30, 29, 28, 27, 26, 25, 16, 15, 14, 13, 12, 11, 10,  9 },
		{ 24, 23, 22, 21, 20, 19, 18, 17,  8,  7,  6,  5,  4,  3,  2,  1 },
	#else
	#  error "unsupported parameters"
	#endif
	};
	const vec_t mults_a = VLOAD(raw_mults[0]);
	const vec_t mults_b = VLOAD(raw_mults[1]);
	const vec_t mults_c = VLOAD(raw_mults[2]);
	const vec_t mults_d = VLOAD(raw_mults[3]);
#endif
	const vec_t zeroes = VSETZERO();
	u32 s1 = adler & 0xFFFF;
	u32 s2 = adler >> 16;

	/*
	 * If the length is large and the pointer is misaligned, align it.
	 * For smaller lengths, just take the misaligned load penalty.
	 */
	if (unlikely(len > 65536 && ((uintptr_t)p & (VL-1)))) {
		do {
			s1 += *p++;
			s2 += s1;
			len--;
		} while ((uintptr_t)p & (VL-1));
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}

#if USE_VNNI
	/*
	 * This is Adler-32 using the vpdpbusd instruction from AVX512VNNI or
	 * AVX-VNNI.  vpdpbusd multiplies the unsigned bytes of one vector by
	 * the signed bytes of another vector and adds the sums in groups of 4
	 * to the 32-bit elements of a third vector.  We use it in two ways:
	 * multiplying the data bytes by a sequence like 64,63,62,...,1 for
	 * calculating part of s2, and multiplying the data bytes by an all-ones
	 * sequence 1,1,1,...,1 for calculating s1 and part of s2.  The all-ones
	 * trick seems to be faster than the alternative of vpsadbw + vpaddd.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX.
		 */
		size_t n = MIN(len, MAX_CHUNK_LEN & ~(4*VL - 1));
		vec_t mults = VLOAD(raw_mults);
		vec_t v_s1 = zeroes;
		vec_t v_s2 = zeroes;

		s2 += s1 * n;
		len -= n;

		if (n >= 4*VL) {
			vec_t v_s1_b = zeroes;
			vec_t v_s1_c = zeroes;
			vec_t v_s1_d = zeroes;
			vec_t v_s2_b = zeroes;
			vec_t v_s2_c = zeroes;
			vec_t v_s2_d = zeroes;
			vec_t v_s1_sums   = zeroes;
			vec_t v_s1_sums_b = zeroes;
			vec_t v_s1_sums_c = zeroes;
			vec_t v_s1_sums_d = zeroes;
			vec_t tmp0, tmp1;

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);
				vec_t data_c = VLOADU(p + 2*VL);
				vec_t data_d = VLOADU(p + 3*VL);

				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+v" (data_a), "+v" (data_b),
					     "+v" (data_c), "+v" (data_d));
			#endif

				v_s2   = VDPBUSD(v_s2,   data_a, mults);
				v_s2_b = VDPBUSD(v_s2_b, data_b, mults);
				v_s2_c = VDPBUSD(v_s2_c, data_c, mults);
				v_s2_d = VDPBUSD(v_s2_d, data_d, mults);

				v_s1_sums   = VADD32(v_s1_sums,   v_s1);
				v_s1_sums_b = VADD32(v_s1_sums_b, v_s1_b);
				v_s1_sums_c = VADD32(v_s1_sums_c, v_s1_c);
				v_s1_sums_d = VADD32(v_s1_sums_d, v_s1_d);

				v_s1   = VDPBUSD(v_s1,   data_a, ones);
				v_s1_b = VDPBUSD(v_s1_b, data_b, ones);
				v_s1_c = VDPBUSD(v_s1_c, data_c, ones);
				v_s1_d = VDPBUSD(v_s1_d, data_d, ones);

				/* Same gcc bug workaround.  See above */
			#if GCC_PREREQ(1, 0) && !defined(ARCH_X86_32)
				__asm__("" : "+v" (v_s2), "+v" (v_s2_b),
					     "+v" (v_s2_c), "+v" (v_s2_d),
					     "+v" (v_s1_sums),
					     "+v" (v_s1_sums_b),
					     "+v" (v_s1_sums_c),
					     "+v" (v_s1_sums_d),
					     "+v" (v_s1), "+v" (v_s1_b),
					     "+v" (v_s1_c), "+v" (v_s1_d));
			#endif
				p += 4*VL;
				n -= 4*VL;
			} while (n >= 4*VL);

			/*
			 * Reduce into v_s1 and v_s2 as follows:
			 *
			 * v_s2 = v_s2 + v_s2_b + v_s2_c + v_s2_d +
			 *	  (4*VL)*(v_s1_sums   + v_s1_sums_b +
			 *		  v_s1_sums_c + v_s1_sums_d) +
			 *	  (3*VL)*v_s1 + (2*VL)*v_s1_b + VL*v_s1_c
			 * v_s1 = v_s1 + v_s1_b + v_s1_c + v_s1_d
			 */
			tmp0 = VADD32(v_s1, v_s1_b);
			tmp1 = VADD32(v_s1, v_s1_c);
			v_s1_sums = VADD32_4X(v_s1_sums, v_s1_sums_b,
					      v_s1_sums_c, v_s1_sums_d);
			v_s1 = VADD32_3X(tmp0, v_s1_c, v_s1_d);
			v_s2 = VADD32_7X(VSLL32(v_s1_sums, LOG2_VL + 2),
					 VSLL32(tmp0, LOG2_VL + 1),
					 VSLL32(tmp1, LOG2_VL),
					 v_s2, v_s2_b, v_s2_c, v_s2_d);
		}

		/* Process the last 0 <= n < 4*VL bytes of the chunk. */
		if (n >= 2*VL) {
			const vec_t data_a = VLOADU(p + 0*VL);
			const vec_t data_b = VLOADU(p + 1*VL);

			v_s2 = VADD32(v_s2, VSLL32(v_s1, LOG2_VL + 1));
			v_s1 = VDPBUSD(v_s1, data_a, ones);
			v_s1 = VDPBUSD(v_s1, data_b, ones);
			v_s2 = VDPBUSD(v_s2, data_a, VSET1_8(VL));
			v_s2 = VDPBUSD(v_s2, data_a, mults);
			v_s2 = VDPBUSD(v_s2, data_b, mults);
			p += 2*VL;
			n -= 2*VL;
		}
		if (n) {
			/* Process the last 0 < n < 2*VL bytes of the chunk. */
			vec_t data;

			v_s2 = VADD32(v_s2, VMULLO32(v_s1, VSET1_32(n)));

			mults = VADD8(mults, VSET1_8((int)n - VL));
			if (n > VL) {
				data = VLOADU(p);
				v_s1 = VDPBUSD(v_s1, data, ones);
				v_s2 = VDPBUSD(v_s2, data, mults);
				p += VL;
				n -= VL;
				mults = VADD8(mults, VSET1_8(-VL));
			}
			/*
			 * Process the last 0 < n <= VL bytes of the chunk.
			 * Utilize a masked load if it's available.
			 */
		#if USE_AVX512
			data = VMASKZ_LOADU((mask_t)-1 >> (VL - n), p);
		#else
			data = zeroes;
			memcpy(&data, p, n);
		#endif
			v_s1 = VDPBUSD(v_s1, data, ones);
			v_s2 = VDPBUSD(v_s2, data, mults);
			p += n;
		}

		reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}
#else /* USE_VNNI */
	/*
	 * This is Adler-32 for SSE2 and AVX2.
	 *
	 * To horizontally sum bytes, use psadbw + paddd, where one of the
	 * arguments to psadbw is all-zeroes.
	 *
	 * For the s2 contribution from (2*VL - i)*data[i] for each of the 2*VL
	 * bytes of each iteration of the inner loop, use punpck{l,h}bw + paddw
	 * to sum, for each i across iterations, byte i into a corresponding
	 * 16-bit counter in v_byte_sums_*.  After the inner loop, use pmaddwd
	 * to multiply each counter by (2*VL - i), then add the products to s2.
	 *
	 * An alternative implementation would use pmaddubsw and pmaddwd in the
	 * inner loop to do (2*VL - i)*data[i] directly and add the products in
	 * groups of 4 to 32-bit counters.  However, on average that approach
	 * seems to be slower than the current approach which delays the
	 * multiplications.  Also, pmaddubsw requires SSSE3; the current
	 * approach keeps the implementation aligned between SSE2 and AVX2.
	 *
	 * The inner loop processes 2*VL bytes per iteration.  Increasing this
	 * to 4*VL doesn't seem to be helpful here.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX, and every
		 * v_byte_sums_* counter is guaranteed to not exceed INT16_MAX.
		 * It's INT16_MAX, not UINT16_MAX, because v_byte_sums_* are
		 * used with pmaddwd which does signed multiplication.  In the
		 * SSE2 case this limits chunks to 4096 bytes instead of 5536.
		 */
		size_t n = MIN(len, MIN(2 * VL * (INT16_MAX / UINT8_MAX),
					MAX_CHUNK_LEN) & ~(2*VL - 1));
		len -= n;

		if (n >= 2*VL) {
			vec_t v_s1 = zeroes;
			vec_t v_s1_sums = zeroes;
			vec_t v_byte_sums_a = zeroes;
			vec_t v_byte_sums_b = zeroes;
			vec_t v_byte_sums_c = zeroes;
			vec_t v_byte_sums_d = zeroes;
			vec_t v_s2;

			s2 += s1 * (n & ~(2*VL - 1));

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);

				v_s1_sums = VADD32(v_s1_sums, v_s1);
				v_byte_sums_a = VADD16(v_byte_sums_a,
						       VUNPACKLO8(data_a, zeroes));
				v_byte_sums_b = VADD16(v_byte_sums_b,
						       VUNPACKHI8(data_a, zeroes));
				v_byte_sums_c = VADD16(v_byte_sums_c,
						       VUNPACKLO8(data_b, zeroes));
				v_byte_sums_d = VADD16(v_byte_sums_d,
						       VUNPACKHI8(data_b, zeroes));
				v_s1 = VADD32(v_s1,
					      VADD32(VSAD8(data_a, zeroes),
						     VSAD8(data_b, zeroes)));
				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+x" (v_s1), "+x" (v_s1_sums),
					     "+x" (v_byte_sums_a),
					     "+x" (v_byte_sums_b),
					     "+x" (v_byte_sums_c),
					     "+x" (v_byte_sums_d));
			#endif
				p += 2*VL;
				n -= 2*VL;
			} while (n >= 2*VL);

			/*
			 * Calculate v_s2 as (2*VL)*v_s1_sums +
			 * [2*VL, 2*VL - 1, 2*VL - 2, ..., 1] * v_byte_sums.
			 * Then update s1 and s2 from v_s1 and v_s2.
			 */
			v_s2 = VADD32_5X(VSLL32(v_s1_sums, LOG2_VL + 1),
					 VMADD16(v_byte_sums_a, mults_a),
					 VMADD16(v_byte_sums_b, mults_b),
					 VMADD16(v_byte_sums_c, mults_c),
					 VMADD16(v_byte_sums_d, mults_d));
			reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		}
		/*
		 * Process the last 0 <= n < 2*VL bytes of the chunk using
		 * scalar instructions and reduce s1 and s2 mod DIVISOR.
		 */
		ADLER32_CHUNK(s1, s2, p, n);
	}
#endif /* !USE_VNNI */
	return (s2 << 16) | s1;
}

#undef vec_t
#undef mask_t
#undef LOG2_VL
#undef VADD8
#undef VADD16
#undef VADD32
#undef VDPBUSD
#undef VLOAD
#undef VLOADU
#undef VMADD16
#undef VMASKZ_LOADU
#undef VMULLO32
#undef VSAD8
#undef VSET1_8
#undef VSET1_32
#undef VSETZERO
#undef VSLL32
#undef VUNPACKLO8
#undef VUNPACKHI8

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_VNNI
#undef USE_AVX512

/*** End of inlined file: adler32_template.h ***/


/*
 * AVX512VNNI implementation using 512-bit vectors.  This is used on CPUs that
 * have a good AVX-512 implementation including AVX512VNNI.
 */
#  define adler32_x86_avx512_vl512_vnni	adler32_x86_avx512_vl512_vnni
#  define SUFFIX				   _avx512_vl512_vnni
#  define ATTRIBUTES		_target_attribute("avx512bw,avx512vnni")
#  define VL			64
#  define USE_VNNI		1
#  define USE_AVX512		1

/*** Start of inlined file: adler32_template.h ***/
/*
 * This file is a "template" for instantiating Adler-32 functions for x86.
 * The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_VNNI=0 && USE_AVX512=0: at least sse2
 *	   VL=32 && USE_VNNI=0 && USE_AVX512=0: at least avx2
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=0: at least avx2,avxvnni
 *	   VL=32 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vl,avx512vnni
 *	   VL=64 && USE_VNNI=1 && USE_AVX512=1: at least avx512bw,avx512vnni
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_VNNI:
 *	If 1, use the VNNI dot product based algorithm.
 *	If 0, use the legacy SSE2 and AVX2 compatible algorithm.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking.  This doesn't
 *	enable the use of 512-bit vectors; the vector length is controlled by
 *	VL.  If 0, assume that the CPU might not support AVX-512.
 */

#if VL == 16
#  define vec_t			__m128i
#  define mask_t		u16
#  define LOG2_VL		4
#  define VADD8(a, b)		_mm_add_epi8((a), (b))
#  define VADD16(a, b)		_mm_add_epi16((a), (b))
#  define VADD32(a, b)		_mm_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm_load_si128((const void *)(p))
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VMADD16(a, b)		_mm_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm_set1_epi8(a)
#  define VSET1_32(a)		_mm_set1_epi32(a)
#  define VSETZERO()		_mm_setzero_si128()
#  define VSLL32(a, b)		_mm_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm_unpackhi_epi8((a), (b))
#elif VL == 32
#  define vec_t			__m256i
#  define mask_t		u32
#  define LOG2_VL		5
#  define VADD8(a, b)		_mm256_add_epi8((a), (b))
#  define VADD16(a, b)		_mm256_add_epi16((a), (b))
#  define VADD32(a, b)		_mm256_add_epi32((a), (b))
#  if USE_AVX512
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_epi32((a), (b), (c))
#  else
#    define VDPBUSD(a, b, c)	_mm256_dpbusd_avx_epi32((a), (b), (c))
#  endif
#  define VLOAD(p)		_mm256_load_si256((const void *)(p))
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VMADD16(a, b)		_mm256_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm256_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm256_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm256_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm256_set1_epi8(a)
#  define VSET1_32(a)		_mm256_set1_epi32(a)
#  define VSETZERO()		_mm256_setzero_si256()
#  define VSLL32(a, b)		_mm256_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm256_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm256_unpackhi_epi8((a), (b))
#elif VL == 64
#  define vec_t			__m512i
#  define mask_t		u64
#  define LOG2_VL		6
#  define VADD8(a, b)		_mm512_add_epi8((a), (b))
#  define VADD16(a, b)		_mm512_add_epi16((a), (b))
#  define VADD32(a, b)		_mm512_add_epi32((a), (b))
#  define VDPBUSD(a, b, c)	_mm512_dpbusd_epi32((a), (b), (c))
#  define VLOAD(p)		_mm512_load_si512((const void *)(p))
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VMADD16(a, b)		_mm512_madd_epi16((a), (b))
#  define VMASKZ_LOADU(mask, p) _mm512_maskz_loadu_epi8((mask), (p))
#  define VMULLO32(a, b)	_mm512_mullo_epi32((a), (b))
#  define VSAD8(a, b)		_mm512_sad_epu8((a), (b))
#  define VSET1_8(a)		_mm512_set1_epi8(a)
#  define VSET1_32(a)		_mm512_set1_epi32(a)
#  define VSETZERO()		_mm512_setzero_si512()
#  define VSLL32(a, b)		_mm512_slli_epi32((a), (b))
#  define VUNPACKLO8(a, b)	_mm512_unpacklo_epi8((a), (b))
#  define VUNPACKHI8(a, b)	_mm512_unpackhi_epi8((a), (b))
#else
#  error "unsupported vector length"
#endif

#define VADD32_3X(a, b, c)	VADD32(VADD32((a), (b)), (c))
#define VADD32_4X(a, b, c, d)	VADD32(VADD32((a), (b)), VADD32((c), (d)))
#define VADD32_5X(a, b, c, d, e) VADD32((a), VADD32_4X((b), (c), (d), (e)))
#define VADD32_7X(a, b, c, d, e, f, g)	\
	VADD32(VADD32_3X((a), (b), (c)), VADD32_4X((d), (e), (f), (g)))

/* Sum the 32-bit elements of v_s1 and add them to s1, and likewise for s2. */
#undef reduce_to_32bits
static forceinline ATTRIBUTES void
ADD_SUFFIX(reduce_to_32bits)(vec_t v_s1, vec_t v_s2, u32 *s1_p, u32 *s2_p)
{
	__m128i v_s1_128, v_s2_128;
#if VL == 16
	{
		v_s1_128 = v_s1;
		v_s2_128 = v_s2;
	}
#else
	{
		__m256i v_s1_256, v_s2_256;
	#if VL == 32
		v_s1_256 = v_s1;
		v_s2_256 = v_s2;
	#else
		/* Reduce 512 bits to 256 bits. */
		v_s1_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s1, 0),
					    _mm512_extracti64x4_epi64(v_s1, 1));
		v_s2_256 = _mm256_add_epi32(_mm512_extracti64x4_epi64(v_s2, 0),
					    _mm512_extracti64x4_epi64(v_s2, 1));
	#endif
		/* Reduce 256 bits to 128 bits. */
		v_s1_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s1_256, 0),
					 _mm256_extracti128_si256(v_s1_256, 1));
		v_s2_128 = _mm_add_epi32(_mm256_extracti128_si256(v_s2_256, 0),
					 _mm256_extracti128_si256(v_s2_256, 1));
	}
#endif

	/*
	 * Reduce 128 bits to 32 bits.
	 *
	 * If the bytes were summed into v_s1 using psadbw + paddd, then ignore
	 * the odd-indexed elements of v_s1_128 since they are zero.
	 */
#if USE_VNNI
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x31));
#endif
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x31));
	v_s1_128 = _mm_add_epi32(v_s1_128, _mm_shuffle_epi32(v_s1_128, 0x02));
	v_s2_128 = _mm_add_epi32(v_s2_128, _mm_shuffle_epi32(v_s2_128, 0x02));

	*s1_p += (u32)_mm_cvtsi128_si32(v_s1_128);
	*s2_p += (u32)_mm_cvtsi128_si32(v_s2_128);
}
#define reduce_to_32bits	ADD_SUFFIX(reduce_to_32bits)

static ATTRIBUTES u32
ADD_SUFFIX(adler32_x86)(u32 adler, const u8 *p, size_t len)
{
#if USE_VNNI
	/* This contains the bytes [VL, VL-1, VL-2, ..., 1]. */
	static const u8 _aligned_attribute(VL) raw_mults[VL] = {
	#if VL == 64
		64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
		48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
	#endif
	#if VL >= 32
		32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
	#endif
		16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,
	};
	const vec_t ones = VSET1_8(1);
#else
	/*
	 * This contains the 16-bit values [2*VL, 2*VL - 1, 2*VL - 2, ..., 1].
	 * For VL==32 the ordering is weird because it has to match the way that
	 * vpunpcklbw and vpunpckhbw work on 128-bit lanes separately.
	 */
	static const u16 _aligned_attribute(VL) raw_mults[4][VL / 2] = {
	#if VL == 16
		{ 32, 31, 30, 29, 28, 27, 26, 25 },
		{ 24, 23, 22, 21, 20, 19, 18, 17 },
		{ 16, 15, 14, 13, 12, 11, 10, 9  },
		{ 8,  7,  6,  5,  4,  3,  2,  1  },
	#elif VL == 32
		{ 64, 63, 62, 61, 60, 59, 58, 57, 48, 47, 46, 45, 44, 43, 42, 41 },
		{ 56, 55, 54, 53, 52, 51, 50, 49, 40, 39, 38, 37, 36, 35, 34, 33 },
		{ 32, 31, 30, 29, 28, 27, 26, 25, 16, 15, 14, 13, 12, 11, 10,  9 },
		{ 24, 23, 22, 21, 20, 19, 18, 17,  8,  7,  6,  5,  4,  3,  2,  1 },
	#else
	#  error "unsupported parameters"
	#endif
	};
	const vec_t mults_a = VLOAD(raw_mults[0]);
	const vec_t mults_b = VLOAD(raw_mults[1]);
	const vec_t mults_c = VLOAD(raw_mults[2]);
	const vec_t mults_d = VLOAD(raw_mults[3]);
#endif
	const vec_t zeroes = VSETZERO();
	u32 s1 = adler & 0xFFFF;
	u32 s2 = adler >> 16;

	/*
	 * If the length is large and the pointer is misaligned, align it.
	 * For smaller lengths, just take the misaligned load penalty.
	 */
	if (unlikely(len > 65536 && ((uintptr_t)p & (VL-1)))) {
		do {
			s1 += *p++;
			s2 += s1;
			len--;
		} while ((uintptr_t)p & (VL-1));
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}

#if USE_VNNI
	/*
	 * This is Adler-32 using the vpdpbusd instruction from AVX512VNNI or
	 * AVX-VNNI.  vpdpbusd multiplies the unsigned bytes of one vector by
	 * the signed bytes of another vector and adds the sums in groups of 4
	 * to the 32-bit elements of a third vector.  We use it in two ways:
	 * multiplying the data bytes by a sequence like 64,63,62,...,1 for
	 * calculating part of s2, and multiplying the data bytes by an all-ones
	 * sequence 1,1,1,...,1 for calculating s1 and part of s2.  The all-ones
	 * trick seems to be faster than the alternative of vpsadbw + vpaddd.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX.
		 */
		size_t n = MIN(len, MAX_CHUNK_LEN & ~(4*VL - 1));
		vec_t mults = VLOAD(raw_mults);
		vec_t v_s1 = zeroes;
		vec_t v_s2 = zeroes;

		s2 += s1 * n;
		len -= n;

		if (n >= 4*VL) {
			vec_t v_s1_b = zeroes;
			vec_t v_s1_c = zeroes;
			vec_t v_s1_d = zeroes;
			vec_t v_s2_b = zeroes;
			vec_t v_s2_c = zeroes;
			vec_t v_s2_d = zeroes;
			vec_t v_s1_sums   = zeroes;
			vec_t v_s1_sums_b = zeroes;
			vec_t v_s1_sums_c = zeroes;
			vec_t v_s1_sums_d = zeroes;
			vec_t tmp0, tmp1;

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);
				vec_t data_c = VLOADU(p + 2*VL);
				vec_t data_d = VLOADU(p + 3*VL);

				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+v" (data_a), "+v" (data_b),
					     "+v" (data_c), "+v" (data_d));
			#endif

				v_s2   = VDPBUSD(v_s2,   data_a, mults);
				v_s2_b = VDPBUSD(v_s2_b, data_b, mults);
				v_s2_c = VDPBUSD(v_s2_c, data_c, mults);
				v_s2_d = VDPBUSD(v_s2_d, data_d, mults);

				v_s1_sums   = VADD32(v_s1_sums,   v_s1);
				v_s1_sums_b = VADD32(v_s1_sums_b, v_s1_b);
				v_s1_sums_c = VADD32(v_s1_sums_c, v_s1_c);
				v_s1_sums_d = VADD32(v_s1_sums_d, v_s1_d);

				v_s1   = VDPBUSD(v_s1,   data_a, ones);
				v_s1_b = VDPBUSD(v_s1_b, data_b, ones);
				v_s1_c = VDPBUSD(v_s1_c, data_c, ones);
				v_s1_d = VDPBUSD(v_s1_d, data_d, ones);

				/* Same gcc bug workaround.  See above */
			#if GCC_PREREQ(1, 0) && !defined(ARCH_X86_32)
				__asm__("" : "+v" (v_s2), "+v" (v_s2_b),
					     "+v" (v_s2_c), "+v" (v_s2_d),
					     "+v" (v_s1_sums),
					     "+v" (v_s1_sums_b),
					     "+v" (v_s1_sums_c),
					     "+v" (v_s1_sums_d),
					     "+v" (v_s1), "+v" (v_s1_b),
					     "+v" (v_s1_c), "+v" (v_s1_d));
			#endif
				p += 4*VL;
				n -= 4*VL;
			} while (n >= 4*VL);

			/*
			 * Reduce into v_s1 and v_s2 as follows:
			 *
			 * v_s2 = v_s2 + v_s2_b + v_s2_c + v_s2_d +
			 *	  (4*VL)*(v_s1_sums   + v_s1_sums_b +
			 *		  v_s1_sums_c + v_s1_sums_d) +
			 *	  (3*VL)*v_s1 + (2*VL)*v_s1_b + VL*v_s1_c
			 * v_s1 = v_s1 + v_s1_b + v_s1_c + v_s1_d
			 */
			tmp0 = VADD32(v_s1, v_s1_b);
			tmp1 = VADD32(v_s1, v_s1_c);
			v_s1_sums = VADD32_4X(v_s1_sums, v_s1_sums_b,
					      v_s1_sums_c, v_s1_sums_d);
			v_s1 = VADD32_3X(tmp0, v_s1_c, v_s1_d);
			v_s2 = VADD32_7X(VSLL32(v_s1_sums, LOG2_VL + 2),
					 VSLL32(tmp0, LOG2_VL + 1),
					 VSLL32(tmp1, LOG2_VL),
					 v_s2, v_s2_b, v_s2_c, v_s2_d);
		}

		/* Process the last 0 <= n < 4*VL bytes of the chunk. */
		if (n >= 2*VL) {
			const vec_t data_a = VLOADU(p + 0*VL);
			const vec_t data_b = VLOADU(p + 1*VL);

			v_s2 = VADD32(v_s2, VSLL32(v_s1, LOG2_VL + 1));
			v_s1 = VDPBUSD(v_s1, data_a, ones);
			v_s1 = VDPBUSD(v_s1, data_b, ones);
			v_s2 = VDPBUSD(v_s2, data_a, VSET1_8(VL));
			v_s2 = VDPBUSD(v_s2, data_a, mults);
			v_s2 = VDPBUSD(v_s2, data_b, mults);
			p += 2*VL;
			n -= 2*VL;
		}
		if (n) {
			/* Process the last 0 < n < 2*VL bytes of the chunk. */
			vec_t data;

			v_s2 = VADD32(v_s2, VMULLO32(v_s1, VSET1_32(n)));

			mults = VADD8(mults, VSET1_8((int)n - VL));
			if (n > VL) {
				data = VLOADU(p);
				v_s1 = VDPBUSD(v_s1, data, ones);
				v_s2 = VDPBUSD(v_s2, data, mults);
				p += VL;
				n -= VL;
				mults = VADD8(mults, VSET1_8(-VL));
			}
			/*
			 * Process the last 0 < n <= VL bytes of the chunk.
			 * Utilize a masked load if it's available.
			 */
		#if USE_AVX512
			data = VMASKZ_LOADU((mask_t)-1 >> (VL - n), p);
		#else
			data = zeroes;
			memcpy(&data, p, n);
		#endif
			v_s1 = VDPBUSD(v_s1, data, ones);
			v_s2 = VDPBUSD(v_s2, data, mults);
			p += n;
		}

		reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		s1 %= DIVISOR;
		s2 %= DIVISOR;
	}
#else /* USE_VNNI */
	/*
	 * This is Adler-32 for SSE2 and AVX2.
	 *
	 * To horizontally sum bytes, use psadbw + paddd, where one of the
	 * arguments to psadbw is all-zeroes.
	 *
	 * For the s2 contribution from (2*VL - i)*data[i] for each of the 2*VL
	 * bytes of each iteration of the inner loop, use punpck{l,h}bw + paddw
	 * to sum, for each i across iterations, byte i into a corresponding
	 * 16-bit counter in v_byte_sums_*.  After the inner loop, use pmaddwd
	 * to multiply each counter by (2*VL - i), then add the products to s2.
	 *
	 * An alternative implementation would use pmaddubsw and pmaddwd in the
	 * inner loop to do (2*VL - i)*data[i] directly and add the products in
	 * groups of 4 to 32-bit counters.  However, on average that approach
	 * seems to be slower than the current approach which delays the
	 * multiplications.  Also, pmaddubsw requires SSSE3; the current
	 * approach keeps the implementation aligned between SSE2 and AVX2.
	 *
	 * The inner loop processes 2*VL bytes per iteration.  Increasing this
	 * to 4*VL doesn't seem to be helpful here.
	 */
	while (len) {
		/*
		 * Calculate the length of the next data chunk such that s1 and
		 * s2 are guaranteed to not exceed UINT32_MAX, and every
		 * v_byte_sums_* counter is guaranteed to not exceed INT16_MAX.
		 * It's INT16_MAX, not UINT16_MAX, because v_byte_sums_* are
		 * used with pmaddwd which does signed multiplication.  In the
		 * SSE2 case this limits chunks to 4096 bytes instead of 5536.
		 */
		size_t n = MIN(len, MIN(2 * VL * (INT16_MAX / UINT8_MAX),
					MAX_CHUNK_LEN) & ~(2*VL - 1));
		len -= n;

		if (n >= 2*VL) {
			vec_t v_s1 = zeroes;
			vec_t v_s1_sums = zeroes;
			vec_t v_byte_sums_a = zeroes;
			vec_t v_byte_sums_b = zeroes;
			vec_t v_byte_sums_c = zeroes;
			vec_t v_byte_sums_d = zeroes;
			vec_t v_s2;

			s2 += s1 * (n & ~(2*VL - 1));

			do {
				vec_t data_a = VLOADU(p + 0*VL);
				vec_t data_b = VLOADU(p + 1*VL);

				v_s1_sums = VADD32(v_s1_sums, v_s1);
				v_byte_sums_a = VADD16(v_byte_sums_a,
						       VUNPACKLO8(data_a, zeroes));
				v_byte_sums_b = VADD16(v_byte_sums_b,
						       VUNPACKHI8(data_a, zeroes));
				v_byte_sums_c = VADD16(v_byte_sums_c,
						       VUNPACKLO8(data_b, zeroes));
				v_byte_sums_d = VADD16(v_byte_sums_d,
						       VUNPACKHI8(data_b, zeroes));
				v_s1 = VADD32(v_s1,
					      VADD32(VSAD8(data_a, zeroes),
						     VSAD8(data_b, zeroes)));
				/*
				 * Workaround for gcc bug where it generates
				 * unnecessary move instructions
				 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107892)
				 */
			#if GCC_PREREQ(1, 0)
				__asm__("" : "+x" (v_s1), "+x" (v_s1_sums),
					     "+x" (v_byte_sums_a),
					     "+x" (v_byte_sums_b),
					     "+x" (v_byte_sums_c),
					     "+x" (v_byte_sums_d));
			#endif
				p += 2*VL;
				n -= 2*VL;
			} while (n >= 2*VL);

			/*
			 * Calculate v_s2 as (2*VL)*v_s1_sums +
			 * [2*VL, 2*VL - 1, 2*VL - 2, ..., 1] * v_byte_sums.
			 * Then update s1 and s2 from v_s1 and v_s2.
			 */
			v_s2 = VADD32_5X(VSLL32(v_s1_sums, LOG2_VL + 1),
					 VMADD16(v_byte_sums_a, mults_a),
					 VMADD16(v_byte_sums_b, mults_b),
					 VMADD16(v_byte_sums_c, mults_c),
					 VMADD16(v_byte_sums_d, mults_d));
			reduce_to_32bits(v_s1, v_s2, &s1, &s2);
		}
		/*
		 * Process the last 0 <= n < 2*VL bytes of the chunk using
		 * scalar instructions and reduce s1 and s2 mod DIVISOR.
		 */
		ADLER32_CHUNK(s1, s2, p, n);
	}
#endif /* !USE_VNNI */
	return (s2 << 16) | s1;
}

#undef vec_t
#undef mask_t
#undef LOG2_VL
#undef VADD8
#undef VADD16
#undef VADD32
#undef VDPBUSD
#undef VLOAD
#undef VLOADU
#undef VMADD16
#undef VMASKZ_LOADU
#undef VMULLO32
#undef VSAD8
#undef VSET1_8
#undef VSET1_32
#undef VSETZERO
#undef VSLL32
#undef VUNPACKLO8
#undef VUNPACKHI8

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_VNNI
#undef USE_AVX512

/*** End of inlined file: adler32_template.h ***/


#endif

static inline adler32_func_t
arch_select_adler32_func(void)
{
	const u32 features MAYBE_UNUSED = get_x86_cpu_features();

#ifdef adler32_x86_avx512_vl512_vnni
	if ((features & X86_CPU_FEATURE_ZMM) &&
	    HAVE_AVX512BW(features) && HAVE_AVX512VNNI(features))
		return adler32_x86_avx512_vl512_vnni;
#endif
#ifdef adler32_x86_avx512_vl256_vnni
	if (HAVE_AVX512BW(features) && HAVE_AVX512VL(features) &&
	    HAVE_AVX512VNNI(features))
		return adler32_x86_avx512_vl256_vnni;
#endif
#ifdef adler32_x86_avx2_vnni
	if (HAVE_AVX2(features) && HAVE_AVXVNNI(features))
		return adler32_x86_avx2_vnni;
#endif
#ifdef adler32_x86_avx2
	if (HAVE_AVX2(features))
		return adler32_x86_avx2;
#endif
#ifdef adler32_x86_sse2
	if (HAVE_SSE2(features))
		return adler32_x86_sse2;
#endif
	return NULL;
}
#define arch_select_adler32_func	arch_select_adler32_func

#endif /* LIB_X86_ADLER32_IMPL_H */

/*** End of inlined file: adler32_impl.h ***/


#endif

#ifndef DEFAULT_IMPL
#  define DEFAULT_IMPL adler32_generic
#endif

#ifdef arch_select_adler32_func
static u32 dispatch_adler32(u32 adler, const u8 *p, size_t len);

static volatile adler32_func_t adler32_impl = dispatch_adler32;

/* Choose the best implementation at runtime. */
static u32 dispatch_adler32(u32 adler, const u8 *p, size_t len)
{
	adler32_func_t f = arch_select_adler32_func();

	if (f == NULL)
		f = DEFAULT_IMPL;

	adler32_impl = f;
	return f(adler, p, len);
}
#else
/* The best implementation is statically known, so call it directly. */
#define adler32_impl DEFAULT_IMPL
#endif

LIBDEFLATEAPI u32
libdeflate_adler32(u32 adler, const void *buffer, size_t len)
{
	if (buffer == NULL) /* Return initial value. */
		return 1;
	return adler32_impl(adler, buffer, len);
}

/*** End of inlined file: adler32.c ***/


/*** Start of inlined file: crc32.c ***/
/*
 * High-level description of CRC
 * =============================
 *
 * Consider a bit sequence 'bits[1...len]'.  Interpret 'bits' as the "message"
 * polynomial M(x) with coefficients in GF(2) (the field of integers modulo 2),
 * where the coefficient of 'x^i' is 'bits[len - i]'.  Then, compute:
 *
 *			R(x) = M(x)*x^n mod G(x)
 *
 * where G(x) is a selected "generator" polynomial of degree 'n'.  The remainder
 * R(x) is a polynomial of max degree 'n - 1'.  The CRC of 'bits' is R(x)
 * interpreted as a bitstring of length 'n'.
 *
 * CRC used in gzip
 * ================
 *
 * In the gzip format (RFC 1952):
 *
 *	- The bitstring to checksum is formed from the bytes of the uncompressed
 *	  data by concatenating the bits from the bytes in order, proceeding
 *	  from the low-order bit to the high-order bit within each byte.
 *
 *	- The generator polynomial G(x) is: x^32 + x^26 + x^23 + x^22 + x^16 +
 *	  x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1.
 *	  Consequently, the CRC length is 32 bits ("CRC-32").
 *
 *	- The highest order 32 coefficients of M(x)*x^n are inverted.
 *
 *	- All 32 coefficients of R(x) are inverted.
 *
 * The two inversions cause added leading and trailing zero bits to affect the
 * resulting CRC, whereas with a regular CRC such bits would have no effect on
 * the CRC.
 *
 * Computation and optimizations
 * =============================
 *
 * We can compute R(x) through "long division", maintaining only 32 bits of
 * state at any given time.  Multiplication by 'x' can be implemented as
 * right-shifting by 1 (assuming the polynomial<=>bitstring mapping where the
 * highest order bit represents the coefficient of x^0), and both addition and
 * subtraction can be implemented as bitwise exclusive OR (since we are working
 * in GF(2)).  Here is an unoptimized implementation:
 *
 *	static u32 crc32_gzip(const u8 *p, size_t len)
 *	{
 *		u32 crc = 0;
 *		const u32 divisor = 0xEDB88320;
 *
 *		for (size_t i = 0; i < len * 8 + 32; i++) {
 *			int bit;
 *			u32 multiple;
 *
 *			if (i < len * 8)
 *				bit = (p[i / 8] >> (i % 8)) & 1;
 *			else
 *				bit = 0; // one of the 32 appended 0 bits
 *
 *			if (i < 32) // the first 32 bits are inverted
 *				bit ^= 1;
 *
 *			if (crc & 1)
 *				multiple = divisor;
 *			else
 *				multiple = 0;
 *
 *			crc >>= 1;
 *			crc |= (u32)bit << 31;
 *			crc ^= multiple;
 *		}
 *
 *		return ~crc;
 *	}
 *
 * In this implementation, the 32-bit integer 'crc' maintains the remainder of
 * the currently processed portion of the message (with 32 zero bits appended)
 * when divided by the generator polynomial.  'crc' is the representation of
 * R(x), and 'divisor' is the representation of G(x) excluding the x^32
 * coefficient.  For each bit to process, we multiply R(x) by 'x^1', then add
 * 'x^0' if the new bit is a 1.  If this causes R(x) to gain a nonzero x^32
 * term, then we subtract G(x) from R(x).
 *
 * We can speed this up by taking advantage of the fact that XOR is commutative
 * and associative, so the order in which we combine the inputs into 'crc' is
 * unimportant.  And since each message bit we add doesn't affect the choice of
 * 'multiple' until 32 bits later, we need not actually add each message bit
 * until that point:
 *
 *	static u32 crc32_gzip(const u8 *p, size_t len)
 *	{
 *		u32 crc = ~0;
 *		const u32 divisor = 0xEDB88320;
 *
 *		for (size_t i = 0; i < len * 8; i++) {
 *			int bit;
 *			u32 multiple;
 *
 *			bit = (p[i / 8] >> (i % 8)) & 1;
 *			crc ^= bit;
 *			if (crc & 1)
 *				multiple = divisor;
 *			else
 *				multiple = 0;
 *			crc >>= 1;
 *			crc ^= multiple;
 *		}
 *
 *		return ~crc;
 *	}
 *
 * With the above implementation we get the effect of 32 appended 0 bits for
 * free; they never affect the choice of a divisor, nor would they change the
 * value of 'crc' if they were to be actually XOR'ed in.  And by starting with a
 * remainder of all 1 bits, we get the effect of complementing the first 32
 * message bits.
 *
 * The next optimization is to process the input in multi-bit units.  Suppose
 * that we insert the next 'n' message bits into the remainder.  Then we get an
 * intermediate remainder of length '32 + n' bits, and the CRC of the extra 'n'
 * bits is the amount by which the low 32 bits of the remainder will change as a
 * result of cancelling out those 'n' bits.  Taking n=8 (one byte) and
 * precomputing a table containing the CRC of each possible byte, we get
 * crc32_slice1() defined below.
 *
 * As a further optimization, we could increase the multi-bit unit size to 16.
 * However, that is inefficient because the table size explodes from 256 entries
 * (1024 bytes) to 65536 entries (262144 bytes), which wastes memory and won't
 * fit in L1 cache on typical processors.
 *
 * However, we can actually process 4 bytes at a time using 4 different tables
 * with 256 entries each.  Logically, we form a 64-bit intermediate remainder
 * and cancel out the high 32 bits in 8-bit chunks.  Bits 32-39 are cancelled
 * out by the CRC of those bits, whereas bits 40-47 are be cancelled out by the
 * CRC of those bits with 8 zero bits appended, and so on.
 *
 * In crc32_slice8(), this method is extended to 8 bytes at a time.  The
 * intermediate remainder (which we never actually store explicitly) is 96 bits.
 *
 * On CPUs that support fast carryless multiplication, CRCs can be computed even
 * more quickly via "folding".  See e.g. the x86 PCLMUL implementations.
 */


/*** Start of inlined file: crc32_multipliers.h ***/
#define CRC32_X159_MODG 0xae689191 /* x^159 mod G(x) */
#define CRC32_X95_MODG 0xccaa009e /* x^95 mod G(x) */

#define CRC32_X287_MODG 0xf1da05aa /* x^287 mod G(x) */
#define CRC32_X223_MODG 0x81256527 /* x^223 mod G(x) */

#define CRC32_X415_MODG 0x3db1ecdc /* x^415 mod G(x) */
#define CRC32_X351_MODG 0xaf449247 /* x^351 mod G(x) */

#define CRC32_X543_MODG 0x8f352d95 /* x^543 mod G(x) */
#define CRC32_X479_MODG 0x1d9513d7 /* x^479 mod G(x) */

#define CRC32_X671_MODG 0x1c279815 /* x^671 mod G(x) */
#define CRC32_X607_MODG 0xae0b5394 /* x^607 mod G(x) */

#define CRC32_X799_MODG 0xdf068dc2 /* x^799 mod G(x) */
#define CRC32_X735_MODG 0x57c54819 /* x^735 mod G(x) */

#define CRC32_X927_MODG 0x31f8303f /* x^927 mod G(x) */
#define CRC32_X863_MODG 0x0cbec0ed /* x^863 mod G(x) */

#define CRC32_X1055_MODG 0x33fff533 /* x^1055 mod G(x) */
#define CRC32_X991_MODG 0x910eeec1 /* x^991 mod G(x) */

#define CRC32_X1183_MODG 0x26b70c3d /* x^1183 mod G(x) */
#define CRC32_X1119_MODG 0x3f41287a /* x^1119 mod G(x) */

#define CRC32_X1311_MODG 0xe3543be0 /* x^1311 mod G(x) */
#define CRC32_X1247_MODG 0x9026d5b1 /* x^1247 mod G(x) */

#define CRC32_X1439_MODG 0x5a1bb05d /* x^1439 mod G(x) */
#define CRC32_X1375_MODG 0xd1df2327 /* x^1375 mod G(x) */

#define CRC32_X1567_MODG 0x596c8d81 /* x^1567 mod G(x) */
#define CRC32_X1503_MODG 0xf5e48c85 /* x^1503 mod G(x) */

#define CRC32_X1695_MODG 0x682bdd4f /* x^1695 mod G(x) */
#define CRC32_X1631_MODG 0x3c656ced /* x^1631 mod G(x) */

#define CRC32_X1823_MODG 0x4a28bd43 /* x^1823 mod G(x) */
#define CRC32_X1759_MODG 0xfe807bbd /* x^1759 mod G(x) */

#define CRC32_X1951_MODG 0x0077f00d /* x^1951 mod G(x) */
#define CRC32_X1887_MODG 0x1f0c2cdd /* x^1887 mod G(x) */

#define CRC32_X2079_MODG 0xce3371cb /* x^2079 mod G(x) */
#define CRC32_X2015_MODG 0xe95c1271 /* x^2015 mod G(x) */

#define CRC32_X2207_MODG 0xa749e894 /* x^2207 mod G(x) */
#define CRC32_X2143_MODG 0xb918a347 /* x^2143 mod G(x) */

#define CRC32_X2335_MODG 0x2c538639 /* x^2335 mod G(x) */
#define CRC32_X2271_MODG 0x71d54a59 /* x^2271 mod G(x) */

#define CRC32_X2463_MODG 0x32b0733c /* x^2463 mod G(x) */
#define CRC32_X2399_MODG 0xff6f2fc2 /* x^2399 mod G(x) */

#define CRC32_X2591_MODG 0x0e9bd5cc /* x^2591 mod G(x) */
#define CRC32_X2527_MODG 0xcec97417 /* x^2527 mod G(x) */

#define CRC32_X2719_MODG 0x76278617 /* x^2719 mod G(x) */
#define CRC32_X2655_MODG 0x1c63267b /* x^2655 mod G(x) */

#define CRC32_X2847_MODG 0xc51b93e3 /* x^2847 mod G(x) */
#define CRC32_X2783_MODG 0xf183c71b /* x^2783 mod G(x) */

#define CRC32_X2975_MODG 0x7eaed122 /* x^2975 mod G(x) */
#define CRC32_X2911_MODG 0x9b9bdbd0 /* x^2911 mod G(x) */

#define CRC32_X3103_MODG 0x2ce423f1 /* x^3103 mod G(x) */
#define CRC32_X3039_MODG 0xd31343ea /* x^3039 mod G(x) */

#define CRC32_X3231_MODG 0x8b8d8645 /* x^3231 mod G(x) */
#define CRC32_X3167_MODG 0x4470ac44 /* x^3167 mod G(x) */

#define CRC32_X3359_MODG 0x4b700aa8 /* x^3359 mod G(x) */
#define CRC32_X3295_MODG 0xeea395c4 /* x^3295 mod G(x) */

#define CRC32_X3487_MODG 0xeff5e99d /* x^3487 mod G(x) */
#define CRC32_X3423_MODG 0xf9d9c7ee /* x^3423 mod G(x) */

#define CRC32_X3615_MODG 0xad0d2bb2 /* x^3615 mod G(x) */
#define CRC32_X3551_MODG 0xcd669a40 /* x^3551 mod G(x) */

#define CRC32_X3743_MODG 0x9fb66bd3 /* x^3743 mod G(x) */
#define CRC32_X3679_MODG 0x6d40f445 /* x^3679 mod G(x) */

#define CRC32_X3871_MODG 0xc2dcc467 /* x^3871 mod G(x) */
#define CRC32_X3807_MODG 0x9ee62949 /* x^3807 mod G(x) */

#define CRC32_X3999_MODG 0x398e2ff2 /* x^3999 mod G(x) */
#define CRC32_X3935_MODG 0x145575d5 /* x^3935 mod G(x) */

#define CRC32_X4127_MODG 0x1072db28 /* x^4127 mod G(x) */
#define CRC32_X4063_MODG 0x0c30f51d /* x^4063 mod G(x) */

#define CRC32_BARRETT_CONSTANT_1 0xb4e5b025f7011641ULL /* floor(x^95 / G(x)) */
#define CRC32_BARRETT_CONSTANT_2 0x00000001db710641ULL /* G(x) */

#define CRC32_NUM_CHUNKS 4
#define CRC32_MIN_VARIABLE_CHUNK_LEN 128UL
#define CRC32_MAX_VARIABLE_CHUNK_LEN 16384UL

/* Multipliers for implementations that use a variable chunk length */
static const u32 crc32_mults_for_chunklen[][CRC32_NUM_CHUNKS - 1] MAYBE_UNUSED = {
	{ 0 /* unused row */ },
	/* chunk_len=128 */
	{ 0xd31343ea /* x^3039 mod G(x) */, 0xe95c1271 /* x^2015 mod G(x) */, 0x910eeec1 /* x^991 mod G(x) */, },
	/* chunk_len=256 */
	{ 0x1d6708a0 /* x^6111 mod G(x) */, 0x0c30f51d /* x^4063 mod G(x) */, 0xe95c1271 /* x^2015 mod G(x) */, },
	/* chunk_len=384 */
	{ 0xdb3839f3 /* x^9183 mod G(x) */, 0x1d6708a0 /* x^6111 mod G(x) */, 0xd31343ea /* x^3039 mod G(x) */, },
	/* chunk_len=512 */
	{ 0x1753ab84 /* x^12255 mod G(x) */, 0xbbf2f6d6 /* x^8159 mod G(x) */, 0x0c30f51d /* x^4063 mod G(x) */, },
	/* chunk_len=640 */
	{ 0x3796455c /* x^15327 mod G(x) */, 0xb8e0e4a8 /* x^10207 mod G(x) */, 0xc352f6de /* x^5087 mod G(x) */, },
	/* chunk_len=768 */
	{ 0x3954de39 /* x^18399 mod G(x) */, 0x1753ab84 /* x^12255 mod G(x) */, 0x1d6708a0 /* x^6111 mod G(x) */, },
	/* chunk_len=896 */
	{ 0x632d78c5 /* x^21471 mod G(x) */, 0x3fc33de4 /* x^14303 mod G(x) */, 0x9a1b53c8 /* x^7135 mod G(x) */, },
	/* chunk_len=1024 */
	{ 0xa0decef3 /* x^24543 mod G(x) */, 0x7b4aa8b7 /* x^16351 mod G(x) */, 0xbbf2f6d6 /* x^8159 mod G(x) */, },
	/* chunk_len=1152 */
	{ 0xe9c09bb0 /* x^27615 mod G(x) */, 0x3954de39 /* x^18399 mod G(x) */, 0xdb3839f3 /* x^9183 mod G(x) */, },
	/* chunk_len=1280 */
	{ 0xd51917a4 /* x^30687 mod G(x) */, 0xcae68461 /* x^20447 mod G(x) */, 0xb8e0e4a8 /* x^10207 mod G(x) */, },
	/* chunk_len=1408 */
	{ 0x154a8a62 /* x^33759 mod G(x) */, 0x41e7589c /* x^22495 mod G(x) */, 0x3e9a43cd /* x^11231 mod G(x) */, },
	/* chunk_len=1536 */
	{ 0xf196555d /* x^36831 mod G(x) */, 0xa0decef3 /* x^24543 mod G(x) */, 0x1753ab84 /* x^12255 mod G(x) */, },
	/* chunk_len=1664 */
	{ 0x8eec2999 /* x^39903 mod G(x) */, 0xefb0a128 /* x^26591 mod G(x) */, 0x6044fbb0 /* x^13279 mod G(x) */, },
	/* chunk_len=1792 */
	{ 0x27892abf /* x^42975 mod G(x) */, 0x48d72bb1 /* x^28639 mod G(x) */, 0x3fc33de4 /* x^14303 mod G(x) */, },
	/* chunk_len=1920 */
	{ 0x77bc2419 /* x^46047 mod G(x) */, 0xd51917a4 /* x^30687 mod G(x) */, 0x3796455c /* x^15327 mod G(x) */, },
	/* chunk_len=2048 */
	{ 0xcea114a5 /* x^49119 mod G(x) */, 0x68c0a2c5 /* x^32735 mod G(x) */, 0x7b4aa8b7 /* x^16351 mod G(x) */, },
	/* chunk_len=2176 */
	{ 0xa1077e85 /* x^52191 mod G(x) */, 0x188cc628 /* x^34783 mod G(x) */, 0x0c21f835 /* x^17375 mod G(x) */, },
	/* chunk_len=2304 */
	{ 0xc5ed75e1 /* x^55263 mod G(x) */, 0xf196555d /* x^36831 mod G(x) */, 0x3954de39 /* x^18399 mod G(x) */, },
	/* chunk_len=2432 */
	{ 0xca4fba3f /* x^58335 mod G(x) */, 0x0acfa26f /* x^38879 mod G(x) */, 0x6cb21510 /* x^19423 mod G(x) */, },
	/* chunk_len=2560 */
	{ 0xcf5bcdc4 /* x^61407 mod G(x) */, 0x4fae7fc0 /* x^40927 mod G(x) */, 0xcae68461 /* x^20447 mod G(x) */, },
	/* chunk_len=2688 */
	{ 0xf36b9d16 /* x^64479 mod G(x) */, 0x27892abf /* x^42975 mod G(x) */, 0x632d78c5 /* x^21471 mod G(x) */, },
	/* chunk_len=2816 */
	{ 0xf76fd988 /* x^67551 mod G(x) */, 0xed5c39b1 /* x^45023 mod G(x) */, 0x41e7589c /* x^22495 mod G(x) */, },
	/* chunk_len=2944 */
	{ 0x6c45d92e /* x^70623 mod G(x) */, 0xff809fcd /* x^47071 mod G(x) */, 0x0c46baec /* x^23519 mod G(x) */, },
	/* chunk_len=3072 */
	{ 0x6116b82b /* x^73695 mod G(x) */, 0xcea114a5 /* x^49119 mod G(x) */, 0xa0decef3 /* x^24543 mod G(x) */, },
	/* chunk_len=3200 */
	{ 0x4d9899bb /* x^76767 mod G(x) */, 0x9f9d8d9c /* x^51167 mod G(x) */, 0x53deb236 /* x^25567 mod G(x) */, },
	/* chunk_len=3328 */
	{ 0x3e7c93b9 /* x^79839 mod G(x) */, 0x6666b805 /* x^53215 mod G(x) */, 0xefb0a128 /* x^26591 mod G(x) */, },
	/* chunk_len=3456 */
	{ 0x388b20ac /* x^82911 mod G(x) */, 0xc5ed75e1 /* x^55263 mod G(x) */, 0xe9c09bb0 /* x^27615 mod G(x) */, },
	/* chunk_len=3584 */
	{ 0x0956d953 /* x^85983 mod G(x) */, 0x97fbdb14 /* x^57311 mod G(x) */, 0x48d72bb1 /* x^28639 mod G(x) */, },
	/* chunk_len=3712 */
	{ 0x55cb4dfe /* x^89055 mod G(x) */, 0x1b37c832 /* x^59359 mod G(x) */, 0xc07331b3 /* x^29663 mod G(x) */, },
	/* chunk_len=3840 */
	{ 0x52222fea /* x^92127 mod G(x) */, 0xcf5bcdc4 /* x^61407 mod G(x) */, 0xd51917a4 /* x^30687 mod G(x) */, },
	/* chunk_len=3968 */
	{ 0x0603989b /* x^95199 mod G(x) */, 0xb03c8112 /* x^63455 mod G(x) */, 0x5e04b9a5 /* x^31711 mod G(x) */, },
	/* chunk_len=4096 */
	{ 0x4470c029 /* x^98271 mod G(x) */, 0x2339d155 /* x^65503 mod G(x) */, 0x68c0a2c5 /* x^32735 mod G(x) */, },
	/* chunk_len=4224 */
	{ 0xb6f35093 /* x^101343 mod G(x) */, 0xf76fd988 /* x^67551 mod G(x) */, 0x154a8a62 /* x^33759 mod G(x) */, },
	/* chunk_len=4352 */
	{ 0xc46805ba /* x^104415 mod G(x) */, 0x416f9449 /* x^69599 mod G(x) */, 0x188cc628 /* x^34783 mod G(x) */, },
	/* chunk_len=4480 */
	{ 0xc3876592 /* x^107487 mod G(x) */, 0x4b809189 /* x^71647 mod G(x) */, 0xc35cf6e7 /* x^35807 mod G(x) */, },
	/* chunk_len=4608 */
	{ 0x5b0c98b9 /* x^110559 mod G(x) */, 0x6116b82b /* x^73695 mod G(x) */, 0xf196555d /* x^36831 mod G(x) */, },
	/* chunk_len=4736 */
	{ 0x30d13e5f /* x^113631 mod G(x) */, 0x4c5a315a /* x^75743 mod G(x) */, 0x8c224466 /* x^37855 mod G(x) */, },
	/* chunk_len=4864 */
	{ 0x54afca53 /* x^116703 mod G(x) */, 0xbccfa2c1 /* x^77791 mod G(x) */, 0x0acfa26f /* x^38879 mod G(x) */, },
	/* chunk_len=4992 */
	{ 0x93102436 /* x^119775 mod G(x) */, 0x3e7c93b9 /* x^79839 mod G(x) */, 0x8eec2999 /* x^39903 mod G(x) */, },
	/* chunk_len=5120 */
	{ 0xbd2655a8 /* x^122847 mod G(x) */, 0x3e116c9d /* x^81887 mod G(x) */, 0x4fae7fc0 /* x^40927 mod G(x) */, },
	/* chunk_len=5248 */
	{ 0x70cd7f26 /* x^125919 mod G(x) */, 0x408e57f2 /* x^83935 mod G(x) */, 0x1691be45 /* x^41951 mod G(x) */, },
	/* chunk_len=5376 */
	{ 0x2d546c53 /* x^128991 mod G(x) */, 0x0956d953 /* x^85983 mod G(x) */, 0x27892abf /* x^42975 mod G(x) */, },
	/* chunk_len=5504 */
	{ 0xb53410a8 /* x^132063 mod G(x) */, 0x42ebf0ad /* x^88031 mod G(x) */, 0x161f3c12 /* x^43999 mod G(x) */, },
	/* chunk_len=5632 */
	{ 0x67a93f75 /* x^135135 mod G(x) */, 0xcf3233e4 /* x^90079 mod G(x) */, 0xed5c39b1 /* x^45023 mod G(x) */, },
	/* chunk_len=5760 */
	{ 0x9830ac33 /* x^138207 mod G(x) */, 0x52222fea /* x^92127 mod G(x) */, 0x77bc2419 /* x^46047 mod G(x) */, },
	/* chunk_len=5888 */
	{ 0xb0b6fc3e /* x^141279 mod G(x) */, 0x2fde73f8 /* x^94175 mod G(x) */, 0xff809fcd /* x^47071 mod G(x) */, },
	/* chunk_len=6016 */
	{ 0x84170f16 /* x^144351 mod G(x) */, 0xced90d99 /* x^96223 mod G(x) */, 0x30de0f98 /* x^48095 mod G(x) */, },
	/* chunk_len=6144 */
	{ 0xd7017a0c /* x^147423 mod G(x) */, 0x4470c029 /* x^98271 mod G(x) */, 0xcea114a5 /* x^49119 mod G(x) */, },
	/* chunk_len=6272 */
	{ 0xadb25de6 /* x^150495 mod G(x) */, 0x84f40beb /* x^100319 mod G(x) */, 0x2b7e0e1b /* x^50143 mod G(x) */, },
	/* chunk_len=6400 */
	{ 0x8282fddc /* x^153567 mod G(x) */, 0xec855937 /* x^102367 mod G(x) */, 0x9f9d8d9c /* x^51167 mod G(x) */, },
	/* chunk_len=6528 */
	{ 0x46362bee /* x^156639 mod G(x) */, 0xc46805ba /* x^104415 mod G(x) */, 0xa1077e85 /* x^52191 mod G(x) */, },
	/* chunk_len=6656 */
	{ 0xb9077a01 /* x^159711 mod G(x) */, 0xdf7a24ac /* x^106463 mod G(x) */, 0x6666b805 /* x^53215 mod G(x) */, },
	/* chunk_len=6784 */
	{ 0xf51d9bc6 /* x^162783 mod G(x) */, 0x2b52dc39 /* x^108511 mod G(x) */, 0x7e774cf6 /* x^54239 mod G(x) */, },
	/* chunk_len=6912 */
	{ 0x4ca19a29 /* x^165855 mod G(x) */, 0x5b0c98b9 /* x^110559 mod G(x) */, 0xc5ed75e1 /* x^55263 mod G(x) */, },
	/* chunk_len=7040 */
	{ 0xdc0fc3fc /* x^168927 mod G(x) */, 0xb939fcdf /* x^112607 mod G(x) */, 0x3678fed2 /* x^56287 mod G(x) */, },
	/* chunk_len=7168 */
	{ 0x63c3d167 /* x^171999 mod G(x) */, 0x70f9947d /* x^114655 mod G(x) */, 0x97fbdb14 /* x^57311 mod G(x) */, },
	/* chunk_len=7296 */
	{ 0x5851d254 /* x^175071 mod G(x) */, 0x54afca53 /* x^116703 mod G(x) */, 0xca4fba3f /* x^58335 mod G(x) */, },
	/* chunk_len=7424 */
	{ 0xfeacf2a1 /* x^178143 mod G(x) */, 0x7a3c0a6a /* x^118751 mod G(x) */, 0x1b37c832 /* x^59359 mod G(x) */, },
	/* chunk_len=7552 */
	{ 0x93b7edc8 /* x^181215 mod G(x) */, 0x1fea4d2a /* x^120799 mod G(x) */, 0x58fa96ee /* x^60383 mod G(x) */, },
	/* chunk_len=7680 */
	{ 0x5539e44a /* x^184287 mod G(x) */, 0xbd2655a8 /* x^122847 mod G(x) */, 0xcf5bcdc4 /* x^61407 mod G(x) */, },
	/* chunk_len=7808 */
	{ 0xde32a3d2 /* x^187359 mod G(x) */, 0x4ff61aa1 /* x^124895 mod G(x) */, 0x6a6a3694 /* x^62431 mod G(x) */, },
	/* chunk_len=7936 */
	{ 0xf0baeeb6 /* x^190431 mod G(x) */, 0x7ae2f6f4 /* x^126943 mod G(x) */, 0xb03c8112 /* x^63455 mod G(x) */, },
	/* chunk_len=8064 */
	{ 0xbe15887f /* x^193503 mod G(x) */, 0x2d546c53 /* x^128991 mod G(x) */, 0xf36b9d16 /* x^64479 mod G(x) */, },
	/* chunk_len=8192 */
	{ 0x64f34a05 /* x^196575 mod G(x) */, 0xe0ee5efe /* x^131039 mod G(x) */, 0x2339d155 /* x^65503 mod G(x) */, },
	/* chunk_len=8320 */
	{ 0x1b6d1aea /* x^199647 mod G(x) */, 0xfeafb67c /* x^133087 mod G(x) */, 0x4fb001a8 /* x^66527 mod G(x) */, },
	/* chunk_len=8448 */
	{ 0x82adb0b8 /* x^202719 mod G(x) */, 0x67a93f75 /* x^135135 mod G(x) */, 0xf76fd988 /* x^67551 mod G(x) */, },
	/* chunk_len=8576 */
	{ 0x694587c7 /* x^205791 mod G(x) */, 0x3b34408b /* x^137183 mod G(x) */, 0xeccb2978 /* x^68575 mod G(x) */, },
	/* chunk_len=8704 */
	{ 0xd2fc57c3 /* x^208863 mod G(x) */, 0x07fcf8c6 /* x^139231 mod G(x) */, 0x416f9449 /* x^69599 mod G(x) */, },
	/* chunk_len=8832 */
	{ 0x9dd6837c /* x^211935 mod G(x) */, 0xb0b6fc3e /* x^141279 mod G(x) */, 0x6c45d92e /* x^70623 mod G(x) */, },
	/* chunk_len=8960 */
	{ 0x3a9d1f97 /* x^215007 mod G(x) */, 0xefd033b2 /* x^143327 mod G(x) */, 0x4b809189 /* x^71647 mod G(x) */, },
	/* chunk_len=9088 */
	{ 0x1eee1d2a /* x^218079 mod G(x) */, 0xf2a6e46e /* x^145375 mod G(x) */, 0x55b4c814 /* x^72671 mod G(x) */, },
	/* chunk_len=9216 */
	{ 0xb57c7728 /* x^221151 mod G(x) */, 0xd7017a0c /* x^147423 mod G(x) */, 0x6116b82b /* x^73695 mod G(x) */, },
	/* chunk_len=9344 */
	{ 0xf2fc5d61 /* x^224223 mod G(x) */, 0x242aac86 /* x^149471 mod G(x) */, 0x05245cf0 /* x^74719 mod G(x) */, },
	/* chunk_len=9472 */
	{ 0x26387824 /* x^227295 mod G(x) */, 0xc15c4ca5 /* x^151519 mod G(x) */, 0x4c5a315a /* x^75743 mod G(x) */, },
	/* chunk_len=9600 */
	{ 0x8c151e77 /* x^230367 mod G(x) */, 0x8282fddc /* x^153567 mod G(x) */, 0x4d9899bb /* x^76767 mod G(x) */, },
	/* chunk_len=9728 */
	{ 0x8ea1f680 /* x^233439 mod G(x) */, 0xf5ff6cdd /* x^155615 mod G(x) */, 0xbccfa2c1 /* x^77791 mod G(x) */, },
	/* chunk_len=9856 */
	{ 0xe8cf3d2a /* x^236511 mod G(x) */, 0x338b1fb1 /* x^157663 mod G(x) */, 0xeda61f70 /* x^78815 mod G(x) */, },
	/* chunk_len=9984 */
	{ 0x21f15b59 /* x^239583 mod G(x) */, 0xb9077a01 /* x^159711 mod G(x) */, 0x3e7c93b9 /* x^79839 mod G(x) */, },
	/* chunk_len=10112 */
	{ 0x6f68d64a /* x^242655 mod G(x) */, 0x901b0161 /* x^161759 mod G(x) */, 0xb9fd3537 /* x^80863 mod G(x) */, },
	/* chunk_len=10240 */
	{ 0x71b74d95 /* x^245727 mod G(x) */, 0xf5ddd5ad /* x^163807 mod G(x) */, 0x3e116c9d /* x^81887 mod G(x) */, },
	/* chunk_len=10368 */
	{ 0x4c2e7261 /* x^248799 mod G(x) */, 0x4ca19a29 /* x^165855 mod G(x) */, 0x388b20ac /* x^82911 mod G(x) */, },
	/* chunk_len=10496 */
	{ 0x8a2d38e8 /* x^251871 mod G(x) */, 0xd27ee0a1 /* x^167903 mod G(x) */, 0x408e57f2 /* x^83935 mod G(x) */, },
	/* chunk_len=10624 */
	{ 0x7e58ca17 /* x^254943 mod G(x) */, 0x69dfedd2 /* x^169951 mod G(x) */, 0x3a76805e /* x^84959 mod G(x) */, },
	/* chunk_len=10752 */
	{ 0xf997967f /* x^258015 mod G(x) */, 0x63c3d167 /* x^171999 mod G(x) */, 0x0956d953 /* x^85983 mod G(x) */, },
	/* chunk_len=10880 */
	{ 0x48215963 /* x^261087 mod G(x) */, 0x71e1dfe0 /* x^174047 mod G(x) */, 0x42a6d410 /* x^87007 mod G(x) */, },
	/* chunk_len=11008 */
	{ 0xa704b94c /* x^264159 mod G(x) */, 0x679f198a /* x^176095 mod G(x) */, 0x42ebf0ad /* x^88031 mod G(x) */, },
	/* chunk_len=11136 */
	{ 0x1d699056 /* x^267231 mod G(x) */, 0xfeacf2a1 /* x^178143 mod G(x) */, 0x55cb4dfe /* x^89055 mod G(x) */, },
	/* chunk_len=11264 */
	{ 0x6800bcc5 /* x^270303 mod G(x) */, 0x16024f15 /* x^180191 mod G(x) */, 0xcf3233e4 /* x^90079 mod G(x) */, },
	/* chunk_len=11392 */
	{ 0x2d48e4ca /* x^273375 mod G(x) */, 0xbe61582f /* x^182239 mod G(x) */, 0x46026283 /* x^91103 mod G(x) */, },
	/* chunk_len=11520 */
	{ 0x4c4c2b55 /* x^276447 mod G(x) */, 0x5539e44a /* x^184287 mod G(x) */, 0x52222fea /* x^92127 mod G(x) */, },
	/* chunk_len=11648 */
	{ 0xd8ce94cb /* x^279519 mod G(x) */, 0xbc613c26 /* x^186335 mod G(x) */, 0x33776b4b /* x^93151 mod G(x) */, },
	/* chunk_len=11776 */
	{ 0xd0b5a02b /* x^282591 mod G(x) */, 0x490d3cc6 /* x^188383 mod G(x) */, 0x2fde73f8 /* x^94175 mod G(x) */, },
	/* chunk_len=11904 */
	{ 0xa223f7ec /* x^285663 mod G(x) */, 0xf0baeeb6 /* x^190431 mod G(x) */, 0x0603989b /* x^95199 mod G(x) */, },
	/* chunk_len=12032 */
	{ 0x58de337a /* x^288735 mod G(x) */, 0x3bf3d597 /* x^192479 mod G(x) */, 0xced90d99 /* x^96223 mod G(x) */, },
	/* chunk_len=12160 */
	{ 0x37f5d8f4 /* x^291807 mod G(x) */, 0x4d5b699b /* x^194527 mod G(x) */, 0xd7262e5f /* x^97247 mod G(x) */, },
	/* chunk_len=12288 */
	{ 0xfa8a435d /* x^294879 mod G(x) */, 0x64f34a05 /* x^196575 mod G(x) */, 0x4470c029 /* x^98271 mod G(x) */, },
	/* chunk_len=12416 */
	{ 0x238709fe /* x^297951 mod G(x) */, 0x52e7458f /* x^198623 mod G(x) */, 0x9a174cd3 /* x^99295 mod G(x) */, },
	/* chunk_len=12544 */
	{ 0x9e1ba6f5 /* x^301023 mod G(x) */, 0xef0272f7 /* x^200671 mod G(x) */, 0x84f40beb /* x^100319 mod G(x) */, },
	/* chunk_len=12672 */
	{ 0xcd8b57fa /* x^304095 mod G(x) */, 0x82adb0b8 /* x^202719 mod G(x) */, 0xb6f35093 /* x^101343 mod G(x) */, },
	/* chunk_len=12800 */
	{ 0x0aed142f /* x^307167 mod G(x) */, 0xb1650290 /* x^204767 mod G(x) */, 0xec855937 /* x^102367 mod G(x) */, },
	/* chunk_len=12928 */
	{ 0xd1f064db /* x^310239 mod G(x) */, 0x6e7340d3 /* x^206815 mod G(x) */, 0x5c28cb52 /* x^103391 mod G(x) */, },
	/* chunk_len=13056 */
	{ 0x464ac895 /* x^313311 mod G(x) */, 0xd2fc57c3 /* x^208863 mod G(x) */, 0xc46805ba /* x^104415 mod G(x) */, },
	/* chunk_len=13184 */
	{ 0xa0e6beea /* x^316383 mod G(x) */, 0xcfeec3d0 /* x^210911 mod G(x) */, 0x0225d214 /* x^105439 mod G(x) */, },
	/* chunk_len=13312 */
	{ 0x78703ce0 /* x^319455 mod G(x) */, 0xc60f6075 /* x^212959 mod G(x) */, 0xdf7a24ac /* x^106463 mod G(x) */, },
	/* chunk_len=13440 */
	{ 0xfea48165 /* x^322527 mod G(x) */, 0x3a9d1f97 /* x^215007 mod G(x) */, 0xc3876592 /* x^107487 mod G(x) */, },
	/* chunk_len=13568 */
	{ 0xdb89b8db /* x^325599 mod G(x) */, 0xa6172211 /* x^217055 mod G(x) */, 0x2b52dc39 /* x^108511 mod G(x) */, },
	/* chunk_len=13696 */
	{ 0x7ca03731 /* x^328671 mod G(x) */, 0x1db42849 /* x^219103 mod G(x) */, 0xc5df246e /* x^109535 mod G(x) */, },
	/* chunk_len=13824 */
	{ 0x8801d0aa /* x^331743 mod G(x) */, 0xb57c7728 /* x^221151 mod G(x) */, 0x5b0c98b9 /* x^110559 mod G(x) */, },
	/* chunk_len=13952 */
	{ 0xf89cd7f0 /* x^334815 mod G(x) */, 0xcc396a0b /* x^223199 mod G(x) */, 0xdb799c51 /* x^111583 mod G(x) */, },
	/* chunk_len=14080 */
	{ 0x1611a808 /* x^337887 mod G(x) */, 0xaeae6105 /* x^225247 mod G(x) */, 0xb939fcdf /* x^112607 mod G(x) */, },
	/* chunk_len=14208 */
	{ 0xe3cdb888 /* x^340959 mod G(x) */, 0x26387824 /* x^227295 mod G(x) */, 0x30d13e5f /* x^113631 mod G(x) */, },
	/* chunk_len=14336 */
	{ 0x552a4cf6 /* x^344031 mod G(x) */, 0xee2d04bb /* x^229343 mod G(x) */, 0x70f9947d /* x^114655 mod G(x) */, },
	/* chunk_len=14464 */
	{ 0x85e248e9 /* x^347103 mod G(x) */, 0x0a79663f /* x^231391 mod G(x) */, 0x53339cf7 /* x^115679 mod G(x) */, },
	/* chunk_len=14592 */
	{ 0x1c61c3e9 /* x^350175 mod G(x) */, 0x8ea1f680 /* x^233439 mod G(x) */, 0x54afca53 /* x^116703 mod G(x) */, },
	/* chunk_len=14720 */
	{ 0xb14cfc2b /* x^353247 mod G(x) */, 0x2e073302 /* x^235487 mod G(x) */, 0x10897992 /* x^117727 mod G(x) */, },
	/* chunk_len=14848 */
	{ 0x6ec444cc /* x^356319 mod G(x) */, 0x9e819f13 /* x^237535 mod G(x) */, 0x7a3c0a6a /* x^118751 mod G(x) */, },
	/* chunk_len=14976 */
	{ 0xe2fa5f80 /* x^359391 mod G(x) */, 0x21f15b59 /* x^239583 mod G(x) */, 0x93102436 /* x^119775 mod G(x) */, },
	/* chunk_len=15104 */
	{ 0x6d33f4c6 /* x^362463 mod G(x) */, 0x31a27455 /* x^241631 mod G(x) */, 0x1fea4d2a /* x^120799 mod G(x) */, },
	/* chunk_len=15232 */
	{ 0xb6dec609 /* x^365535 mod G(x) */, 0x4d437056 /* x^243679 mod G(x) */, 0x42eb1e2a /* x^121823 mod G(x) */, },
	/* chunk_len=15360 */
	{ 0x1846c518 /* x^368607 mod G(x) */, 0x71b74d95 /* x^245727 mod G(x) */, 0xbd2655a8 /* x^122847 mod G(x) */, },
	/* chunk_len=15488 */
	{ 0x9f947f8a /* x^371679 mod G(x) */, 0x2b501619 /* x^247775 mod G(x) */, 0xa4924b0e /* x^123871 mod G(x) */, },
	/* chunk_len=15616 */
	{ 0xb7442f4d /* x^374751 mod G(x) */, 0xba30a5d8 /* x^249823 mod G(x) */, 0x4ff61aa1 /* x^124895 mod G(x) */, },
	/* chunk_len=15744 */
	{ 0xe2c93242 /* x^377823 mod G(x) */, 0x8a2d38e8 /* x^251871 mod G(x) */, 0x70cd7f26 /* x^125919 mod G(x) */, },
	/* chunk_len=15872 */
	{ 0xcd6863df /* x^380895 mod G(x) */, 0x78fd88dc /* x^253919 mod G(x) */, 0x7ae2f6f4 /* x^126943 mod G(x) */, },
	/* chunk_len=16000 */
	{ 0xd512001d /* x^383967 mod G(x) */, 0xe6612dff /* x^255967 mod G(x) */, 0x5c4d0ca9 /* x^127967 mod G(x) */, },
	/* chunk_len=16128 */
	{ 0x4e8d6b6c /* x^387039 mod G(x) */, 0xf997967f /* x^258015 mod G(x) */, 0x2d546c53 /* x^128991 mod G(x) */, },
	/* chunk_len=16256 */
	{ 0xfa653ba1 /* x^390111 mod G(x) */, 0xc99014d4 /* x^260063 mod G(x) */, 0xa0c9fd27 /* x^130015 mod G(x) */, },
	/* chunk_len=16384 */
	{ 0x49893408 /* x^393183 mod G(x) */, 0x29c2448b /* x^262111 mod G(x) */, 0xe0ee5efe /* x^131039 mod G(x) */, },
};

/* Multipliers for implementations that use a large fixed chunk length */
#define CRC32_FIXED_CHUNK_LEN 32768UL
#define CRC32_FIXED_CHUNK_MULT_1 0x29c2448b /* x^262111 mod G(x) */
#define CRC32_FIXED_CHUNK_MULT_2 0x4b912f53 /* x^524255 mod G(x) */
#define CRC32_FIXED_CHUNK_MULT_3 0x454c93be /* x^786399 mod G(x) */

/*** End of inlined file: crc32_multipliers.h ***/


/*** Start of inlined file: crc32_tables.h ***/
static const u32 crc32_slice1_table[] MAYBE_UNUSED = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

static const u32 crc32_slice8_table[] MAYBE_UNUSED = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
	0x00000000, 0x191b3141, 0x32366282, 0x2b2d53c3,
	0x646cc504, 0x7d77f445, 0x565aa786, 0x4f4196c7,
	0xc8d98a08, 0xd1c2bb49, 0xfaefe88a, 0xe3f4d9cb,
	0xacb54f0c, 0xb5ae7e4d, 0x9e832d8e, 0x87981ccf,
	0x4ac21251, 0x53d92310, 0x78f470d3, 0x61ef4192,
	0x2eaed755, 0x37b5e614, 0x1c98b5d7, 0x05838496,
	0x821b9859, 0x9b00a918, 0xb02dfadb, 0xa936cb9a,
	0xe6775d5d, 0xff6c6c1c, 0xd4413fdf, 0xcd5a0e9e,
	0x958424a2, 0x8c9f15e3, 0xa7b24620, 0xbea97761,
	0xf1e8e1a6, 0xe8f3d0e7, 0xc3de8324, 0xdac5b265,
	0x5d5daeaa, 0x44469feb, 0x6f6bcc28, 0x7670fd69,
	0x39316bae, 0x202a5aef, 0x0b07092c, 0x121c386d,
	0xdf4636f3, 0xc65d07b2, 0xed705471, 0xf46b6530,
	0xbb2af3f7, 0xa231c2b6, 0x891c9175, 0x9007a034,
	0x179fbcfb, 0x0e848dba, 0x25a9de79, 0x3cb2ef38,
	0x73f379ff, 0x6ae848be, 0x41c51b7d, 0x58de2a3c,
	0xf0794f05, 0xe9627e44, 0xc24f2d87, 0xdb541cc6,
	0x94158a01, 0x8d0ebb40, 0xa623e883, 0xbf38d9c2,
	0x38a0c50d, 0x21bbf44c, 0x0a96a78f, 0x138d96ce,
	0x5ccc0009, 0x45d73148, 0x6efa628b, 0x77e153ca,
	0xbabb5d54, 0xa3a06c15, 0x888d3fd6, 0x91960e97,
	0xded79850, 0xc7cca911, 0xece1fad2, 0xf5facb93,
	0x7262d75c, 0x6b79e61d, 0x4054b5de, 0x594f849f,
	0x160e1258, 0x0f152319, 0x243870da, 0x3d23419b,
	0x65fd6ba7, 0x7ce65ae6, 0x57cb0925, 0x4ed03864,
	0x0191aea3, 0x188a9fe2, 0x33a7cc21, 0x2abcfd60,
	0xad24e1af, 0xb43fd0ee, 0x9f12832d, 0x8609b26c,
	0xc94824ab, 0xd05315ea, 0xfb7e4629, 0xe2657768,
	0x2f3f79f6, 0x362448b7, 0x1d091b74, 0x04122a35,
	0x4b53bcf2, 0x52488db3, 0x7965de70, 0x607eef31,
	0xe7e6f3fe, 0xfefdc2bf, 0xd5d0917c, 0xcccba03d,
	0x838a36fa, 0x9a9107bb, 0xb1bc5478, 0xa8a76539,
	0x3b83984b, 0x2298a90a, 0x09b5fac9, 0x10aecb88,
	0x5fef5d4f, 0x46f46c0e, 0x6dd93fcd, 0x74c20e8c,
	0xf35a1243, 0xea412302, 0xc16c70c1, 0xd8774180,
	0x9736d747, 0x8e2de606, 0xa500b5c5, 0xbc1b8484,
	0x71418a1a, 0x685abb5b, 0x4377e898, 0x5a6cd9d9,
	0x152d4f1e, 0x0c367e5f, 0x271b2d9c, 0x3e001cdd,
	0xb9980012, 0xa0833153, 0x8bae6290, 0x92b553d1,
	0xddf4c516, 0xc4eff457, 0xefc2a794, 0xf6d996d5,
	0xae07bce9, 0xb71c8da8, 0x9c31de6b, 0x852aef2a,
	0xca6b79ed, 0xd37048ac, 0xf85d1b6f, 0xe1462a2e,
	0x66de36e1, 0x7fc507a0, 0x54e85463, 0x4df36522,
	0x02b2f3e5, 0x1ba9c2a4, 0x30849167, 0x299fa026,
	0xe4c5aeb8, 0xfdde9ff9, 0xd6f3cc3a, 0xcfe8fd7b,
	0x80a96bbc, 0x99b25afd, 0xb29f093e, 0xab84387f,
	0x2c1c24b0, 0x350715f1, 0x1e2a4632, 0x07317773,
	0x4870e1b4, 0x516bd0f5, 0x7a468336, 0x635db277,
	0xcbfad74e, 0xd2e1e60f, 0xf9ccb5cc, 0xe0d7848d,
	0xaf96124a, 0xb68d230b, 0x9da070c8, 0x84bb4189,
	0x03235d46, 0x1a386c07, 0x31153fc4, 0x280e0e85,
	0x674f9842, 0x7e54a903, 0x5579fac0, 0x4c62cb81,
	0x8138c51f, 0x9823f45e, 0xb30ea79d, 0xaa1596dc,
	0xe554001b, 0xfc4f315a, 0xd7626299, 0xce7953d8,
	0x49e14f17, 0x50fa7e56, 0x7bd72d95, 0x62cc1cd4,
	0x2d8d8a13, 0x3496bb52, 0x1fbbe891, 0x06a0d9d0,
	0x5e7ef3ec, 0x4765c2ad, 0x6c48916e, 0x7553a02f,
	0x3a1236e8, 0x230907a9, 0x0824546a, 0x113f652b,
	0x96a779e4, 0x8fbc48a5, 0xa4911b66, 0xbd8a2a27,
	0xf2cbbce0, 0xebd08da1, 0xc0fdde62, 0xd9e6ef23,
	0x14bce1bd, 0x0da7d0fc, 0x268a833f, 0x3f91b27e,
	0x70d024b9, 0x69cb15f8, 0x42e6463b, 0x5bfd777a,
	0xdc656bb5, 0xc57e5af4, 0xee530937, 0xf7483876,
	0xb809aeb1, 0xa1129ff0, 0x8a3fcc33, 0x9324fd72,
	0x00000000, 0x01c26a37, 0x0384d46e, 0x0246be59,
	0x0709a8dc, 0x06cbc2eb, 0x048d7cb2, 0x054f1685,
	0x0e1351b8, 0x0fd13b8f, 0x0d9785d6, 0x0c55efe1,
	0x091af964, 0x08d89353, 0x0a9e2d0a, 0x0b5c473d,
	0x1c26a370, 0x1de4c947, 0x1fa2771e, 0x1e601d29,
	0x1b2f0bac, 0x1aed619b, 0x18abdfc2, 0x1969b5f5,
	0x1235f2c8, 0x13f798ff, 0x11b126a6, 0x10734c91,
	0x153c5a14, 0x14fe3023, 0x16b88e7a, 0x177ae44d,
	0x384d46e0, 0x398f2cd7, 0x3bc9928e, 0x3a0bf8b9,
	0x3f44ee3c, 0x3e86840b, 0x3cc03a52, 0x3d025065,
	0x365e1758, 0x379c7d6f, 0x35dac336, 0x3418a901,
	0x3157bf84, 0x3095d5b3, 0x32d36bea, 0x331101dd,
	0x246be590, 0x25a98fa7, 0x27ef31fe, 0x262d5bc9,
	0x23624d4c, 0x22a0277b, 0x20e69922, 0x2124f315,
	0x2a78b428, 0x2bbade1f, 0x29fc6046, 0x283e0a71,
	0x2d711cf4, 0x2cb376c3, 0x2ef5c89a, 0x2f37a2ad,
	0x709a8dc0, 0x7158e7f7, 0x731e59ae, 0x72dc3399,
	0x7793251c, 0x76514f2b, 0x7417f172, 0x75d59b45,
	0x7e89dc78, 0x7f4bb64f, 0x7d0d0816, 0x7ccf6221,
	0x798074a4, 0x78421e93, 0x7a04a0ca, 0x7bc6cafd,
	0x6cbc2eb0, 0x6d7e4487, 0x6f38fade, 0x6efa90e9,
	0x6bb5866c, 0x6a77ec5b, 0x68315202, 0x69f33835,
	0x62af7f08, 0x636d153f, 0x612bab66, 0x60e9c151,
	0x65a6d7d4, 0x6464bde3, 0x662203ba, 0x67e0698d,
	0x48d7cb20, 0x4915a117, 0x4b531f4e, 0x4a917579,
	0x4fde63fc, 0x4e1c09cb, 0x4c5ab792, 0x4d98dda5,
	0x46c49a98, 0x4706f0af, 0x45404ef6, 0x448224c1,
	0x41cd3244, 0x400f5873, 0x4249e62a, 0x438b8c1d,
	0x54f16850, 0x55330267, 0x5775bc3e, 0x56b7d609,
	0x53f8c08c, 0x523aaabb, 0x507c14e2, 0x51be7ed5,
	0x5ae239e8, 0x5b2053df, 0x5966ed86, 0x58a487b1,
	0x5deb9134, 0x5c29fb03, 0x5e6f455a, 0x5fad2f6d,
	0xe1351b80, 0xe0f771b7, 0xe2b1cfee, 0xe373a5d9,
	0xe63cb35c, 0xe7fed96b, 0xe5b86732, 0xe47a0d05,
	0xef264a38, 0xeee4200f, 0xeca29e56, 0xed60f461,
	0xe82fe2e4, 0xe9ed88d3, 0xebab368a, 0xea695cbd,
	0xfd13b8f0, 0xfcd1d2c7, 0xfe976c9e, 0xff5506a9,
	0xfa1a102c, 0xfbd87a1b, 0xf99ec442, 0xf85cae75,
	0xf300e948, 0xf2c2837f, 0xf0843d26, 0xf1465711,
	0xf4094194, 0xf5cb2ba3, 0xf78d95fa, 0xf64fffcd,
	0xd9785d60, 0xd8ba3757, 0xdafc890e, 0xdb3ee339,
	0xde71f5bc, 0xdfb39f8b, 0xddf521d2, 0xdc374be5,
	0xd76b0cd8, 0xd6a966ef, 0xd4efd8b6, 0xd52db281,
	0xd062a404, 0xd1a0ce33, 0xd3e6706a, 0xd2241a5d,
	0xc55efe10, 0xc49c9427, 0xc6da2a7e, 0xc7184049,
	0xc25756cc, 0xc3953cfb, 0xc1d382a2, 0xc011e895,
	0xcb4dafa8, 0xca8fc59f, 0xc8c97bc6, 0xc90b11f1,
	0xcc440774, 0xcd866d43, 0xcfc0d31a, 0xce02b92d,
	0x91af9640, 0x906dfc77, 0x922b422e, 0x93e92819,
	0x96a63e9c, 0x976454ab, 0x9522eaf2, 0x94e080c5,
	0x9fbcc7f8, 0x9e7eadcf, 0x9c381396, 0x9dfa79a1,
	0x98b56f24, 0x99770513, 0x9b31bb4a, 0x9af3d17d,
	0x8d893530, 0x8c4b5f07, 0x8e0de15e, 0x8fcf8b69,
	0x8a809dec, 0x8b42f7db, 0x89044982, 0x88c623b5,
	0x839a6488, 0x82580ebf, 0x801eb0e6, 0x81dcdad1,
	0x8493cc54, 0x8551a663, 0x8717183a, 0x86d5720d,
	0xa9e2d0a0, 0xa820ba97, 0xaa6604ce, 0xaba46ef9,
	0xaeeb787c, 0xaf29124b, 0xad6fac12, 0xacadc625,
	0xa7f18118, 0xa633eb2f, 0xa4755576, 0xa5b73f41,
	0xa0f829c4, 0xa13a43f3, 0xa37cfdaa, 0xa2be979d,
	0xb5c473d0, 0xb40619e7, 0xb640a7be, 0xb782cd89,
	0xb2cddb0c, 0xb30fb13b, 0xb1490f62, 0xb08b6555,
	0xbbd72268, 0xba15485f, 0xb853f606, 0xb9919c31,
	0xbcde8ab4, 0xbd1ce083, 0xbf5a5eda, 0xbe9834ed,
	0x00000000, 0xb8bc6765, 0xaa09c88b, 0x12b5afee,
	0x8f629757, 0x37def032, 0x256b5fdc, 0x9dd738b9,
	0xc5b428ef, 0x7d084f8a, 0x6fbde064, 0xd7018701,
	0x4ad6bfb8, 0xf26ad8dd, 0xe0df7733, 0x58631056,
	0x5019579f, 0xe8a530fa, 0xfa109f14, 0x42acf871,
	0xdf7bc0c8, 0x67c7a7ad, 0x75720843, 0xcdce6f26,
	0x95ad7f70, 0x2d111815, 0x3fa4b7fb, 0x8718d09e,
	0x1acfe827, 0xa2738f42, 0xb0c620ac, 0x087a47c9,
	0xa032af3e, 0x188ec85b, 0x0a3b67b5, 0xb28700d0,
	0x2f503869, 0x97ec5f0c, 0x8559f0e2, 0x3de59787,
	0x658687d1, 0xdd3ae0b4, 0xcf8f4f5a, 0x7733283f,
	0xeae41086, 0x525877e3, 0x40edd80d, 0xf851bf68,
	0xf02bf8a1, 0x48979fc4, 0x5a22302a, 0xe29e574f,
	0x7f496ff6, 0xc7f50893, 0xd540a77d, 0x6dfcc018,
	0x359fd04e, 0x8d23b72b, 0x9f9618c5, 0x272a7fa0,
	0xbafd4719, 0x0241207c, 0x10f48f92, 0xa848e8f7,
	0x9b14583d, 0x23a83f58, 0x311d90b6, 0x89a1f7d3,
	0x1476cf6a, 0xaccaa80f, 0xbe7f07e1, 0x06c36084,
	0x5ea070d2, 0xe61c17b7, 0xf4a9b859, 0x4c15df3c,
	0xd1c2e785, 0x697e80e0, 0x7bcb2f0e, 0xc377486b,
	0xcb0d0fa2, 0x73b168c7, 0x6104c729, 0xd9b8a04c,
	0x446f98f5, 0xfcd3ff90, 0xee66507e, 0x56da371b,
	0x0eb9274d, 0xb6054028, 0xa4b0efc6, 0x1c0c88a3,
	0x81dbb01a, 0x3967d77f, 0x2bd27891, 0x936e1ff4,
	0x3b26f703, 0x839a9066, 0x912f3f88, 0x299358ed,
	0xb4446054, 0x0cf80731, 0x1e4da8df, 0xa6f1cfba,
	0xfe92dfec, 0x462eb889, 0x549b1767, 0xec277002,
	0x71f048bb, 0xc94c2fde, 0xdbf98030, 0x6345e755,
	0x6b3fa09c, 0xd383c7f9, 0xc1366817, 0x798a0f72,
	0xe45d37cb, 0x5ce150ae, 0x4e54ff40, 0xf6e89825,
	0xae8b8873, 0x1637ef16, 0x048240f8, 0xbc3e279d,
	0x21e91f24, 0x99557841, 0x8be0d7af, 0x335cb0ca,
	0xed59b63b, 0x55e5d15e, 0x47507eb0, 0xffec19d5,
	0x623b216c, 0xda874609, 0xc832e9e7, 0x708e8e82,
	0x28ed9ed4, 0x9051f9b1, 0x82e4565f, 0x3a58313a,
	0xa78f0983, 0x1f336ee6, 0x0d86c108, 0xb53aa66d,
	0xbd40e1a4, 0x05fc86c1, 0x1749292f, 0xaff54e4a,
	0x322276f3, 0x8a9e1196, 0x982bbe78, 0x2097d91d,
	0x78f4c94b, 0xc048ae2e, 0xd2fd01c0, 0x6a4166a5,
	0xf7965e1c, 0x4f2a3979, 0x5d9f9697, 0xe523f1f2,
	0x4d6b1905, 0xf5d77e60, 0xe762d18e, 0x5fdeb6eb,
	0xc2098e52, 0x7ab5e937, 0x680046d9, 0xd0bc21bc,
	0x88df31ea, 0x3063568f, 0x22d6f961, 0x9a6a9e04,
	0x07bda6bd, 0xbf01c1d8, 0xadb46e36, 0x15080953,
	0x1d724e9a, 0xa5ce29ff, 0xb77b8611, 0x0fc7e174,
	0x9210d9cd, 0x2aacbea8, 0x38191146, 0x80a57623,
	0xd8c66675, 0x607a0110, 0x72cfaefe, 0xca73c99b,
	0x57a4f122, 0xef189647, 0xfdad39a9, 0x45115ecc,
	0x764dee06, 0xcef18963, 0xdc44268d, 0x64f841e8,
	0xf92f7951, 0x41931e34, 0x5326b1da, 0xeb9ad6bf,
	0xb3f9c6e9, 0x0b45a18c, 0x19f00e62, 0xa14c6907,
	0x3c9b51be, 0x842736db, 0x96929935, 0x2e2efe50,
	0x2654b999, 0x9ee8defc, 0x8c5d7112, 0x34e11677,
	0xa9362ece, 0x118a49ab, 0x033fe645, 0xbb838120,
	0xe3e09176, 0x5b5cf613, 0x49e959fd, 0xf1553e98,
	0x6c820621, 0xd43e6144, 0xc68bceaa, 0x7e37a9cf,
	0xd67f4138, 0x6ec3265d, 0x7c7689b3, 0xc4caeed6,
	0x591dd66f, 0xe1a1b10a, 0xf3141ee4, 0x4ba87981,
	0x13cb69d7, 0xab770eb2, 0xb9c2a15c, 0x017ec639,
	0x9ca9fe80, 0x241599e5, 0x36a0360b, 0x8e1c516e,
	0x866616a7, 0x3eda71c2, 0x2c6fde2c, 0x94d3b949,
	0x090481f0, 0xb1b8e695, 0xa30d497b, 0x1bb12e1e,
	0x43d23e48, 0xfb6e592d, 0xe9dbf6c3, 0x516791a6,
	0xccb0a91f, 0x740cce7a, 0x66b96194, 0xde0506f1,
	0x00000000, 0x3d6029b0, 0x7ac05360, 0x47a07ad0,
	0xf580a6c0, 0xc8e08f70, 0x8f40f5a0, 0xb220dc10,
	0x30704bc1, 0x0d106271, 0x4ab018a1, 0x77d03111,
	0xc5f0ed01, 0xf890c4b1, 0xbf30be61, 0x825097d1,
	0x60e09782, 0x5d80be32, 0x1a20c4e2, 0x2740ed52,
	0x95603142, 0xa80018f2, 0xefa06222, 0xd2c04b92,
	0x5090dc43, 0x6df0f5f3, 0x2a508f23, 0x1730a693,
	0xa5107a83, 0x98705333, 0xdfd029e3, 0xe2b00053,
	0xc1c12f04, 0xfca106b4, 0xbb017c64, 0x866155d4,
	0x344189c4, 0x0921a074, 0x4e81daa4, 0x73e1f314,
	0xf1b164c5, 0xccd14d75, 0x8b7137a5, 0xb6111e15,
	0x0431c205, 0x3951ebb5, 0x7ef19165, 0x4391b8d5,
	0xa121b886, 0x9c419136, 0xdbe1ebe6, 0xe681c256,
	0x54a11e46, 0x69c137f6, 0x2e614d26, 0x13016496,
	0x9151f347, 0xac31daf7, 0xeb91a027, 0xd6f18997,
	0x64d15587, 0x59b17c37, 0x1e1106e7, 0x23712f57,
	0x58f35849, 0x659371f9, 0x22330b29, 0x1f532299,
	0xad73fe89, 0x9013d739, 0xd7b3ade9, 0xead38459,
	0x68831388, 0x55e33a38, 0x124340e8, 0x2f236958,
	0x9d03b548, 0xa0639cf8, 0xe7c3e628, 0xdaa3cf98,
	0x3813cfcb, 0x0573e67b, 0x42d39cab, 0x7fb3b51b,
	0xcd93690b, 0xf0f340bb, 0xb7533a6b, 0x8a3313db,
	0x0863840a, 0x3503adba, 0x72a3d76a, 0x4fc3feda,
	0xfde322ca, 0xc0830b7a, 0x872371aa, 0xba43581a,
	0x9932774d, 0xa4525efd, 0xe3f2242d, 0xde920d9d,
	0x6cb2d18d, 0x51d2f83d, 0x167282ed, 0x2b12ab5d,
	0xa9423c8c, 0x9422153c, 0xd3826fec, 0xeee2465c,
	0x5cc29a4c, 0x61a2b3fc, 0x2602c92c, 0x1b62e09c,
	0xf9d2e0cf, 0xc4b2c97f, 0x8312b3af, 0xbe729a1f,
	0x0c52460f, 0x31326fbf, 0x7692156f, 0x4bf23cdf,
	0xc9a2ab0e, 0xf4c282be, 0xb362f86e, 0x8e02d1de,
	0x3c220dce, 0x0142247e, 0x46e25eae, 0x7b82771e,
	0xb1e6b092, 0x8c869922, 0xcb26e3f2, 0xf646ca42,
	0x44661652, 0x79063fe2, 0x3ea64532, 0x03c66c82,
	0x8196fb53, 0xbcf6d2e3, 0xfb56a833, 0xc6368183,
	0x74165d93, 0x49767423, 0x0ed60ef3, 0x33b62743,
	0xd1062710, 0xec660ea0, 0xabc67470, 0x96a65dc0,
	0x248681d0, 0x19e6a860, 0x5e46d2b0, 0x6326fb00,
	0xe1766cd1, 0xdc164561, 0x9bb63fb1, 0xa6d61601,
	0x14f6ca11, 0x2996e3a1, 0x6e369971, 0x5356b0c1,
	0x70279f96, 0x4d47b626, 0x0ae7ccf6, 0x3787e546,
	0x85a73956, 0xb8c710e6, 0xff676a36, 0xc2074386,
	0x4057d457, 0x7d37fde7, 0x3a978737, 0x07f7ae87,
	0xb5d77297, 0x88b75b27, 0xcf1721f7, 0xf2770847,
	0x10c70814, 0x2da721a4, 0x6a075b74, 0x576772c4,
	0xe547aed4, 0xd8278764, 0x9f87fdb4, 0xa2e7d404,
	0x20b743d5, 0x1dd76a65, 0x5a7710b5, 0x67173905,
	0xd537e515, 0xe857cca5, 0xaff7b675, 0x92979fc5,
	0xe915e8db, 0xd475c16b, 0x93d5bbbb, 0xaeb5920b,
	0x1c954e1b, 0x21f567ab, 0x66551d7b, 0x5b3534cb,
	0xd965a31a, 0xe4058aaa, 0xa3a5f07a, 0x9ec5d9ca,
	0x2ce505da, 0x11852c6a, 0x562556ba, 0x6b457f0a,
	0x89f57f59, 0xb49556e9, 0xf3352c39, 0xce550589,
	0x7c75d999, 0x4115f029, 0x06b58af9, 0x3bd5a349,
	0xb9853498, 0x84e51d28, 0xc34567f8, 0xfe254e48,
	0x4c059258, 0x7165bbe8, 0x36c5c138, 0x0ba5e888,
	0x28d4c7df, 0x15b4ee6f, 0x521494bf, 0x6f74bd0f,
	0xdd54611f, 0xe03448af, 0xa794327f, 0x9af41bcf,
	0x18a48c1e, 0x25c4a5ae, 0x6264df7e, 0x5f04f6ce,
	0xed242ade, 0xd044036e, 0x97e479be, 0xaa84500e,
	0x4834505d, 0x755479ed, 0x32f4033d, 0x0f942a8d,
	0xbdb4f69d, 0x80d4df2d, 0xc774a5fd, 0xfa148c4d,
	0x78441b9c, 0x4524322c, 0x028448fc, 0x3fe4614c,
	0x8dc4bd5c, 0xb0a494ec, 0xf704ee3c, 0xca64c78c,
	0x00000000, 0xcb5cd3a5, 0x4dc8a10b, 0x869472ae,
	0x9b914216, 0x50cd91b3, 0xd659e31d, 0x1d0530b8,
	0xec53826d, 0x270f51c8, 0xa19b2366, 0x6ac7f0c3,
	0x77c2c07b, 0xbc9e13de, 0x3a0a6170, 0xf156b2d5,
	0x03d6029b, 0xc88ad13e, 0x4e1ea390, 0x85427035,
	0x9847408d, 0x531b9328, 0xd58fe186, 0x1ed33223,
	0xef8580f6, 0x24d95353, 0xa24d21fd, 0x6911f258,
	0x7414c2e0, 0xbf481145, 0x39dc63eb, 0xf280b04e,
	0x07ac0536, 0xccf0d693, 0x4a64a43d, 0x81387798,
	0x9c3d4720, 0x57619485, 0xd1f5e62b, 0x1aa9358e,
	0xebff875b, 0x20a354fe, 0xa6372650, 0x6d6bf5f5,
	0x706ec54d, 0xbb3216e8, 0x3da66446, 0xf6fab7e3,
	0x047a07ad, 0xcf26d408, 0x49b2a6a6, 0x82ee7503,
	0x9feb45bb, 0x54b7961e, 0xd223e4b0, 0x197f3715,
	0xe82985c0, 0x23755665, 0xa5e124cb, 0x6ebdf76e,
	0x73b8c7d6, 0xb8e41473, 0x3e7066dd, 0xf52cb578,
	0x0f580a6c, 0xc404d9c9, 0x4290ab67, 0x89cc78c2,
	0x94c9487a, 0x5f959bdf, 0xd901e971, 0x125d3ad4,
	0xe30b8801, 0x28575ba4, 0xaec3290a, 0x659ffaaf,
	0x789aca17, 0xb3c619b2, 0x35526b1c, 0xfe0eb8b9,
	0x0c8e08f7, 0xc7d2db52, 0x4146a9fc, 0x8a1a7a59,
	0x971f4ae1, 0x5c439944, 0xdad7ebea, 0x118b384f,
	0xe0dd8a9a, 0x2b81593f, 0xad152b91, 0x6649f834,
	0x7b4cc88c, 0xb0101b29, 0x36846987, 0xfdd8ba22,
	0x08f40f5a, 0xc3a8dcff, 0x453cae51, 0x8e607df4,
	0x93654d4c, 0x58399ee9, 0xdeadec47, 0x15f13fe2,
	0xe4a78d37, 0x2ffb5e92, 0xa96f2c3c, 0x6233ff99,
	0x7f36cf21, 0xb46a1c84, 0x32fe6e2a, 0xf9a2bd8f,
	0x0b220dc1, 0xc07ede64, 0x46eaacca, 0x8db67f6f,
	0x90b34fd7, 0x5bef9c72, 0xdd7beedc, 0x16273d79,
	0xe7718fac, 0x2c2d5c09, 0xaab92ea7, 0x61e5fd02,
	0x7ce0cdba, 0xb7bc1e1f, 0x31286cb1, 0xfa74bf14,
	0x1eb014d8, 0xd5ecc77d, 0x5378b5d3, 0x98246676,
	0x852156ce, 0x4e7d856b, 0xc8e9f7c5, 0x03b52460,
	0xf2e396b5, 0x39bf4510, 0xbf2b37be, 0x7477e41b,
	0x6972d4a3, 0xa22e0706, 0x24ba75a8, 0xefe6a60d,
	0x1d661643, 0xd63ac5e6, 0x50aeb748, 0x9bf264ed,
	0x86f75455, 0x4dab87f0, 0xcb3ff55e, 0x006326fb,
	0xf135942e, 0x3a69478b, 0xbcfd3525, 0x77a1e680,
	0x6aa4d638, 0xa1f8059d, 0x276c7733, 0xec30a496,
	0x191c11ee, 0xd240c24b, 0x54d4b0e5, 0x9f886340,
	0x828d53f8, 0x49d1805d, 0xcf45f2f3, 0x04192156,
	0xf54f9383, 0x3e134026, 0xb8873288, 0x73dbe12d,
	0x6eded195, 0xa5820230, 0x2316709e, 0xe84aa33b,
	0x1aca1375, 0xd196c0d0, 0x5702b27e, 0x9c5e61db,
	0x815b5163, 0x4a0782c6, 0xcc93f068, 0x07cf23cd,
	0xf6999118, 0x3dc542bd, 0xbb513013, 0x700de3b6,
	0x6d08d30e, 0xa65400ab, 0x20c07205, 0xeb9ca1a0,
	0x11e81eb4, 0xdab4cd11, 0x5c20bfbf, 0x977c6c1a,
	0x8a795ca2, 0x41258f07, 0xc7b1fda9, 0x0ced2e0c,
	0xfdbb9cd9, 0x36e74f7c, 0xb0733dd2, 0x7b2fee77,
	0x662adecf, 0xad760d6a, 0x2be27fc4, 0xe0beac61,
	0x123e1c2f, 0xd962cf8a, 0x5ff6bd24, 0x94aa6e81,
	0x89af5e39, 0x42f38d9c, 0xc467ff32, 0x0f3b2c97,
	0xfe6d9e42, 0x35314de7, 0xb3a53f49, 0x78f9ecec,
	0x65fcdc54, 0xaea00ff1, 0x28347d5f, 0xe368aefa,
	0x16441b82, 0xdd18c827, 0x5b8cba89, 0x90d0692c,
	0x8dd55994, 0x46898a31, 0xc01df89f, 0x0b412b3a,
	0xfa1799ef, 0x314b4a4a, 0xb7df38e4, 0x7c83eb41,
	0x6186dbf9, 0xaada085c, 0x2c4e7af2, 0xe712a957,
	0x15921919, 0xdececabc, 0x585ab812, 0x93066bb7,
	0x8e035b0f, 0x455f88aa, 0xc3cbfa04, 0x089729a1,
	0xf9c19b74, 0x329d48d1, 0xb4093a7f, 0x7f55e9da,
	0x6250d962, 0xa90c0ac7, 0x2f987869, 0xe4c4abcc,
	0x00000000, 0xa6770bb4, 0x979f1129, 0x31e81a9d,
	0xf44f2413, 0x52382fa7, 0x63d0353a, 0xc5a73e8e,
	0x33ef4e67, 0x959845d3, 0xa4705f4e, 0x020754fa,
	0xc7a06a74, 0x61d761c0, 0x503f7b5d, 0xf64870e9,
	0x67de9cce, 0xc1a9977a, 0xf0418de7, 0x56368653,
	0x9391b8dd, 0x35e6b369, 0x040ea9f4, 0xa279a240,
	0x5431d2a9, 0xf246d91d, 0xc3aec380, 0x65d9c834,
	0xa07ef6ba, 0x0609fd0e, 0x37e1e793, 0x9196ec27,
	0xcfbd399c, 0x69ca3228, 0x582228b5, 0xfe552301,
	0x3bf21d8f, 0x9d85163b, 0xac6d0ca6, 0x0a1a0712,
	0xfc5277fb, 0x5a257c4f, 0x6bcd66d2, 0xcdba6d66,
	0x081d53e8, 0xae6a585c, 0x9f8242c1, 0x39f54975,
	0xa863a552, 0x0e14aee6, 0x3ffcb47b, 0x998bbfcf,
	0x5c2c8141, 0xfa5b8af5, 0xcbb39068, 0x6dc49bdc,
	0x9b8ceb35, 0x3dfbe081, 0x0c13fa1c, 0xaa64f1a8,
	0x6fc3cf26, 0xc9b4c492, 0xf85cde0f, 0x5e2bd5bb,
	0x440b7579, 0xe27c7ecd, 0xd3946450, 0x75e36fe4,
	0xb044516a, 0x16335ade, 0x27db4043, 0x81ac4bf7,
	0x77e43b1e, 0xd19330aa, 0xe07b2a37, 0x460c2183,
	0x83ab1f0d, 0x25dc14b9, 0x14340e24, 0xb2430590,
	0x23d5e9b7, 0x85a2e203, 0xb44af89e, 0x123df32a,
	0xd79acda4, 0x71edc610, 0x4005dc8d, 0xe672d739,
	0x103aa7d0, 0xb64dac64, 0x87a5b6f9, 0x21d2bd4d,
	0xe47583c3, 0x42028877, 0x73ea92ea, 0xd59d995e,
	0x8bb64ce5, 0x2dc14751, 0x1c295dcc, 0xba5e5678,
	0x7ff968f6, 0xd98e6342, 0xe86679df, 0x4e11726b,
	0xb8590282, 0x1e2e0936, 0x2fc613ab, 0x89b1181f,
	0x4c162691, 0xea612d25, 0xdb8937b8, 0x7dfe3c0c,
	0xec68d02b, 0x4a1fdb9f, 0x7bf7c102, 0xdd80cab6,
	0x1827f438, 0xbe50ff8c, 0x8fb8e511, 0x29cfeea5,
	0xdf879e4c, 0x79f095f8, 0x48188f65, 0xee6f84d1,
	0x2bc8ba5f, 0x8dbfb1eb, 0xbc57ab76, 0x1a20a0c2,
	0x8816eaf2, 0x2e61e146, 0x1f89fbdb, 0xb9fef06f,
	0x7c59cee1, 0xda2ec555, 0xebc6dfc8, 0x4db1d47c,
	0xbbf9a495, 0x1d8eaf21, 0x2c66b5bc, 0x8a11be08,
	0x4fb68086, 0xe9c18b32, 0xd82991af, 0x7e5e9a1b,
	0xefc8763c, 0x49bf7d88, 0x78576715, 0xde206ca1,
	0x1b87522f, 0xbdf0599b, 0x8c184306, 0x2a6f48b2,
	0xdc27385b, 0x7a5033ef, 0x4bb82972, 0xedcf22c6,
	0x28681c48, 0x8e1f17fc, 0xbff70d61, 0x198006d5,
	0x47abd36e, 0xe1dcd8da, 0xd034c247, 0x7643c9f3,
	0xb3e4f77d, 0x1593fcc9, 0x247be654, 0x820cede0,
	0x74449d09, 0xd23396bd, 0xe3db8c20, 0x45ac8794,
	0x800bb91a, 0x267cb2ae, 0x1794a833, 0xb1e3a387,
	0x20754fa0, 0x86024414, 0xb7ea5e89, 0x119d553d,
	0xd43a6bb3, 0x724d6007, 0x43a57a9a, 0xe5d2712e,
	0x139a01c7, 0xb5ed0a73, 0x840510ee, 0x22721b5a,
	0xe7d525d4, 0x41a22e60, 0x704a34fd, 0xd63d3f49,
	0xcc1d9f8b, 0x6a6a943f, 0x5b828ea2, 0xfdf58516,
	0x3852bb98, 0x9e25b02c, 0xafcdaab1, 0x09baa105,
	0xfff2d1ec, 0x5985da58, 0x686dc0c5, 0xce1acb71,
	0x0bbdf5ff, 0xadcafe4b, 0x9c22e4d6, 0x3a55ef62,
	0xabc30345, 0x0db408f1, 0x3c5c126c, 0x9a2b19d8,
	0x5f8c2756, 0xf9fb2ce2, 0xc813367f, 0x6e643dcb,
	0x982c4d22, 0x3e5b4696, 0x0fb35c0b, 0xa9c457bf,
	0x6c636931, 0xca146285, 0xfbfc7818, 0x5d8b73ac,
	0x03a0a617, 0xa5d7ada3, 0x943fb73e, 0x3248bc8a,
	0xf7ef8204, 0x519889b0, 0x6070932d, 0xc6079899,
	0x304fe870, 0x9638e3c4, 0xa7d0f959, 0x01a7f2ed,
	0xc400cc63, 0x6277c7d7, 0x539fdd4a, 0xf5e8d6fe,
	0x647e3ad9, 0xc209316d, 0xf3e12bf0, 0x55962044,
	0x90311eca, 0x3646157e, 0x07ae0fe3, 0xa1d90457,
	0x579174be, 0xf1e67f0a, 0xc00e6597, 0x66796e23,
	0xa3de50ad, 0x05a95b19, 0x34414184, 0x92364a30,
	0x00000000, 0xccaa009e, 0x4225077d, 0x8e8f07e3,
	0x844a0efa, 0x48e00e64, 0xc66f0987, 0x0ac50919,
	0xd3e51bb5, 0x1f4f1b2b, 0x91c01cc8, 0x5d6a1c56,
	0x57af154f, 0x9b0515d1, 0x158a1232, 0xd92012ac,
	0x7cbb312b, 0xb01131b5, 0x3e9e3656, 0xf23436c8,
	0xf8f13fd1, 0x345b3f4f, 0xbad438ac, 0x767e3832,
	0xaf5e2a9e, 0x63f42a00, 0xed7b2de3, 0x21d12d7d,
	0x2b142464, 0xe7be24fa, 0x69312319, 0xa59b2387,
	0xf9766256, 0x35dc62c8, 0xbb53652b, 0x77f965b5,
	0x7d3c6cac, 0xb1966c32, 0x3f196bd1, 0xf3b36b4f,
	0x2a9379e3, 0xe639797d, 0x68b67e9e, 0xa41c7e00,
	0xaed97719, 0x62737787, 0xecfc7064, 0x205670fa,
	0x85cd537d, 0x496753e3, 0xc7e85400, 0x0b42549e,
	0x01875d87, 0xcd2d5d19, 0x43a25afa, 0x8f085a64,
	0x562848c8, 0x9a824856, 0x140d4fb5, 0xd8a74f2b,
	0xd2624632, 0x1ec846ac, 0x9047414f, 0x5ced41d1,
	0x299dc2ed, 0xe537c273, 0x6bb8c590, 0xa712c50e,
	0xadd7cc17, 0x617dcc89, 0xeff2cb6a, 0x2358cbf4,
	0xfa78d958, 0x36d2d9c6, 0xb85dde25, 0x74f7debb,
	0x7e32d7a2, 0xb298d73c, 0x3c17d0df, 0xf0bdd041,
	0x5526f3c6, 0x998cf358, 0x1703f4bb, 0xdba9f425,
	0xd16cfd3c, 0x1dc6fda2, 0x9349fa41, 0x5fe3fadf,
	0x86c3e873, 0x4a69e8ed, 0xc4e6ef0e, 0x084cef90,
	0x0289e689, 0xce23e617, 0x40ace1f4, 0x8c06e16a,
	0xd0eba0bb, 0x1c41a025, 0x92cea7c6, 0x5e64a758,
	0x54a1ae41, 0x980baedf, 0x1684a93c, 0xda2ea9a2,
	0x030ebb0e, 0xcfa4bb90, 0x412bbc73, 0x8d81bced,
	0x8744b5f4, 0x4beeb56a, 0xc561b289, 0x09cbb217,
	0xac509190, 0x60fa910e, 0xee7596ed, 0x22df9673,
	0x281a9f6a, 0xe4b09ff4, 0x6a3f9817, 0xa6959889,
	0x7fb58a25, 0xb31f8abb, 0x3d908d58, 0xf13a8dc6,
	0xfbff84df, 0x37558441, 0xb9da83a2, 0x7570833c,
	0x533b85da, 0x9f918544, 0x111e82a7, 0xddb48239,
	0xd7718b20, 0x1bdb8bbe, 0x95548c5d, 0x59fe8cc3,
	0x80de9e6f, 0x4c749ef1, 0xc2fb9912, 0x0e51998c,
	0x04949095, 0xc83e900b, 0x46b197e8, 0x8a1b9776,
	0x2f80b4f1, 0xe32ab46f, 0x6da5b38c, 0xa10fb312,
	0xabcaba0b, 0x6760ba95, 0xe9efbd76, 0x2545bde8,
	0xfc65af44, 0x30cfafda, 0xbe40a839, 0x72eaa8a7,
	0x782fa1be, 0xb485a120, 0x3a0aa6c3, 0xf6a0a65d,
	0xaa4de78c, 0x66e7e712, 0xe868e0f1, 0x24c2e06f,
	0x2e07e976, 0xe2ade9e8, 0x6c22ee0b, 0xa088ee95,
	0x79a8fc39, 0xb502fca7, 0x3b8dfb44, 0xf727fbda,
	0xfde2f2c3, 0x3148f25d, 0xbfc7f5be, 0x736df520,
	0xd6f6d6a7, 0x1a5cd639, 0x94d3d1da, 0x5879d144,
	0x52bcd85d, 0x9e16d8c3, 0x1099df20, 0xdc33dfbe,
	0x0513cd12, 0xc9b9cd8c, 0x4736ca6f, 0x8b9ccaf1,
	0x8159c3e8, 0x4df3c376, 0xc37cc495, 0x0fd6c40b,
	0x7aa64737, 0xb60c47a9, 0x3883404a, 0xf42940d4,
	0xfeec49cd, 0x32464953, 0xbcc94eb0, 0x70634e2e,
	0xa9435c82, 0x65e95c1c, 0xeb665bff, 0x27cc5b61,
	0x2d095278, 0xe1a352e6, 0x6f2c5505, 0xa386559b,
	0x061d761c, 0xcab77682, 0x44387161, 0x889271ff,
	0x825778e6, 0x4efd7878, 0xc0727f9b, 0x0cd87f05,
	0xd5f86da9, 0x19526d37, 0x97dd6ad4, 0x5b776a4a,
	0x51b26353, 0x9d1863cd, 0x1397642e, 0xdf3d64b0,
	0x83d02561, 0x4f7a25ff, 0xc1f5221c, 0x0d5f2282,
	0x079a2b9b, 0xcb302b05, 0x45bf2ce6, 0x89152c78,
	0x50353ed4, 0x9c9f3e4a, 0x121039a9, 0xdeba3937,
	0xd47f302e, 0x18d530b0, 0x965a3753, 0x5af037cd,
	0xff6b144a, 0x33c114d4, 0xbd4e1337, 0x71e413a9,
	0x7b211ab0, 0xb78b1a2e, 0x39041dcd, 0xf5ae1d53,
	0x2c8e0fff, 0xe0240f61, 0x6eab0882, 0xa201081c,
	0xa8c40105, 0x646e019b, 0xeae10678, 0x264b06e6,
};

/*** End of inlined file: crc32_tables.h ***/

/* This is the default implementation.  It uses the slice-by-8 method. */
static u32 MAYBE_UNUSED
crc32_slice8(u32 crc, const u8 *p, size_t len)
{
	const u8 * const end = p + len;
	const u8 *end64;

	for (; ((uintptr_t)p & 7) && p != end; p++)
		crc = (crc >> 8) ^ crc32_slice8_table[(u8)crc ^ *p];

	end64 = p + ((end - p) & ~7);
	for (; p != end64; p += 8) {
		u32 v1 = le32_bswap(*(const u32 *)(p + 0));
		u32 v2 = le32_bswap(*(const u32 *)(p + 4));

		crc = crc32_slice8_table[0x700 + (u8)((crc ^ v1) >> 0)] ^
		      crc32_slice8_table[0x600 + (u8)((crc ^ v1) >> 8)] ^
		      crc32_slice8_table[0x500 + (u8)((crc ^ v1) >> 16)] ^
		      crc32_slice8_table[0x400 + (u8)((crc ^ v1) >> 24)] ^
		      crc32_slice8_table[0x300 + (u8)(v2 >> 0)] ^
		      crc32_slice8_table[0x200 + (u8)(v2 >> 8)] ^
		      crc32_slice8_table[0x100 + (u8)(v2 >> 16)] ^
		      crc32_slice8_table[0x000 + (u8)(v2 >> 24)];
	}

	for (; p != end; p++)
		crc = (crc >> 8) ^ crc32_slice8_table[(u8)crc ^ *p];

	return crc;
}

/*
 * This is a more lightweight generic implementation, which can be used as a
 * subroutine by architecture-specific implementations to process small amounts
 * of unaligned data at the beginning and/or end of the buffer.
 */
static forceinline u32 MAYBE_UNUSED
crc32_slice1(u32 crc, const u8 *p, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		crc = (crc >> 8) ^ crc32_slice1_table[(u8)crc ^ p[i]];
	return crc;
}

/* Include architecture-specific implementation(s) if available. */
#undef DEFAULT_IMPL
#undef arch_select_crc32_func
typedef u32 (*crc32_func_t)(u32 crc, const u8 *p, size_t len);
#if defined(ARCH_ARM32) || defined(ARCH_ARM64)

/*** Start of inlined file: crc32_impl.h ***/
#ifndef LIB_ARM_CRC32_IMPL_H
#define LIB_ARM_CRC32_IMPL_H

/*
 * crc32_arm_crc() - implementation using crc32 instructions (only)
 *
 * In general this implementation is straightforward.  However, naive use of the
 * crc32 instructions is serial: one of the two inputs to each crc32 instruction
 * is the output of the previous one.  To take advantage of CPUs that can
 * execute multiple crc32 instructions in parallel, when possible we interleave
 * the checksumming of several adjacent chunks, then combine their CRCs.
 *
 * However, without pmull, combining CRCs is fairly slow.  So in this pmull-less
 * version, we only use a large chunk length, and thus we only do chunked
 * processing if there is a lot of data to checksum.  This also means that a
 * variable chunk length wouldn't help much, so we just support a fixed length.
 */
#if HAVE_CRC32_INTRIN
#  ifdef __clang__
#    define ATTRIBUTES	_target_attribute("crc")
#  else
#    define ATTRIBUTES	_target_attribute("+crc")
#  endif

/*
 * Combine the CRCs for 4 adjacent chunks of length L = CRC32_FIXED_CHUNK_LEN
 * bytes each by computing:
 *
 *	[ crc0*x^(3*8*L) + crc1*x^(2*8*L) + crc2*x^(1*8*L) + crc3 ] mod G(x)
 *
 * This has been optimized in several ways:
 *
 *    - The needed multipliers (x to some power, reduced mod G(x)) were
 *	precomputed.
 *
 *    - The 3 multiplications are interleaved.
 *
 *    - The reduction mod G(x) is delayed to the end and done using __crc32d.
 *	Note that the use of __crc32d introduces an extra factor of x^32.  To
 *	cancel that out along with the extra factor of x^1 that gets introduced
 *	because of how the 63-bit products are aligned in their 64-bit integers,
 *	the multipliers are actually x^(j*8*L - 33) instead of x^(j*8*L).
 */
static forceinline ATTRIBUTES u32
combine_crcs_slow(u32 crc0, u32 crc1, u32 crc2, u32 crc3)
{
	u64 res0 = 0, res1 = 0, res2 = 0;
	int i;

	/* Multiply crc{0,1,2} by CRC32_FIXED_CHUNK_MULT_{3,2,1}. */
	for (i = 0; i < 32; i++) {
		if (CRC32_FIXED_CHUNK_MULT_3 & (1U << i))
			res0 ^= (u64)crc0 << i;
		if (CRC32_FIXED_CHUNK_MULT_2 & (1U << i))
			res1 ^= (u64)crc1 << i;
		if (CRC32_FIXED_CHUNK_MULT_1 & (1U << i))
			res2 ^= (u64)crc2 << i;
	}
	/* Add the different parts and reduce mod G(x). */
	return __crc32d(0, res0 ^ res1 ^ res2) ^ crc3;
}

#define crc32_arm_crc	crc32_arm_crc
static ATTRIBUTES u32
crc32_arm_crc(u32 crc, const u8 *p, size_t len)
{
	if (len >= 64) {
		const size_t align = -(uintptr_t)p & 7;

		/* Align p to the next 8-byte boundary. */
		if (align) {
			if (align & 1)
				crc = __crc32b(crc, *p++);
			if (align & 2) {
				crc = __crc32h(crc, le16_bswap(*(u16 *)p));
				p += 2;
			}
			if (align & 4) {
				crc = __crc32w(crc, le32_bswap(*(u32 *)p));
				p += 4;
			}
			len -= align;
		}
		/*
		 * Interleave the processing of multiple adjacent data chunks to
		 * take advantage of instruction-level parallelism.
		 *
		 * Some CPUs don't prefetch the data if it's being fetched in
		 * multiple interleaved streams, so do explicit prefetching.
		 */
		while (len >= CRC32_NUM_CHUNKS * CRC32_FIXED_CHUNK_LEN) {
			const u64 *wp0 = (const u64 *)p;
			const u64 * const wp0_end =
				(const u64 *)(p + CRC32_FIXED_CHUNK_LEN);
			u32 crc1 = 0, crc2 = 0, crc3 = 0;

			STATIC_ASSERT(CRC32_NUM_CHUNKS == 4);
			STATIC_ASSERT(CRC32_FIXED_CHUNK_LEN % (4 * 8) == 0);
			do {
				prefetchr(&wp0[64 + 0*CRC32_FIXED_CHUNK_LEN/8]);
				prefetchr(&wp0[64 + 1*CRC32_FIXED_CHUNK_LEN/8]);
				prefetchr(&wp0[64 + 2*CRC32_FIXED_CHUNK_LEN/8]);
				prefetchr(&wp0[64 + 3*CRC32_FIXED_CHUNK_LEN/8]);
				crc  = __crc32d(crc,  le64_bswap(wp0[0*CRC32_FIXED_CHUNK_LEN/8]));
				crc1 = __crc32d(crc1, le64_bswap(wp0[1*CRC32_FIXED_CHUNK_LEN/8]));
				crc2 = __crc32d(crc2, le64_bswap(wp0[2*CRC32_FIXED_CHUNK_LEN/8]));
				crc3 = __crc32d(crc3, le64_bswap(wp0[3*CRC32_FIXED_CHUNK_LEN/8]));
				wp0++;
				crc  = __crc32d(crc,  le64_bswap(wp0[0*CRC32_FIXED_CHUNK_LEN/8]));
				crc1 = __crc32d(crc1, le64_bswap(wp0[1*CRC32_FIXED_CHUNK_LEN/8]));
				crc2 = __crc32d(crc2, le64_bswap(wp0[2*CRC32_FIXED_CHUNK_LEN/8]));
				crc3 = __crc32d(crc3, le64_bswap(wp0[3*CRC32_FIXED_CHUNK_LEN/8]));
				wp0++;
				crc  = __crc32d(crc,  le64_bswap(wp0[0*CRC32_FIXED_CHUNK_LEN/8]));
				crc1 = __crc32d(crc1, le64_bswap(wp0[1*CRC32_FIXED_CHUNK_LEN/8]));
				crc2 = __crc32d(crc2, le64_bswap(wp0[2*CRC32_FIXED_CHUNK_LEN/8]));
				crc3 = __crc32d(crc3, le64_bswap(wp0[3*CRC32_FIXED_CHUNK_LEN/8]));
				wp0++;
				crc  = __crc32d(crc,  le64_bswap(wp0[0*CRC32_FIXED_CHUNK_LEN/8]));
				crc1 = __crc32d(crc1, le64_bswap(wp0[1*CRC32_FIXED_CHUNK_LEN/8]));
				crc2 = __crc32d(crc2, le64_bswap(wp0[2*CRC32_FIXED_CHUNK_LEN/8]));
				crc3 = __crc32d(crc3, le64_bswap(wp0[3*CRC32_FIXED_CHUNK_LEN/8]));
				wp0++;
			} while (wp0 != wp0_end);
			crc = combine_crcs_slow(crc, crc1, crc2, crc3);
			p += CRC32_NUM_CHUNKS * CRC32_FIXED_CHUNK_LEN;
			len -= CRC32_NUM_CHUNKS * CRC32_FIXED_CHUNK_LEN;
		}
		/*
		 * Due to the large fixed chunk length used above, there might
		 * still be a lot of data left.  So use a 64-byte loop here,
		 * instead of a loop that is less unrolled.
		 */
		while (len >= 64) {
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 0)));
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 8)));
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 16)));
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 24)));
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 32)));
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 40)));
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 48)));
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 56)));
			p += 64;
			len -= 64;
		}
	}
	if (len & 32) {
		crc = __crc32d(crc, get_unaligned_le64(p + 0));
		crc = __crc32d(crc, get_unaligned_le64(p + 8));
		crc = __crc32d(crc, get_unaligned_le64(p + 16));
		crc = __crc32d(crc, get_unaligned_le64(p + 24));
		p += 32;
	}
	if (len & 16) {
		crc = __crc32d(crc, get_unaligned_le64(p + 0));
		crc = __crc32d(crc, get_unaligned_le64(p + 8));
		p += 16;
	}
	if (len & 8) {
		crc = __crc32d(crc, get_unaligned_le64(p));
		p += 8;
	}
	if (len & 4) {
		crc = __crc32w(crc, get_unaligned_le32(p));
		p += 4;
	}
	if (len & 2) {
		crc = __crc32h(crc, get_unaligned_le16(p));
		p += 2;
	}
	if (len & 1)
		crc = __crc32b(crc, *p);
	return crc;
}
#undef ATTRIBUTES
#endif /* crc32_arm_crc() */

/*
 * crc32_arm_crc_pmullcombine() - implementation using crc32 instructions, plus
 *	pmull instructions for CRC combining
 *
 * This is similar to crc32_arm_crc(), but it enables the use of pmull
 * (carryless multiplication) instructions for the steps where the CRCs of
 * adjacent data chunks are combined.  As this greatly speeds up CRC
 * combination, this implementation also differs from crc32_arm_crc() in that it
 * uses a variable chunk length which can get fairly small.  The precomputed
 * multipliers needed for the selected chunk length are loaded from a table.
 *
 * Note that pmull is used here only for combining the CRCs of separately
 * checksummed chunks, not for folding the data itself.  See crc32_arm_pmull*()
 * for implementations that use pmull for folding the data itself.
 */
#if HAVE_CRC32_INTRIN && HAVE_PMULL_INTRIN
#  ifdef __clang__
#    define ATTRIBUTES	_target_attribute("crc,aes")
#  else
#    define ATTRIBUTES	_target_attribute("+crc,+crypto")
#  endif

/* Do carryless multiplication of two 32-bit values. */
static forceinline ATTRIBUTES u64
clmul_u32(u32 a, u32 b)
{
	uint64x2_t res = vreinterpretq_u64_p128(
				compat_vmull_p64((poly64_t)a, (poly64_t)b));

	return vgetq_lane_u64(res, 0);
}

/*
 * Like combine_crcs_slow(), but uses vmull_p64 to do the multiplications more
 * quickly, and supports a variable chunk length.  The chunk length is
 * 'i * CRC32_MIN_VARIABLE_CHUNK_LEN'
 * where 1 <= i < ARRAY_LEN(crc32_mults_for_chunklen).
 */
static forceinline ATTRIBUTES u32
combine_crcs_fast(u32 crc0, u32 crc1, u32 crc2, u32 crc3, size_t i)
{
	u64 res0 = clmul_u32(crc0, crc32_mults_for_chunklen[i][0]);
	u64 res1 = clmul_u32(crc1, crc32_mults_for_chunklen[i][1]);
	u64 res2 = clmul_u32(crc2, crc32_mults_for_chunklen[i][2]);

	return __crc32d(0, res0 ^ res1 ^ res2) ^ crc3;
}

#define crc32_arm_crc_pmullcombine	crc32_arm_crc_pmullcombine
static ATTRIBUTES u32
crc32_arm_crc_pmullcombine(u32 crc, const u8 *p, size_t len)
{
	const size_t align = -(uintptr_t)p & 7;

	if (len >= align + CRC32_NUM_CHUNKS * CRC32_MIN_VARIABLE_CHUNK_LEN) {
		/* Align p to the next 8-byte boundary. */
		if (align) {
			if (align & 1)
				crc = __crc32b(crc, *p++);
			if (align & 2) {
				crc = __crc32h(crc, le16_bswap(*(u16 *)p));
				p += 2;
			}
			if (align & 4) {
				crc = __crc32w(crc, le32_bswap(*(u32 *)p));
				p += 4;
			}
			len -= align;
		}
		/*
		 * Handle CRC32_MAX_VARIABLE_CHUNK_LEN specially, so that better
		 * code is generated for it.
		 */
		while (len >= CRC32_NUM_CHUNKS * CRC32_MAX_VARIABLE_CHUNK_LEN) {
			const u64 *wp0 = (const u64 *)p;
			const u64 * const wp0_end =
				(const u64 *)(p + CRC32_MAX_VARIABLE_CHUNK_LEN);
			u32 crc1 = 0, crc2 = 0, crc3 = 0;

			STATIC_ASSERT(CRC32_NUM_CHUNKS == 4);
			STATIC_ASSERT(CRC32_MAX_VARIABLE_CHUNK_LEN % (4 * 8) == 0);
			do {
				prefetchr(&wp0[64 + 0*CRC32_MAX_VARIABLE_CHUNK_LEN/8]);
				prefetchr(&wp0[64 + 1*CRC32_MAX_VARIABLE_CHUNK_LEN/8]);
				prefetchr(&wp0[64 + 2*CRC32_MAX_VARIABLE_CHUNK_LEN/8]);
				prefetchr(&wp0[64 + 3*CRC32_MAX_VARIABLE_CHUNK_LEN/8]);
				crc  = __crc32d(crc,  le64_bswap(wp0[0*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc1 = __crc32d(crc1, le64_bswap(wp0[1*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc2 = __crc32d(crc2, le64_bswap(wp0[2*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc3 = __crc32d(crc3, le64_bswap(wp0[3*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				wp0++;
				crc  = __crc32d(crc,  le64_bswap(wp0[0*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc1 = __crc32d(crc1, le64_bswap(wp0[1*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc2 = __crc32d(crc2, le64_bswap(wp0[2*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc3 = __crc32d(crc3, le64_bswap(wp0[3*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				wp0++;
				crc  = __crc32d(crc,  le64_bswap(wp0[0*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc1 = __crc32d(crc1, le64_bswap(wp0[1*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc2 = __crc32d(crc2, le64_bswap(wp0[2*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc3 = __crc32d(crc3, le64_bswap(wp0[3*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				wp0++;
				crc  = __crc32d(crc,  le64_bswap(wp0[0*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc1 = __crc32d(crc1, le64_bswap(wp0[1*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc2 = __crc32d(crc2, le64_bswap(wp0[2*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				crc3 = __crc32d(crc3, le64_bswap(wp0[3*CRC32_MAX_VARIABLE_CHUNK_LEN/8]));
				wp0++;
			} while (wp0 != wp0_end);
			crc = combine_crcs_fast(crc, crc1, crc2, crc3,
						ARRAY_LEN(crc32_mults_for_chunklen) - 1);
			p += CRC32_NUM_CHUNKS * CRC32_MAX_VARIABLE_CHUNK_LEN;
			len -= CRC32_NUM_CHUNKS * CRC32_MAX_VARIABLE_CHUNK_LEN;
		}
		/* Handle up to one variable-length chunk. */
		if (len >= CRC32_NUM_CHUNKS * CRC32_MIN_VARIABLE_CHUNK_LEN) {
			const size_t i = len / (CRC32_NUM_CHUNKS *
						CRC32_MIN_VARIABLE_CHUNK_LEN);
			const size_t chunk_len =
				i * CRC32_MIN_VARIABLE_CHUNK_LEN;
			const u64 *wp0 = (const u64 *)(p + 0*chunk_len);
			const u64 *wp1 = (const u64 *)(p + 1*chunk_len);
			const u64 *wp2 = (const u64 *)(p + 2*chunk_len);
			const u64 *wp3 = (const u64 *)(p + 3*chunk_len);
			const u64 * const wp0_end = wp1;
			u32 crc1 = 0, crc2 = 0, crc3 = 0;

			STATIC_ASSERT(CRC32_NUM_CHUNKS == 4);
			STATIC_ASSERT(CRC32_MIN_VARIABLE_CHUNK_LEN % (4 * 8) == 0);
			do {
				prefetchr(wp0 + 64);
				prefetchr(wp1 + 64);
				prefetchr(wp2 + 64);
				prefetchr(wp3 + 64);
				crc  = __crc32d(crc,  le64_bswap(*wp0++));
				crc1 = __crc32d(crc1, le64_bswap(*wp1++));
				crc2 = __crc32d(crc2, le64_bswap(*wp2++));
				crc3 = __crc32d(crc3, le64_bswap(*wp3++));
				crc  = __crc32d(crc,  le64_bswap(*wp0++));
				crc1 = __crc32d(crc1, le64_bswap(*wp1++));
				crc2 = __crc32d(crc2, le64_bswap(*wp2++));
				crc3 = __crc32d(crc3, le64_bswap(*wp3++));
				crc  = __crc32d(crc,  le64_bswap(*wp0++));
				crc1 = __crc32d(crc1, le64_bswap(*wp1++));
				crc2 = __crc32d(crc2, le64_bswap(*wp2++));
				crc3 = __crc32d(crc3, le64_bswap(*wp3++));
				crc  = __crc32d(crc,  le64_bswap(*wp0++));
				crc1 = __crc32d(crc1, le64_bswap(*wp1++));
				crc2 = __crc32d(crc2, le64_bswap(*wp2++));
				crc3 = __crc32d(crc3, le64_bswap(*wp3++));
			} while (wp0 != wp0_end);
			crc = combine_crcs_fast(crc, crc1, crc2, crc3, i);
			p += CRC32_NUM_CHUNKS * chunk_len;
			len -= CRC32_NUM_CHUNKS * chunk_len;
		}

		while (len >= 32) {
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 0)));
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 8)));
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 16)));
			crc = __crc32d(crc, le64_bswap(*(u64 *)(p + 24)));
			p += 32;
			len -= 32;
		}
	} else {
		while (len >= 32) {
			crc = __crc32d(crc, get_unaligned_le64(p + 0));
			crc = __crc32d(crc, get_unaligned_le64(p + 8));
			crc = __crc32d(crc, get_unaligned_le64(p + 16));
			crc = __crc32d(crc, get_unaligned_le64(p + 24));
			p += 32;
			len -= 32;
		}
	}
	if (len & 16) {
		crc = __crc32d(crc, get_unaligned_le64(p + 0));
		crc = __crc32d(crc, get_unaligned_le64(p + 8));
		p += 16;
	}
	if (len & 8) {
		crc = __crc32d(crc, get_unaligned_le64(p));
		p += 8;
	}
	if (len & 4) {
		crc = __crc32w(crc, get_unaligned_le32(p));
		p += 4;
	}
	if (len & 2) {
		crc = __crc32h(crc, get_unaligned_le16(p));
		p += 2;
	}
	if (len & 1)
		crc = __crc32b(crc, *p);
	return crc;
}
#undef ATTRIBUTES
#endif /* crc32_arm_crc_pmullcombine() */

/*
 * crc32_arm_pmullx4() - implementation using "folding" with pmull instructions
 *
 * This implementation is intended for CPUs that support pmull instructions but
 * not crc32 instructions.
 */
#if HAVE_PMULL_INTRIN
#  define crc32_arm_pmullx4	crc32_arm_pmullx4
#  define SUFFIX			 _pmullx4
#  ifdef __clang__
     /*
      * This used to use "crypto", but that stopped working with clang 16.
      * Now only "aes" works.  "aes" works with older versions too, so use
      * that.  No "+" prefix; clang 15 and earlier doesn't accept that.
      */
#    define ATTRIBUTES	_target_attribute("aes")
#  else
     /*
      * With gcc, only "+crypto" works.  Both the "+" prefix and the
      * "crypto" (not "aes") are essential...
      */
#    define ATTRIBUTES	_target_attribute("+crypto")
#  endif
#  define ENABLE_EOR3		0

/*** Start of inlined file: crc32_pmull_helpers.h ***/
/*
 * This file is a "template" for instantiating helper functions for CRC folding
 * with pmull instructions.  It accepts the following parameters:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.
 * ENABLE_EOR3:
 *	Use the eor3 instruction (from the sha3 extension).
 */

/* Create a vector with 'a' in the first 4 bytes, and the rest zeroed out. */
#undef u32_to_bytevec
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(u32_to_bytevec)(u32 a)
{
	return vreinterpretq_u8_u32(vsetq_lane_u32(a, vdupq_n_u32(0), 0));
}
#define u32_to_bytevec	ADD_SUFFIX(u32_to_bytevec)

/* Load two 64-bit values into a vector. */
#undef load_multipliers
static forceinline ATTRIBUTES poly64x2_t
ADD_SUFFIX(load_multipliers)(const u64 p[2])
{
	return vreinterpretq_p64_u64(vld1q_u64(p));
}
#define load_multipliers	ADD_SUFFIX(load_multipliers)

/* Do carryless multiplication of the low halves of two vectors. */
#undef clmul_low
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(clmul_low)(uint8x16_t a, poly64x2_t b)
{
	return vreinterpretq_u8_p128(
		     compat_vmull_p64(vgetq_lane_p64(vreinterpretq_p64_u8(a), 0),
				      vgetq_lane_p64(b, 0)));
}
#define clmul_low	ADD_SUFFIX(clmul_low)

/* Do carryless multiplication of the high halves of two vectors. */
#undef clmul_high
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(clmul_high)(uint8x16_t a, poly64x2_t b)
{
#ifdef __clang__
	/*
	 * Use inline asm to ensure that pmull2 is really used.  This works
	 * around clang bug https://github.com/llvm/llvm-project/issues/52868.
	 */
	uint8x16_t res;

	__asm__("pmull2 %0.1q, %1.2d, %2.2d" : "=w" (res) : "w" (a), "w" (b));
	return res;
#else
	return vreinterpretq_u8_p128(vmull_high_p64(vreinterpretq_p64_u8(a), b));
#endif
}
#define clmul_high	ADD_SUFFIX(clmul_high)

#undef eor3
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(eor3)(uint8x16_t a, uint8x16_t b, uint8x16_t c)
{
#if ENABLE_EOR3
	return veor3q_u8(a, b, c);
#else
	return veorq_u8(veorq_u8(a, b), c);
#endif
}
#define eor3	ADD_SUFFIX(eor3)

#undef fold_vec
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(fold_vec)(uint8x16_t src, uint8x16_t dst, poly64x2_t multipliers)
{
	uint8x16_t a = clmul_low(src, multipliers);
	uint8x16_t b = clmul_high(src, multipliers);

	return eor3(a, b, dst);
}
#define fold_vec	ADD_SUFFIX(fold_vec)

/*
 * Given v containing a 16-byte polynomial, and a pointer 'p' that points to the
 * next '1 <= len <= 15' data bytes, rearrange the concatenation of v and the
 * data into vectors x0 and x1 that contain 'len' bytes and 16 bytes,
 * respectively.  Then fold x0 into x1 and return the result.  Assumes that
 * 'p + len - 16' is in-bounds.
 */
#undef fold_partial_vec
static forceinline ATTRIBUTES MAYBE_UNUSED uint8x16_t
ADD_SUFFIX(fold_partial_vec)(uint8x16_t v, const u8 *p, size_t len,
			     poly64x2_t multipliers_1)
{
	/*
	 * vqtbl1q_u8(v, shift_tab[len..len+15]) left shifts v by 16-len bytes.
	 * vqtbl1q_u8(v, shift_tab[len+16..len+31]) right shifts v by len bytes.
	 */
	static const u8 shift_tab[48] = {
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	};
	const uint8x16_t lshift = vld1q_u8(&shift_tab[len]);
	const uint8x16_t rshift = vld1q_u8(&shift_tab[len + 16]);
	uint8x16_t x0, x1, bsl_mask;

	/* x0 = v left-shifted by '16 - len' bytes */
	x0 = vqtbl1q_u8(v, lshift);

	/* Create a vector of '16 - len' 0x00 bytes, then 'len' 0xff bytes. */
	bsl_mask = vreinterpretq_u8_s8(
			vshrq_n_s8(vreinterpretq_s8_u8(rshift), 7));

	/*
	 * x1 = the last '16 - len' bytes from v (i.e. v right-shifted by 'len'
	 * bytes) followed by the remaining data.
	 */
	x1 = vbslq_u8(bsl_mask /* 0 bits select from arg3, 1 bits from arg2 */,
		      vld1q_u8(p + len - 16), vqtbl1q_u8(v, rshift));

	return fold_vec(x0, x1, multipliers_1);
}
#define fold_partial_vec	ADD_SUFFIX(fold_partial_vec)

/*** End of inlined file: crc32_pmull_helpers.h ***/


static ATTRIBUTES u32
crc32_arm_pmullx4(u32 crc, const u8 *p, size_t len)
{
	static const u64 _aligned_attribute(16) mults[3][2] = {
		{ CRC32_X159_MODG, CRC32_X95_MODG },  /* 1 vecs */
		{ CRC32_X543_MODG, CRC32_X479_MODG }, /* 4 vecs */
		{ CRC32_X287_MODG, CRC32_X223_MODG }, /* 2 vecs */
	};
	static const u64 _aligned_attribute(16) barrett_consts[3][2] = {
		{ CRC32_X95_MODG, },
		{ CRC32_BARRETT_CONSTANT_1, },
		{ CRC32_BARRETT_CONSTANT_2, },
	};
	const poly64x2_t multipliers_1 = load_multipliers(mults[0]);
	uint8x16_t v0, v1, v2, v3;

	if (len < 64 + 15) {
		if (len < 16)
			return crc32_slice1(crc, p, len);
		v0 = veorq_u8(vld1q_u8(p), u32_to_bytevec(crc));
		p += 16;
		len -= 16;
		while (len >= 16) {
			v0 = fold_vec(v0, vld1q_u8(p), multipliers_1);
			p += 16;
			len -= 16;
		}
	} else {
		const poly64x2_t multipliers_4 = load_multipliers(mults[1]);
		const poly64x2_t multipliers_2 = load_multipliers(mults[2]);
		const size_t align = -(uintptr_t)p & 15;
		const uint8x16_t *vp;

		v0 = veorq_u8(vld1q_u8(p), u32_to_bytevec(crc));
		p += 16;
		/* Align p to the next 16-byte boundary. */
		if (align) {
			v0 = fold_partial_vec(v0, p, align, multipliers_1);
			p += align;
			len -= align;
		}
		vp = (const uint8x16_t *)p;
		v1 = *vp++;
		v2 = *vp++;
		v3 = *vp++;
		while (len >= 64 + 64) {
			v0 = fold_vec(v0, *vp++, multipliers_4);
			v1 = fold_vec(v1, *vp++, multipliers_4);
			v2 = fold_vec(v2, *vp++, multipliers_4);
			v3 = fold_vec(v3, *vp++, multipliers_4);
			len -= 64;
		}
		v0 = fold_vec(v0, v2, multipliers_2);
		v1 = fold_vec(v1, v3, multipliers_2);
		if (len & 32) {
			v0 = fold_vec(v0, *vp++, multipliers_2);
			v1 = fold_vec(v1, *vp++, multipliers_2);
		}
		v0 = fold_vec(v0, v1, multipliers_1);
		if (len & 16)
			v0 = fold_vec(v0, *vp++, multipliers_1);
		p = (const u8 *)vp;
		len &= 15;
	}

	/* Handle any remaining partial block now before reducing to 32 bits. */
	if (len)
		v0 = fold_partial_vec(v0, p, len, multipliers_1);

	/* Reduce to 32 bits, following lib/x86/crc32_pclmul_template.h */
	v0 = veorq_u8(clmul_low(v0, load_multipliers(barrett_consts[0])),
		      vextq_u8(v0, vdupq_n_u8(0), 8));
	v1 = clmul_low(v0, load_multipliers(barrett_consts[1]));
	v1 = clmul_low(v1, load_multipliers(barrett_consts[2]));
	v0 = veorq_u8(v0, v1);
	return vgetq_lane_u32(vreinterpretq_u32_u8(v0), 2);
}
#undef SUFFIX
#undef ATTRIBUTES
#undef ENABLE_EOR3
#endif /* crc32_arm_pmullx4() */

/*
 * crc32_arm_pmullx12_crc() - large-stride implementation using "folding" with
 *	pmull instructions, where crc32 instructions are also available
 *
 * See crc32_pmull_wide.h for explanation.
 */
#if HAVE_PMULL_INTRIN && HAVE_CRC32_INTRIN
#  define crc32_arm_pmullx12_crc	crc32_arm_pmullx12_crc
#  define SUFFIX				 _pmullx12_crc
#  ifdef __clang__
#    define ATTRIBUTES	_target_attribute("aes,crc")
#  else
#    define ATTRIBUTES	_target_attribute("+crypto,+crc")
#  endif
#  define ENABLE_EOR3	0

/*** Start of inlined file: crc32_pmull_wide.h ***/
/*
 * This file is a "template" for instantiating PMULL-based crc32_arm functions.
 * The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.
 * ENABLE_EOR3:
 *	Use the eor3 instruction (from the sha3 extension).
 *
 * This is the extra-wide version; it uses an unusually large stride length of
 * 12, and it assumes that crc32 instructions are available too.  It's intended
 * for powerful CPUs that support both pmull and crc32 instructions, but where
 * throughput of pmull and xor (given enough instructions issued in parallel) is
 * significantly higher than that of crc32, thus making the crc32 instructions
 * (counterintuitively) not actually the fastest way to compute the CRC-32.  The
 * Apple M1 processor is an example of such a CPU.
 */


/*** Start of inlined file: crc32_pmull_helpers.h ***/
/*
 * This file is a "template" for instantiating helper functions for CRC folding
 * with pmull instructions.  It accepts the following parameters:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.
 * ENABLE_EOR3:
 *	Use the eor3 instruction (from the sha3 extension).
 */

/* Create a vector with 'a' in the first 4 bytes, and the rest zeroed out. */
#undef u32_to_bytevec
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(u32_to_bytevec)(u32 a)
{
	return vreinterpretq_u8_u32(vsetq_lane_u32(a, vdupq_n_u32(0), 0));
}
#define u32_to_bytevec	ADD_SUFFIX(u32_to_bytevec)

/* Load two 64-bit values into a vector. */
#undef load_multipliers
static forceinline ATTRIBUTES poly64x2_t
ADD_SUFFIX(load_multipliers)(const u64 p[2])
{
	return vreinterpretq_p64_u64(vld1q_u64(p));
}
#define load_multipliers	ADD_SUFFIX(load_multipliers)

/* Do carryless multiplication of the low halves of two vectors. */
#undef clmul_low
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(clmul_low)(uint8x16_t a, poly64x2_t b)
{
	return vreinterpretq_u8_p128(
		     compat_vmull_p64(vgetq_lane_p64(vreinterpretq_p64_u8(a), 0),
				      vgetq_lane_p64(b, 0)));
}
#define clmul_low	ADD_SUFFIX(clmul_low)

/* Do carryless multiplication of the high halves of two vectors. */
#undef clmul_high
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(clmul_high)(uint8x16_t a, poly64x2_t b)
{
#ifdef __clang__
	/*
	 * Use inline asm to ensure that pmull2 is really used.  This works
	 * around clang bug https://github.com/llvm/llvm-project/issues/52868.
	 */
	uint8x16_t res;

	__asm__("pmull2 %0.1q, %1.2d, %2.2d" : "=w" (res) : "w" (a), "w" (b));
	return res;
#else
	return vreinterpretq_u8_p128(vmull_high_p64(vreinterpretq_p64_u8(a), b));
#endif
}
#define clmul_high	ADD_SUFFIX(clmul_high)

#undef eor3
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(eor3)(uint8x16_t a, uint8x16_t b, uint8x16_t c)
{
#if ENABLE_EOR3
	return veor3q_u8(a, b, c);
#else
	return veorq_u8(veorq_u8(a, b), c);
#endif
}
#define eor3	ADD_SUFFIX(eor3)

#undef fold_vec
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(fold_vec)(uint8x16_t src, uint8x16_t dst, poly64x2_t multipliers)
{
	uint8x16_t a = clmul_low(src, multipliers);
	uint8x16_t b = clmul_high(src, multipliers);

	return eor3(a, b, dst);
}
#define fold_vec	ADD_SUFFIX(fold_vec)

/*
 * Given v containing a 16-byte polynomial, and a pointer 'p' that points to the
 * next '1 <= len <= 15' data bytes, rearrange the concatenation of v and the
 * data into vectors x0 and x1 that contain 'len' bytes and 16 bytes,
 * respectively.  Then fold x0 into x1 and return the result.  Assumes that
 * 'p + len - 16' is in-bounds.
 */
#undef fold_partial_vec
static forceinline ATTRIBUTES MAYBE_UNUSED uint8x16_t
ADD_SUFFIX(fold_partial_vec)(uint8x16_t v, const u8 *p, size_t len,
			     poly64x2_t multipliers_1)
{
	/*
	 * vqtbl1q_u8(v, shift_tab[len..len+15]) left shifts v by 16-len bytes.
	 * vqtbl1q_u8(v, shift_tab[len+16..len+31]) right shifts v by len bytes.
	 */
	static const u8 shift_tab[48] = {
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	};
	const uint8x16_t lshift = vld1q_u8(&shift_tab[len]);
	const uint8x16_t rshift = vld1q_u8(&shift_tab[len + 16]);
	uint8x16_t x0, x1, bsl_mask;

	/* x0 = v left-shifted by '16 - len' bytes */
	x0 = vqtbl1q_u8(v, lshift);

	/* Create a vector of '16 - len' 0x00 bytes, then 'len' 0xff bytes. */
	bsl_mask = vreinterpretq_u8_s8(
			vshrq_n_s8(vreinterpretq_s8_u8(rshift), 7));

	/*
	 * x1 = the last '16 - len' bytes from v (i.e. v right-shifted by 'len'
	 * bytes) followed by the remaining data.
	 */
	x1 = vbslq_u8(bsl_mask /* 0 bits select from arg3, 1 bits from arg2 */,
		      vld1q_u8(p + len - 16), vqtbl1q_u8(v, rshift));

	return fold_vec(x0, x1, multipliers_1);
}
#define fold_partial_vec	ADD_SUFFIX(fold_partial_vec)

/*** End of inlined file: crc32_pmull_helpers.h ***/

static ATTRIBUTES u32
ADD_SUFFIX(crc32_arm)(u32 crc, const u8 *p, size_t len)
{
	uint8x16_t v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11;

	if (len < 3 * 192) {
		static const u64 _aligned_attribute(16) mults[3][2] = {
			{ CRC32_X543_MODG, CRC32_X479_MODG }, /* 4 vecs */
			{ CRC32_X287_MODG, CRC32_X223_MODG }, /* 2 vecs */
			{ CRC32_X159_MODG, CRC32_X95_MODG },  /* 1 vecs */
		};
		poly64x2_t multipliers_4, multipliers_2, multipliers_1;

		if (len < 64)
			goto tail;
		multipliers_4 = load_multipliers(mults[0]);
		multipliers_2 = load_multipliers(mults[1]);
		multipliers_1 = load_multipliers(mults[2]);
		/*
		 * Short length; don't bother aligning the pointer, and fold
		 * 64 bytes (4 vectors) at a time, at most.
		 */
		v0 = veorq_u8(vld1q_u8(p + 0), u32_to_bytevec(crc));
		v1 = vld1q_u8(p + 16);
		v2 = vld1q_u8(p + 32);
		v3 = vld1q_u8(p + 48);
		p += 64;
		len -= 64;
		while (len >= 64) {
			v0 = fold_vec(v0, vld1q_u8(p + 0), multipliers_4);
			v1 = fold_vec(v1, vld1q_u8(p + 16), multipliers_4);
			v2 = fold_vec(v2, vld1q_u8(p + 32), multipliers_4);
			v3 = fold_vec(v3, vld1q_u8(p + 48), multipliers_4);
			p += 64;
			len -= 64;
		}
		v0 = fold_vec(v0, v2, multipliers_2);
		v1 = fold_vec(v1, v3, multipliers_2);
		if (len >= 32) {
			v0 = fold_vec(v0, vld1q_u8(p + 0), multipliers_2);
			v1 = fold_vec(v1, vld1q_u8(p + 16), multipliers_2);
			p += 32;
			len -= 32;
		}
		v0 = fold_vec(v0, v1, multipliers_1);
	} else {
		static const u64 _aligned_attribute(16) mults[4][2] = {
			{ CRC32_X1567_MODG, CRC32_X1503_MODG }, /* 12 vecs */
			{ CRC32_X799_MODG, CRC32_X735_MODG },   /* 6 vecs */
			{ CRC32_X415_MODG, CRC32_X351_MODG },   /* 3 vecs */
			{ CRC32_X159_MODG, CRC32_X95_MODG },    /* 1 vecs */
		};
		const poly64x2_t multipliers_12 = load_multipliers(mults[0]);
		const poly64x2_t multipliers_6 = load_multipliers(mults[1]);
		const poly64x2_t multipliers_3 = load_multipliers(mults[2]);
		const poly64x2_t multipliers_1 = load_multipliers(mults[3]);
		const size_t align = -(uintptr_t)p & 15;
		const uint8x16_t *vp;

		/* Align p to the next 16-byte boundary. */
		if (align) {
			if (align & 1)
				crc = __crc32b(crc, *p++);
			if (align & 2) {
				crc = __crc32h(crc, le16_bswap(*(u16 *)p));
				p += 2;
			}
			if (align & 4) {
				crc = __crc32w(crc, le32_bswap(*(u32 *)p));
				p += 4;
			}
			if (align & 8) {
				crc = __crc32d(crc, le64_bswap(*(u64 *)p));
				p += 8;
			}
			len -= align;
		}
		vp = (const uint8x16_t *)p;
		v0 = veorq_u8(*vp++, u32_to_bytevec(crc));
		v1 = *vp++;
		v2 = *vp++;
		v3 = *vp++;
		v4 = *vp++;
		v5 = *vp++;
		v6 = *vp++;
		v7 = *vp++;
		v8 = *vp++;
		v9 = *vp++;
		v10 = *vp++;
		v11 = *vp++;
		len -= 192;
		/* Fold 192 bytes (12 vectors) at a time. */
		do {
			v0 = fold_vec(v0, *vp++, multipliers_12);
			v1 = fold_vec(v1, *vp++, multipliers_12);
			v2 = fold_vec(v2, *vp++, multipliers_12);
			v3 = fold_vec(v3, *vp++, multipliers_12);
			v4 = fold_vec(v4, *vp++, multipliers_12);
			v5 = fold_vec(v5, *vp++, multipliers_12);
			v6 = fold_vec(v6, *vp++, multipliers_12);
			v7 = fold_vec(v7, *vp++, multipliers_12);
			v8 = fold_vec(v8, *vp++, multipliers_12);
			v9 = fold_vec(v9, *vp++, multipliers_12);
			v10 = fold_vec(v10, *vp++, multipliers_12);
			v11 = fold_vec(v11, *vp++, multipliers_12);
			len -= 192;
		} while (len >= 192);

		/*
		 * Fewer than 192 bytes left.  Fold v0-v11 down to just v0,
		 * while processing up to 144 more bytes.
		 */
		v0 = fold_vec(v0, v6, multipliers_6);
		v1 = fold_vec(v1, v7, multipliers_6);
		v2 = fold_vec(v2, v8, multipliers_6);
		v3 = fold_vec(v3, v9, multipliers_6);
		v4 = fold_vec(v4, v10, multipliers_6);
		v5 = fold_vec(v5, v11, multipliers_6);
		if (len >= 96) {
			v0 = fold_vec(v0, *vp++, multipliers_6);
			v1 = fold_vec(v1, *vp++, multipliers_6);
			v2 = fold_vec(v2, *vp++, multipliers_6);
			v3 = fold_vec(v3, *vp++, multipliers_6);
			v4 = fold_vec(v4, *vp++, multipliers_6);
			v5 = fold_vec(v5, *vp++, multipliers_6);
			len -= 96;
		}
		v0 = fold_vec(v0, v3, multipliers_3);
		v1 = fold_vec(v1, v4, multipliers_3);
		v2 = fold_vec(v2, v5, multipliers_3);
		if (len >= 48) {
			v0 = fold_vec(v0, *vp++, multipliers_3);
			v1 = fold_vec(v1, *vp++, multipliers_3);
			v2 = fold_vec(v2, *vp++, multipliers_3);
			len -= 48;
		}
		v0 = fold_vec(v0, v1, multipliers_1);
		v0 = fold_vec(v0, v2, multipliers_1);
		p = (const u8 *)vp;
	}
	/* Reduce 128 to 32 bits using crc32 instructions. */
	crc = __crc32d(0, vgetq_lane_u64(vreinterpretq_u64_u8(v0), 0));
	crc = __crc32d(crc, vgetq_lane_u64(vreinterpretq_u64_u8(v0), 1));
tail:
	/* Finish up the remainder using crc32 instructions. */
	if (len & 32) {
		crc = __crc32d(crc, get_unaligned_le64(p + 0));
		crc = __crc32d(crc, get_unaligned_le64(p + 8));
		crc = __crc32d(crc, get_unaligned_le64(p + 16));
		crc = __crc32d(crc, get_unaligned_le64(p + 24));
		p += 32;
	}
	if (len & 16) {
		crc = __crc32d(crc, get_unaligned_le64(p + 0));
		crc = __crc32d(crc, get_unaligned_le64(p + 8));
		p += 16;
	}
	if (len & 8) {
		crc = __crc32d(crc, get_unaligned_le64(p));
		p += 8;
	}
	if (len & 4) {
		crc = __crc32w(crc, get_unaligned_le32(p));
		p += 4;
	}
	if (len & 2) {
		crc = __crc32h(crc, get_unaligned_le16(p));
		p += 2;
	}
	if (len & 1)
		crc = __crc32b(crc, *p);
	return crc;
}

#undef SUFFIX
#undef ATTRIBUTES
#undef ENABLE_EOR3

/*** End of inlined file: crc32_pmull_wide.h ***/


#endif

/*
 * crc32_arm_pmullx12_crc_eor3()
 *
 * This like crc32_arm_pmullx12_crc(), but it adds the eor3 instruction (from
 * the sha3 extension) for even better performance.
 */
#if HAVE_PMULL_INTRIN && HAVE_CRC32_INTRIN && HAVE_SHA3_INTRIN && \
	!defined(LIBDEFLATE_ASSEMBLER_DOES_NOT_SUPPORT_SHA3)
#  define crc32_arm_pmullx12_crc_eor3	crc32_arm_pmullx12_crc_eor3
#  define SUFFIX				 _pmullx12_crc_eor3
#  ifdef __clang__
#    define ATTRIBUTES	_target_attribute("aes,crc,sha3")
   /*
    * Both gcc and binutils originally considered sha3 to depend on
    * arch=armv8.2-a or later.  This was fixed in gcc 13.2 by commit
    * 9aac37ab8a7b ("aarch64: Remove architecture dependencies from intrinsics")
    * and in binutils 2.41 by commit 205e4380c800 ("aarch64: Remove version
    * dependencies from features").  Unfortunately, always using arch=armv8.2-a
    * causes build errors with some compiler options because it may reduce the
    * arch rather than increase it.  Therefore we try to omit the arch whenever
    * possible.  If gcc is 14 or later, then both gcc and binutils are probably
    * fixed, so we omit the arch.  We also omit the arch if a feature that
    * depends on armv8.2-a or later (in gcc 13.1 and earlier) is present.
    */
#  elif GCC_PREREQ(14, 0) || defined(__ARM_FEATURE_JCVT) \
			  || defined(__ARM_FEATURE_DOTPROD)
#    define ATTRIBUTES	_target_attribute("+crypto,+crc,+sha3")
#  else
#    define ATTRIBUTES	_target_attribute("arch=armv8.2-a+crypto+crc+sha3")
#  endif
#  define ENABLE_EOR3	1

/*** Start of inlined file: crc32_pmull_wide.h ***/
/*
 * This file is a "template" for instantiating PMULL-based crc32_arm functions.
 * The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.
 * ENABLE_EOR3:
 *	Use the eor3 instruction (from the sha3 extension).
 *
 * This is the extra-wide version; it uses an unusually large stride length of
 * 12, and it assumes that crc32 instructions are available too.  It's intended
 * for powerful CPUs that support both pmull and crc32 instructions, but where
 * throughput of pmull and xor (given enough instructions issued in parallel) is
 * significantly higher than that of crc32, thus making the crc32 instructions
 * (counterintuitively) not actually the fastest way to compute the CRC-32.  The
 * Apple M1 processor is an example of such a CPU.
 */


/*** Start of inlined file: crc32_pmull_helpers.h ***/
/*
 * This file is a "template" for instantiating helper functions for CRC folding
 * with pmull instructions.  It accepts the following parameters:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.
 * ENABLE_EOR3:
 *	Use the eor3 instruction (from the sha3 extension).
 */

/* Create a vector with 'a' in the first 4 bytes, and the rest zeroed out. */
#undef u32_to_bytevec
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(u32_to_bytevec)(u32 a)
{
	return vreinterpretq_u8_u32(vsetq_lane_u32(a, vdupq_n_u32(0), 0));
}
#define u32_to_bytevec	ADD_SUFFIX(u32_to_bytevec)

/* Load two 64-bit values into a vector. */
#undef load_multipliers
static forceinline ATTRIBUTES poly64x2_t
ADD_SUFFIX(load_multipliers)(const u64 p[2])
{
	return vreinterpretq_p64_u64(vld1q_u64(p));
}
#define load_multipliers	ADD_SUFFIX(load_multipliers)

/* Do carryless multiplication of the low halves of two vectors. */
#undef clmul_low
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(clmul_low)(uint8x16_t a, poly64x2_t b)
{
	return vreinterpretq_u8_p128(
		     compat_vmull_p64(vgetq_lane_p64(vreinterpretq_p64_u8(a), 0),
				      vgetq_lane_p64(b, 0)));
}
#define clmul_low	ADD_SUFFIX(clmul_low)

/* Do carryless multiplication of the high halves of two vectors. */
#undef clmul_high
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(clmul_high)(uint8x16_t a, poly64x2_t b)
{
#ifdef __clang__
	/*
	 * Use inline asm to ensure that pmull2 is really used.  This works
	 * around clang bug https://github.com/llvm/llvm-project/issues/52868.
	 */
	uint8x16_t res;

	__asm__("pmull2 %0.1q, %1.2d, %2.2d" : "=w" (res) : "w" (a), "w" (b));
	return res;
#else
	return vreinterpretq_u8_p128(vmull_high_p64(vreinterpretq_p64_u8(a), b));
#endif
}
#define clmul_high	ADD_SUFFIX(clmul_high)

#undef eor3
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(eor3)(uint8x16_t a, uint8x16_t b, uint8x16_t c)
{
#if ENABLE_EOR3
	return veor3q_u8(a, b, c);
#else
	return veorq_u8(veorq_u8(a, b), c);
#endif
}
#define eor3	ADD_SUFFIX(eor3)

#undef fold_vec
static forceinline ATTRIBUTES uint8x16_t
ADD_SUFFIX(fold_vec)(uint8x16_t src, uint8x16_t dst, poly64x2_t multipliers)
{
	uint8x16_t a = clmul_low(src, multipliers);
	uint8x16_t b = clmul_high(src, multipliers);

	return eor3(a, b, dst);
}
#define fold_vec	ADD_SUFFIX(fold_vec)

/*
 * Given v containing a 16-byte polynomial, and a pointer 'p' that points to the
 * next '1 <= len <= 15' data bytes, rearrange the concatenation of v and the
 * data into vectors x0 and x1 that contain 'len' bytes and 16 bytes,
 * respectively.  Then fold x0 into x1 and return the result.  Assumes that
 * 'p + len - 16' is in-bounds.
 */
#undef fold_partial_vec
static forceinline ATTRIBUTES MAYBE_UNUSED uint8x16_t
ADD_SUFFIX(fold_partial_vec)(uint8x16_t v, const u8 *p, size_t len,
			     poly64x2_t multipliers_1)
{
	/*
	 * vqtbl1q_u8(v, shift_tab[len..len+15]) left shifts v by 16-len bytes.
	 * vqtbl1q_u8(v, shift_tab[len+16..len+31]) right shifts v by len bytes.
	 */
	static const u8 shift_tab[48] = {
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	};
	const uint8x16_t lshift = vld1q_u8(&shift_tab[len]);
	const uint8x16_t rshift = vld1q_u8(&shift_tab[len + 16]);
	uint8x16_t x0, x1, bsl_mask;

	/* x0 = v left-shifted by '16 - len' bytes */
	x0 = vqtbl1q_u8(v, lshift);

	/* Create a vector of '16 - len' 0x00 bytes, then 'len' 0xff bytes. */
	bsl_mask = vreinterpretq_u8_s8(
			vshrq_n_s8(vreinterpretq_s8_u8(rshift), 7));

	/*
	 * x1 = the last '16 - len' bytes from v (i.e. v right-shifted by 'len'
	 * bytes) followed by the remaining data.
	 */
	x1 = vbslq_u8(bsl_mask /* 0 bits select from arg3, 1 bits from arg2 */,
		      vld1q_u8(p + len - 16), vqtbl1q_u8(v, rshift));

	return fold_vec(x0, x1, multipliers_1);
}
#define fold_partial_vec	ADD_SUFFIX(fold_partial_vec)

/*** End of inlined file: crc32_pmull_helpers.h ***/

static ATTRIBUTES u32
ADD_SUFFIX(crc32_arm)(u32 crc, const u8 *p, size_t len)
{
	uint8x16_t v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11;

	if (len < 3 * 192) {
		static const u64 _aligned_attribute(16) mults[3][2] = {
			{ CRC32_X543_MODG, CRC32_X479_MODG }, /* 4 vecs */
			{ CRC32_X287_MODG, CRC32_X223_MODG }, /* 2 vecs */
			{ CRC32_X159_MODG, CRC32_X95_MODG },  /* 1 vecs */
		};
		poly64x2_t multipliers_4, multipliers_2, multipliers_1;

		if (len < 64)
			goto tail;
		multipliers_4 = load_multipliers(mults[0]);
		multipliers_2 = load_multipliers(mults[1]);
		multipliers_1 = load_multipliers(mults[2]);
		/*
		 * Short length; don't bother aligning the pointer, and fold
		 * 64 bytes (4 vectors) at a time, at most.
		 */
		v0 = veorq_u8(vld1q_u8(p + 0), u32_to_bytevec(crc));
		v1 = vld1q_u8(p + 16);
		v2 = vld1q_u8(p + 32);
		v3 = vld1q_u8(p + 48);
		p += 64;
		len -= 64;
		while (len >= 64) {
			v0 = fold_vec(v0, vld1q_u8(p + 0), multipliers_4);
			v1 = fold_vec(v1, vld1q_u8(p + 16), multipliers_4);
			v2 = fold_vec(v2, vld1q_u8(p + 32), multipliers_4);
			v3 = fold_vec(v3, vld1q_u8(p + 48), multipliers_4);
			p += 64;
			len -= 64;
		}
		v0 = fold_vec(v0, v2, multipliers_2);
		v1 = fold_vec(v1, v3, multipliers_2);
		if (len >= 32) {
			v0 = fold_vec(v0, vld1q_u8(p + 0), multipliers_2);
			v1 = fold_vec(v1, vld1q_u8(p + 16), multipliers_2);
			p += 32;
			len -= 32;
		}
		v0 = fold_vec(v0, v1, multipliers_1);
	} else {
		static const u64 _aligned_attribute(16) mults[4][2] = {
			{ CRC32_X1567_MODG, CRC32_X1503_MODG }, /* 12 vecs */
			{ CRC32_X799_MODG, CRC32_X735_MODG },   /* 6 vecs */
			{ CRC32_X415_MODG, CRC32_X351_MODG },   /* 3 vecs */
			{ CRC32_X159_MODG, CRC32_X95_MODG },    /* 1 vecs */
		};
		const poly64x2_t multipliers_12 = load_multipliers(mults[0]);
		const poly64x2_t multipliers_6 = load_multipliers(mults[1]);
		const poly64x2_t multipliers_3 = load_multipliers(mults[2]);
		const poly64x2_t multipliers_1 = load_multipliers(mults[3]);
		const size_t align = -(uintptr_t)p & 15;
		const uint8x16_t *vp;

		/* Align p to the next 16-byte boundary. */
		if (align) {
			if (align & 1)
				crc = __crc32b(crc, *p++);
			if (align & 2) {
				crc = __crc32h(crc, le16_bswap(*(u16 *)p));
				p += 2;
			}
			if (align & 4) {
				crc = __crc32w(crc, le32_bswap(*(u32 *)p));
				p += 4;
			}
			if (align & 8) {
				crc = __crc32d(crc, le64_bswap(*(u64 *)p));
				p += 8;
			}
			len -= align;
		}
		vp = (const uint8x16_t *)p;
		v0 = veorq_u8(*vp++, u32_to_bytevec(crc));
		v1 = *vp++;
		v2 = *vp++;
		v3 = *vp++;
		v4 = *vp++;
		v5 = *vp++;
		v6 = *vp++;
		v7 = *vp++;
		v8 = *vp++;
		v9 = *vp++;
		v10 = *vp++;
		v11 = *vp++;
		len -= 192;
		/* Fold 192 bytes (12 vectors) at a time. */
		do {
			v0 = fold_vec(v0, *vp++, multipliers_12);
			v1 = fold_vec(v1, *vp++, multipliers_12);
			v2 = fold_vec(v2, *vp++, multipliers_12);
			v3 = fold_vec(v3, *vp++, multipliers_12);
			v4 = fold_vec(v4, *vp++, multipliers_12);
			v5 = fold_vec(v5, *vp++, multipliers_12);
			v6 = fold_vec(v6, *vp++, multipliers_12);
			v7 = fold_vec(v7, *vp++, multipliers_12);
			v8 = fold_vec(v8, *vp++, multipliers_12);
			v9 = fold_vec(v9, *vp++, multipliers_12);
			v10 = fold_vec(v10, *vp++, multipliers_12);
			v11 = fold_vec(v11, *vp++, multipliers_12);
			len -= 192;
		} while (len >= 192);

		/*
		 * Fewer than 192 bytes left.  Fold v0-v11 down to just v0,
		 * while processing up to 144 more bytes.
		 */
		v0 = fold_vec(v0, v6, multipliers_6);
		v1 = fold_vec(v1, v7, multipliers_6);
		v2 = fold_vec(v2, v8, multipliers_6);
		v3 = fold_vec(v3, v9, multipliers_6);
		v4 = fold_vec(v4, v10, multipliers_6);
		v5 = fold_vec(v5, v11, multipliers_6);
		if (len >= 96) {
			v0 = fold_vec(v0, *vp++, multipliers_6);
			v1 = fold_vec(v1, *vp++, multipliers_6);
			v2 = fold_vec(v2, *vp++, multipliers_6);
			v3 = fold_vec(v3, *vp++, multipliers_6);
			v4 = fold_vec(v4, *vp++, multipliers_6);
			v5 = fold_vec(v5, *vp++, multipliers_6);
			len -= 96;
		}
		v0 = fold_vec(v0, v3, multipliers_3);
		v1 = fold_vec(v1, v4, multipliers_3);
		v2 = fold_vec(v2, v5, multipliers_3);
		if (len >= 48) {
			v0 = fold_vec(v0, *vp++, multipliers_3);
			v1 = fold_vec(v1, *vp++, multipliers_3);
			v2 = fold_vec(v2, *vp++, multipliers_3);
			len -= 48;
		}
		v0 = fold_vec(v0, v1, multipliers_1);
		v0 = fold_vec(v0, v2, multipliers_1);
		p = (const u8 *)vp;
	}
	/* Reduce 128 to 32 bits using crc32 instructions. */
	crc = __crc32d(0, vgetq_lane_u64(vreinterpretq_u64_u8(v0), 0));
	crc = __crc32d(crc, vgetq_lane_u64(vreinterpretq_u64_u8(v0), 1));
tail:
	/* Finish up the remainder using crc32 instructions. */
	if (len & 32) {
		crc = __crc32d(crc, get_unaligned_le64(p + 0));
		crc = __crc32d(crc, get_unaligned_le64(p + 8));
		crc = __crc32d(crc, get_unaligned_le64(p + 16));
		crc = __crc32d(crc, get_unaligned_le64(p + 24));
		p += 32;
	}
	if (len & 16) {
		crc = __crc32d(crc, get_unaligned_le64(p + 0));
		crc = __crc32d(crc, get_unaligned_le64(p + 8));
		p += 16;
	}
	if (len & 8) {
		crc = __crc32d(crc, get_unaligned_le64(p));
		p += 8;
	}
	if (len & 4) {
		crc = __crc32w(crc, get_unaligned_le32(p));
		p += 4;
	}
	if (len & 2) {
		crc = __crc32h(crc, get_unaligned_le16(p));
		p += 2;
	}
	if (len & 1)
		crc = __crc32b(crc, *p);
	return crc;
}

#undef SUFFIX
#undef ATTRIBUTES
#undef ENABLE_EOR3

/*** End of inlined file: crc32_pmull_wide.h ***/


#endif

static inline crc32_func_t
arch_select_crc32_func(void)
{
	const u32 features MAYBE_UNUSED = get_arm_cpu_features();

#ifdef crc32_arm_pmullx12_crc_eor3
	if ((features & ARM_CPU_FEATURE_PREFER_PMULL) &&
	    HAVE_PMULL(features) && HAVE_CRC32(features) && HAVE_SHA3(features))
		return crc32_arm_pmullx12_crc_eor3;
#endif
#ifdef crc32_arm_pmullx12_crc
	if ((features & ARM_CPU_FEATURE_PREFER_PMULL) &&
	    HAVE_PMULL(features) && HAVE_CRC32(features))
		return crc32_arm_pmullx12_crc;
#endif
#ifdef crc32_arm_crc_pmullcombine
	if (HAVE_CRC32(features) && HAVE_PMULL(features))
		return crc32_arm_crc_pmullcombine;
#endif
#ifdef crc32_arm_crc
	if (HAVE_CRC32(features))
		return crc32_arm_crc;
#endif
#ifdef crc32_arm_pmullx4
	if (HAVE_PMULL(features))
		return crc32_arm_pmullx4;
#endif
	return NULL;
}
#define arch_select_crc32_func	arch_select_crc32_func

#endif /* LIB_ARM_CRC32_IMPL_H */

/*** End of inlined file: crc32_impl.h ***/


#elif defined(ARCH_X86_32) || defined(ARCH_X86_64)

/*** Start of inlined file: crc32_impl.h ***/
#ifndef LIB_X86_CRC32_IMPL_H
#define LIB_X86_CRC32_IMPL_H

/*
 * pshufb(x, shift_tab[len..len+15]) left shifts x by 16-len bytes.
 * pshufb(x, shift_tab[len+16..len+31]) right shifts x by len bytes.
 */
static const u8 MAYBE_UNUSED shift_tab[48] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
/*
 * PCLMULQDQ implementation.  This targets PCLMULQDQ+SSE4.1, since in practice
 * all CPUs that support PCLMULQDQ also support SSE4.1.
 */
#  define crc32_x86_pclmulqdq	crc32_x86_pclmulqdq
#  define SUFFIX			 _pclmulqdq
#  define ATTRIBUTES		_target_attribute("pclmul,sse4.1")
#  define VL			16
#  define USE_AVX512		0

/*** Start of inlined file: crc32_pclmul_template.h ***/
/*
 * This file is a "template" for instantiating PCLMULQDQ-based crc32_x86
 * functions.  The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_AVX512=0: at least pclmul,sse4.1
 *	   VL=32 && USE_AVX512=0: at least vpclmulqdq,pclmul,avx2
 *	   VL=32 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   VL=64 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking and the
 *	vpternlog instruction.  This doesn't enable the use of 512-bit vectors;
 *	the vector length is controlled by VL.  If 0, assume that the CPU might
 *	not support AVX-512.
 *
 * The overall algorithm used is CRC folding with carryless multiplication
 * instructions.  Note that the x86 crc32 instruction cannot be used, as it is
 * for a different polynomial, not the gzip one.  For an explanation of CRC
 * folding with carryless multiplication instructions, see
 * scripts/gen-crc32-consts.py and the following blog posts and papers:
 *
 *	"An alternative exposition of crc32_4k_pclmulqdq"
 *	https://www.corsix.org/content/alternative-exposition-crc32_4k_pclmulqdq
 *
 *	"Fast CRC Computation for Generic Polynomials Using PCLMULQDQ Instruction"
 *	https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/fast-crc-computation-generic-polynomials-pclmulqdq-paper.pdf
 *
 * The original pclmulqdq instruction does one 64x64 to 128-bit carryless
 * multiplication.  The VPCLMULQDQ feature added instructions that do two
 * parallel 64x64 to 128-bit carryless multiplications in combination with AVX
 * or AVX512VL, or four in combination with AVX512F.
 */

#if VL == 16
#  define vec_t			__m128i
#  define fold_vec		fold_vec128
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VXOR(a, b)		_mm_xor_si128((a), (b))
#  define M128I_TO_VEC(a)	a
#  define MULTS_8V		_mm_set_epi64x(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_4V		_mm_set_epi64x(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_2V		_mm_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG)
#  define MULTS_1V		_mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG)
#elif VL == 32
#  define vec_t			__m256i
#  define fold_vec		fold_vec256
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VXOR(a, b)		_mm256_xor_si256((a), (b))
#  define M128I_TO_VEC(a)	_mm256_zextsi128_si256(a)
#  define MULTS(a, b)		_mm256_set_epi64x(a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_4V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_2V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_1V		MULTS(CRC32_X223_MODG, CRC32_X287_MODG)
#elif VL == 64
#  define vec_t			__m512i
#  define fold_vec		fold_vec512
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VXOR(a, b)		_mm512_xor_si512((a), (b))
#  define M128I_TO_VEC(a)	_mm512_zextsi128_si512(a)
#  define MULTS(a, b)		_mm512_set_epi64(a, b, a, b, a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X4063_MODG, CRC32_X4127_MODG)
#  define MULTS_4V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_2V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_1V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#else
#  error "unsupported vector length"
#endif

#undef fold_vec128
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_vec128)(__m128i src, __m128i dst, __m128i /* __v2du */ mults)
{
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x00));
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x11));
	return dst;
}
#define fold_vec128	ADD_SUFFIX(fold_vec128)

#if VL >= 32
#undef fold_vec256
static forceinline ATTRIBUTES __m256i
ADD_SUFFIX(fold_vec256)(__m256i src, __m256i dst, __m256i /* __v4du */ mults)
{
#if USE_AVX512
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm256_ternarylogic_epi32(
			_mm256_clmulepi64_epi128(src, mults, 0x00),
			_mm256_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
#else
	return _mm256_xor_si256(
			_mm256_xor_si256(dst,
					 _mm256_clmulepi64_epi128(src, mults, 0x00)),
			_mm256_clmulepi64_epi128(src, mults, 0x11));
#endif
}
#define fold_vec256	ADD_SUFFIX(fold_vec256)
#endif /* VL >= 32 */

#if VL >= 64
#undef fold_vec512
static forceinline ATTRIBUTES __m512i
ADD_SUFFIX(fold_vec512)(__m512i src, __m512i dst, __m512i /* __v8du */ mults)
{
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm512_ternarylogic_epi32(
			_mm512_clmulepi64_epi128(src, mults, 0x00),
			_mm512_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
}
#define fold_vec512	ADD_SUFFIX(fold_vec512)
#endif /* VL >= 64 */

/*
 * Given 'x' containing a 16-byte polynomial, and a pointer 'p' that points to
 * the next '1 <= len <= 15' data bytes, rearrange the concatenation of 'x' and
 * the data into vectors x0 and x1 that contain 'len' bytes and 16 bytes,
 * respectively.  Then fold x0 into x1 and return the result.
 * Assumes that 'p + len - 16' is in-bounds.
 */
#undef fold_lessthan16bytes
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_lessthan16bytes)(__m128i x, const u8 *p, size_t len,
				 __m128i /* __v2du */ mults_128b)
{
	__m128i lshift = _mm_loadu_si128((const void *)&shift_tab[len]);
	__m128i rshift = _mm_loadu_si128((const void *)&shift_tab[len + 16]);
	__m128i x0, x1;

	/* x0 = x left-shifted by '16 - len' bytes */
	x0 = _mm_shuffle_epi8(x, lshift);

	/*
	 * x1 = the last '16 - len' bytes from x (i.e. x right-shifted by 'len'
	 * bytes) followed by the remaining data.
	 */
	x1 = _mm_blendv_epi8(_mm_shuffle_epi8(x, rshift),
			     _mm_loadu_si128((const void *)(p + len - 16)),
			     /* msb 0/1 of each byte selects byte from arg1/2 */
			     rshift);

	return fold_vec128(x0, x1, mults_128b);
}
#define fold_lessthan16bytes	ADD_SUFFIX(fold_lessthan16bytes)

static ATTRIBUTES u32
ADD_SUFFIX(crc32_x86)(u32 crc, const u8 *p, size_t len)
{
	/*
	 * mults_{N}v are the vectors of multipliers for folding across N vec_t
	 * vectors, i.e. N*VL*8 bits.  mults_128b are the two multipliers for
	 * folding across 128 bits.  mults_128b differs from mults_1v when
	 * VL != 16.  All multipliers are 64-bit, to match what pclmulqdq needs,
	 * but since this is for CRC-32 only their low 32 bits are nonzero.
	 * For more details, see scripts/gen-crc32-consts.py.
	 */
	const vec_t mults_8v = MULTS_8V;
	const vec_t mults_4v = MULTS_4V;
	const vec_t mults_2v = MULTS_2V;
	const vec_t mults_1v = MULTS_1V;
	const __m128i mults_128b = _mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG);
	const __m128i barrett_reduction_constants =
		_mm_set_epi64x(CRC32_BARRETT_CONSTANT_2, CRC32_BARRETT_CONSTANT_1);
	vec_t v0, v1, v2, v3, v4, v5, v6, v7;
	__m128i x0 = _mm_cvtsi32_si128(crc);
	__m128i x1;

	if (len < 8*VL) {
		if (len < VL) {
			STATIC_ASSERT(VL == 16 || VL == 32 || VL == 64);
			if (len < 16) {
			#if USE_AVX512
				if (len < 4)
					return crc32_slice1(crc, p, len);
				/*
				 * Handle 4 <= len <= 15 bytes by doing a masked
				 * load, XOR'ing the current CRC with the first
				 * 4 bytes, left-shifting by '16 - len' bytes to
				 * align the result to the end of x0 (so that it
				 * becomes the low-order coefficients of a
				 * 128-bit polynomial), and then doing the usual
				 * reduction from 128 bits to 32 bits.
				 */
				x0 = _mm_xor_si128(
					x0, _mm_maskz_loadu_epi8((1 << len) - 1, p));
				x0 = _mm_shuffle_epi8(
					x0, _mm_loadu_si128((const void *)&shift_tab[len]));
				goto reduce_x0;
			#else
				return crc32_slice1(crc, p, len);
			#endif
			}
			/*
			 * Handle 16 <= len < VL bytes where VL is 32 or 64.
			 * Use 128-bit instructions so that these lengths aren't
			 * slower with VL > 16 than with VL=16.
			 */
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			if (len >= 32) {
				x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 16)),
						 mults_128b);
				if (len >= 48)
					x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 32)),
							 mults_128b);
			}
			p += len & ~15;
			goto less_than_16_remaining;
		}
		v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		if (len < 2*VL) {
			p += VL;
			goto less_than_vl_remaining;
		}
		v1 = VLOADU(p + 1*VL);
		if (len < 4*VL) {
			p += 2*VL;
			goto less_than_2vl_remaining;
		}
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		p += 4*VL;
	} else {
		/*
		 * If the length is large and the pointer is misaligned, align
		 * it.  For smaller lengths, just take the misaligned load
		 * penalty.  Note that on recent x86 CPUs, vmovdqu with an
		 * aligned address is just as fast as vmovdqa, so there's no
		 * need to use vmovdqa in the main loop.
		 */
		if (len > 65536 && ((uintptr_t)p & (VL-1))) {
			size_t align = -(uintptr_t)p & (VL-1);

			len -= align;
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			p += 16;
			if (align & 15) {
				x0 = fold_lessthan16bytes(x0, p, align & 15,
							  mults_128b);
				p += align & 15;
				align &= ~15;
			}
			while (align) {
				x0 = fold_vec128(x0, *(const __m128i *)p,
						 mults_128b);
				p += 16;
				align -= 16;
			}
			v0 = M128I_TO_VEC(x0);
		#  if VL == 32
			v0 = _mm256_inserti128_si256(v0, *(const __m128i *)p, 1);
		#  elif VL == 64
			v0 = _mm512_inserti32x4(v0, *(const __m128i *)p, 1);
			v0 = _mm512_inserti64x4(v0, *(const __m256i *)(p + 16), 1);
		#  endif
			p -= 16;
		} else {
			v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		}
		v1 = VLOADU(p + 1*VL);
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		v4 = VLOADU(p + 4*VL);
		v5 = VLOADU(p + 5*VL);
		v6 = VLOADU(p + 6*VL);
		v7 = VLOADU(p + 7*VL);
		p += 8*VL;

		/*
		 * This is the main loop, processing 8*VL bytes per iteration.
		 * 4*VL is usually enough and would result in smaller code, but
		 * Skylake and Cascade Lake need 8*VL to get full performance.
		 */
		while (len >= 16*VL) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_8v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_8v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_8v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_8v);
			v4 = fold_vec(v4, VLOADU(p + 4*VL), mults_8v);
			v5 = fold_vec(v5, VLOADU(p + 5*VL), mults_8v);
			v6 = fold_vec(v6, VLOADU(p + 6*VL), mults_8v);
			v7 = fold_vec(v7, VLOADU(p + 7*VL), mults_8v);
			p += 8*VL;
			len -= 8*VL;
		}

		/* Fewer than 8*VL bytes remain. */
		v0 = fold_vec(v0, v4, mults_4v);
		v1 = fold_vec(v1, v5, mults_4v);
		v2 = fold_vec(v2, v6, mults_4v);
		v3 = fold_vec(v3, v7, mults_4v);
		if (len & (4*VL)) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_4v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_4v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_4v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_4v);
			p += 4*VL;
		}
	}
	/* Fewer than 4*VL bytes remain. */
	v0 = fold_vec(v0, v2, mults_2v);
	v1 = fold_vec(v1, v3, mults_2v);
	if (len & (2*VL)) {
		v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_2v);
		v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_2v);
		p += 2*VL;
	}
less_than_2vl_remaining:
	/* Fewer than 2*VL bytes remain. */
	v0 = fold_vec(v0, v1, mults_1v);
	if (len & VL) {
		v0 = fold_vec(v0, VLOADU(p), mults_1v);
		p += VL;
	}
less_than_vl_remaining:
	/*
	 * Fewer than VL bytes remain.  Reduce v0 (length VL bytes) to x0
	 * (length 16 bytes) and fold in any 16-byte data segments that remain.
	 */
#if VL == 16
	x0 = v0;
#else
	{
	#if VL == 32
		__m256i y0 = v0;
	#else
		const __m256i mults_256b =
			_mm256_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG,
					  CRC32_X223_MODG, CRC32_X287_MODG);
		__m256i y0 = fold_vec256(_mm512_extracti64x4_epi64(v0, 0),
					 _mm512_extracti64x4_epi64(v0, 1),
					 mults_256b);
		if (len & 32) {
			y0 = fold_vec256(y0, _mm256_loadu_si256((const void *)p),
					 mults_256b);
			p += 32;
		}
	#endif
		x0 = fold_vec128(_mm256_extracti128_si256(y0, 0),
				 _mm256_extracti128_si256(y0, 1), mults_128b);
	}
	if (len & 16) {
		x0 = fold_vec128(x0, _mm_loadu_si128((const void *)p),
				 mults_128b);
		p += 16;
	}
#endif
less_than_16_remaining:
	len &= 15;

	/* Handle any remainder of 1 to 15 bytes. */
	if (len)
		x0 = fold_lessthan16bytes(x0, p, len, mults_128b);
#if USE_AVX512
reduce_x0:
#endif
	/*
	 * Multiply the remaining 128-bit message polynomial 'x0' by x^32, then
	 * reduce it modulo the generator polynomial G.  This gives the CRC.
	 *
	 * This implementation matches that used in crc-pclmul-template.S from
	 * https://lore.kernel.org/r/20250210174540.161705-4-ebiggers@kernel.org/
	 * with the parameters n=32 and LSB_CRC=1 (what the gzip CRC uses).  See
	 * there for a detailed explanation of the math used here.
	 */
	x0 = _mm_xor_si128(_mm_clmulepi64_si128(x0, mults_128b, 0x10),
			   _mm_bsrli_si128(x0, 8));
	x1 = _mm_clmulepi64_si128(x0, barrett_reduction_constants, 0x00);
	x1 = _mm_clmulepi64_si128(x1, barrett_reduction_constants, 0x10);
	x0 = _mm_xor_si128(x0, x1);
	return _mm_extract_epi32(x0, 2);
}

#undef vec_t
#undef fold_vec
#undef VLOADU
#undef VXOR
#undef M128I_TO_VEC
#undef MULTS
#undef MULTS_8V
#undef MULTS_4V
#undef MULTS_2V
#undef MULTS_1V

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_AVX512

/*** End of inlined file: crc32_pclmul_template.h ***/


/*
 * PCLMULQDQ/AVX implementation.  Same as above, but this is compiled with AVX
 * enabled so that the compiler can generate VEX-coded instructions which can be
 * slightly more efficient.  It still uses 128-bit vectors.
 */
#  define crc32_x86_pclmulqdq_avx	crc32_x86_pclmulqdq_avx
#  define SUFFIX				 _pclmulqdq_avx
#  define ATTRIBUTES		_target_attribute("pclmul,avx")
#  define VL			16
#  define USE_AVX512		0

/*** Start of inlined file: crc32_pclmul_template.h ***/
/*
 * This file is a "template" for instantiating PCLMULQDQ-based crc32_x86
 * functions.  The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_AVX512=0: at least pclmul,sse4.1
 *	   VL=32 && USE_AVX512=0: at least vpclmulqdq,pclmul,avx2
 *	   VL=32 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   VL=64 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking and the
 *	vpternlog instruction.  This doesn't enable the use of 512-bit vectors;
 *	the vector length is controlled by VL.  If 0, assume that the CPU might
 *	not support AVX-512.
 *
 * The overall algorithm used is CRC folding with carryless multiplication
 * instructions.  Note that the x86 crc32 instruction cannot be used, as it is
 * for a different polynomial, not the gzip one.  For an explanation of CRC
 * folding with carryless multiplication instructions, see
 * scripts/gen-crc32-consts.py and the following blog posts and papers:
 *
 *	"An alternative exposition of crc32_4k_pclmulqdq"
 *	https://www.corsix.org/content/alternative-exposition-crc32_4k_pclmulqdq
 *
 *	"Fast CRC Computation for Generic Polynomials Using PCLMULQDQ Instruction"
 *	https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/fast-crc-computation-generic-polynomials-pclmulqdq-paper.pdf
 *
 * The original pclmulqdq instruction does one 64x64 to 128-bit carryless
 * multiplication.  The VPCLMULQDQ feature added instructions that do two
 * parallel 64x64 to 128-bit carryless multiplications in combination with AVX
 * or AVX512VL, or four in combination with AVX512F.
 */

#if VL == 16
#  define vec_t			__m128i
#  define fold_vec		fold_vec128
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VXOR(a, b)		_mm_xor_si128((a), (b))
#  define M128I_TO_VEC(a)	a
#  define MULTS_8V		_mm_set_epi64x(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_4V		_mm_set_epi64x(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_2V		_mm_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG)
#  define MULTS_1V		_mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG)
#elif VL == 32
#  define vec_t			__m256i
#  define fold_vec		fold_vec256
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VXOR(a, b)		_mm256_xor_si256((a), (b))
#  define M128I_TO_VEC(a)	_mm256_zextsi128_si256(a)
#  define MULTS(a, b)		_mm256_set_epi64x(a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_4V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_2V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_1V		MULTS(CRC32_X223_MODG, CRC32_X287_MODG)
#elif VL == 64
#  define vec_t			__m512i
#  define fold_vec		fold_vec512
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VXOR(a, b)		_mm512_xor_si512((a), (b))
#  define M128I_TO_VEC(a)	_mm512_zextsi128_si512(a)
#  define MULTS(a, b)		_mm512_set_epi64(a, b, a, b, a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X4063_MODG, CRC32_X4127_MODG)
#  define MULTS_4V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_2V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_1V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#else
#  error "unsupported vector length"
#endif

#undef fold_vec128
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_vec128)(__m128i src, __m128i dst, __m128i /* __v2du */ mults)
{
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x00));
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x11));
	return dst;
}
#define fold_vec128	ADD_SUFFIX(fold_vec128)

#if VL >= 32
#undef fold_vec256
static forceinline ATTRIBUTES __m256i
ADD_SUFFIX(fold_vec256)(__m256i src, __m256i dst, __m256i /* __v4du */ mults)
{
#if USE_AVX512
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm256_ternarylogic_epi32(
			_mm256_clmulepi64_epi128(src, mults, 0x00),
			_mm256_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
#else
	return _mm256_xor_si256(
			_mm256_xor_si256(dst,
					 _mm256_clmulepi64_epi128(src, mults, 0x00)),
			_mm256_clmulepi64_epi128(src, mults, 0x11));
#endif
}
#define fold_vec256	ADD_SUFFIX(fold_vec256)
#endif /* VL >= 32 */

#if VL >= 64
#undef fold_vec512
static forceinline ATTRIBUTES __m512i
ADD_SUFFIX(fold_vec512)(__m512i src, __m512i dst, __m512i /* __v8du */ mults)
{
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm512_ternarylogic_epi32(
			_mm512_clmulepi64_epi128(src, mults, 0x00),
			_mm512_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
}
#define fold_vec512	ADD_SUFFIX(fold_vec512)
#endif /* VL >= 64 */

/*
 * Given 'x' containing a 16-byte polynomial, and a pointer 'p' that points to
 * the next '1 <= len <= 15' data bytes, rearrange the concatenation of 'x' and
 * the data into vectors x0 and x1 that contain 'len' bytes and 16 bytes,
 * respectively.  Then fold x0 into x1 and return the result.
 * Assumes that 'p + len - 16' is in-bounds.
 */
#undef fold_lessthan16bytes
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_lessthan16bytes)(__m128i x, const u8 *p, size_t len,
				 __m128i /* __v2du */ mults_128b)
{
	__m128i lshift = _mm_loadu_si128((const void *)&shift_tab[len]);
	__m128i rshift = _mm_loadu_si128((const void *)&shift_tab[len + 16]);
	__m128i x0, x1;

	/* x0 = x left-shifted by '16 - len' bytes */
	x0 = _mm_shuffle_epi8(x, lshift);

	/*
	 * x1 = the last '16 - len' bytes from x (i.e. x right-shifted by 'len'
	 * bytes) followed by the remaining data.
	 */
	x1 = _mm_blendv_epi8(_mm_shuffle_epi8(x, rshift),
			     _mm_loadu_si128((const void *)(p + len - 16)),
			     /* msb 0/1 of each byte selects byte from arg1/2 */
			     rshift);

	return fold_vec128(x0, x1, mults_128b);
}
#define fold_lessthan16bytes	ADD_SUFFIX(fold_lessthan16bytes)

static ATTRIBUTES u32
ADD_SUFFIX(crc32_x86)(u32 crc, const u8 *p, size_t len)
{
	/*
	 * mults_{N}v are the vectors of multipliers for folding across N vec_t
	 * vectors, i.e. N*VL*8 bits.  mults_128b are the two multipliers for
	 * folding across 128 bits.  mults_128b differs from mults_1v when
	 * VL != 16.  All multipliers are 64-bit, to match what pclmulqdq needs,
	 * but since this is for CRC-32 only their low 32 bits are nonzero.
	 * For more details, see scripts/gen-crc32-consts.py.
	 */
	const vec_t mults_8v = MULTS_8V;
	const vec_t mults_4v = MULTS_4V;
	const vec_t mults_2v = MULTS_2V;
	const vec_t mults_1v = MULTS_1V;
	const __m128i mults_128b = _mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG);
	const __m128i barrett_reduction_constants =
		_mm_set_epi64x(CRC32_BARRETT_CONSTANT_2, CRC32_BARRETT_CONSTANT_1);
	vec_t v0, v1, v2, v3, v4, v5, v6, v7;
	__m128i x0 = _mm_cvtsi32_si128(crc);
	__m128i x1;

	if (len < 8*VL) {
		if (len < VL) {
			STATIC_ASSERT(VL == 16 || VL == 32 || VL == 64);
			if (len < 16) {
			#if USE_AVX512
				if (len < 4)
					return crc32_slice1(crc, p, len);
				/*
				 * Handle 4 <= len <= 15 bytes by doing a masked
				 * load, XOR'ing the current CRC with the first
				 * 4 bytes, left-shifting by '16 - len' bytes to
				 * align the result to the end of x0 (so that it
				 * becomes the low-order coefficients of a
				 * 128-bit polynomial), and then doing the usual
				 * reduction from 128 bits to 32 bits.
				 */
				x0 = _mm_xor_si128(
					x0, _mm_maskz_loadu_epi8((1 << len) - 1, p));
				x0 = _mm_shuffle_epi8(
					x0, _mm_loadu_si128((const void *)&shift_tab[len]));
				goto reduce_x0;
			#else
				return crc32_slice1(crc, p, len);
			#endif
			}
			/*
			 * Handle 16 <= len < VL bytes where VL is 32 or 64.
			 * Use 128-bit instructions so that these lengths aren't
			 * slower with VL > 16 than with VL=16.
			 */
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			if (len >= 32) {
				x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 16)),
						 mults_128b);
				if (len >= 48)
					x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 32)),
							 mults_128b);
			}
			p += len & ~15;
			goto less_than_16_remaining;
		}
		v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		if (len < 2*VL) {
			p += VL;
			goto less_than_vl_remaining;
		}
		v1 = VLOADU(p + 1*VL);
		if (len < 4*VL) {
			p += 2*VL;
			goto less_than_2vl_remaining;
		}
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		p += 4*VL;
	} else {
		/*
		 * If the length is large and the pointer is misaligned, align
		 * it.  For smaller lengths, just take the misaligned load
		 * penalty.  Note that on recent x86 CPUs, vmovdqu with an
		 * aligned address is just as fast as vmovdqa, so there's no
		 * need to use vmovdqa in the main loop.
		 */
		if (len > 65536 && ((uintptr_t)p & (VL-1))) {
			size_t align = -(uintptr_t)p & (VL-1);

			len -= align;
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			p += 16;
			if (align & 15) {
				x0 = fold_lessthan16bytes(x0, p, align & 15,
							  mults_128b);
				p += align & 15;
				align &= ~15;
			}
			while (align) {
				x0 = fold_vec128(x0, *(const __m128i *)p,
						 mults_128b);
				p += 16;
				align -= 16;
			}
			v0 = M128I_TO_VEC(x0);
		#  if VL == 32
			v0 = _mm256_inserti128_si256(v0, *(const __m128i *)p, 1);
		#  elif VL == 64
			v0 = _mm512_inserti32x4(v0, *(const __m128i *)p, 1);
			v0 = _mm512_inserti64x4(v0, *(const __m256i *)(p + 16), 1);
		#  endif
			p -= 16;
		} else {
			v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		}
		v1 = VLOADU(p + 1*VL);
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		v4 = VLOADU(p + 4*VL);
		v5 = VLOADU(p + 5*VL);
		v6 = VLOADU(p + 6*VL);
		v7 = VLOADU(p + 7*VL);
		p += 8*VL;

		/*
		 * This is the main loop, processing 8*VL bytes per iteration.
		 * 4*VL is usually enough and would result in smaller code, but
		 * Skylake and Cascade Lake need 8*VL to get full performance.
		 */
		while (len >= 16*VL) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_8v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_8v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_8v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_8v);
			v4 = fold_vec(v4, VLOADU(p + 4*VL), mults_8v);
			v5 = fold_vec(v5, VLOADU(p + 5*VL), mults_8v);
			v6 = fold_vec(v6, VLOADU(p + 6*VL), mults_8v);
			v7 = fold_vec(v7, VLOADU(p + 7*VL), mults_8v);
			p += 8*VL;
			len -= 8*VL;
		}

		/* Fewer than 8*VL bytes remain. */
		v0 = fold_vec(v0, v4, mults_4v);
		v1 = fold_vec(v1, v5, mults_4v);
		v2 = fold_vec(v2, v6, mults_4v);
		v3 = fold_vec(v3, v7, mults_4v);
		if (len & (4*VL)) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_4v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_4v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_4v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_4v);
			p += 4*VL;
		}
	}
	/* Fewer than 4*VL bytes remain. */
	v0 = fold_vec(v0, v2, mults_2v);
	v1 = fold_vec(v1, v3, mults_2v);
	if (len & (2*VL)) {
		v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_2v);
		v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_2v);
		p += 2*VL;
	}
less_than_2vl_remaining:
	/* Fewer than 2*VL bytes remain. */
	v0 = fold_vec(v0, v1, mults_1v);
	if (len & VL) {
		v0 = fold_vec(v0, VLOADU(p), mults_1v);
		p += VL;
	}
less_than_vl_remaining:
	/*
	 * Fewer than VL bytes remain.  Reduce v0 (length VL bytes) to x0
	 * (length 16 bytes) and fold in any 16-byte data segments that remain.
	 */
#if VL == 16
	x0 = v0;
#else
	{
	#if VL == 32
		__m256i y0 = v0;
	#else
		const __m256i mults_256b =
			_mm256_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG,
					  CRC32_X223_MODG, CRC32_X287_MODG);
		__m256i y0 = fold_vec256(_mm512_extracti64x4_epi64(v0, 0),
					 _mm512_extracti64x4_epi64(v0, 1),
					 mults_256b);
		if (len & 32) {
			y0 = fold_vec256(y0, _mm256_loadu_si256((const void *)p),
					 mults_256b);
			p += 32;
		}
	#endif
		x0 = fold_vec128(_mm256_extracti128_si256(y0, 0),
				 _mm256_extracti128_si256(y0, 1), mults_128b);
	}
	if (len & 16) {
		x0 = fold_vec128(x0, _mm_loadu_si128((const void *)p),
				 mults_128b);
		p += 16;
	}
#endif
less_than_16_remaining:
	len &= 15;

	/* Handle any remainder of 1 to 15 bytes. */
	if (len)
		x0 = fold_lessthan16bytes(x0, p, len, mults_128b);
#if USE_AVX512
reduce_x0:
#endif
	/*
	 * Multiply the remaining 128-bit message polynomial 'x0' by x^32, then
	 * reduce it modulo the generator polynomial G.  This gives the CRC.
	 *
	 * This implementation matches that used in crc-pclmul-template.S from
	 * https://lore.kernel.org/r/20250210174540.161705-4-ebiggers@kernel.org/
	 * with the parameters n=32 and LSB_CRC=1 (what the gzip CRC uses).  See
	 * there for a detailed explanation of the math used here.
	 */
	x0 = _mm_xor_si128(_mm_clmulepi64_si128(x0, mults_128b, 0x10),
			   _mm_bsrli_si128(x0, 8));
	x1 = _mm_clmulepi64_si128(x0, barrett_reduction_constants, 0x00);
	x1 = _mm_clmulepi64_si128(x1, barrett_reduction_constants, 0x10);
	x0 = _mm_xor_si128(x0, x1);
	return _mm_extract_epi32(x0, 2);
}

#undef vec_t
#undef fold_vec
#undef VLOADU
#undef VXOR
#undef M128I_TO_VEC
#undef MULTS
#undef MULTS_8V
#undef MULTS_4V
#undef MULTS_2V
#undef MULTS_1V

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_AVX512

/*** End of inlined file: crc32_pclmul_template.h ***/


#endif

/*
 * VPCLMULQDQ/AVX2 implementation.  This is used on CPUs that have AVX2 and
 * VPCLMULQDQ but don't have AVX-512, for example Intel Alder Lake.
 *
 * Currently this can't be enabled with MSVC because MSVC has a bug where it
 * incorrectly assumes that VPCLMULQDQ implies AVX-512:
 * https://developercommunity.visualstudio.com/t/Compiler-incorrectly-assumes-VAES-and-VP/10578785
 *
 * gcc 8.1 and 8.2 had a similar bug where they assumed that
 * _mm256_clmulepi64_epi128() always needed AVX512.  It's fixed in gcc 8.3.
 *
 * _mm256_zextsi128_si256() requires gcc 10.
 */
#if (GCC_PREREQ(10, 1) || CLANG_PREREQ(6, 0, 10000000)) && \
	!defined(LIBDEFLATE_ASSEMBLER_DOES_NOT_SUPPORT_VPCLMULQDQ)
#  define crc32_x86_vpclmulqdq_avx2	crc32_x86_vpclmulqdq_avx2
#  define SUFFIX				 _vpclmulqdq_avx2
#  define ATTRIBUTES		_target_attribute("vpclmulqdq,pclmul,avx2")
#  define VL			32
#  define USE_AVX512		0

/*** Start of inlined file: crc32_pclmul_template.h ***/
/*
 * This file is a "template" for instantiating PCLMULQDQ-based crc32_x86
 * functions.  The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_AVX512=0: at least pclmul,sse4.1
 *	   VL=32 && USE_AVX512=0: at least vpclmulqdq,pclmul,avx2
 *	   VL=32 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   VL=64 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking and the
 *	vpternlog instruction.  This doesn't enable the use of 512-bit vectors;
 *	the vector length is controlled by VL.  If 0, assume that the CPU might
 *	not support AVX-512.
 *
 * The overall algorithm used is CRC folding with carryless multiplication
 * instructions.  Note that the x86 crc32 instruction cannot be used, as it is
 * for a different polynomial, not the gzip one.  For an explanation of CRC
 * folding with carryless multiplication instructions, see
 * scripts/gen-crc32-consts.py and the following blog posts and papers:
 *
 *	"An alternative exposition of crc32_4k_pclmulqdq"
 *	https://www.corsix.org/content/alternative-exposition-crc32_4k_pclmulqdq
 *
 *	"Fast CRC Computation for Generic Polynomials Using PCLMULQDQ Instruction"
 *	https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/fast-crc-computation-generic-polynomials-pclmulqdq-paper.pdf
 *
 * The original pclmulqdq instruction does one 64x64 to 128-bit carryless
 * multiplication.  The VPCLMULQDQ feature added instructions that do two
 * parallel 64x64 to 128-bit carryless multiplications in combination with AVX
 * or AVX512VL, or four in combination with AVX512F.
 */

#if VL == 16
#  define vec_t			__m128i
#  define fold_vec		fold_vec128
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VXOR(a, b)		_mm_xor_si128((a), (b))
#  define M128I_TO_VEC(a)	a
#  define MULTS_8V		_mm_set_epi64x(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_4V		_mm_set_epi64x(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_2V		_mm_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG)
#  define MULTS_1V		_mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG)
#elif VL == 32
#  define vec_t			__m256i
#  define fold_vec		fold_vec256
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VXOR(a, b)		_mm256_xor_si256((a), (b))
#  define M128I_TO_VEC(a)	_mm256_zextsi128_si256(a)
#  define MULTS(a, b)		_mm256_set_epi64x(a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_4V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_2V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_1V		MULTS(CRC32_X223_MODG, CRC32_X287_MODG)
#elif VL == 64
#  define vec_t			__m512i
#  define fold_vec		fold_vec512
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VXOR(a, b)		_mm512_xor_si512((a), (b))
#  define M128I_TO_VEC(a)	_mm512_zextsi128_si512(a)
#  define MULTS(a, b)		_mm512_set_epi64(a, b, a, b, a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X4063_MODG, CRC32_X4127_MODG)
#  define MULTS_4V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_2V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_1V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#else
#  error "unsupported vector length"
#endif

#undef fold_vec128
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_vec128)(__m128i src, __m128i dst, __m128i /* __v2du */ mults)
{
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x00));
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x11));
	return dst;
}
#define fold_vec128	ADD_SUFFIX(fold_vec128)

#if VL >= 32
#undef fold_vec256
static forceinline ATTRIBUTES __m256i
ADD_SUFFIX(fold_vec256)(__m256i src, __m256i dst, __m256i /* __v4du */ mults)
{
#if USE_AVX512
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm256_ternarylogic_epi32(
			_mm256_clmulepi64_epi128(src, mults, 0x00),
			_mm256_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
#else
	return _mm256_xor_si256(
			_mm256_xor_si256(dst,
					 _mm256_clmulepi64_epi128(src, mults, 0x00)),
			_mm256_clmulepi64_epi128(src, mults, 0x11));
#endif
}
#define fold_vec256	ADD_SUFFIX(fold_vec256)
#endif /* VL >= 32 */

#if VL >= 64
#undef fold_vec512
static forceinline ATTRIBUTES __m512i
ADD_SUFFIX(fold_vec512)(__m512i src, __m512i dst, __m512i /* __v8du */ mults)
{
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm512_ternarylogic_epi32(
			_mm512_clmulepi64_epi128(src, mults, 0x00),
			_mm512_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
}
#define fold_vec512	ADD_SUFFIX(fold_vec512)
#endif /* VL >= 64 */

/*
 * Given 'x' containing a 16-byte polynomial, and a pointer 'p' that points to
 * the next '1 <= len <= 15' data bytes, rearrange the concatenation of 'x' and
 * the data into vectors x0 and x1 that contain 'len' bytes and 16 bytes,
 * respectively.  Then fold x0 into x1 and return the result.
 * Assumes that 'p + len - 16' is in-bounds.
 */
#undef fold_lessthan16bytes
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_lessthan16bytes)(__m128i x, const u8 *p, size_t len,
				 __m128i /* __v2du */ mults_128b)
{
	__m128i lshift = _mm_loadu_si128((const void *)&shift_tab[len]);
	__m128i rshift = _mm_loadu_si128((const void *)&shift_tab[len + 16]);
	__m128i x0, x1;

	/* x0 = x left-shifted by '16 - len' bytes */
	x0 = _mm_shuffle_epi8(x, lshift);

	/*
	 * x1 = the last '16 - len' bytes from x (i.e. x right-shifted by 'len'
	 * bytes) followed by the remaining data.
	 */
	x1 = _mm_blendv_epi8(_mm_shuffle_epi8(x, rshift),
			     _mm_loadu_si128((const void *)(p + len - 16)),
			     /* msb 0/1 of each byte selects byte from arg1/2 */
			     rshift);

	return fold_vec128(x0, x1, mults_128b);
}
#define fold_lessthan16bytes	ADD_SUFFIX(fold_lessthan16bytes)

static ATTRIBUTES u32
ADD_SUFFIX(crc32_x86)(u32 crc, const u8 *p, size_t len)
{
	/*
	 * mults_{N}v are the vectors of multipliers for folding across N vec_t
	 * vectors, i.e. N*VL*8 bits.  mults_128b are the two multipliers for
	 * folding across 128 bits.  mults_128b differs from mults_1v when
	 * VL != 16.  All multipliers are 64-bit, to match what pclmulqdq needs,
	 * but since this is for CRC-32 only their low 32 bits are nonzero.
	 * For more details, see scripts/gen-crc32-consts.py.
	 */
	const vec_t mults_8v = MULTS_8V;
	const vec_t mults_4v = MULTS_4V;
	const vec_t mults_2v = MULTS_2V;
	const vec_t mults_1v = MULTS_1V;
	const __m128i mults_128b = _mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG);
	const __m128i barrett_reduction_constants =
		_mm_set_epi64x(CRC32_BARRETT_CONSTANT_2, CRC32_BARRETT_CONSTANT_1);
	vec_t v0, v1, v2, v3, v4, v5, v6, v7;
	__m128i x0 = _mm_cvtsi32_si128(crc);
	__m128i x1;

	if (len < 8*VL) {
		if (len < VL) {
			STATIC_ASSERT(VL == 16 || VL == 32 || VL == 64);
			if (len < 16) {
			#if USE_AVX512
				if (len < 4)
					return crc32_slice1(crc, p, len);
				/*
				 * Handle 4 <= len <= 15 bytes by doing a masked
				 * load, XOR'ing the current CRC with the first
				 * 4 bytes, left-shifting by '16 - len' bytes to
				 * align the result to the end of x0 (so that it
				 * becomes the low-order coefficients of a
				 * 128-bit polynomial), and then doing the usual
				 * reduction from 128 bits to 32 bits.
				 */
				x0 = _mm_xor_si128(
					x0, _mm_maskz_loadu_epi8((1 << len) - 1, p));
				x0 = _mm_shuffle_epi8(
					x0, _mm_loadu_si128((const void *)&shift_tab[len]));
				goto reduce_x0;
			#else
				return crc32_slice1(crc, p, len);
			#endif
			}
			/*
			 * Handle 16 <= len < VL bytes where VL is 32 or 64.
			 * Use 128-bit instructions so that these lengths aren't
			 * slower with VL > 16 than with VL=16.
			 */
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			if (len >= 32) {
				x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 16)),
						 mults_128b);
				if (len >= 48)
					x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 32)),
							 mults_128b);
			}
			p += len & ~15;
			goto less_than_16_remaining;
		}
		v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		if (len < 2*VL) {
			p += VL;
			goto less_than_vl_remaining;
		}
		v1 = VLOADU(p + 1*VL);
		if (len < 4*VL) {
			p += 2*VL;
			goto less_than_2vl_remaining;
		}
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		p += 4*VL;
	} else {
		/*
		 * If the length is large and the pointer is misaligned, align
		 * it.  For smaller lengths, just take the misaligned load
		 * penalty.  Note that on recent x86 CPUs, vmovdqu with an
		 * aligned address is just as fast as vmovdqa, so there's no
		 * need to use vmovdqa in the main loop.
		 */
		if (len > 65536 && ((uintptr_t)p & (VL-1))) {
			size_t align = -(uintptr_t)p & (VL-1);

			len -= align;
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			p += 16;
			if (align & 15) {
				x0 = fold_lessthan16bytes(x0, p, align & 15,
							  mults_128b);
				p += align & 15;
				align &= ~15;
			}
			while (align) {
				x0 = fold_vec128(x0, *(const __m128i *)p,
						 mults_128b);
				p += 16;
				align -= 16;
			}
			v0 = M128I_TO_VEC(x0);
		#  if VL == 32
			v0 = _mm256_inserti128_si256(v0, *(const __m128i *)p, 1);
		#  elif VL == 64
			v0 = _mm512_inserti32x4(v0, *(const __m128i *)p, 1);
			v0 = _mm512_inserti64x4(v0, *(const __m256i *)(p + 16), 1);
		#  endif
			p -= 16;
		} else {
			v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		}
		v1 = VLOADU(p + 1*VL);
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		v4 = VLOADU(p + 4*VL);
		v5 = VLOADU(p + 5*VL);
		v6 = VLOADU(p + 6*VL);
		v7 = VLOADU(p + 7*VL);
		p += 8*VL;

		/*
		 * This is the main loop, processing 8*VL bytes per iteration.
		 * 4*VL is usually enough and would result in smaller code, but
		 * Skylake and Cascade Lake need 8*VL to get full performance.
		 */
		while (len >= 16*VL) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_8v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_8v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_8v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_8v);
			v4 = fold_vec(v4, VLOADU(p + 4*VL), mults_8v);
			v5 = fold_vec(v5, VLOADU(p + 5*VL), mults_8v);
			v6 = fold_vec(v6, VLOADU(p + 6*VL), mults_8v);
			v7 = fold_vec(v7, VLOADU(p + 7*VL), mults_8v);
			p += 8*VL;
			len -= 8*VL;
		}

		/* Fewer than 8*VL bytes remain. */
		v0 = fold_vec(v0, v4, mults_4v);
		v1 = fold_vec(v1, v5, mults_4v);
		v2 = fold_vec(v2, v6, mults_4v);
		v3 = fold_vec(v3, v7, mults_4v);
		if (len & (4*VL)) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_4v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_4v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_4v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_4v);
			p += 4*VL;
		}
	}
	/* Fewer than 4*VL bytes remain. */
	v0 = fold_vec(v0, v2, mults_2v);
	v1 = fold_vec(v1, v3, mults_2v);
	if (len & (2*VL)) {
		v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_2v);
		v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_2v);
		p += 2*VL;
	}
less_than_2vl_remaining:
	/* Fewer than 2*VL bytes remain. */
	v0 = fold_vec(v0, v1, mults_1v);
	if (len & VL) {
		v0 = fold_vec(v0, VLOADU(p), mults_1v);
		p += VL;
	}
less_than_vl_remaining:
	/*
	 * Fewer than VL bytes remain.  Reduce v0 (length VL bytes) to x0
	 * (length 16 bytes) and fold in any 16-byte data segments that remain.
	 */
#if VL == 16
	x0 = v0;
#else
	{
	#if VL == 32
		__m256i y0 = v0;
	#else
		const __m256i mults_256b =
			_mm256_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG,
					  CRC32_X223_MODG, CRC32_X287_MODG);
		__m256i y0 = fold_vec256(_mm512_extracti64x4_epi64(v0, 0),
					 _mm512_extracti64x4_epi64(v0, 1),
					 mults_256b);
		if (len & 32) {
			y0 = fold_vec256(y0, _mm256_loadu_si256((const void *)p),
					 mults_256b);
			p += 32;
		}
	#endif
		x0 = fold_vec128(_mm256_extracti128_si256(y0, 0),
				 _mm256_extracti128_si256(y0, 1), mults_128b);
	}
	if (len & 16) {
		x0 = fold_vec128(x0, _mm_loadu_si128((const void *)p),
				 mults_128b);
		p += 16;
	}
#endif
less_than_16_remaining:
	len &= 15;

	/* Handle any remainder of 1 to 15 bytes. */
	if (len)
		x0 = fold_lessthan16bytes(x0, p, len, mults_128b);
#if USE_AVX512
reduce_x0:
#endif
	/*
	 * Multiply the remaining 128-bit message polynomial 'x0' by x^32, then
	 * reduce it modulo the generator polynomial G.  This gives the CRC.
	 *
	 * This implementation matches that used in crc-pclmul-template.S from
	 * https://lore.kernel.org/r/20250210174540.161705-4-ebiggers@kernel.org/
	 * with the parameters n=32 and LSB_CRC=1 (what the gzip CRC uses).  See
	 * there for a detailed explanation of the math used here.
	 */
	x0 = _mm_xor_si128(_mm_clmulepi64_si128(x0, mults_128b, 0x10),
			   _mm_bsrli_si128(x0, 8));
	x1 = _mm_clmulepi64_si128(x0, barrett_reduction_constants, 0x00);
	x1 = _mm_clmulepi64_si128(x1, barrett_reduction_constants, 0x10);
	x0 = _mm_xor_si128(x0, x1);
	return _mm_extract_epi32(x0, 2);
}

#undef vec_t
#undef fold_vec
#undef VLOADU
#undef VXOR
#undef M128I_TO_VEC
#undef MULTS
#undef MULTS_8V
#undef MULTS_4V
#undef MULTS_2V
#undef MULTS_1V

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_AVX512

/*** End of inlined file: crc32_pclmul_template.h ***/


#endif

#if (GCC_PREREQ(10, 1) || CLANG_PREREQ(6, 0, 10000000) || MSVC_PREREQ(1920)) && \
	!defined(LIBDEFLATE_ASSEMBLER_DOES_NOT_SUPPORT_VPCLMULQDQ)
/*
 * VPCLMULQDQ/AVX512 implementation using 256-bit vectors.  This is very similar
 * to the VPCLMULQDQ/AVX2 implementation but takes advantage of the vpternlog
 * instruction and more registers.  This is used on certain older Intel CPUs,
 * specifically Ice Lake and Tiger Lake, which support VPCLMULQDQ and AVX512 but
 * downclock a bit too eagerly when ZMM registers are used.
 *
 * _mm256_zextsi128_si256() requires gcc 10.
 */
#  define crc32_x86_vpclmulqdq_avx512_vl256  crc32_x86_vpclmulqdq_avx512_vl256
#  define SUFFIX				      _vpclmulqdq_avx512_vl256
#  define ATTRIBUTES		_target_attribute("vpclmulqdq,pclmul,avx512bw,avx512vl")
#  define VL			32
#  define USE_AVX512		1

/*** Start of inlined file: crc32_pclmul_template.h ***/
/*
 * This file is a "template" for instantiating PCLMULQDQ-based crc32_x86
 * functions.  The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_AVX512=0: at least pclmul,sse4.1
 *	   VL=32 && USE_AVX512=0: at least vpclmulqdq,pclmul,avx2
 *	   VL=32 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   VL=64 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking and the
 *	vpternlog instruction.  This doesn't enable the use of 512-bit vectors;
 *	the vector length is controlled by VL.  If 0, assume that the CPU might
 *	not support AVX-512.
 *
 * The overall algorithm used is CRC folding with carryless multiplication
 * instructions.  Note that the x86 crc32 instruction cannot be used, as it is
 * for a different polynomial, not the gzip one.  For an explanation of CRC
 * folding with carryless multiplication instructions, see
 * scripts/gen-crc32-consts.py and the following blog posts and papers:
 *
 *	"An alternative exposition of crc32_4k_pclmulqdq"
 *	https://www.corsix.org/content/alternative-exposition-crc32_4k_pclmulqdq
 *
 *	"Fast CRC Computation for Generic Polynomials Using PCLMULQDQ Instruction"
 *	https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/fast-crc-computation-generic-polynomials-pclmulqdq-paper.pdf
 *
 * The original pclmulqdq instruction does one 64x64 to 128-bit carryless
 * multiplication.  The VPCLMULQDQ feature added instructions that do two
 * parallel 64x64 to 128-bit carryless multiplications in combination with AVX
 * or AVX512VL, or four in combination with AVX512F.
 */

#if VL == 16
#  define vec_t			__m128i
#  define fold_vec		fold_vec128
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VXOR(a, b)		_mm_xor_si128((a), (b))
#  define M128I_TO_VEC(a)	a
#  define MULTS_8V		_mm_set_epi64x(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_4V		_mm_set_epi64x(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_2V		_mm_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG)
#  define MULTS_1V		_mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG)
#elif VL == 32
#  define vec_t			__m256i
#  define fold_vec		fold_vec256
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VXOR(a, b)		_mm256_xor_si256((a), (b))
#  define M128I_TO_VEC(a)	_mm256_zextsi128_si256(a)
#  define MULTS(a, b)		_mm256_set_epi64x(a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_4V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_2V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_1V		MULTS(CRC32_X223_MODG, CRC32_X287_MODG)
#elif VL == 64
#  define vec_t			__m512i
#  define fold_vec		fold_vec512
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VXOR(a, b)		_mm512_xor_si512((a), (b))
#  define M128I_TO_VEC(a)	_mm512_zextsi128_si512(a)
#  define MULTS(a, b)		_mm512_set_epi64(a, b, a, b, a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X4063_MODG, CRC32_X4127_MODG)
#  define MULTS_4V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_2V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_1V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#else
#  error "unsupported vector length"
#endif

#undef fold_vec128
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_vec128)(__m128i src, __m128i dst, __m128i /* __v2du */ mults)
{
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x00));
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x11));
	return dst;
}
#define fold_vec128	ADD_SUFFIX(fold_vec128)

#if VL >= 32
#undef fold_vec256
static forceinline ATTRIBUTES __m256i
ADD_SUFFIX(fold_vec256)(__m256i src, __m256i dst, __m256i /* __v4du */ mults)
{
#if USE_AVX512
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm256_ternarylogic_epi32(
			_mm256_clmulepi64_epi128(src, mults, 0x00),
			_mm256_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
#else
	return _mm256_xor_si256(
			_mm256_xor_si256(dst,
					 _mm256_clmulepi64_epi128(src, mults, 0x00)),
			_mm256_clmulepi64_epi128(src, mults, 0x11));
#endif
}
#define fold_vec256	ADD_SUFFIX(fold_vec256)
#endif /* VL >= 32 */

#if VL >= 64
#undef fold_vec512
static forceinline ATTRIBUTES __m512i
ADD_SUFFIX(fold_vec512)(__m512i src, __m512i dst, __m512i /* __v8du */ mults)
{
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm512_ternarylogic_epi32(
			_mm512_clmulepi64_epi128(src, mults, 0x00),
			_mm512_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
}
#define fold_vec512	ADD_SUFFIX(fold_vec512)
#endif /* VL >= 64 */

/*
 * Given 'x' containing a 16-byte polynomial, and a pointer 'p' that points to
 * the next '1 <= len <= 15' data bytes, rearrange the concatenation of 'x' and
 * the data into vectors x0 and x1 that contain 'len' bytes and 16 bytes,
 * respectively.  Then fold x0 into x1 and return the result.
 * Assumes that 'p + len - 16' is in-bounds.
 */
#undef fold_lessthan16bytes
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_lessthan16bytes)(__m128i x, const u8 *p, size_t len,
				 __m128i /* __v2du */ mults_128b)
{
	__m128i lshift = _mm_loadu_si128((const void *)&shift_tab[len]);
	__m128i rshift = _mm_loadu_si128((const void *)&shift_tab[len + 16]);
	__m128i x0, x1;

	/* x0 = x left-shifted by '16 - len' bytes */
	x0 = _mm_shuffle_epi8(x, lshift);

	/*
	 * x1 = the last '16 - len' bytes from x (i.e. x right-shifted by 'len'
	 * bytes) followed by the remaining data.
	 */
	x1 = _mm_blendv_epi8(_mm_shuffle_epi8(x, rshift),
			     _mm_loadu_si128((const void *)(p + len - 16)),
			     /* msb 0/1 of each byte selects byte from arg1/2 */
			     rshift);

	return fold_vec128(x0, x1, mults_128b);
}
#define fold_lessthan16bytes	ADD_SUFFIX(fold_lessthan16bytes)

static ATTRIBUTES u32
ADD_SUFFIX(crc32_x86)(u32 crc, const u8 *p, size_t len)
{
	/*
	 * mults_{N}v are the vectors of multipliers for folding across N vec_t
	 * vectors, i.e. N*VL*8 bits.  mults_128b are the two multipliers for
	 * folding across 128 bits.  mults_128b differs from mults_1v when
	 * VL != 16.  All multipliers are 64-bit, to match what pclmulqdq needs,
	 * but since this is for CRC-32 only their low 32 bits are nonzero.
	 * For more details, see scripts/gen-crc32-consts.py.
	 */
	const vec_t mults_8v = MULTS_8V;
	const vec_t mults_4v = MULTS_4V;
	const vec_t mults_2v = MULTS_2V;
	const vec_t mults_1v = MULTS_1V;
	const __m128i mults_128b = _mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG);
	const __m128i barrett_reduction_constants =
		_mm_set_epi64x(CRC32_BARRETT_CONSTANT_2, CRC32_BARRETT_CONSTANT_1);
	vec_t v0, v1, v2, v3, v4, v5, v6, v7;
	__m128i x0 = _mm_cvtsi32_si128(crc);
	__m128i x1;

	if (len < 8*VL) {
		if (len < VL) {
			STATIC_ASSERT(VL == 16 || VL == 32 || VL == 64);
			if (len < 16) {
			#if USE_AVX512
				if (len < 4)
					return crc32_slice1(crc, p, len);
				/*
				 * Handle 4 <= len <= 15 bytes by doing a masked
				 * load, XOR'ing the current CRC with the first
				 * 4 bytes, left-shifting by '16 - len' bytes to
				 * align the result to the end of x0 (so that it
				 * becomes the low-order coefficients of a
				 * 128-bit polynomial), and then doing the usual
				 * reduction from 128 bits to 32 bits.
				 */
				x0 = _mm_xor_si128(
					x0, _mm_maskz_loadu_epi8((1 << len) - 1, p));
				x0 = _mm_shuffle_epi8(
					x0, _mm_loadu_si128((const void *)&shift_tab[len]));
				goto reduce_x0;
			#else
				return crc32_slice1(crc, p, len);
			#endif
			}
			/*
			 * Handle 16 <= len < VL bytes where VL is 32 or 64.
			 * Use 128-bit instructions so that these lengths aren't
			 * slower with VL > 16 than with VL=16.
			 */
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			if (len >= 32) {
				x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 16)),
						 mults_128b);
				if (len >= 48)
					x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 32)),
							 mults_128b);
			}
			p += len & ~15;
			goto less_than_16_remaining;
		}
		v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		if (len < 2*VL) {
			p += VL;
			goto less_than_vl_remaining;
		}
		v1 = VLOADU(p + 1*VL);
		if (len < 4*VL) {
			p += 2*VL;
			goto less_than_2vl_remaining;
		}
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		p += 4*VL;
	} else {
		/*
		 * If the length is large and the pointer is misaligned, align
		 * it.  For smaller lengths, just take the misaligned load
		 * penalty.  Note that on recent x86 CPUs, vmovdqu with an
		 * aligned address is just as fast as vmovdqa, so there's no
		 * need to use vmovdqa in the main loop.
		 */
		if (len > 65536 && ((uintptr_t)p & (VL-1))) {
			size_t align = -(uintptr_t)p & (VL-1);

			len -= align;
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			p += 16;
			if (align & 15) {
				x0 = fold_lessthan16bytes(x0, p, align & 15,
							  mults_128b);
				p += align & 15;
				align &= ~15;
			}
			while (align) {
				x0 = fold_vec128(x0, *(const __m128i *)p,
						 mults_128b);
				p += 16;
				align -= 16;
			}
			v0 = M128I_TO_VEC(x0);
		#  if VL == 32
			v0 = _mm256_inserti128_si256(v0, *(const __m128i *)p, 1);
		#  elif VL == 64
			v0 = _mm512_inserti32x4(v0, *(const __m128i *)p, 1);
			v0 = _mm512_inserti64x4(v0, *(const __m256i *)(p + 16), 1);
		#  endif
			p -= 16;
		} else {
			v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		}
		v1 = VLOADU(p + 1*VL);
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		v4 = VLOADU(p + 4*VL);
		v5 = VLOADU(p + 5*VL);
		v6 = VLOADU(p + 6*VL);
		v7 = VLOADU(p + 7*VL);
		p += 8*VL;

		/*
		 * This is the main loop, processing 8*VL bytes per iteration.
		 * 4*VL is usually enough and would result in smaller code, but
		 * Skylake and Cascade Lake need 8*VL to get full performance.
		 */
		while (len >= 16*VL) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_8v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_8v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_8v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_8v);
			v4 = fold_vec(v4, VLOADU(p + 4*VL), mults_8v);
			v5 = fold_vec(v5, VLOADU(p + 5*VL), mults_8v);
			v6 = fold_vec(v6, VLOADU(p + 6*VL), mults_8v);
			v7 = fold_vec(v7, VLOADU(p + 7*VL), mults_8v);
			p += 8*VL;
			len -= 8*VL;
		}

		/* Fewer than 8*VL bytes remain. */
		v0 = fold_vec(v0, v4, mults_4v);
		v1 = fold_vec(v1, v5, mults_4v);
		v2 = fold_vec(v2, v6, mults_4v);
		v3 = fold_vec(v3, v7, mults_4v);
		if (len & (4*VL)) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_4v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_4v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_4v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_4v);
			p += 4*VL;
		}
	}
	/* Fewer than 4*VL bytes remain. */
	v0 = fold_vec(v0, v2, mults_2v);
	v1 = fold_vec(v1, v3, mults_2v);
	if (len & (2*VL)) {
		v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_2v);
		v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_2v);
		p += 2*VL;
	}
less_than_2vl_remaining:
	/* Fewer than 2*VL bytes remain. */
	v0 = fold_vec(v0, v1, mults_1v);
	if (len & VL) {
		v0 = fold_vec(v0, VLOADU(p), mults_1v);
		p += VL;
	}
less_than_vl_remaining:
	/*
	 * Fewer than VL bytes remain.  Reduce v0 (length VL bytes) to x0
	 * (length 16 bytes) and fold in any 16-byte data segments that remain.
	 */
#if VL == 16
	x0 = v0;
#else
	{
	#if VL == 32
		__m256i y0 = v0;
	#else
		const __m256i mults_256b =
			_mm256_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG,
					  CRC32_X223_MODG, CRC32_X287_MODG);
		__m256i y0 = fold_vec256(_mm512_extracti64x4_epi64(v0, 0),
					 _mm512_extracti64x4_epi64(v0, 1),
					 mults_256b);
		if (len & 32) {
			y0 = fold_vec256(y0, _mm256_loadu_si256((const void *)p),
					 mults_256b);
			p += 32;
		}
	#endif
		x0 = fold_vec128(_mm256_extracti128_si256(y0, 0),
				 _mm256_extracti128_si256(y0, 1), mults_128b);
	}
	if (len & 16) {
		x0 = fold_vec128(x0, _mm_loadu_si128((const void *)p),
				 mults_128b);
		p += 16;
	}
#endif
less_than_16_remaining:
	len &= 15;

	/* Handle any remainder of 1 to 15 bytes. */
	if (len)
		x0 = fold_lessthan16bytes(x0, p, len, mults_128b);
#if USE_AVX512
reduce_x0:
#endif
	/*
	 * Multiply the remaining 128-bit message polynomial 'x0' by x^32, then
	 * reduce it modulo the generator polynomial G.  This gives the CRC.
	 *
	 * This implementation matches that used in crc-pclmul-template.S from
	 * https://lore.kernel.org/r/20250210174540.161705-4-ebiggers@kernel.org/
	 * with the parameters n=32 and LSB_CRC=1 (what the gzip CRC uses).  See
	 * there for a detailed explanation of the math used here.
	 */
	x0 = _mm_xor_si128(_mm_clmulepi64_si128(x0, mults_128b, 0x10),
			   _mm_bsrli_si128(x0, 8));
	x1 = _mm_clmulepi64_si128(x0, barrett_reduction_constants, 0x00);
	x1 = _mm_clmulepi64_si128(x1, barrett_reduction_constants, 0x10);
	x0 = _mm_xor_si128(x0, x1);
	return _mm_extract_epi32(x0, 2);
}

#undef vec_t
#undef fold_vec
#undef VLOADU
#undef VXOR
#undef M128I_TO_VEC
#undef MULTS
#undef MULTS_8V
#undef MULTS_4V
#undef MULTS_2V
#undef MULTS_1V

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_AVX512

/*** End of inlined file: crc32_pclmul_template.h ***/


/*
 * VPCLMULQDQ/AVX512 implementation using 512-bit vectors.  This is used on CPUs
 * that have a good AVX-512 implementation including VPCLMULQDQ.
 *
 * _mm512_zextsi128_si512() requires gcc 10.
 */
#  define crc32_x86_vpclmulqdq_avx512_vl512  crc32_x86_vpclmulqdq_avx512_vl512
#  define SUFFIX				      _vpclmulqdq_avx512_vl512
#  define ATTRIBUTES		_target_attribute("vpclmulqdq,pclmul,avx512bw,avx512vl")
#  define VL			64
#  define USE_AVX512		1

/*** Start of inlined file: crc32_pclmul_template.h ***/
/*
 * This file is a "template" for instantiating PCLMULQDQ-based crc32_x86
 * functions.  The "parameters" are:
 *
 * SUFFIX:
 *	Name suffix to append to all instantiated functions.
 * ATTRIBUTES:
 *	Target function attributes to use.  Must satisfy the dependencies of the
 *	other parameters as follows:
 *	   VL=16 && USE_AVX512=0: at least pclmul,sse4.1
 *	   VL=32 && USE_AVX512=0: at least vpclmulqdq,pclmul,avx2
 *	   VL=32 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   VL=64 && USE_AVX512=1: at least vpclmulqdq,pclmul,avx512bw,avx512vl
 *	   (Other combinations are not useful and have not been tested.)
 * VL:
 *	Vector length in bytes.  Must be 16, 32, or 64.
 * USE_AVX512:
 *	If 1, take advantage of AVX-512 features such as masking and the
 *	vpternlog instruction.  This doesn't enable the use of 512-bit vectors;
 *	the vector length is controlled by VL.  If 0, assume that the CPU might
 *	not support AVX-512.
 *
 * The overall algorithm used is CRC folding with carryless multiplication
 * instructions.  Note that the x86 crc32 instruction cannot be used, as it is
 * for a different polynomial, not the gzip one.  For an explanation of CRC
 * folding with carryless multiplication instructions, see
 * scripts/gen-crc32-consts.py and the following blog posts and papers:
 *
 *	"An alternative exposition of crc32_4k_pclmulqdq"
 *	https://www.corsix.org/content/alternative-exposition-crc32_4k_pclmulqdq
 *
 *	"Fast CRC Computation for Generic Polynomials Using PCLMULQDQ Instruction"
 *	https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/fast-crc-computation-generic-polynomials-pclmulqdq-paper.pdf
 *
 * The original pclmulqdq instruction does one 64x64 to 128-bit carryless
 * multiplication.  The VPCLMULQDQ feature added instructions that do two
 * parallel 64x64 to 128-bit carryless multiplications in combination with AVX
 * or AVX512VL, or four in combination with AVX512F.
 */

#if VL == 16
#  define vec_t			__m128i
#  define fold_vec		fold_vec128
#  define VLOADU(p)		_mm_loadu_si128((const void *)(p))
#  define VXOR(a, b)		_mm_xor_si128((a), (b))
#  define M128I_TO_VEC(a)	a
#  define MULTS_8V		_mm_set_epi64x(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_4V		_mm_set_epi64x(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_2V		_mm_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG)
#  define MULTS_1V		_mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG)
#elif VL == 32
#  define vec_t			__m256i
#  define fold_vec		fold_vec256
#  define VLOADU(p)		_mm256_loadu_si256((const void *)(p))
#  define VXOR(a, b)		_mm256_xor_si256((a), (b))
#  define M128I_TO_VEC(a)	_mm256_zextsi128_si256(a)
#  define MULTS(a, b)		_mm256_set_epi64x(a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_4V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_2V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#  define MULTS_1V		MULTS(CRC32_X223_MODG, CRC32_X287_MODG)
#elif VL == 64
#  define vec_t			__m512i
#  define fold_vec		fold_vec512
#  define VLOADU(p)		_mm512_loadu_si512((const void *)(p))
#  define VXOR(a, b)		_mm512_xor_si512((a), (b))
#  define M128I_TO_VEC(a)	_mm512_zextsi128_si512(a)
#  define MULTS(a, b)		_mm512_set_epi64(a, b, a, b, a, b, a, b)
#  define MULTS_8V		MULTS(CRC32_X4063_MODG, CRC32_X4127_MODG)
#  define MULTS_4V		MULTS(CRC32_X2015_MODG, CRC32_X2079_MODG)
#  define MULTS_2V		MULTS(CRC32_X991_MODG, CRC32_X1055_MODG)
#  define MULTS_1V		MULTS(CRC32_X479_MODG, CRC32_X543_MODG)
#else
#  error "unsupported vector length"
#endif

#undef fold_vec128
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_vec128)(__m128i src, __m128i dst, __m128i /* __v2du */ mults)
{
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x00));
	dst = _mm_xor_si128(dst, _mm_clmulepi64_si128(src, mults, 0x11));
	return dst;
}
#define fold_vec128	ADD_SUFFIX(fold_vec128)

#if VL >= 32
#undef fold_vec256
static forceinline ATTRIBUTES __m256i
ADD_SUFFIX(fold_vec256)(__m256i src, __m256i dst, __m256i /* __v4du */ mults)
{
#if USE_AVX512
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm256_ternarylogic_epi32(
			_mm256_clmulepi64_epi128(src, mults, 0x00),
			_mm256_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
#else
	return _mm256_xor_si256(
			_mm256_xor_si256(dst,
					 _mm256_clmulepi64_epi128(src, mults, 0x00)),
			_mm256_clmulepi64_epi128(src, mults, 0x11));
#endif
}
#define fold_vec256	ADD_SUFFIX(fold_vec256)
#endif /* VL >= 32 */

#if VL >= 64
#undef fold_vec512
static forceinline ATTRIBUTES __m512i
ADD_SUFFIX(fold_vec512)(__m512i src, __m512i dst, __m512i /* __v8du */ mults)
{
	/* vpternlog with immediate 0x96 is a three-argument XOR. */
	return _mm512_ternarylogic_epi32(
			_mm512_clmulepi64_epi128(src, mults, 0x00),
			_mm512_clmulepi64_epi128(src, mults, 0x11),
			dst,
			0x96);
}
#define fold_vec512	ADD_SUFFIX(fold_vec512)
#endif /* VL >= 64 */

/*
 * Given 'x' containing a 16-byte polynomial, and a pointer 'p' that points to
 * the next '1 <= len <= 15' data bytes, rearrange the concatenation of 'x' and
 * the data into vectors x0 and x1 that contain 'len' bytes and 16 bytes,
 * respectively.  Then fold x0 into x1 and return the result.
 * Assumes that 'p + len - 16' is in-bounds.
 */
#undef fold_lessthan16bytes
static forceinline ATTRIBUTES __m128i
ADD_SUFFIX(fold_lessthan16bytes)(__m128i x, const u8 *p, size_t len,
				 __m128i /* __v2du */ mults_128b)
{
	__m128i lshift = _mm_loadu_si128((const void *)&shift_tab[len]);
	__m128i rshift = _mm_loadu_si128((const void *)&shift_tab[len + 16]);
	__m128i x0, x1;

	/* x0 = x left-shifted by '16 - len' bytes */
	x0 = _mm_shuffle_epi8(x, lshift);

	/*
	 * x1 = the last '16 - len' bytes from x (i.e. x right-shifted by 'len'
	 * bytes) followed by the remaining data.
	 */
	x1 = _mm_blendv_epi8(_mm_shuffle_epi8(x, rshift),
			     _mm_loadu_si128((const void *)(p + len - 16)),
			     /* msb 0/1 of each byte selects byte from arg1/2 */
			     rshift);

	return fold_vec128(x0, x1, mults_128b);
}
#define fold_lessthan16bytes	ADD_SUFFIX(fold_lessthan16bytes)

static ATTRIBUTES u32
ADD_SUFFIX(crc32_x86)(u32 crc, const u8 *p, size_t len)
{
	/*
	 * mults_{N}v are the vectors of multipliers for folding across N vec_t
	 * vectors, i.e. N*VL*8 bits.  mults_128b are the two multipliers for
	 * folding across 128 bits.  mults_128b differs from mults_1v when
	 * VL != 16.  All multipliers are 64-bit, to match what pclmulqdq needs,
	 * but since this is for CRC-32 only their low 32 bits are nonzero.
	 * For more details, see scripts/gen-crc32-consts.py.
	 */
	const vec_t mults_8v = MULTS_8V;
	const vec_t mults_4v = MULTS_4V;
	const vec_t mults_2v = MULTS_2V;
	const vec_t mults_1v = MULTS_1V;
	const __m128i mults_128b = _mm_set_epi64x(CRC32_X95_MODG, CRC32_X159_MODG);
	const __m128i barrett_reduction_constants =
		_mm_set_epi64x(CRC32_BARRETT_CONSTANT_2, CRC32_BARRETT_CONSTANT_1);
	vec_t v0, v1, v2, v3, v4, v5, v6, v7;
	__m128i x0 = _mm_cvtsi32_si128(crc);
	__m128i x1;

	if (len < 8*VL) {
		if (len < VL) {
			STATIC_ASSERT(VL == 16 || VL == 32 || VL == 64);
			if (len < 16) {
			#if USE_AVX512
				if (len < 4)
					return crc32_slice1(crc, p, len);
				/*
				 * Handle 4 <= len <= 15 bytes by doing a masked
				 * load, XOR'ing the current CRC with the first
				 * 4 bytes, left-shifting by '16 - len' bytes to
				 * align the result to the end of x0 (so that it
				 * becomes the low-order coefficients of a
				 * 128-bit polynomial), and then doing the usual
				 * reduction from 128 bits to 32 bits.
				 */
				x0 = _mm_xor_si128(
					x0, _mm_maskz_loadu_epi8((1 << len) - 1, p));
				x0 = _mm_shuffle_epi8(
					x0, _mm_loadu_si128((const void *)&shift_tab[len]));
				goto reduce_x0;
			#else
				return crc32_slice1(crc, p, len);
			#endif
			}
			/*
			 * Handle 16 <= len < VL bytes where VL is 32 or 64.
			 * Use 128-bit instructions so that these lengths aren't
			 * slower with VL > 16 than with VL=16.
			 */
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			if (len >= 32) {
				x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 16)),
						 mults_128b);
				if (len >= 48)
					x0 = fold_vec128(x0, _mm_loadu_si128((const void *)(p + 32)),
							 mults_128b);
			}
			p += len & ~15;
			goto less_than_16_remaining;
		}
		v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		if (len < 2*VL) {
			p += VL;
			goto less_than_vl_remaining;
		}
		v1 = VLOADU(p + 1*VL);
		if (len < 4*VL) {
			p += 2*VL;
			goto less_than_2vl_remaining;
		}
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		p += 4*VL;
	} else {
		/*
		 * If the length is large and the pointer is misaligned, align
		 * it.  For smaller lengths, just take the misaligned load
		 * penalty.  Note that on recent x86 CPUs, vmovdqu with an
		 * aligned address is just as fast as vmovdqa, so there's no
		 * need to use vmovdqa in the main loop.
		 */
		if (len > 65536 && ((uintptr_t)p & (VL-1))) {
			size_t align = -(uintptr_t)p & (VL-1);

			len -= align;
			x0 = _mm_xor_si128(_mm_loadu_si128((const void *)p), x0);
			p += 16;
			if (align & 15) {
				x0 = fold_lessthan16bytes(x0, p, align & 15,
							  mults_128b);
				p += align & 15;
				align &= ~15;
			}
			while (align) {
				x0 = fold_vec128(x0, *(const __m128i *)p,
						 mults_128b);
				p += 16;
				align -= 16;
			}
			v0 = M128I_TO_VEC(x0);
		#  if VL == 32
			v0 = _mm256_inserti128_si256(v0, *(const __m128i *)p, 1);
		#  elif VL == 64
			v0 = _mm512_inserti32x4(v0, *(const __m128i *)p, 1);
			v0 = _mm512_inserti64x4(v0, *(const __m256i *)(p + 16), 1);
		#  endif
			p -= 16;
		} else {
			v0 = VXOR(VLOADU(p), M128I_TO_VEC(x0));
		}
		v1 = VLOADU(p + 1*VL);
		v2 = VLOADU(p + 2*VL);
		v3 = VLOADU(p + 3*VL);
		v4 = VLOADU(p + 4*VL);
		v5 = VLOADU(p + 5*VL);
		v6 = VLOADU(p + 6*VL);
		v7 = VLOADU(p + 7*VL);
		p += 8*VL;

		/*
		 * This is the main loop, processing 8*VL bytes per iteration.
		 * 4*VL is usually enough and would result in smaller code, but
		 * Skylake and Cascade Lake need 8*VL to get full performance.
		 */
		while (len >= 16*VL) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_8v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_8v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_8v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_8v);
			v4 = fold_vec(v4, VLOADU(p + 4*VL), mults_8v);
			v5 = fold_vec(v5, VLOADU(p + 5*VL), mults_8v);
			v6 = fold_vec(v6, VLOADU(p + 6*VL), mults_8v);
			v7 = fold_vec(v7, VLOADU(p + 7*VL), mults_8v);
			p += 8*VL;
			len -= 8*VL;
		}

		/* Fewer than 8*VL bytes remain. */
		v0 = fold_vec(v0, v4, mults_4v);
		v1 = fold_vec(v1, v5, mults_4v);
		v2 = fold_vec(v2, v6, mults_4v);
		v3 = fold_vec(v3, v7, mults_4v);
		if (len & (4*VL)) {
			v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_4v);
			v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_4v);
			v2 = fold_vec(v2, VLOADU(p + 2*VL), mults_4v);
			v3 = fold_vec(v3, VLOADU(p + 3*VL), mults_4v);
			p += 4*VL;
		}
	}
	/* Fewer than 4*VL bytes remain. */
	v0 = fold_vec(v0, v2, mults_2v);
	v1 = fold_vec(v1, v3, mults_2v);
	if (len & (2*VL)) {
		v0 = fold_vec(v0, VLOADU(p + 0*VL), mults_2v);
		v1 = fold_vec(v1, VLOADU(p + 1*VL), mults_2v);
		p += 2*VL;
	}
less_than_2vl_remaining:
	/* Fewer than 2*VL bytes remain. */
	v0 = fold_vec(v0, v1, mults_1v);
	if (len & VL) {
		v0 = fold_vec(v0, VLOADU(p), mults_1v);
		p += VL;
	}
less_than_vl_remaining:
	/*
	 * Fewer than VL bytes remain.  Reduce v0 (length VL bytes) to x0
	 * (length 16 bytes) and fold in any 16-byte data segments that remain.
	 */
#if VL == 16
	x0 = v0;
#else
	{
	#if VL == 32
		__m256i y0 = v0;
	#else
		const __m256i mults_256b =
			_mm256_set_epi64x(CRC32_X223_MODG, CRC32_X287_MODG,
					  CRC32_X223_MODG, CRC32_X287_MODG);
		__m256i y0 = fold_vec256(_mm512_extracti64x4_epi64(v0, 0),
					 _mm512_extracti64x4_epi64(v0, 1),
					 mults_256b);
		if (len & 32) {
			y0 = fold_vec256(y0, _mm256_loadu_si256((const void *)p),
					 mults_256b);
			p += 32;
		}
	#endif
		x0 = fold_vec128(_mm256_extracti128_si256(y0, 0),
				 _mm256_extracti128_si256(y0, 1), mults_128b);
	}
	if (len & 16) {
		x0 = fold_vec128(x0, _mm_loadu_si128((const void *)p),
				 mults_128b);
		p += 16;
	}
#endif
less_than_16_remaining:
	len &= 15;

	/* Handle any remainder of 1 to 15 bytes. */
	if (len)
		x0 = fold_lessthan16bytes(x0, p, len, mults_128b);
#if USE_AVX512
reduce_x0:
#endif
	/*
	 * Multiply the remaining 128-bit message polynomial 'x0' by x^32, then
	 * reduce it modulo the generator polynomial G.  This gives the CRC.
	 *
	 * This implementation matches that used in crc-pclmul-template.S from
	 * https://lore.kernel.org/r/20250210174540.161705-4-ebiggers@kernel.org/
	 * with the parameters n=32 and LSB_CRC=1 (what the gzip CRC uses).  See
	 * there for a detailed explanation of the math used here.
	 */
	x0 = _mm_xor_si128(_mm_clmulepi64_si128(x0, mults_128b, 0x10),
			   _mm_bsrli_si128(x0, 8));
	x1 = _mm_clmulepi64_si128(x0, barrett_reduction_constants, 0x00);
	x1 = _mm_clmulepi64_si128(x1, barrett_reduction_constants, 0x10);
	x0 = _mm_xor_si128(x0, x1);
	return _mm_extract_epi32(x0, 2);
}

#undef vec_t
#undef fold_vec
#undef VLOADU
#undef VXOR
#undef M128I_TO_VEC
#undef MULTS
#undef MULTS_8V
#undef MULTS_4V
#undef MULTS_2V
#undef MULTS_1V

#undef SUFFIX
#undef ATTRIBUTES
#undef VL
#undef USE_AVX512

/*** End of inlined file: crc32_pclmul_template.h ***/


#endif

static inline crc32_func_t
arch_select_crc32_func(void)
{
	const u32 features MAYBE_UNUSED = get_x86_cpu_features();

#ifdef crc32_x86_vpclmulqdq_avx512_vl512
	if ((features & X86_CPU_FEATURE_ZMM) &&
	    HAVE_VPCLMULQDQ(features) && HAVE_PCLMULQDQ(features) &&
	    HAVE_AVX512BW(features) && HAVE_AVX512VL(features))
		return crc32_x86_vpclmulqdq_avx512_vl512;
#endif
#ifdef crc32_x86_vpclmulqdq_avx512_vl256
	if (HAVE_VPCLMULQDQ(features) && HAVE_PCLMULQDQ(features) &&
	    HAVE_AVX512BW(features) && HAVE_AVX512VL(features))
		return crc32_x86_vpclmulqdq_avx512_vl256;
#endif
#ifdef crc32_x86_vpclmulqdq_avx2
	if (HAVE_VPCLMULQDQ(features) && HAVE_PCLMULQDQ(features) &&
	    HAVE_AVX2(features))
		return crc32_x86_vpclmulqdq_avx2;
#endif
#ifdef crc32_x86_pclmulqdq_avx
	if (HAVE_PCLMULQDQ(features) && HAVE_AVX(features))
		return crc32_x86_pclmulqdq_avx;
#endif
#ifdef crc32_x86_pclmulqdq
	if (HAVE_PCLMULQDQ(features))
		return crc32_x86_pclmulqdq;
#endif
	return NULL;
}
#define arch_select_crc32_func	arch_select_crc32_func

#endif /* LIB_X86_CRC32_IMPL_H */

/*** End of inlined file: crc32_impl.h ***/


#endif

#ifndef DEFAULT_IMPL
#  define DEFAULT_IMPL crc32_slice8
#endif

#ifdef arch_select_crc32_func
static u32 dispatch_crc32(u32 crc, const u8 *p, size_t len);

static volatile crc32_func_t crc32_impl = dispatch_crc32;

/* Choose the best implementation at runtime. */
static u32 dispatch_crc32(u32 crc, const u8 *p, size_t len)
{
	crc32_func_t f = arch_select_crc32_func();

	if (f == NULL)
		f = DEFAULT_IMPL;

	crc32_impl = f;
	return f(crc, p, len);
}
#else
/* The best implementation is statically known, so call it directly. */
#define crc32_impl DEFAULT_IMPL
#endif

LIBDEFLATEAPI u32
libdeflate_crc32(u32 crc, const void *p, size_t len)
{
	if (p == NULL) /* Return initial value. */
		return 0;
	return ~crc32_impl(~crc, p, len);
}

/*** End of inlined file: crc32.c ***/


/*** Start of inlined file: deflate_compress.c ***/

/*** Start of inlined file: deflate_compress.h ***/
#ifndef LIB_DEFLATE_COMPRESS_H
#define LIB_DEFLATE_COMPRESS_H

/*
 * DEFLATE compression is private to deflate_compress.c, but we do need to be
 * able to query the compression level for zlib and gzip header generation.
 */

struct libdeflate_compressor;

unsigned int libdeflate_get_compression_level(struct libdeflate_compressor *c);

#endif /* LIB_DEFLATE_COMPRESS_H */

/*** End of inlined file: deflate_compress.h ***/


/*** Start of inlined file: deflate_constants.h ***/
#ifndef LIB_DEFLATE_CONSTANTS_H
#define LIB_DEFLATE_CONSTANTS_H

/* Valid block types  */
#define DEFLATE_BLOCKTYPE_UNCOMPRESSED		0
#define DEFLATE_BLOCKTYPE_STATIC_HUFFMAN	1
#define DEFLATE_BLOCKTYPE_DYNAMIC_HUFFMAN	2

/* Minimum and maximum supported match lengths (in bytes)  */
#define DEFLATE_MIN_MATCH_LEN			3
#define DEFLATE_MAX_MATCH_LEN			258

/* Maximum supported match offset (in bytes) */
#define DEFLATE_MAX_MATCH_OFFSET		32768

/* log2 of DEFLATE_MAX_MATCH_OFFSET */
#define DEFLATE_WINDOW_ORDER			15

/* Number of symbols in each Huffman code.  Note: for the literal/length
 * and offset codes, these are actually the maximum values; a given block
 * might use fewer symbols.  */
#define DEFLATE_NUM_PRECODE_SYMS		19
#define DEFLATE_NUM_LITLEN_SYMS			288
#define DEFLATE_NUM_OFFSET_SYMS			32

/* The maximum number of symbols across all codes  */
#define DEFLATE_MAX_NUM_SYMS			288

/* Division of symbols in the literal/length code  */
#define DEFLATE_NUM_LITERALS			256
#define DEFLATE_END_OF_BLOCK			256
#define DEFLATE_FIRST_LEN_SYM			257

/* Maximum codeword length, in bits, within each Huffman code  */
#define DEFLATE_MAX_PRE_CODEWORD_LEN		7
#define DEFLATE_MAX_LITLEN_CODEWORD_LEN		15
#define DEFLATE_MAX_OFFSET_CODEWORD_LEN		15

/* The maximum codeword length across all codes  */
#define DEFLATE_MAX_CODEWORD_LEN		15

/* Maximum possible overrun when decoding codeword lengths  */
#define DEFLATE_MAX_LENS_OVERRUN		137

/*
 * Maximum number of extra bits that may be required to represent a match
 * length or offset.
 */
#define DEFLATE_MAX_EXTRA_LENGTH_BITS		5
#define DEFLATE_MAX_EXTRA_OFFSET_BITS		13

#endif /* LIB_DEFLATE_CONSTANTS_H */

/*** End of inlined file: deflate_constants.h ***/

/******************************************************************************/

/*
 * The following parameters can be changed at build time to customize the
 * compression algorithms slightly:
 *
 * (Note, not all customizable parameters are here.  Some others can be found in
 * libdeflate_alloc_compressor() and in *_matchfinder.h.)
 */

/*
 * If this parameter is defined to 1, then the near-optimal parsing algorithm
 * will be included, and compression levels 10-12 will use it.  This algorithm
 * usually produces a compression ratio significantly better than the other
 * algorithms.  However, it is slow.  If this parameter is defined to 0, then
 * levels 10-12 will be the same as level 9 and will use the lazy2 algorithm.
 */
#define SUPPORT_NEAR_OPTIMAL_PARSING	1

/*
 * This is the minimum block length that the compressor will use, in
 * uncompressed bytes.  This should be a value below which using shorter blocks
 * is unlikely to be worthwhile, due to the per-block overhead.  This value does
 * not apply to the final block, which may be shorter than this (if the input is
 * shorter, it will have to be), or to the final uncompressed block in a series
 * of uncompressed blocks that cover more than UINT16_MAX bytes.
 *
 * This value is also approximately the amount by which what would otherwise be
 * the second-to-last block is allowed to grow past the soft maximum length in
 * order to avoid having to use a very short final block.
 *
 * Defining a fixed minimum block length is needed in order to guarantee a
 * reasonable upper bound on the compressed size.  It's also needed because our
 * block splitting algorithm doesn't work well on very short blocks.
 */
#define MIN_BLOCK_LENGTH	5000

/*
 * For the greedy, lazy, lazy2, and near-optimal compressors: This is the soft
 * maximum block length, in uncompressed bytes.  The compressor will try to end
 * blocks at this length, but it may go slightly past it if there is a match
 * that straddles this limit or if the input data ends soon after this limit.
 * This parameter doesn't apply to uncompressed blocks, which the DEFLATE format
 * limits to 65535 bytes.
 *
 * This should be a value above which it is very likely that splitting the block
 * would produce a better compression ratio.  For the near-optimal compressor,
 * increasing/decreasing this parameter will increase/decrease per-compressor
 * memory usage linearly.
 */
#define SOFT_MAX_BLOCK_LENGTH	300000

/*
 * For the greedy, lazy, and lazy2 compressors: this is the length of the
 * sequence store, which is an array where the compressor temporarily stores
 * matches that it's going to use in the current block.  This value is the
 * maximum number of matches that can be used in a block.  If the sequence store
 * fills up, then the compressor will be forced to end the block early.  This
 * value should be large enough so that this rarely happens, due to the block
 * being ended normally before then.  Increasing/decreasing this value will
 * increase/decrease per-compressor memory usage linearly.
 */
#define SEQ_STORE_LENGTH	50000

/*
 * For deflate_compress_fastest(): This is the soft maximum block length.
 * deflate_compress_fastest() doesn't use the regular block splitting algorithm;
 * it only ends blocks when they reach FAST_SOFT_MAX_BLOCK_LENGTH bytes or
 * FAST_SEQ_STORE_LENGTH matches.  Therefore, this value should be lower than
 * the regular SOFT_MAX_BLOCK_LENGTH.
 */
#define FAST_SOFT_MAX_BLOCK_LENGTH	65535

/*
 * For deflate_compress_fastest(): this is the length of the sequence store.
 * This is like SEQ_STORE_LENGTH, but this should be a lower value.
 */
#define FAST_SEQ_STORE_LENGTH	8192

/*
 * These are the maximum codeword lengths, in bits, the compressor will use for
 * each Huffman code.  The DEFLATE format defines limits for these.  However,
 * further limiting litlen codewords to 14 bits is beneficial, since it has
 * negligible effect on compression ratio but allows some optimizations when
 * outputting bits.  (It allows 4 literals to be written at once rather than 3.)
 */
#define MAX_LITLEN_CODEWORD_LEN		14
#define MAX_OFFSET_CODEWORD_LEN		DEFLATE_MAX_OFFSET_CODEWORD_LEN
#define MAX_PRE_CODEWORD_LEN		DEFLATE_MAX_PRE_CODEWORD_LEN

#if SUPPORT_NEAR_OPTIMAL_PARSING

/* Parameters specific to the near-optimal parsing algorithm */

/*
 * BIT_COST is a scaling factor that allows the near-optimal compressor to
 * consider fractional bit costs when deciding which literal/match sequence to
 * use.  This is useful when the true symbol costs are unknown.  For example, if
 * the compressor thinks that a symbol has 6.5 bits of entropy, it can set its
 * cost to 6.5 bits rather than have to use 6 or 7 bits.  Although in the end
 * each symbol will use a whole number of bits due to the Huffman coding,
 * considering fractional bits can be helpful due to the limited information.
 *
 * BIT_COST should be a power of 2.  A value of 8 or 16 works well.  A higher
 * value isn't very useful since the calculations are approximate anyway.
 *
 * BIT_COST doesn't apply to deflate_flush_block() and
 * deflate_compute_true_cost(), which consider whole bits.
 */
#define BIT_COST	16

/*
 * The NOSTAT_BITS value for a given alphabet is the number of bits assumed to
 * be needed to output a symbol that was unused in the previous optimization
 * pass.  Assigning a default cost allows the symbol to be used in the next
 * optimization pass.  However, the cost should be relatively high because the
 * symbol probably won't be used very many times (if at all).
 */
#define LITERAL_NOSTAT_BITS	13
#define LENGTH_NOSTAT_BITS	13
#define OFFSET_NOSTAT_BITS	10

/*
 * This is (slightly less than) the maximum number of matches that the
 * near-optimal compressor will cache per block.  This behaves similarly to
 * SEQ_STORE_LENGTH for the other compressors.
 */
#define MATCH_CACHE_LENGTH	(SOFT_MAX_BLOCK_LENGTH * 5)

#endif /* SUPPORT_NEAR_OPTIMAL_PARSING */

/******************************************************************************/

/* Include the needed matchfinders. */
#define MATCHFINDER_WINDOW_ORDER	DEFLATE_WINDOW_ORDER

/*** Start of inlined file: hc_matchfinder.h ***/
#ifndef LIB_HC_MATCHFINDER_H
#define LIB_HC_MATCHFINDER_H


/*** Start of inlined file: matchfinder_common.h ***/
#ifndef LIB_MATCHFINDER_COMMON_H
#define LIB_MATCHFINDER_COMMON_H

#ifndef MATCHFINDER_WINDOW_ORDER
#  error "MATCHFINDER_WINDOW_ORDER must be defined!"
#endif

/*
 * Given a 32-bit value that was loaded with the platform's native endianness,
 * return a 32-bit value whose high-order 8 bits are 0 and whose low-order 24
 * bits contain the first 3 bytes, arranged in octets in a platform-dependent
 * order, at the memory location from which the input 32-bit value was loaded.
 */
static forceinline u32
loaded_u32_to_u24(u32 v)
{
	if (CPU_IS_LITTLE_ENDIAN())
		return v & 0xFFFFFF;
	else
		return v >> 8;
}

/*
 * Load the next 3 bytes from @p into the 24 low-order bits of a 32-bit value.
 * The order in which the 3 bytes will be arranged as octets in the 24 bits is
 * platform-dependent.  At least 4 bytes (not 3) must be available at @p.
 */
static forceinline u32
load_u24_unaligned(const u8 *p)
{
#if UNALIGNED_ACCESS_IS_FAST
	return loaded_u32_to_u24(load_u32_unaligned(p));
#else
	if (CPU_IS_LITTLE_ENDIAN())
		return ((u32)p[0] << 0) | ((u32)p[1] << 8) | ((u32)p[2] << 16);
	else
		return ((u32)p[2] << 0) | ((u32)p[1] << 8) | ((u32)p[0] << 16);
#endif
}

#define MATCHFINDER_WINDOW_SIZE (1UL << MATCHFINDER_WINDOW_ORDER)

typedef s16 mf_pos_t;

#define MATCHFINDER_INITVAL ((mf_pos_t)-MATCHFINDER_WINDOW_SIZE)

/*
 * This is the memory address alignment, in bytes, required for the matchfinder
 * buffers by the architecture-specific implementations of matchfinder_init()
 * and matchfinder_rebase().  "Matchfinder buffer" means an entire struct
 * hc_matchfinder, bt_matchfinder, or ht_matchfinder; the next_tab field of
 * struct hc_matchfinder; or the child_tab field of struct bt_matchfinder.
 *
 * This affects how the entire 'struct deflate_compressor' is allocated, since
 * the matchfinder structures are embedded inside it.
 *
 * Currently the maximum memory address alignment required is 32 bytes, needed
 * by the AVX-2 matchfinder functions.
 */
#define MATCHFINDER_MEM_ALIGNMENT	32

/*
 * This declares a size, in bytes, that is guaranteed to divide the sizes of the
 * matchfinder buffers (where "matchfinder buffers" is as defined for
 * MATCHFINDER_MEM_ALIGNMENT).  The architecture-specific implementations of
 * matchfinder_init() and matchfinder_rebase() take advantage of this value.
 *
 * Currently the maximum size alignment required is 128 bytes, needed by
 * the AVX-2 matchfinder functions.  However, the RISC-V Vector Extension
 * matchfinder functions can, in principle, take advantage of a larger size
 * alignment.  Therefore, we set this to 1024, which still easily divides the
 * actual sizes that result from the current matchfinder struct definitions.
 * This value can safely be changed to any power of two that is >= 128.
 */
#define MATCHFINDER_SIZE_ALIGNMENT	1024

#undef matchfinder_init
#undef matchfinder_rebase
#ifdef _aligned_attribute
#  define MATCHFINDER_ALIGNED _aligned_attribute(MATCHFINDER_MEM_ALIGNMENT)
#  if defined(ARCH_ARM32) || defined(ARCH_ARM64)

/*** Start of inlined file: matchfinder_impl.h ***/
#ifndef LIB_ARM_MATCHFINDER_IMPL_H
#define LIB_ARM_MATCHFINDER_IMPL_H

#if HAVE_NEON_NATIVE
static forceinline void
matchfinder_init_neon(mf_pos_t *data, size_t size)
{
	int16x8_t *p = (int16x8_t *)data;
	int16x8_t v = vdupq_n_s16(MATCHFINDER_INITVAL);

	STATIC_ASSERT(MATCHFINDER_MEM_ALIGNMENT % sizeof(*p) == 0);
	STATIC_ASSERT(MATCHFINDER_SIZE_ALIGNMENT % (4 * sizeof(*p)) == 0);
	STATIC_ASSERT(sizeof(mf_pos_t) == 2);

	do {
		p[0] = v;
		p[1] = v;
		p[2] = v;
		p[3] = v;
		p += 4;
		size -= 4 * sizeof(*p);
	} while (size != 0);
}
#define matchfinder_init matchfinder_init_neon

static forceinline void
matchfinder_rebase_neon(mf_pos_t *data, size_t size)
{
	int16x8_t *p = (int16x8_t *)data;
	int16x8_t v = vdupq_n_s16((u16)-MATCHFINDER_WINDOW_SIZE);

	STATIC_ASSERT(MATCHFINDER_MEM_ALIGNMENT % sizeof(*p) == 0);
	STATIC_ASSERT(MATCHFINDER_SIZE_ALIGNMENT % (4 * sizeof(*p)) == 0);
	STATIC_ASSERT(sizeof(mf_pos_t) == 2);

	do {
		p[0] = vqaddq_s16(p[0], v);
		p[1] = vqaddq_s16(p[1], v);
		p[2] = vqaddq_s16(p[2], v);
		p[3] = vqaddq_s16(p[3], v);
		p += 4;
		size -= 4 * sizeof(*p);
	} while (size != 0);
}
#define matchfinder_rebase matchfinder_rebase_neon

#endif /* HAVE_NEON_NATIVE */

#endif /* LIB_ARM_MATCHFINDER_IMPL_H */

/*** End of inlined file: matchfinder_impl.h ***/


#  elif defined(ARCH_RISCV)

/*** Start of inlined file: matchfinder_impl.h ***/
#ifndef LIB_RISCV_MATCHFINDER_IMPL_H
#define LIB_RISCV_MATCHFINDER_IMPL_H

#if defined(ARCH_RISCV) && defined(__riscv_vector)
#include <riscv_vector.h>

/*
 * Return the maximum number of 16-bit (mf_pos_t) elements that fit in 8 RISC-V
 * vector registers and also evenly divide the sizes of the matchfinder buffers.
 */
static forceinline size_t
riscv_matchfinder_vl(void)
{
	const size_t vl = __riscv_vsetvlmax_e16m8();

	STATIC_ASSERT(sizeof(mf_pos_t) == sizeof(s16));
	/*
	 * MATCHFINDER_SIZE_ALIGNMENT is a power of 2, as is 'vl' because the
	 * RISC-V Vector Extension requires that the vector register length
	 * (VLEN) be a power of 2.  Thus, a simple MIN() gives the correct
	 * answer here; rounding to a power of 2 is not required.
	 */
	STATIC_ASSERT((MATCHFINDER_SIZE_ALIGNMENT &
		       (MATCHFINDER_SIZE_ALIGNMENT - 1)) == 0);
	ASSERT((vl & (vl - 1)) == 0);
	return MIN(vl, MATCHFINDER_SIZE_ALIGNMENT / sizeof(mf_pos_t));
}

/* matchfinder_init() optimized using the RISC-V Vector Extension */
static forceinline void
matchfinder_init_rvv(mf_pos_t *p, size_t size)
{
	const size_t vl = riscv_matchfinder_vl();
	const vint16m8_t v = __riscv_vmv_v_x_i16m8(MATCHFINDER_INITVAL, vl);

	ASSERT(size > 0 && size % (vl * sizeof(p[0])) == 0);
	do {
		__riscv_vse16_v_i16m8(p, v, vl);
		p += vl;
		size -= vl * sizeof(p[0]);
	} while (size != 0);
}
#define matchfinder_init matchfinder_init_rvv

/* matchfinder_rebase() optimized using the RISC-V Vector Extension */
static forceinline void
matchfinder_rebase_rvv(mf_pos_t *p, size_t size)
{
	const size_t vl = riscv_matchfinder_vl();

	ASSERT(size > 0 && size % (vl * sizeof(p[0])) == 0);
	do {
		vint16m8_t v = __riscv_vle16_v_i16m8(p, vl);

		/*
		 * This should generate the vsadd.vx instruction
		 * (Vector Saturating Add, integer vector-scalar)
		 */
		v = __riscv_vsadd_vx_i16m8(v, (s16)-MATCHFINDER_WINDOW_SIZE,
					   vl);
		__riscv_vse16_v_i16m8(p, v, vl);
		p += vl;
		size -= vl * sizeof(p[0]);
	} while (size != 0);
}
#define matchfinder_rebase matchfinder_rebase_rvv

#endif /* ARCH_RISCV && __riscv_vector */

#endif /* LIB_RISCV_MATCHFINDER_IMPL_H */

/*** End of inlined file: matchfinder_impl.h ***/


#  elif defined(ARCH_X86_32) || defined(ARCH_X86_64)

/*** Start of inlined file: matchfinder_impl.h ***/
#ifndef LIB_X86_MATCHFINDER_IMPL_H
#define LIB_X86_MATCHFINDER_IMPL_H

#ifdef __AVX2__
static forceinline void
matchfinder_init_avx2(mf_pos_t *data, size_t size)
{
	__m256i *p = (__m256i *)data;
	__m256i v = _mm256_set1_epi16(MATCHFINDER_INITVAL);

	STATIC_ASSERT(MATCHFINDER_MEM_ALIGNMENT % sizeof(*p) == 0);
	STATIC_ASSERT(MATCHFINDER_SIZE_ALIGNMENT % (4 * sizeof(*p)) == 0);
	STATIC_ASSERT(sizeof(mf_pos_t) == 2);

	do {
		p[0] = v;
		p[1] = v;
		p[2] = v;
		p[3] = v;
		p += 4;
		size -= 4 * sizeof(*p);
	} while (size != 0);
}
#define matchfinder_init matchfinder_init_avx2

static forceinline void
matchfinder_rebase_avx2(mf_pos_t *data, size_t size)
{
	__m256i *p = (__m256i *)data;
	__m256i v = _mm256_set1_epi16((u16)-MATCHFINDER_WINDOW_SIZE);

	STATIC_ASSERT(MATCHFINDER_MEM_ALIGNMENT % sizeof(*p) == 0);
	STATIC_ASSERT(MATCHFINDER_SIZE_ALIGNMENT % (4 * sizeof(*p)) == 0);
	STATIC_ASSERT(sizeof(mf_pos_t) == 2);

	do {
		/* PADDSW: Add Packed Signed Integers With Signed Saturation  */
		p[0] = _mm256_adds_epi16(p[0], v);
		p[1] = _mm256_adds_epi16(p[1], v);
		p[2] = _mm256_adds_epi16(p[2], v);
		p[3] = _mm256_adds_epi16(p[3], v);
		p += 4;
		size -= 4 * sizeof(*p);
	} while (size != 0);
}
#define matchfinder_rebase matchfinder_rebase_avx2

#elif HAVE_SSE2_NATIVE
static forceinline void
matchfinder_init_sse2(mf_pos_t *data, size_t size)
{
	__m128i *p = (__m128i *)data;
	__m128i v = _mm_set1_epi16(MATCHFINDER_INITVAL);

	STATIC_ASSERT(MATCHFINDER_MEM_ALIGNMENT % sizeof(*p) == 0);
	STATIC_ASSERT(MATCHFINDER_SIZE_ALIGNMENT % (4 * sizeof(*p)) == 0);
	STATIC_ASSERT(sizeof(mf_pos_t) == 2);

	do {
		p[0] = v;
		p[1] = v;
		p[2] = v;
		p[3] = v;
		p += 4;
		size -= 4 * sizeof(*p);
	} while (size != 0);
}
#define matchfinder_init matchfinder_init_sse2

static forceinline void
matchfinder_rebase_sse2(mf_pos_t *data, size_t size)
{
	__m128i *p = (__m128i *)data;
	__m128i v = _mm_set1_epi16((u16)-MATCHFINDER_WINDOW_SIZE);

	STATIC_ASSERT(MATCHFINDER_MEM_ALIGNMENT % sizeof(*p) == 0);
	STATIC_ASSERT(MATCHFINDER_SIZE_ALIGNMENT % (4 * sizeof(*p)) == 0);
	STATIC_ASSERT(sizeof(mf_pos_t) == 2);

	do {
		/* PADDSW: Add Packed Signed Integers With Signed Saturation  */
		p[0] = _mm_adds_epi16(p[0], v);
		p[1] = _mm_adds_epi16(p[1], v);
		p[2] = _mm_adds_epi16(p[2], v);
		p[3] = _mm_adds_epi16(p[3], v);
		p += 4;
		size -= 4 * sizeof(*p);
	} while (size != 0);
}
#define matchfinder_rebase matchfinder_rebase_sse2
#endif /* HAVE_SSE2_NATIVE */

#endif /* LIB_X86_MATCHFINDER_IMPL_H */

/*** End of inlined file: matchfinder_impl.h ***/


#  endif
#else
#  define MATCHFINDER_ALIGNED
#endif

/*
 * Initialize the hash table portion of the matchfinder.
 *
 * Essentially, this is an optimized memset().
 *
 * 'data' must be aligned to a MATCHFINDER_MEM_ALIGNMENT boundary, and
 * 'size' must be a multiple of MATCHFINDER_SIZE_ALIGNMENT.
 */
#ifndef matchfinder_init
static forceinline void
matchfinder_init(mf_pos_t *data, size_t size)
{
	size_t num_entries = size / sizeof(*data);
	size_t i;

	for (i = 0; i < num_entries; i++)
		data[i] = MATCHFINDER_INITVAL;
}
#endif

/*
 * Slide the matchfinder by MATCHFINDER_WINDOW_SIZE bytes.
 *
 * This must be called just after each MATCHFINDER_WINDOW_SIZE bytes have been
 * run through the matchfinder.
 *
 * This subtracts MATCHFINDER_WINDOW_SIZE bytes from each entry in the given
 * array, making the entries be relative to the current position rather than the
 * position MATCHFINDER_WINDOW_SIZE bytes prior.  To avoid integer underflows,
 * entries that would become less than -MATCHFINDER_WINDOW_SIZE stay at
 * -MATCHFINDER_WINDOW_SIZE, keeping them permanently out of bounds.
 *
 * The given array must contain all matchfinder data that is position-relative:
 * the hash table(s) as well as any hash chain or binary tree links.  Its
 * address must be aligned to a MATCHFINDER_MEM_ALIGNMENT boundary, and its size
 * must be a multiple of MATCHFINDER_SIZE_ALIGNMENT.
 */
#ifndef matchfinder_rebase
static forceinline void
matchfinder_rebase(mf_pos_t *data, size_t size)
{
	size_t num_entries = size / sizeof(*data);
	size_t i;

	if (MATCHFINDER_WINDOW_SIZE == 32768) {
		/*
		 * Branchless version for 32768-byte windows.  Clear all bits if
		 * the value was already negative, then set the sign bit.  This
		 * is equivalent to subtracting 32768 with signed saturation.
		 */
		for (i = 0; i < num_entries; i++)
			data[i] = 0x8000 | (data[i] & ~(data[i] >> 15));
	} else {
		for (i = 0; i < num_entries; i++) {
			if (data[i] >= 0)
				data[i] -= (mf_pos_t)-MATCHFINDER_WINDOW_SIZE;
			else
				data[i] = (mf_pos_t)-MATCHFINDER_WINDOW_SIZE;
		}
	}
}
#endif

/*
 * The hash function: given a sequence prefix held in the low-order bits of a
 * 32-bit value, multiply by a carefully-chosen large constant.  Discard any
 * bits of the product that don't fit in a 32-bit value, but take the
 * next-highest @num_bits bits of the product as the hash value, as those have
 * the most randomness.
 */
static forceinline u32
lz_hash(u32 seq, unsigned num_bits)
{
	return (u32)(seq * 0x1E35A7BD) >> (32 - num_bits);
}

/*
 * Return the number of bytes at @matchptr that match the bytes at @strptr, up
 * to a maximum of @max_len.  Initially, @start_len bytes are matched.
 */
static forceinline u32
lz_extend(const u8 * const strptr, const u8 * const matchptr,
	  const u32 start_len, const u32 max_len)
{
	u32 len = start_len;
	machine_word_t v_word;

	if (UNALIGNED_ACCESS_IS_FAST) {

		if (likely(max_len - len >= 4 * WORDBYTES)) {

		#define COMPARE_WORD_STEP				\
			v_word = load_word_unaligned(&matchptr[len]) ^	\
				 load_word_unaligned(&strptr[len]);	\
			if (v_word != 0)				\
				goto word_differs;			\
			len += WORDBYTES;				\

			COMPARE_WORD_STEP
			COMPARE_WORD_STEP
			COMPARE_WORD_STEP
			COMPARE_WORD_STEP
		#undef COMPARE_WORD_STEP
		}

		while (len + WORDBYTES <= max_len) {
			v_word = load_word_unaligned(&matchptr[len]) ^
				 load_word_unaligned(&strptr[len]);
			if (v_word != 0)
				goto word_differs;
			len += WORDBYTES;
		}
	}

	while (len < max_len && matchptr[len] == strptr[len])
		len++;
	return len;

word_differs:
	if (CPU_IS_LITTLE_ENDIAN())
		len += (bsfw(v_word) >> 3);
	else
		len += (WORDBITS - 1 - bsrw(v_word)) >> 3;
	return len;
}

#endif /* LIB_MATCHFINDER_COMMON_H */

/*** End of inlined file: matchfinder_common.h ***/

#define HC_MATCHFINDER_HASH3_ORDER	15
#define HC_MATCHFINDER_HASH4_ORDER	16

#define HC_MATCHFINDER_TOTAL_HASH_SIZE			\
	(((1UL << HC_MATCHFINDER_HASH3_ORDER) +		\
	  (1UL << HC_MATCHFINDER_HASH4_ORDER)) * sizeof(mf_pos_t))

struct MATCHFINDER_ALIGNED hc_matchfinder  {

	/* The hash table for finding length 3 matches  */
	mf_pos_t hash3_tab[1UL << HC_MATCHFINDER_HASH3_ORDER];

	/* The hash table which contains the first nodes of the linked lists for
	 * finding length 4+ matches  */
	mf_pos_t hash4_tab[1UL << HC_MATCHFINDER_HASH4_ORDER];

	/* The "next node" references for the linked lists.  The "next node" of
	 * the node for the sequence with position 'pos' is 'next_tab[pos]'.  */
	mf_pos_t next_tab[MATCHFINDER_WINDOW_SIZE];
};

/* Prepare the matchfinder for a new input buffer.  */
static forceinline void
hc_matchfinder_init(struct hc_matchfinder *mf)
{
	STATIC_ASSERT(HC_MATCHFINDER_TOTAL_HASH_SIZE %
		      MATCHFINDER_SIZE_ALIGNMENT == 0);

	matchfinder_init((mf_pos_t *)mf, HC_MATCHFINDER_TOTAL_HASH_SIZE);
}

static forceinline void
hc_matchfinder_slide_window(struct hc_matchfinder *mf)
{
	STATIC_ASSERT(sizeof(*mf) % MATCHFINDER_SIZE_ALIGNMENT == 0);

	matchfinder_rebase((mf_pos_t *)mf, sizeof(*mf));
}

/*
 * Find the longest match longer than 'best_len' bytes.
 *
 * @mf
 *	The matchfinder structure.
 * @in_base_p
 *	Location of a pointer which points to the place in the input data the
 *	matchfinder currently stores positions relative to.  This may be updated
 *	by this function.
 * @in_next
 *	Pointer to the next position in the input buffer, i.e. the sequence
 *	being matched against.
 * @best_len
 *	Require a match longer than this length.
 * @max_len
 *	The maximum permissible match length at this position.
 * @nice_len
 *	Stop searching if a match of at least this length is found.
 *	Must be <= @max_len.
 * @max_search_depth
 *	Limit on the number of potential matches to consider.  Must be >= 1.
 * @next_hashes
 *	The precomputed hash codes for the sequence beginning at @in_next.
 *	These will be used and then updated with the precomputed hashcodes for
 *	the sequence beginning at @in_next + 1.
 * @offset_ret
 *	If a match is found, its offset is returned in this location.
 *
 * Return the length of the match found, or 'best_len' if no match longer than
 * 'best_len' was found.
 */
static forceinline u32
hc_matchfinder_longest_match(struct hc_matchfinder * const mf,
			     const u8 ** const in_base_p,
			     const u8 * const in_next,
			     u32 best_len,
			     const u32 max_len,
			     const u32 nice_len,
			     const u32 max_search_depth,
			     u32 * const next_hashes,
			     u32 * const offset_ret)
{
	u32 depth_remaining = max_search_depth;
	const u8 *best_matchptr = in_next;
	mf_pos_t cur_node3, cur_node4;
	u32 hash3, hash4;
	u32 next_hashseq;
	u32 seq4;
	const u8 *matchptr;
	u32 len;
	u32 cur_pos = in_next - *in_base_p;
	const u8 *in_base;
	mf_pos_t cutoff;

	if (cur_pos == MATCHFINDER_WINDOW_SIZE) {
		hc_matchfinder_slide_window(mf);
		*in_base_p += MATCHFINDER_WINDOW_SIZE;
		cur_pos = 0;
	}

	in_base = *in_base_p;
	cutoff = cur_pos - MATCHFINDER_WINDOW_SIZE;

	if (unlikely(max_len < 5)) /* can we read 4 bytes from 'in_next + 1'? */
		goto out;

	/* Get the precomputed hash codes.  */
	hash3 = next_hashes[0];
	hash4 = next_hashes[1];

	/* From the hash buckets, get the first node of each linked list.  */
	cur_node3 = mf->hash3_tab[hash3];
	cur_node4 = mf->hash4_tab[hash4];

	/* Update for length 3 matches.  This replaces the singleton node in the
	 * 'hash3' bucket with the node for the current sequence.  */
	mf->hash3_tab[hash3] = cur_pos;

	/* Update for length 4 matches.  This prepends the node for the current
	 * sequence to the linked list in the 'hash4' bucket.  */
	mf->hash4_tab[hash4] = cur_pos;
	mf->next_tab[cur_pos] = cur_node4;

	/* Compute the next hash codes.  */
	next_hashseq = get_unaligned_le32(in_next + 1);
	next_hashes[0] = lz_hash(next_hashseq & 0xFFFFFF, HC_MATCHFINDER_HASH3_ORDER);
	next_hashes[1] = lz_hash(next_hashseq, HC_MATCHFINDER_HASH4_ORDER);
	prefetchw(&mf->hash3_tab[next_hashes[0]]);
	prefetchw(&mf->hash4_tab[next_hashes[1]]);

	if (best_len < 4) {  /* No match of length >= 4 found yet?  */

		/* Check for a length 3 match if needed.  */

		if (cur_node3 <= cutoff)
			goto out;

		seq4 = load_u32_unaligned(in_next);

		if (best_len < 3) {
			matchptr = &in_base[cur_node3];
			if (load_u24_unaligned(matchptr) == loaded_u32_to_u24(seq4)) {
				best_len = 3;
				best_matchptr = matchptr;
			}
		}

		/* Check for a length 4 match.  */

		if (cur_node4 <= cutoff)
			goto out;

		for (;;) {
			/* No length 4 match found yet.  Check the first 4 bytes.  */
			matchptr = &in_base[cur_node4];

			if (load_u32_unaligned(matchptr) == seq4)
				break;

			/* The first 4 bytes did not match.  Keep trying.  */
			cur_node4 = mf->next_tab[cur_node4 & (MATCHFINDER_WINDOW_SIZE - 1)];
			if (cur_node4 <= cutoff || !--depth_remaining)
				goto out;
		}

		/* Found a match of length >= 4.  Extend it to its full length.  */
		best_matchptr = matchptr;
		best_len = lz_extend(in_next, best_matchptr, 4, max_len);
		if (best_len >= nice_len)
			goto out;
		cur_node4 = mf->next_tab[cur_node4 & (MATCHFINDER_WINDOW_SIZE - 1)];
		if (cur_node4 <= cutoff || !--depth_remaining)
			goto out;
	} else {
		if (cur_node4 <= cutoff || best_len >= nice_len)
			goto out;
	}

	/* Check for matches of length >= 5.  */

	for (;;) {
		for (;;) {
			matchptr = &in_base[cur_node4];

			/* Already found a length 4 match.  Try for a longer
			 * match; start by checking either the last 4 bytes and
			 * the first 4 bytes, or the last byte.  (The last byte,
			 * the one which would extend the match length by 1, is
			 * the most important.)  */
		#if UNALIGNED_ACCESS_IS_FAST
			if ((load_u32_unaligned(matchptr + best_len - 3) ==
			     load_u32_unaligned(in_next + best_len - 3)) &&
			    (load_u32_unaligned(matchptr) ==
			     load_u32_unaligned(in_next)))
		#else
			if (matchptr[best_len] == in_next[best_len])
		#endif
				break;

			/* Continue to the next node in the list.  */
			cur_node4 = mf->next_tab[cur_node4 & (MATCHFINDER_WINDOW_SIZE - 1)];
			if (cur_node4 <= cutoff || !--depth_remaining)
				goto out;
		}

	#if UNALIGNED_ACCESS_IS_FAST
		len = 4;
	#else
		len = 0;
	#endif
		len = lz_extend(in_next, matchptr, len, max_len);
		if (len > best_len) {
			/* This is the new longest match.  */
			best_len = len;
			best_matchptr = matchptr;
			if (best_len >= nice_len)
				goto out;
		}

		/* Continue to the next node in the list.  */
		cur_node4 = mf->next_tab[cur_node4 & (MATCHFINDER_WINDOW_SIZE - 1)];
		if (cur_node4 <= cutoff || !--depth_remaining)
			goto out;
	}
out:
	*offset_ret = in_next - best_matchptr;
	return best_len;
}

/*
 * Advance the matchfinder, but don't search for matches.
 *
 * @mf
 *	The matchfinder structure.
 * @in_base_p
 *	Location of a pointer which points to the place in the input data the
 *	matchfinder currently stores positions relative to.  This may be updated
 *	by this function.
 * @in_next
 *	Pointer to the next position in the input buffer.
 * @in_end
 *	Pointer to the end of the input buffer.
 * @count
 *	The number of bytes to advance.  Must be > 0.
 * @next_hashes
 *	The precomputed hash codes for the sequence beginning at @in_next.
 *	These will be used and then updated with the precomputed hashcodes for
 *	the sequence beginning at @in_next + @count.
 */
static forceinline void
hc_matchfinder_skip_bytes(struct hc_matchfinder * const mf,
			  const u8 ** const in_base_p,
			  const u8 *in_next,
			  const u8 * const in_end,
			  const u32 count,
			  u32 * const next_hashes)
{
	u32 cur_pos;
	u32 hash3, hash4;
	u32 next_hashseq;
	u32 remaining = count;

	if (unlikely(count + 5 > in_end - in_next))
		return;

	cur_pos = in_next - *in_base_p;
	hash3 = next_hashes[0];
	hash4 = next_hashes[1];
	do {
		if (cur_pos == MATCHFINDER_WINDOW_SIZE) {
			hc_matchfinder_slide_window(mf);
			*in_base_p += MATCHFINDER_WINDOW_SIZE;
			cur_pos = 0;
		}
		mf->hash3_tab[hash3] = cur_pos;
		mf->next_tab[cur_pos] = mf->hash4_tab[hash4];
		mf->hash4_tab[hash4] = cur_pos;

		next_hashseq = get_unaligned_le32(++in_next);
		hash3 = lz_hash(next_hashseq & 0xFFFFFF, HC_MATCHFINDER_HASH3_ORDER);
		hash4 = lz_hash(next_hashseq, HC_MATCHFINDER_HASH4_ORDER);
		cur_pos++;
	} while (--remaining);

	prefetchw(&mf->hash3_tab[hash3]);
	prefetchw(&mf->hash4_tab[hash4]);
	next_hashes[0] = hash3;
	next_hashes[1] = hash4;
}

#endif /* LIB_HC_MATCHFINDER_H */

/*** End of inlined file: hc_matchfinder.h ***/



/*** Start of inlined file: ht_matchfinder.h ***/
#ifndef LIB_HT_MATCHFINDER_H
#define LIB_HT_MATCHFINDER_H

#define HT_MATCHFINDER_HASH_ORDER	15
#define HT_MATCHFINDER_BUCKET_SIZE	2

#define HT_MATCHFINDER_MIN_MATCH_LEN	4
/* Minimum value of max_len for ht_matchfinder_longest_match() */
#define HT_MATCHFINDER_REQUIRED_NBYTES	5

struct MATCHFINDER_ALIGNED ht_matchfinder {
	mf_pos_t hash_tab[1UL << HT_MATCHFINDER_HASH_ORDER]
			 [HT_MATCHFINDER_BUCKET_SIZE];
};

static forceinline void
ht_matchfinder_init(struct ht_matchfinder *mf)
{
	STATIC_ASSERT(sizeof(*mf) % MATCHFINDER_SIZE_ALIGNMENT == 0);

	matchfinder_init((mf_pos_t *)mf, sizeof(*mf));
}

static forceinline void
ht_matchfinder_slide_window(struct ht_matchfinder *mf)
{
	matchfinder_rebase((mf_pos_t *)mf, sizeof(*mf));
}

/* Note: max_len must be >= HT_MATCHFINDER_REQUIRED_NBYTES */
static forceinline u32
ht_matchfinder_longest_match(struct ht_matchfinder * const mf,
			     const u8 ** const in_base_p,
			     const u8 * const in_next,
			     const u32 max_len,
			     const u32 nice_len,
			     u32 * const next_hash,
			     u32 * const offset_ret)
{
	u32 best_len = 0;
	const u8 *best_matchptr = in_next;
	u32 cur_pos = in_next - *in_base_p;
	const u8 *in_base;
	mf_pos_t cutoff;
	u32 hash;
	u32 seq;
	mf_pos_t cur_node;
	const u8 *matchptr;
#if HT_MATCHFINDER_BUCKET_SIZE > 1
	mf_pos_t to_insert;
	u32 len;
#endif
#if HT_MATCHFINDER_BUCKET_SIZE > 2
	int i;
#endif

	/* This is assumed throughout this function. */
	STATIC_ASSERT(HT_MATCHFINDER_MIN_MATCH_LEN == 4);

	if (cur_pos == MATCHFINDER_WINDOW_SIZE) {
		ht_matchfinder_slide_window(mf);
		*in_base_p += MATCHFINDER_WINDOW_SIZE;
		cur_pos = 0;
	}
	in_base = *in_base_p;
	cutoff = cur_pos - MATCHFINDER_WINDOW_SIZE;

	hash = *next_hash;
	STATIC_ASSERT(HT_MATCHFINDER_REQUIRED_NBYTES == 5);
	*next_hash = lz_hash(get_unaligned_le32(in_next + 1),
			     HT_MATCHFINDER_HASH_ORDER);
	seq = load_u32_unaligned(in_next);
	prefetchw(&mf->hash_tab[*next_hash]);
#if HT_MATCHFINDER_BUCKET_SIZE == 1
	/* Hand-unrolled version for BUCKET_SIZE == 1 */
	cur_node = mf->hash_tab[hash][0];
	mf->hash_tab[hash][0] = cur_pos;
	if (cur_node <= cutoff)
		goto out;
	matchptr = &in_base[cur_node];
	if (load_u32_unaligned(matchptr) == seq) {
		best_len = lz_extend(in_next, matchptr, 4, max_len);
		best_matchptr = matchptr;
	}
#elif HT_MATCHFINDER_BUCKET_SIZE == 2
	/*
	 * Hand-unrolled version for BUCKET_SIZE == 2.  The logic here also
	 * differs slightly in that it copies the first entry to the second even
	 * if nice_len is reached on the first, as this can be slightly faster.
	 */
	cur_node = mf->hash_tab[hash][0];
	mf->hash_tab[hash][0] = cur_pos;
	if (cur_node <= cutoff)
		goto out;
	matchptr = &in_base[cur_node];

	to_insert = cur_node;
	cur_node = mf->hash_tab[hash][1];
	mf->hash_tab[hash][1] = to_insert;

	if (load_u32_unaligned(matchptr) == seq) {
		best_len = lz_extend(in_next, matchptr, 4, max_len);
		best_matchptr = matchptr;
		if (cur_node <= cutoff || best_len >= nice_len)
			goto out;
		matchptr = &in_base[cur_node];
		if (load_u32_unaligned(matchptr) == seq &&
		    load_u32_unaligned(matchptr + best_len - 3) ==
		    load_u32_unaligned(in_next + best_len - 3)) {
			len = lz_extend(in_next, matchptr, 4, max_len);
			if (len > best_len) {
				best_len = len;
				best_matchptr = matchptr;
			}
		}
	} else {
		if (cur_node <= cutoff)
			goto out;
		matchptr = &in_base[cur_node];
		if (load_u32_unaligned(matchptr) == seq) {
			best_len = lz_extend(in_next, matchptr, 4, max_len);
			best_matchptr = matchptr;
		}
	}
#else
	/* Generic version for HT_MATCHFINDER_BUCKET_SIZE > 2 */
	to_insert = cur_pos;
	for (i = 0; i < HT_MATCHFINDER_BUCKET_SIZE; i++) {
		cur_node = mf->hash_tab[hash][i];
		mf->hash_tab[hash][i] = to_insert;
		if (cur_node <= cutoff)
			goto out;
		matchptr = &in_base[cur_node];
		if (load_u32_unaligned(matchptr) == seq) {
			len = lz_extend(in_next, matchptr, 4, max_len);
			if (len > best_len) {
				best_len = len;
				best_matchptr = matchptr;
				if (best_len >= nice_len)
					goto out;
			}
		}
		to_insert = cur_node;
	}
#endif
out:
	*offset_ret = in_next - best_matchptr;
	return best_len;
}

static forceinline void
ht_matchfinder_skip_bytes(struct ht_matchfinder * const mf,
			  const u8 ** const in_base_p,
			  const u8 *in_next,
			  const u8 * const in_end,
			  const u32 count,
			  u32 * const next_hash)
{
	s32 cur_pos = in_next - *in_base_p;
	u32 hash;
	u32 remaining = count;
	int i;

	if (unlikely(count + HT_MATCHFINDER_REQUIRED_NBYTES > in_end - in_next))
		return;

	if (cur_pos + count - 1 >= MATCHFINDER_WINDOW_SIZE) {
		ht_matchfinder_slide_window(mf);
		*in_base_p += MATCHFINDER_WINDOW_SIZE;
		cur_pos -= MATCHFINDER_WINDOW_SIZE;
	}

	hash = *next_hash;
	do {
		for (i = HT_MATCHFINDER_BUCKET_SIZE - 1; i > 0; i--)
			mf->hash_tab[hash][i] = mf->hash_tab[hash][i - 1];
		mf->hash_tab[hash][0] = cur_pos;

		hash = lz_hash(get_unaligned_le32(++in_next),
			       HT_MATCHFINDER_HASH_ORDER);
		cur_pos++;
	} while (--remaining);

	prefetchw(&mf->hash_tab[hash]);
	*next_hash = hash;
}

#endif /* LIB_HT_MATCHFINDER_H */

/*** End of inlined file: ht_matchfinder.h ***/

#if SUPPORT_NEAR_OPTIMAL_PARSING

/*** Start of inlined file: bt_matchfinder.h ***/
#ifndef LIB_BT_MATCHFINDER_H
#define LIB_BT_MATCHFINDER_H

#define BT_MATCHFINDER_HASH3_ORDER 16
#define BT_MATCHFINDER_HASH3_WAYS  2
#define BT_MATCHFINDER_HASH4_ORDER 16

#define BT_MATCHFINDER_TOTAL_HASH_SIZE		\
	(((1UL << BT_MATCHFINDER_HASH3_ORDER) * BT_MATCHFINDER_HASH3_WAYS + \
	  (1UL << BT_MATCHFINDER_HASH4_ORDER)) * sizeof(mf_pos_t))

/* Representation of a match found by the bt_matchfinder  */
struct lz_match {

	/* The number of bytes matched.  */
	u16 length;

	/* The offset back from the current position that was matched.  */
	u16 offset;
};

struct MATCHFINDER_ALIGNED bt_matchfinder {

	/* The hash table for finding length 3 matches  */
	mf_pos_t hash3_tab[1UL << BT_MATCHFINDER_HASH3_ORDER][BT_MATCHFINDER_HASH3_WAYS];

	/* The hash table which contains the roots of the binary trees for
	 * finding length 4+ matches  */
	mf_pos_t hash4_tab[1UL << BT_MATCHFINDER_HASH4_ORDER];

	/* The child node references for the binary trees.  The left and right
	 * children of the node for the sequence with position 'pos' are
	 * 'child_tab[pos * 2]' and 'child_tab[pos * 2 + 1]', respectively.  */
	mf_pos_t child_tab[2UL * MATCHFINDER_WINDOW_SIZE];
};

/* Prepare the matchfinder for a new input buffer.  */
static forceinline void
bt_matchfinder_init(struct bt_matchfinder *mf)
{
	STATIC_ASSERT(BT_MATCHFINDER_TOTAL_HASH_SIZE %
		      MATCHFINDER_SIZE_ALIGNMENT == 0);

	matchfinder_init((mf_pos_t *)mf, BT_MATCHFINDER_TOTAL_HASH_SIZE);
}

static forceinline void
bt_matchfinder_slide_window(struct bt_matchfinder *mf)
{
	STATIC_ASSERT(sizeof(*mf) % MATCHFINDER_SIZE_ALIGNMENT == 0);

	matchfinder_rebase((mf_pos_t *)mf, sizeof(*mf));
}

static forceinline mf_pos_t *
bt_left_child(struct bt_matchfinder *mf, s32 node)
{
	return &mf->child_tab[2 * (node & (MATCHFINDER_WINDOW_SIZE - 1)) + 0];
}

static forceinline mf_pos_t *
bt_right_child(struct bt_matchfinder *mf, s32 node)
{
	return &mf->child_tab[2 * (node & (MATCHFINDER_WINDOW_SIZE - 1)) + 1];
}

/* The minimum permissible value of 'max_len' for bt_matchfinder_get_matches()
 * and bt_matchfinder_skip_byte().  There must be sufficiently many bytes
 * remaining to load a 32-bit integer from the *next* position.  */
#define BT_MATCHFINDER_REQUIRED_NBYTES	5

/* Advance the binary tree matchfinder by one byte, optionally recording
 * matches.  @record_matches should be a compile-time constant.  */
static forceinline struct lz_match *
bt_matchfinder_advance_one_byte(struct bt_matchfinder * const mf,
				const u8 * const in_base,
				const ptrdiff_t cur_pos,
				const u32 max_len,
				const u32 nice_len,
				const u32 max_search_depth,
				u32 * const next_hashes,
				struct lz_match *lz_matchptr,
				const bool record_matches)
{
	const u8 *in_next = in_base + cur_pos;
	u32 depth_remaining = max_search_depth;
	const s32 cutoff = cur_pos - MATCHFINDER_WINDOW_SIZE;
	u32 next_hashseq;
	u32 hash3;
	u32 hash4;
	s32 cur_node;
#if BT_MATCHFINDER_HASH3_WAYS >= 2
	s32 cur_node_2;
#endif
	const u8 *matchptr;
	mf_pos_t *pending_lt_ptr, *pending_gt_ptr;
	u32 best_lt_len, best_gt_len;
	u32 len;
	u32 best_len = 3;

	STATIC_ASSERT(BT_MATCHFINDER_HASH3_WAYS >= 1 &&
		      BT_MATCHFINDER_HASH3_WAYS <= 2);

	next_hashseq = get_unaligned_le32(in_next + 1);

	hash3 = next_hashes[0];
	hash4 = next_hashes[1];

	next_hashes[0] = lz_hash(next_hashseq & 0xFFFFFF, BT_MATCHFINDER_HASH3_ORDER);
	next_hashes[1] = lz_hash(next_hashseq, BT_MATCHFINDER_HASH4_ORDER);
	prefetchw(&mf->hash3_tab[next_hashes[0]]);
	prefetchw(&mf->hash4_tab[next_hashes[1]]);

	cur_node = mf->hash3_tab[hash3][0];
	mf->hash3_tab[hash3][0] = cur_pos;
#if BT_MATCHFINDER_HASH3_WAYS >= 2
	cur_node_2 = mf->hash3_tab[hash3][1];
	mf->hash3_tab[hash3][1] = cur_node;
#endif
	if (record_matches && cur_node > cutoff) {
		u32 seq3 = load_u24_unaligned(in_next);
		if (seq3 == load_u24_unaligned(&in_base[cur_node])) {
			lz_matchptr->length = 3;
			lz_matchptr->offset = in_next - &in_base[cur_node];
			lz_matchptr++;
		}
	#if BT_MATCHFINDER_HASH3_WAYS >= 2
		else if (cur_node_2 > cutoff &&
			seq3 == load_u24_unaligned(&in_base[cur_node_2]))
		{
			lz_matchptr->length = 3;
			lz_matchptr->offset = in_next - &in_base[cur_node_2];
			lz_matchptr++;
		}
	#endif
	}

	cur_node = mf->hash4_tab[hash4];
	mf->hash4_tab[hash4] = cur_pos;

	pending_lt_ptr = bt_left_child(mf, cur_pos);
	pending_gt_ptr = bt_right_child(mf, cur_pos);

	if (cur_node <= cutoff) {
		*pending_lt_ptr = MATCHFINDER_INITVAL;
		*pending_gt_ptr = MATCHFINDER_INITVAL;
		return lz_matchptr;
	}

	best_lt_len = 0;
	best_gt_len = 0;
	len = 0;

	for (;;) {
		matchptr = &in_base[cur_node];

		if (matchptr[len] == in_next[len]) {
			len = lz_extend(in_next, matchptr, len + 1, max_len);
			if (!record_matches || len > best_len) {
				if (record_matches) {
					best_len = len;
					lz_matchptr->length = len;
					lz_matchptr->offset = in_next - matchptr;
					lz_matchptr++;
				}
				if (len >= nice_len) {
					*pending_lt_ptr = *bt_left_child(mf, cur_node);
					*pending_gt_ptr = *bt_right_child(mf, cur_node);
					return lz_matchptr;
				}
			}
		}

		if (matchptr[len] < in_next[len]) {
			*pending_lt_ptr = cur_node;
			pending_lt_ptr = bt_right_child(mf, cur_node);
			cur_node = *pending_lt_ptr;
			best_lt_len = len;
			if (best_gt_len < len)
				len = best_gt_len;
		} else {
			*pending_gt_ptr = cur_node;
			pending_gt_ptr = bt_left_child(mf, cur_node);
			cur_node = *pending_gt_ptr;
			best_gt_len = len;
			if (best_lt_len < len)
				len = best_lt_len;
		}

		if (cur_node <= cutoff || !--depth_remaining) {
			*pending_lt_ptr = MATCHFINDER_INITVAL;
			*pending_gt_ptr = MATCHFINDER_INITVAL;
			return lz_matchptr;
		}
	}
}

/*
 * Retrieve a list of matches with the current position.
 *
 * @mf
 *	The matchfinder structure.
 * @in_base
 *	Pointer to the next byte in the input buffer to process _at the last
 *	time bt_matchfinder_init() or bt_matchfinder_slide_window() was called_.
 * @cur_pos
 *	The current position in the input buffer relative to @in_base (the
 *	position of the sequence being matched against).
 * @max_len
 *	The maximum permissible match length at this position.  Must be >=
 *	BT_MATCHFINDER_REQUIRED_NBYTES.
 * @nice_len
 *	Stop searching if a match of at least this length is found.
 *	Must be <= @max_len.
 * @max_search_depth
 *	Limit on the number of potential matches to consider.  Must be >= 1.
 * @next_hashes
 *	The precomputed hash codes for the sequence beginning at @in_next.
 *	These will be used and then updated with the precomputed hashcodes for
 *	the sequence beginning at @in_next + 1.
 * @lz_matchptr
 *	An array in which this function will record the matches.  The recorded
 *	matches will be sorted by strictly increasing length and (non-strictly)
 *	increasing offset.  The maximum number of matches that may be found is
 *	'nice_len - 2'.
 *
 * The return value is a pointer to the next available slot in the @lz_matchptr
 * array.  (If no matches were found, this will be the same as @lz_matchptr.)
 */
static forceinline struct lz_match *
bt_matchfinder_get_matches(struct bt_matchfinder *mf,
			   const u8 *in_base,
			   ptrdiff_t cur_pos,
			   u32 max_len,
			   u32 nice_len,
			   u32 max_search_depth,
			   u32 next_hashes[2],
			   struct lz_match *lz_matchptr)
{
	return bt_matchfinder_advance_one_byte(mf,
					       in_base,
					       cur_pos,
					       max_len,
					       nice_len,
					       max_search_depth,
					       next_hashes,
					       lz_matchptr,
					       true);
}

/*
 * Advance the matchfinder, but don't record any matches.
 *
 * This is very similar to bt_matchfinder_get_matches() because both functions
 * must do hashing and tree re-rooting.
 */
static forceinline void
bt_matchfinder_skip_byte(struct bt_matchfinder *mf,
			 const u8 *in_base,
			 ptrdiff_t cur_pos,
			 u32 nice_len,
			 u32 max_search_depth,
			 u32 next_hashes[2])
{
	bt_matchfinder_advance_one_byte(mf,
					in_base,
					cur_pos,
					nice_len,
					nice_len,
					max_search_depth,
					next_hashes,
					NULL,
					false);
}

#endif /* LIB_BT_MATCHFINDER_H */

/*** End of inlined file: bt_matchfinder.h ***/


/*
 * This is the maximum number of matches the binary trees matchfinder can find
 * at a single position.  Since the matchfinder never finds more than one match
 * for the same length, presuming one of each possible length is sufficient for
 * an upper bound.  (This says nothing about whether it is worthwhile to
 * consider so many matches; this is just defining the worst case.)
 */
#define MAX_MATCHES_PER_POS	\
	(DEFLATE_MAX_MATCH_LEN - DEFLATE_MIN_MATCH_LEN + 1)
#endif

/*
 * The largest block length we will ever use is when the final block is of
 * length SOFT_MAX_BLOCK_LENGTH + MIN_BLOCK_LENGTH - 1, or when any block is of
 * length SOFT_MAX_BLOCK_LENGTH + 1 + DEFLATE_MAX_MATCH_LEN.  The latter case
 * occurs when the lazy2 compressor chooses two literals and a maximum-length
 * match, starting at SOFT_MAX_BLOCK_LENGTH - 1.
 */
#define MAX_BLOCK_LENGTH	\
	MAX(SOFT_MAX_BLOCK_LENGTH + MIN_BLOCK_LENGTH - 1,	\
	    SOFT_MAX_BLOCK_LENGTH + 1 + DEFLATE_MAX_MATCH_LEN)

static forceinline void
check_buildtime_parameters(void)
{
	/*
	 * Verify that MIN_BLOCK_LENGTH is being honored, as
	 * libdeflate_deflate_compress_bound() depends on it.
	 */
	STATIC_ASSERT(SOFT_MAX_BLOCK_LENGTH >= MIN_BLOCK_LENGTH);
	STATIC_ASSERT(FAST_SOFT_MAX_BLOCK_LENGTH >= MIN_BLOCK_LENGTH);
	STATIC_ASSERT(SEQ_STORE_LENGTH * DEFLATE_MIN_MATCH_LEN >=
		      MIN_BLOCK_LENGTH);
	STATIC_ASSERT(FAST_SEQ_STORE_LENGTH * HT_MATCHFINDER_MIN_MATCH_LEN >=
		      MIN_BLOCK_LENGTH);
#if SUPPORT_NEAR_OPTIMAL_PARSING
	STATIC_ASSERT(MIN_BLOCK_LENGTH * MAX_MATCHES_PER_POS <=
		      MATCH_CACHE_LENGTH);
#endif

	/* The definition of MAX_BLOCK_LENGTH assumes this. */
	STATIC_ASSERT(FAST_SOFT_MAX_BLOCK_LENGTH <= SOFT_MAX_BLOCK_LENGTH);

	/* Verify that the sequence stores aren't uselessly large. */
	STATIC_ASSERT(SEQ_STORE_LENGTH * DEFLATE_MIN_MATCH_LEN <=
		      SOFT_MAX_BLOCK_LENGTH + MIN_BLOCK_LENGTH);
	STATIC_ASSERT(FAST_SEQ_STORE_LENGTH * HT_MATCHFINDER_MIN_MATCH_LEN <=
		      FAST_SOFT_MAX_BLOCK_LENGTH + MIN_BLOCK_LENGTH);

	/* Verify that the maximum codeword lengths are valid. */
	STATIC_ASSERT(
		MAX_LITLEN_CODEWORD_LEN <= DEFLATE_MAX_LITLEN_CODEWORD_LEN);
	STATIC_ASSERT(
		MAX_OFFSET_CODEWORD_LEN <= DEFLATE_MAX_OFFSET_CODEWORD_LEN);
	STATIC_ASSERT(
		MAX_PRE_CODEWORD_LEN <= DEFLATE_MAX_PRE_CODEWORD_LEN);
	STATIC_ASSERT(
		(1U << MAX_LITLEN_CODEWORD_LEN) >= DEFLATE_NUM_LITLEN_SYMS);
	STATIC_ASSERT(
		(1U << MAX_OFFSET_CODEWORD_LEN) >= DEFLATE_NUM_OFFSET_SYMS);
	STATIC_ASSERT(
		(1U << MAX_PRE_CODEWORD_LEN) >= DEFLATE_NUM_PRECODE_SYMS);
}

/******************************************************************************/

/* Table: length slot => length slot base value */
static const u32 deflate_length_slot_base[] = {
	3,    4,    5,    6,    7,    8,    9,    10,
	11,   13,   15,   17,   19,   23,   27,   31,
	35,   43,   51,   59,   67,   83,   99,   115,
	131,  163,  195,  227,  258,
};

/* Table: length slot => number of extra length bits */
static const u8 deflate_extra_length_bits[] = {
	0,    0,    0,    0,    0,    0,    0,    0,
	1,    1,    1,    1,    2,    2,    2,    2,
	3,    3,    3,    3,    4,    4,    4,    4,
	5,    5,    5,    5,    0,
};

/* Table: offset slot => offset slot base value */
static const u32 deflate_offset_slot_base[] = {
	1,     2,     3,     4,     5,     7,     9,     13,
	17,    25,    33,    49,    65,    97,    129,   193,
	257,   385,   513,   769,   1025,  1537,  2049,  3073,
	4097,  6145,  8193,  12289, 16385, 24577,
};

/* Table: offset slot => number of extra offset bits */
static const u8 deflate_extra_offset_bits[] = {
	0,     0,     0,     0,     1,     1,     2,     2,
	3,     3,     4,     4,     5,     5,     6,     6,
	7,     7,     8,     8,     9,     9,     10,    10,
	11,    11,    12,    12,    13,    13,
};

/* Table: length => length slot */
static const u8 deflate_length_slot[DEFLATE_MAX_MATCH_LEN + 1] = {
	0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 12,
	12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 16,
	16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18,
	18, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
	21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25,
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
	27, 27, 28,
};

/*
 * Table: 'offset - 1 => offset_slot' for offset <= 256.
 * This was generated by scripts/gen_offset_slot_map.py.
 */
static const u8 deflate_offset_slot[256] = {
	0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
};

/* The order in which precode codeword lengths are stored */
static const u8 deflate_precode_lens_permutation[DEFLATE_NUM_PRECODE_SYMS] = {
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

/* Table: precode symbol => number of extra bits */
static const u8 deflate_extra_precode_bits[DEFLATE_NUM_PRECODE_SYMS] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 7
};

/* Codewords for the DEFLATE Huffman codes */
struct deflate_codewords {
	u32 litlen[DEFLATE_NUM_LITLEN_SYMS];
	u32 offset[DEFLATE_NUM_OFFSET_SYMS];
};

/*
 * Codeword lengths (in bits) for the DEFLATE Huffman codes.
 * A zero length means the corresponding symbol had zero frequency.
 */
struct deflate_lens {
	u8 litlen[DEFLATE_NUM_LITLEN_SYMS];
	u8 offset[DEFLATE_NUM_OFFSET_SYMS];
};

/* Codewords and lengths for the DEFLATE Huffman codes */
struct deflate_codes {
	struct deflate_codewords codewords;
	struct deflate_lens lens;
};

/* Symbol frequency counters for the DEFLATE Huffman codes */
struct deflate_freqs {
	u32 litlen[DEFLATE_NUM_LITLEN_SYMS];
	u32 offset[DEFLATE_NUM_OFFSET_SYMS];
};

/*
 * Represents a run of literals followed by a match or end-of-block.  This
 * struct is needed to temporarily store items chosen by the parser, since items
 * cannot be written until all items for the block have been chosen and the
 * block's Huffman codes have been computed.
 */
struct deflate_sequence {

	/*
	 * Bits 0..22: the number of literals in this run.  This may be 0 and
	 * can be at most MAX_BLOCK_LENGTH.  The literals are not stored
	 * explicitly in this structure; instead, they are read directly from
	 * the uncompressed data.
	 *
	 * Bits 23..31: the length of the match which follows the literals, or 0
	 * if this literal run was the last in the block, so there is no match
	 * which follows it.
	 */
#define SEQ_LENGTH_SHIFT 23
#define SEQ_LITRUNLEN_MASK (((u32)1 << SEQ_LENGTH_SHIFT) - 1)
	u32 litrunlen_and_length;

	/*
	 * If 'length' doesn't indicate end-of-block, then this is the offset of
	 * the match which follows the literals.
	 */
	u16 offset;

	/*
	 * If 'length' doesn't indicate end-of-block, then this is the offset
	 * slot of the match which follows the literals.
	 */
	u16 offset_slot;
};

#if SUPPORT_NEAR_OPTIMAL_PARSING

/* Costs for the near-optimal parsing algorithm */
struct deflate_costs {

	/* The cost to output each possible literal */
	u32 literal[DEFLATE_NUM_LITERALS];

	/* The cost to output each possible match length */
	u32 length[DEFLATE_MAX_MATCH_LEN + 1];

	/* The cost to output a match offset of each possible offset slot */
	u32 offset_slot[DEFLATE_NUM_OFFSET_SYMS];
};

/*
 * This structure represents a byte position in the input data and a node in the
 * graph of possible match/literal choices for the current block.
 *
 * Logically, each incoming edge to this node is labeled with a literal or a
 * match that can be taken to reach this position from an earlier position; and
 * each outgoing edge from this node is labeled with a literal or a match that
 * can be taken to advance from this position to a later position.
 *
 * But these "edges" are actually stored elsewhere (in 'match_cache').  Here we
 * associate with each node just two pieces of information:
 *
 *	'cost_to_end' is the minimum cost to reach the end of the block from
 *	this position.
 *
 *	'item' represents the literal or match that must be chosen from here to
 *	reach the end of the block with the minimum cost.  Equivalently, this
 *	can be interpreted as the label of the outgoing edge on the minimum-cost
 *	path to the "end of block" node from this node.
 */
struct deflate_optimum_node {

	u32 cost_to_end;

	/*
	 * Notes on the match/literal representation used here:
	 *
	 *	The low bits of 'item' are the length: 1 if this is a literal,
	 *	or the match length if this is a match.
	 *
	 *	The high bits of 'item' are the actual literal byte if this is a
	 *	literal, or the match offset if this is a match.
	 */
#define OPTIMUM_OFFSET_SHIFT 9
#define OPTIMUM_LEN_MASK (((u32)1 << OPTIMUM_OFFSET_SHIFT) - 1)
	u32 item;

};

#endif /* SUPPORT_NEAR_OPTIMAL_PARSING */

/* Block split statistics.  See "Block splitting algorithm" below. */
#define NUM_LITERAL_OBSERVATION_TYPES 8
#define NUM_MATCH_OBSERVATION_TYPES 2
#define NUM_OBSERVATION_TYPES (NUM_LITERAL_OBSERVATION_TYPES + \
			       NUM_MATCH_OBSERVATION_TYPES)
#define NUM_OBSERVATIONS_PER_BLOCK_CHECK 512
struct block_split_stats {
	u32 new_observations[NUM_OBSERVATION_TYPES];
	u32 observations[NUM_OBSERVATION_TYPES];
	u32 num_new_observations;
	u32 num_observations;
};

struct deflate_output_bitstream;

/* The main DEFLATE compressor structure */
struct libdeflate_compressor {

	/* Pointer to the compress() implementation chosen at allocation time */
	void (*impl)(struct libdeflate_compressor *restrict c, const u8 *in,
		     size_t in_nbytes, struct deflate_output_bitstream *os);

	/* The free() function for this struct, chosen at allocation time */
	free_func_t free_func;

	/* The compression level with which this compressor was created */
	unsigned compression_level;

	/* Anything of this size or less we won't bother trying to compress. */
	size_t max_passthrough_size;

	/*
	 * The maximum search depth: consider at most this many potential
	 * matches at each position
	 */
	u32 max_search_depth;

	/*
	 * The "nice" match length: if a match of this length is found, choose
	 * it immediately without further consideration
	 */
	u32 nice_match_length;

	/* Frequency counters for the current block */
	struct deflate_freqs freqs;

	/* Block split statistics for the current block */
	struct block_split_stats split_stats;

	/* Dynamic Huffman codes for the current block */
	struct deflate_codes codes;

	/* The static Huffman codes defined by the DEFLATE format */
	struct deflate_codes static_codes;

	/* Temporary space for block flushing */
	union {
		/* Information about the precode */
		struct {
			u32 freqs[DEFLATE_NUM_PRECODE_SYMS];
			u32 codewords[DEFLATE_NUM_PRECODE_SYMS];
			u8 lens[DEFLATE_NUM_PRECODE_SYMS];
			unsigned items[DEFLATE_NUM_LITLEN_SYMS +
				       DEFLATE_NUM_OFFSET_SYMS];
			unsigned num_litlen_syms;
			unsigned num_offset_syms;
			unsigned num_explicit_lens;
			unsigned num_items;
		} precode;
		/*
		 * The "full" length codewords.  Used only after the information
		 * in 'precode' is no longer needed.
		 */
		struct {
			u32 codewords[DEFLATE_MAX_MATCH_LEN + 1];
			u8 lens[DEFLATE_MAX_MATCH_LEN + 1];
		} length;
	} o;

	union {
		/* Data for greedy or lazy parsing */
		struct {
			/* Hash chains matchfinder */
			struct hc_matchfinder hc_mf;

			/* Matches and literals chosen for the current block */
			struct deflate_sequence sequences[SEQ_STORE_LENGTH + 1];

		} g; /* (g)reedy */

		/* Data for fastest parsing */
		struct {
			/* Hash table matchfinder */
			struct ht_matchfinder ht_mf;

			/* Matches and literals chosen for the current block */
			struct deflate_sequence sequences[
						FAST_SEQ_STORE_LENGTH + 1];

		} f; /* (f)astest */

	#if SUPPORT_NEAR_OPTIMAL_PARSING
		/* Data for near-optimal parsing */
		struct {

			/* Binary tree matchfinder */
			struct bt_matchfinder bt_mf;

			/*
			 * Cached matches for the current block.  This array
			 * contains the matches that were found at each position
			 * in the block.  Specifically, for each position, there
			 * is a list of matches found at that position, if any,
			 * sorted by strictly increasing length.  In addition,
			 * following the matches for each position, there is a
			 * special 'struct lz_match' whose 'length' member
			 * contains the number of matches found at that
			 * position, and whose 'offset' member contains the
			 * literal at that position.
			 *
			 * Note: in rare cases, there will be a very high number
			 * of matches in the block and this array will overflow.
			 * If this happens, we force the end of the current
			 * block.  MATCH_CACHE_LENGTH is the length at which we
			 * actually check for overflow.  The extra slots beyond
			 * this are enough to absorb the worst case overflow,
			 * which occurs if starting at
			 * &match_cache[MATCH_CACHE_LENGTH - 1], we write
			 * MAX_MATCHES_PER_POS matches and a match count header,
			 * then skip searching for matches at
			 * 'DEFLATE_MAX_MATCH_LEN - 1' positions and write the
			 * match count header for each.
			 */
			struct lz_match match_cache[MATCH_CACHE_LENGTH +
						    MAX_MATCHES_PER_POS +
						    DEFLATE_MAX_MATCH_LEN - 1];

			/*
			 * Array of nodes, one per position, for running the
			 * minimum-cost path algorithm.
			 *
			 * This array must be large enough to accommodate the
			 * worst-case number of nodes, which is MAX_BLOCK_LENGTH
			 * plus 1 for the end-of-block node.
			 */
			struct deflate_optimum_node optimum_nodes[
				MAX_BLOCK_LENGTH + 1];

			/* The current cost model being used */
			struct deflate_costs costs;

			/* Saved cost model */
			struct deflate_costs costs_saved;

			/*
			 * A table that maps match offset to offset slot.  This
			 * differs from deflate_offset_slot[] in that this is a
			 * full map, not a condensed one.  The full map is more
			 * appropriate for the near-optimal parser, since the
			 * near-optimal parser does more offset => offset_slot
			 * translations, it doesn't intersperse them with
			 * matchfinding (so cache evictions are less of a
			 * concern), and it uses more memory anyway.
			 */
			u8 offset_slot_full[DEFLATE_MAX_MATCH_OFFSET + 1];

			/* Literal/match statistics saved from previous block */
			u32 prev_observations[NUM_OBSERVATION_TYPES];
			u32 prev_num_observations;

			/*
			 * Approximate match length frequencies based on a
			 * greedy parse, gathered during matchfinding.  This is
			 * used for setting the initial symbol costs.
			 */
			u32 new_match_len_freqs[DEFLATE_MAX_MATCH_LEN + 1];
			u32 match_len_freqs[DEFLATE_MAX_MATCH_LEN + 1];

			/*
			 * The maximum number of optimization passes
			 * (min-cost path searches) per block.
			 * Larger values = more compression.
			 */
			unsigned max_optim_passes;

			/*
			 * If an optimization pass improves the cost by fewer
			 * than this number of bits, then optimization will stop
			 * early, before max_optim_passes has been reached.
			 * Smaller values = more compression.
			 */
			u32 min_improvement_to_continue;

			/*
			 * The minimum number of bits that would need to be
			 * saved for it to be considered worth the time to
			 * regenerate and use the min-cost path from a previous
			 * optimization pass, in the case where the final
			 * optimization pass actually increased the cost.
			 * Smaller values = more compression.
			 */
			u32 min_bits_to_use_nonfinal_path;

			/*
			 * The maximum block length, in uncompressed bytes, at
			 * which to find and consider the optimal match/literal
			 * list for the static Huffman codes.  This strategy
			 * improves the compression ratio produced by static
			 * Huffman blocks and can discover more cases in which
			 * static blocks are worthwhile.  This helps mostly with
			 * small blocks, hence why this parameter is a max_len.
			 *
			 * Above this block length, static Huffman blocks are
			 * only used opportunistically.  I.e. a static Huffman
			 * block is only used if a static block using the same
			 * match/literal list as the optimized dynamic block
			 * happens to be cheaper than the dynamic block itself.
			 */
			u32 max_len_to_optimize_static_block;

		} n; /* (n)ear-optimal */
	#endif /* SUPPORT_NEAR_OPTIMAL_PARSING */

	} p; /* (p)arser */
};

/*
 * The type for the bitbuffer variable, which temporarily holds bits that are
 * being packed into bytes and written to the output buffer.  For best
 * performance, this should have size equal to a machine word.
 */
typedef machine_word_t bitbuf_t;

/*
 * The capacity of the bitbuffer, in bits.  This is 1 less than the real size,
 * in order to avoid undefined behavior when doing bitbuf >>= bitcount & ~7.
 */
#define BITBUF_NBITS	(8 * sizeof(bitbuf_t) - 1)

/*
 * Can the specified number of bits always be added to 'bitbuf' after any
 * pending bytes have been flushed?  There can be up to 7 bits remaining after a
 * flush, so the count must not exceed BITBUF_NBITS after adding 'n' more bits.
 */
#define CAN_BUFFER(n)	(7 + (n) <= BITBUF_NBITS)

/*
 * Structure to keep track of the current state of sending bits to the
 * compressed output buffer
 */
struct deflate_output_bitstream {

	/* Bits that haven't yet been written to the output buffer */
	bitbuf_t bitbuf;

	/*
	 * Number of bits currently held in @bitbuf.  This can be between 0 and
	 * BITBUF_NBITS in general, or between 0 and 7 after a flush.
	 */
	unsigned bitcount;

	/*
	 * Pointer to the position in the output buffer at which the next byte
	 * should be written
	 */
	u8 *next;

	/* Pointer to the end of the output buffer */
	u8 *end;

	/* true if the output buffer ran out of space */
	bool overflow;
};

/*
 * Add some bits to the bitbuffer variable of the output bitstream.  The caller
 * must ensure that 'bitcount + n <= BITBUF_NBITS', by calling FLUSH_BITS()
 * frequently enough.
 */
#define ADD_BITS(bits, n)			\
do {						\
	bitbuf |= (bitbuf_t)(bits) << bitcount;	\
	bitcount += (n);			\
	ASSERT(bitcount <= BITBUF_NBITS);	\
} while (0)

/*
 * Flush bits from the bitbuffer variable to the output buffer.  After this, the
 * bitbuffer will contain at most 7 bits (a partial byte).
 *
 * Since deflate_flush_block() verified ahead of time that there is enough space
 * remaining before actually writing the block, it's guaranteed that out_next
 * won't exceed os->end.  However, there might not be enough space remaining to
 * flush a whole word, even though that's fastest.  Therefore, flush a whole
 * word if there is space for it, otherwise flush a byte at a time.
 */
#define FLUSH_BITS()							\
do {									\
	if (UNALIGNED_ACCESS_IS_FAST && likely(out_next < out_fast_end)) { \
		/* Flush a whole word (branchlessly). */		\
		put_unaligned_leword(bitbuf, out_next);			\
		bitbuf >>= bitcount & ~7;				\
		out_next += bitcount >> 3;				\
		bitcount &= 7;						\
	} else {							\
		/* Flush a byte at a time. */				\
		while (bitcount >= 8) {					\
			ASSERT(out_next < os->end);			\
			*out_next++ = bitbuf;				\
			bitcount -= 8;					\
			bitbuf >>= 8;					\
		}							\
	}								\
} while (0)

/*
 * Given the binary tree node A[subtree_idx] whose children already satisfy the
 * maxheap property, swap the node with its greater child until it is greater
 * than or equal to both of its children, so that the maxheap property is
 * satisfied in the subtree rooted at A[subtree_idx].  'A' uses 1-based indices.
 */
static void
heapify_subtree(u32 A[], unsigned length, unsigned subtree_idx)
{
	unsigned parent_idx;
	unsigned child_idx;
	u32 v;

	v = A[subtree_idx];
	parent_idx = subtree_idx;
	while ((child_idx = parent_idx * 2) <= length) {
		if (child_idx < length && A[child_idx + 1] > A[child_idx])
			child_idx++;
		if (v >= A[child_idx])
			break;
		A[parent_idx] = A[child_idx];
		parent_idx = child_idx;
	}
	A[parent_idx] = v;
}

/*
 * Rearrange the array 'A' so that it satisfies the maxheap property.
 * 'A' uses 1-based indices, so the children of A[i] are A[i*2] and A[i*2 + 1].
 */
static void
heapify_array(u32 A[], unsigned length)
{
	unsigned subtree_idx;

	for (subtree_idx = length / 2; subtree_idx >= 1; subtree_idx--)
		heapify_subtree(A, length, subtree_idx);
}

/*
 * Sort the array 'A', which contains 'length' unsigned 32-bit integers.
 *
 * Note: name this function heap_sort() instead of heapsort() to avoid colliding
 * with heapsort() from stdlib.h on BSD-derived systems.
 */
static void
heap_sort(u32 A[], unsigned length)
{
	A--; /* Use 1-based indices  */

	heapify_array(A, length);

	while (length >= 2) {
		u32 tmp = A[length];

		A[length] = A[1];
		A[1] = tmp;
		length--;
		heapify_subtree(A, length, 1);
	}
}

#define NUM_SYMBOL_BITS 10
#define NUM_FREQ_BITS	(32 - NUM_SYMBOL_BITS)
#define SYMBOL_MASK	((1 << NUM_SYMBOL_BITS) - 1)
#define FREQ_MASK	(~SYMBOL_MASK)

#define GET_NUM_COUNTERS(num_syms)	(num_syms)

/*
 * Sort the symbols primarily by frequency and secondarily by symbol value.
 * Discard symbols with zero frequency and fill in an array with the remaining
 * symbols, along with their frequencies.  The low NUM_SYMBOL_BITS bits of each
 * array entry will contain the symbol value, and the remaining bits will
 * contain the frequency.
 *
 * @num_syms
 *	Number of symbols in the alphabet, at most 1 << NUM_SYMBOL_BITS.
 *
 * @freqs[num_syms]
 *	Frequency of each symbol, summing to at most (1 << NUM_FREQ_BITS) - 1.
 *
 * @lens[num_syms]
 *	An array that eventually will hold the length of each codeword.  This
 *	function only fills in the codeword lengths for symbols that have zero
 *	frequency, which are not well defined per se but will be set to 0.
 *
 * @symout[num_syms]
 *	The output array, described above.
 *
 * Returns the number of entries in 'symout' that were filled.  This is the
 * number of symbols that have nonzero frequency.
 */
static unsigned
sort_symbols(unsigned num_syms, const u32 freqs[], u8 lens[], u32 symout[])
{
	unsigned sym;
	unsigned i;
	unsigned num_used_syms;
	unsigned num_counters;
	unsigned counters[GET_NUM_COUNTERS(DEFLATE_MAX_NUM_SYMS)];

	/*
	 * We use heapsort, but with an added optimization.  Since often most
	 * symbol frequencies are low, we first do a count sort using a limited
	 * number of counters.  High frequencies are counted in the last
	 * counter, and only they will be sorted with heapsort.
	 *
	 * Note: with more symbols, it is generally beneficial to have more
	 * counters.  About 1 counter per symbol seems fastest.
	 */

	num_counters = GET_NUM_COUNTERS(num_syms);

	memset(counters, 0, num_counters * sizeof(counters[0]));

	/* Count the frequencies. */
	for (sym = 0; sym < num_syms; sym++)
		counters[MIN(freqs[sym], num_counters - 1)]++;

	/*
	 * Make the counters cumulative, ignoring the zero-th, which counted
	 * symbols with zero frequency.  As a side effect, this calculates the
	 * number of symbols with nonzero frequency.
	 */
	num_used_syms = 0;
	for (i = 1; i < num_counters; i++) {
		unsigned count = counters[i];

		counters[i] = num_used_syms;
		num_used_syms += count;
	}

	/*
	 * Sort nonzero-frequency symbols using the counters.  At the same time,
	 * set the codeword lengths of zero-frequency symbols to 0.
	 */
	for (sym = 0; sym < num_syms; sym++) {
		u32 freq = freqs[sym];

		if (freq != 0) {
			symout[counters[MIN(freq, num_counters - 1)]++] =
				sym | (freq << NUM_SYMBOL_BITS);
		} else {
			lens[sym] = 0;
		}
	}

	/* Sort the symbols counted in the last counter. */
	heap_sort(symout + counters[num_counters - 2],
		  counters[num_counters - 1] - counters[num_counters - 2]);

	return num_used_syms;
}

/*
 * Build a Huffman tree.
 *
 * This is an optimized implementation that
 *	(a) takes advantage of the frequencies being already sorted;
 *	(b) only generates non-leaf nodes, since the non-leaf nodes of a Huffman
 *	    tree are sufficient to generate a canonical code;
 *	(c) Only stores parent pointers, not child pointers;
 *	(d) Produces the nodes in the same memory used for input frequency
 *	    information.
 *
 * Array 'A', which contains 'sym_count' entries, is used for both input and
 * output.  For this function, 'sym_count' must be at least 2.
 *
 * For input, the array must contain the frequencies of the symbols, sorted in
 * increasing order.  Specifically, each entry must contain a frequency left
 * shifted by NUM_SYMBOL_BITS bits.  Any data in the low NUM_SYMBOL_BITS bits of
 * the entries will be ignored by this function.  Although these bits will, in
 * fact, contain the symbols that correspond to the frequencies, this function
 * is concerned with frequencies only and keeps the symbols as-is.
 *
 * For output, this function will produce the non-leaf nodes of the Huffman
 * tree.  These nodes will be stored in the first (sym_count - 1) entries of the
 * array.  Entry A[sym_count - 2] will represent the root node.  Each other node
 * will contain the zero-based index of its parent node in 'A', left shifted by
 * NUM_SYMBOL_BITS bits.  The low NUM_SYMBOL_BITS bits of each entry in A will
 * be kept as-is.  Again, note that although these low bits will, in fact,
 * contain a symbol value, this symbol will have *no relationship* with the
 * Huffman tree node that happens to occupy the same slot.  This is because this
 * implementation only generates the non-leaf nodes of the tree.
 */
static void
build_tree(u32 A[], unsigned sym_count)
{
	const unsigned last_idx = sym_count - 1;

	/* Index of the next lowest frequency leaf that still needs a parent */
	unsigned i = 0;

	/*
	 * Index of the next lowest frequency non-leaf that still needs a
	 * parent, or 'e' if there is currently no such node
	 */
	unsigned b = 0;

	/* Index of the next spot for a non-leaf (will overwrite a leaf) */
	unsigned e = 0;

	do {
		u32 new_freq;

		/*
		 * Select the next two lowest frequency nodes among the leaves
		 * A[i] and non-leaves A[b], and create a new node A[e] to be
		 * their parent.  Set the new node's frequency to the sum of the
		 * frequencies of its two children.
		 *
		 * Usually the next two lowest frequency nodes are of the same
		 * type (leaf or non-leaf), so check those cases first.
		 */
		if (i + 1 <= last_idx &&
		    (b == e || (A[i + 1] & FREQ_MASK) <= (A[b] & FREQ_MASK))) {
			/* Two leaves */
			new_freq = (A[i] & FREQ_MASK) + (A[i + 1] & FREQ_MASK);
			i += 2;
		} else if (b + 2 <= e &&
			   (i > last_idx ||
			    (A[b + 1] & FREQ_MASK) < (A[i] & FREQ_MASK))) {
			/* Two non-leaves */
			new_freq = (A[b] & FREQ_MASK) + (A[b + 1] & FREQ_MASK);
			A[b] = (e << NUM_SYMBOL_BITS) | (A[b] & SYMBOL_MASK);
			A[b + 1] = (e << NUM_SYMBOL_BITS) |
				   (A[b + 1] & SYMBOL_MASK);
			b += 2;
		} else {
			/* One leaf and one non-leaf */
			new_freq = (A[i] & FREQ_MASK) + (A[b] & FREQ_MASK);
			A[b] = (e << NUM_SYMBOL_BITS) | (A[b] & SYMBOL_MASK);
			i++;
			b++;
		}
		A[e] = new_freq | (A[e] & SYMBOL_MASK);
		/*
		 * A binary tree with 'n' leaves has 'n - 1' non-leaves, so the
		 * tree is complete once we've created 'n - 1' non-leaves.
		 */
	} while (++e < last_idx);
}

/*
 * Given the stripped-down Huffman tree constructed by build_tree(), determine
 * the number of codewords that should be assigned each possible length, taking
 * into account the length-limited constraint.
 *
 * @A
 *	The array produced by build_tree(), containing parent index information
 *	for the non-leaf nodes of the Huffman tree.  Each entry in this array is
 *	a node; a node's parent always has a greater index than that node
 *	itself.  This function will overwrite the parent index information in
 *	this array, so essentially it will destroy the tree.  However, the data
 *	in the low NUM_SYMBOL_BITS of each entry will be preserved.
 *
 * @root_idx
 *	The 0-based index of the root node in 'A', and consequently one less
 *	than the number of tree node entries in 'A'.  (Or, really 2 less than
 *	the actual length of 'A'.)
 *
 * @len_counts
 *	An array of length ('max_codeword_len' + 1) in which the number of
 *	codewords having each length <= max_codeword_len will be returned.
 *
 * @max_codeword_len
 *	The maximum permissible codeword length.
 */
static void
compute_length_counts(u32 A[], unsigned root_idx, unsigned len_counts[],
		      unsigned max_codeword_len)
{
	unsigned len;
	int node;

	/*
	 * The key observations are:
	 *
	 * (1) We can traverse the non-leaf nodes of the tree, always visiting a
	 *     parent before its children, by simply iterating through the array
	 *     in reverse order.  Consequently, we can compute the depth of each
	 *     node in one pass, overwriting the parent indices with depths.
	 *
	 * (2) We can initially assume that in the real Huffman tree, both
	 *     children of the root are leaves.  This corresponds to two
	 *     codewords of length 1.  Then, whenever we visit a (non-leaf) node
	 *     during the traversal, we modify this assumption to account for
	 *     the current node *not* being a leaf, but rather its two children
	 *     being leaves.  This causes the loss of one codeword for the
	 *     current depth and the addition of two codewords for the current
	 *     depth plus one.
	 *
	 * (3) We can handle the length-limited constraint fairly easily by
	 *     simply using the largest length available when a depth exceeds
	 *     max_codeword_len.
	 */

	for (len = 0; len <= max_codeword_len; len++)
		len_counts[len] = 0;
	len_counts[1] = 2;

	/* Set the root node's depth to 0. */
	A[root_idx] &= SYMBOL_MASK;

	for (node = root_idx - 1; node >= 0; node--) {

		/* Calculate the depth of this node. */

		unsigned parent = A[node] >> NUM_SYMBOL_BITS;
		unsigned parent_depth = A[parent] >> NUM_SYMBOL_BITS;
		unsigned depth = parent_depth + 1;

		/*
		 * Set the depth of this node so that it is available when its
		 * children (if any) are processed.
		 */
		A[node] = (A[node] & SYMBOL_MASK) | (depth << NUM_SYMBOL_BITS);

		/*
		 * If needed, decrease the length to meet the length-limited
		 * constraint.  This is not the optimal method for generating
		 * length-limited Huffman codes!  But it should be good enough.
		 */
		if (depth >= max_codeword_len) {
			depth = max_codeword_len;
			do {
				depth--;
			} while (len_counts[depth] == 0);
		}

		/*
		 * Account for the fact that we have a non-leaf node at the
		 * current depth.
		 */
		len_counts[depth]--;
		len_counts[depth + 1] += 2;
	}
}

/*
 * DEFLATE uses bit-reversed codewords, so we must bit-reverse the codewords
 * after generating them.  All codewords have length <= 16 bits.  If the CPU has
 * a bit-reversal instruction, then that is the fastest method.  Otherwise the
 * fastest method is to reverse the bits in each of the two bytes using a table.
 * The table method is slightly faster than using bitwise operations to flip
 * adjacent 1, 2, 4, and then 8-bit fields, even if 2 to 4 codewords are packed
 * into a machine word and processed together using that method.
 */

#ifdef rbit32
static forceinline u32 reverse_codeword(u32 codeword, u8 len)
{
	return rbit32(codeword) >> ((32 - len) & 31);
}
#else
/* Generated by scripts/gen_bitreverse_tab.py */
static const u8 bitreverse_tab[256] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

static forceinline u32 reverse_codeword(u32 codeword, u8 len)
{
	STATIC_ASSERT(DEFLATE_MAX_CODEWORD_LEN <= 16);
	codeword = ((u32)bitreverse_tab[codeword & 0xff] << 8) |
		   bitreverse_tab[codeword >> 8];
	return codeword >> (16 - len);
}
#endif /* !rbit32 */

/*
 * Generate the codewords for a canonical Huffman code.
 *
 * @A
 *	The output array for codewords.  In addition, initially this
 *	array must contain the symbols, sorted primarily by frequency and
 *	secondarily by symbol value, in the low NUM_SYMBOL_BITS bits of
 *	each entry.
 *
 * @len
 *	Output array for codeword lengths.
 *
 * @len_counts
 *	An array that provides the number of codewords that will have
 *	each possible length <= max_codeword_len.
 *
 * @max_codeword_len
 *	Maximum length, in bits, of each codeword.
 *
 * @num_syms
 *	Number of symbols in the alphabet, including symbols with zero
 *	frequency.  This is the length of the 'A' and 'len' arrays.
 */
static void
gen_codewords(u32 A[], u8 lens[], const unsigned len_counts[],
	      unsigned max_codeword_len, unsigned num_syms)
{
	u32 next_codewords[DEFLATE_MAX_CODEWORD_LEN + 1];
	unsigned i;
	unsigned len;
	unsigned sym;

	/*
	 * Given the number of codewords that will have each length, assign
	 * codeword lengths to symbols.  We do this by assigning the lengths in
	 * decreasing order to the symbols sorted primarily by increasing
	 * frequency and secondarily by increasing symbol value.
	 */
	for (i = 0, len = max_codeword_len; len >= 1; len--) {
		unsigned count = len_counts[len];

		while (count--)
			lens[A[i++] & SYMBOL_MASK] = len;
	}

	/*
	 * Generate the codewords themselves.  We initialize the
	 * 'next_codewords' array to provide the lexicographically first
	 * codeword of each length, then assign codewords in symbol order.  This
	 * produces a canonical code.
	 */
	next_codewords[0] = 0;
	next_codewords[1] = 0;
	for (len = 2; len <= max_codeword_len; len++)
		next_codewords[len] =
			(next_codewords[len - 1] + len_counts[len - 1]) << 1;

	for (sym = 0; sym < num_syms; sym++) {
		/* DEFLATE requires bit-reversed codewords. */
		A[sym] = reverse_codeword(next_codewords[lens[sym]]++,
					  lens[sym]);
	}
}

/*
 * ---------------------------------------------------------------------
 *			deflate_make_huffman_code()
 * ---------------------------------------------------------------------
 *
 * Given an alphabet and the frequency of each symbol in it, construct a
 * length-limited canonical Huffman code.
 *
 * @num_syms
 *	The number of symbols in the alphabet.  The symbols are the integers in
 *	the range [0, num_syms - 1].  This parameter must be at least 2 and
 *	must not exceed (1 << NUM_SYMBOL_BITS).
 *
 * @max_codeword_len
 *	The maximum permissible codeword length.
 *
 * @freqs
 *	An array of length @num_syms that gives the frequency of each symbol.
 *	It is valid for some, none, or all of the frequencies to be 0.  The sum
 *	of frequencies must not exceed (1 << NUM_FREQ_BITS) - 1.
 *
 * @lens
 *	An array of @num_syms entries in which this function will return the
 *	length, in bits, of the codeword assigned to each symbol.  Symbols with
 *	0 frequency will not have codewords per se, but their entries in this
 *	array will be set to 0.  No lengths greater than @max_codeword_len will
 *	be assigned.
 *
 * @codewords
 *	An array of @num_syms entries in which this function will return the
 *	codeword for each symbol, right-justified and padded on the left with
 *	zeroes.  Codewords for symbols with 0 frequency will be undefined.
 *
 * ---------------------------------------------------------------------
 *
 * This function builds a length-limited canonical Huffman code.
 *
 * A length-limited Huffman code contains no codewords longer than some
 * specified length, and has exactly (with some algorithms) or approximately
 * (with the algorithm used here) the minimum weighted path length from the
 * root, given this constraint.
 *
 * A canonical Huffman code satisfies the properties that a longer codeword
 * never lexicographically precedes a shorter codeword, and the lexicographic
 * ordering of codewords of the same length is the same as the lexicographic
 * ordering of the corresponding symbols.  A canonical Huffman code, or more
 * generally a canonical prefix code, can be reconstructed from only a list
 * containing the codeword length of each symbol.
 *
 * The classic algorithm to generate a Huffman code creates a node for each
 * symbol, then inserts these nodes into a min-heap keyed by symbol frequency.
 * Then, repeatedly, the two lowest-frequency nodes are removed from the
 * min-heap and added as the children of a new node having frequency equal to
 * the sum of its two children, which is then inserted into the min-heap.  When
 * only a single node remains in the min-heap, it is the root of the Huffman
 * tree.  The codeword for each symbol is determined by the path needed to reach
 * the corresponding node from the root.  Descending to the left child appends a
 * 0 bit, whereas descending to the right child appends a 1 bit.
 *
 * The classic algorithm is relatively easy to understand, but it is subject to
 * a number of inefficiencies.  In practice, it is fastest to first sort the
 * symbols by frequency.  (This itself can be subject to an optimization based
 * on the fact that most frequencies tend to be low.)  At the same time, we sort
 * secondarily by symbol value, which aids the process of generating a canonical
 * code.  Then, during tree construction, no heap is necessary because both the
 * leaf nodes and the unparented non-leaf nodes can be easily maintained in
 * sorted order.  Consequently, there can never be more than two possibilities
 * for the next-lowest-frequency node.
 *
 * In addition, because we're generating a canonical code, we actually don't
 * need the leaf nodes of the tree at all, only the non-leaf nodes.  This is
 * because for canonical code generation we don't need to know where the symbols
 * are in the tree.  Rather, we only need to know how many leaf nodes have each
 * depth (codeword length).  And this information can, in fact, be quickly
 * generated from the tree of non-leaves only.
 *
 * Furthermore, we can build this stripped-down Huffman tree directly in the
 * array in which the codewords are to be generated, provided that these array
 * slots are large enough to hold a symbol and frequency value.
 *
 * Still furthermore, we don't even need to maintain explicit child pointers.
 * We only need the parent pointers, and even those can be overwritten in-place
 * with depth information as part of the process of extracting codeword lengths
 * from the tree.  So in summary, we do NOT need a big structure like:
 *
 *	struct huffman_tree_node {
 *		unsigned int symbol;
 *		unsigned int frequency;
 *		unsigned int depth;
 *		struct huffman_tree_node *left_child;
 *		struct huffman_tree_node *right_child;
 *	};
 *
 *
 * ... which often gets used in "naive" implementations of Huffman code
 * generation.
 *
 * Many of these optimizations are based on the implementation in 7-Zip (source
 * file: C/HuffEnc.c), which was placed in the public domain by Igor Pavlov.
 */
static void
deflate_make_huffman_code(unsigned num_syms, unsigned max_codeword_len,
			  const u32 freqs[], u8 lens[], u32 codewords[])
{
	u32 *A = codewords;
	unsigned num_used_syms;

	STATIC_ASSERT(DEFLATE_MAX_NUM_SYMS <= 1 << NUM_SYMBOL_BITS);
	STATIC_ASSERT(MAX_BLOCK_LENGTH <= ((u32)1 << NUM_FREQ_BITS) - 1);

	/*
	 * We begin by sorting the symbols primarily by frequency and
	 * secondarily by symbol value.  As an optimization, the array used for
	 * this purpose ('A') shares storage with the space in which we will
	 * eventually return the codewords.
	 */
	num_used_syms = sort_symbols(num_syms, freqs, lens, A);
	/*
	 * 'num_used_syms' is the number of symbols with nonzero frequency.
	 * This may be less than @num_syms.  'num_used_syms' is also the number
	 * of entries in 'A' that are valid.  Each entry consists of a distinct
	 * symbol and a nonzero frequency packed into a 32-bit integer.
	 */

	/*
	 * A complete Huffman code must contain at least 2 codewords.  Yet, it's
	 * possible that fewer than 2 symbols were used.  When this happens,
	 * it's usually for the offset code (0-1 symbols used).  But it's also
	 * theoretically possible for the litlen and pre codes (1 symbol used).
	 *
	 * The DEFLATE RFC explicitly allows the offset code to contain just 1
	 * codeword, or even be completely empty.  But it's silent about the
	 * other codes.  It also doesn't say whether, in the 1-codeword case,
	 * the codeword (which it says must be 1 bit) is '0' or '1'.
	 *
	 * In any case, some DEFLATE decompressors reject these cases.  zlib
	 * generally allows them, but it does reject precodes that have just 1
	 * codeword.  More problematically, zlib v1.2.1 and earlier rejected
	 * empty offset codes, and this behavior can also be seen in Windows
	 * Explorer's ZIP unpacker (supposedly even still in Windows 11).
	 *
	 * Other DEFLATE compressors, including zlib, always send at least 2
	 * codewords in order to make a complete Huffman code.  Therefore, this
	 * is a case where practice does not entirely match the specification.
	 * We follow practice by generating 2 codewords of length 1: codeword
	 * '0' for symbol 0, and codeword '1' for another symbol -- the used
	 * symbol if it exists and is not symbol 0, otherwise symbol 1.  This
	 * does worsen the compression ratio by having to send an unnecessary
	 * offset codeword length.  But this only affects rare cases such as
	 * blocks containing all literals, and it only makes a tiny difference.
	 */
	if (unlikely(num_used_syms < 2)) {
		unsigned sym = num_used_syms ? (A[0] & SYMBOL_MASK) : 0;
		unsigned nonzero_idx = sym ? sym : 1;

		codewords[0] = 0;
		lens[0] = 1;
		codewords[nonzero_idx] = 1;
		lens[nonzero_idx] = 1;
		return;
	}

	/*
	 * Build a stripped-down version of the Huffman tree, sharing the array
	 * 'A' with the symbol values.  Then extract length counts from the tree
	 * and use them to generate the final codewords.
	 */

	build_tree(A, num_used_syms);

	{
		unsigned len_counts[DEFLATE_MAX_CODEWORD_LEN + 1];

		compute_length_counts(A, num_used_syms - 2,
				      len_counts, max_codeword_len);

		gen_codewords(A, lens, len_counts, max_codeword_len, num_syms);
	}
}

/*
 * Clear the Huffman symbol frequency counters.  This must be called when
 * starting a new DEFLATE block.
 */
static void
deflate_reset_symbol_frequencies(struct libdeflate_compressor *c)
{
	memset(&c->freqs, 0, sizeof(c->freqs));
}

/*
 * Build the literal/length and offset Huffman codes for a DEFLATE block.
 *
 * This takes as input the frequency tables for each alphabet and produces as
 * output a set of tables that map symbols to codewords and codeword lengths.
 */
static void
deflate_make_huffman_codes(const struct deflate_freqs *freqs,
			   struct deflate_codes *codes)
{
	deflate_make_huffman_code(DEFLATE_NUM_LITLEN_SYMS,
				  MAX_LITLEN_CODEWORD_LEN,
				  freqs->litlen,
				  codes->lens.litlen,
				  codes->codewords.litlen);

	deflate_make_huffman_code(DEFLATE_NUM_OFFSET_SYMS,
				  MAX_OFFSET_CODEWORD_LEN,
				  freqs->offset,
				  codes->lens.offset,
				  codes->codewords.offset);
}

/* Initialize c->static_codes. */
static void
deflate_init_static_codes(struct libdeflate_compressor *c)
{
	unsigned i;

	for (i = 0; i < 144; i++)
		c->freqs.litlen[i] = 1 << (9 - 8);
	for (; i < 256; i++)
		c->freqs.litlen[i] = 1 << (9 - 9);
	for (; i < 280; i++)
		c->freqs.litlen[i] = 1 << (9 - 7);
	for (; i < 288; i++)
		c->freqs.litlen[i] = 1 << (9 - 8);

	for (i = 0; i < 32; i++)
		c->freqs.offset[i] = 1 << (5 - 5);

	deflate_make_huffman_codes(&c->freqs, &c->static_codes);
}

/* Return the offset slot for the given match offset, using the small map. */
static forceinline unsigned
deflate_get_offset_slot(u32 offset)
{
	/*
	 * 1 <= offset <= 32768 here.  For 1 <= offset <= 256,
	 * deflate_offset_slot[offset - 1] gives the slot.
	 *
	 * For 257 <= offset <= 32768, we take advantage of the fact that 257 is
	 * the beginning of slot 16, and each slot [16..30) is exactly 1 << 7 ==
	 * 128 times larger than each slot [2..16) (since the number of extra
	 * bits increases by 1 every 2 slots).  Thus, the slot is:
	 *
	 *	deflate_offset_slot[2 + ((offset - 257) >> 7)] + (16 - 2)
	 *   == deflate_offset_slot[((offset - 1) >> 7)] + 14
	 *
	 * Define 'n = (offset <= 256) ? 0 : 7'.  Then any offset is handled by:
	 *
	 *      deflate_offset_slot[(offset - 1) >> n] + (n << 1)
	 *
	 * For better performance, replace 'n = (offset <= 256) ? 0 : 7' with
	 * the equivalent (for offset <= 536871168) 'n = (256 - offset) >> 29'.
	 */
	unsigned n = (256 - offset) >> 29;

	ASSERT(offset >= 1 && offset <= 32768);

	return deflate_offset_slot[(offset - 1) >> n] + (n << 1);
}

static unsigned
deflate_compute_precode_items(const u8 lens[], const unsigned num_lens,
			      u32 precode_freqs[], unsigned precode_items[])
{
	unsigned *itemptr;
	unsigned run_start;
	unsigned run_end;
	unsigned extra_bits;
	u8 len;

	memset(precode_freqs, 0,
	       DEFLATE_NUM_PRECODE_SYMS * sizeof(precode_freqs[0]));

	itemptr = precode_items;
	run_start = 0;
	do {
		/* Find the next run of codeword lengths. */

		/* len = the length being repeated */
		len = lens[run_start];

		/* Extend the run. */
		run_end = run_start;
		do {
			run_end++;
		} while (run_end != num_lens && len == lens[run_end]);

		if (len == 0) {
			/* Run of zeroes. */

			/* Symbol 18: RLE 11 to 138 zeroes at a time. */
			while ((run_end - run_start) >= 11) {
				extra_bits = MIN((run_end - run_start) - 11,
						 0x7F);
				precode_freqs[18]++;
				*itemptr++ = 18 | (extra_bits << 5);
				run_start += 11 + extra_bits;
			}

			/* Symbol 17: RLE 3 to 10 zeroes at a time. */
			if ((run_end - run_start) >= 3) {
				extra_bits = MIN((run_end - run_start) - 3,
						 0x7);
				precode_freqs[17]++;
				*itemptr++ = 17 | (extra_bits << 5);
				run_start += 3 + extra_bits;
			}
		} else {

			/* A run of nonzero lengths. */

			/* Symbol 16: RLE 3 to 6 of the previous length. */
			if ((run_end - run_start) >= 4) {
				precode_freqs[len]++;
				*itemptr++ = len;
				run_start++;
				do {
					extra_bits = MIN((run_end - run_start) -
							 3, 0x3);
					precode_freqs[16]++;
					*itemptr++ = 16 | (extra_bits << 5);
					run_start += 3 + extra_bits;
				} while ((run_end - run_start) >= 3);
			}
		}

		/* Output any remaining lengths without RLE. */
		while (run_start != run_end) {
			precode_freqs[len]++;
			*itemptr++ = len;
			run_start++;
		}
	} while (run_start != num_lens);

	return itemptr - precode_items;
}

/*
 * Huffman codeword lengths for dynamic Huffman blocks are compressed using a
 * separate Huffman code, the "precode", which contains a symbol for each
 * possible codeword length in the larger code as well as several special
 * symbols to represent repeated codeword lengths (a form of run-length
 * encoding).  The precode is itself constructed in canonical form, and its
 * codeword lengths are represented literally in 19 3-bit fields that
 * immediately precede the compressed codeword lengths of the larger code.
 */

/* Precompute the information needed to output dynamic Huffman codes. */
static void
deflate_precompute_huffman_header(struct libdeflate_compressor *c)
{
	/* Compute how many litlen and offset symbols are needed. */

	for (c->o.precode.num_litlen_syms = DEFLATE_NUM_LITLEN_SYMS;
	     c->o.precode.num_litlen_syms > 257;
	     c->o.precode.num_litlen_syms--)
		if (c->codes.lens.litlen[c->o.precode.num_litlen_syms - 1] != 0)
			break;

	for (c->o.precode.num_offset_syms = DEFLATE_NUM_OFFSET_SYMS;
	     c->o.precode.num_offset_syms > 1;
	     c->o.precode.num_offset_syms--)
		if (c->codes.lens.offset[c->o.precode.num_offset_syms - 1] != 0)
			break;

	/*
	 * If we're not using the full set of literal/length codeword lengths,
	 * then temporarily move the offset codeword lengths over so that the
	 * literal/length and offset codeword lengths are contiguous.
	 */
	STATIC_ASSERT(offsetof(struct deflate_lens, offset) ==
		      DEFLATE_NUM_LITLEN_SYMS);
	if (c->o.precode.num_litlen_syms != DEFLATE_NUM_LITLEN_SYMS) {
		memmove((u8 *)&c->codes.lens + c->o.precode.num_litlen_syms,
			(u8 *)&c->codes.lens + DEFLATE_NUM_LITLEN_SYMS,
			c->o.precode.num_offset_syms);
	}

	/*
	 * Compute the "items" (RLE / literal tokens and extra bits) with which
	 * the codeword lengths in the larger code will be output.
	 */
	c->o.precode.num_items =
		deflate_compute_precode_items((u8 *)&c->codes.lens,
					      c->o.precode.num_litlen_syms +
					      c->o.precode.num_offset_syms,
					      c->o.precode.freqs,
					      c->o.precode.items);

	/* Build the precode. */
	deflate_make_huffman_code(DEFLATE_NUM_PRECODE_SYMS,
				  MAX_PRE_CODEWORD_LEN,
				  c->o.precode.freqs, c->o.precode.lens,
				  c->o.precode.codewords);

	/* Count how many precode lengths we actually need to output. */
	for (c->o.precode.num_explicit_lens = DEFLATE_NUM_PRECODE_SYMS;
	     c->o.precode.num_explicit_lens > 4;
	     c->o.precode.num_explicit_lens--)
		if (c->o.precode.lens[deflate_precode_lens_permutation[
				c->o.precode.num_explicit_lens - 1]] != 0)
			break;

	/* Restore the offset codeword lengths if needed. */
	if (c->o.precode.num_litlen_syms != DEFLATE_NUM_LITLEN_SYMS) {
		memmove((u8 *)&c->codes.lens + DEFLATE_NUM_LITLEN_SYMS,
			(u8 *)&c->codes.lens + c->o.precode.num_litlen_syms,
			c->o.precode.num_offset_syms);
	}
}

/*
 * To make it faster to output matches, compute the "full" match length
 * codewords, i.e. the concatenation of the litlen codeword and the extra bits
 * for each possible match length.
 */
static void
deflate_compute_full_len_codewords(struct libdeflate_compressor *c,
				   const struct deflate_codes *codes)
{
	u32 len;

	STATIC_ASSERT(MAX_LITLEN_CODEWORD_LEN +
		      DEFLATE_MAX_EXTRA_LENGTH_BITS <= 32);

	for (len = DEFLATE_MIN_MATCH_LEN; len <= DEFLATE_MAX_MATCH_LEN; len++) {
		unsigned slot = deflate_length_slot[len];
		unsigned litlen_sym = DEFLATE_FIRST_LEN_SYM + slot;
		u32 extra_bits = len - deflate_length_slot_base[slot];

		c->o.length.codewords[len] =
			codes->codewords.litlen[litlen_sym] |
			(extra_bits << codes->lens.litlen[litlen_sym]);
		c->o.length.lens[len] = codes->lens.litlen[litlen_sym] +
					deflate_extra_length_bits[slot];
	}
}

/* Write a match to the output buffer. */
#define WRITE_MATCH(c_, codes_, length_, offset_, offset_slot_)		\
do {									\
	const struct libdeflate_compressor *c__ = (c_);			\
	const struct deflate_codes *codes__ = (codes_);			\
	u32 length__ = (length_);					\
	u32 offset__ = (offset_);					\
	unsigned offset_slot__ = (offset_slot_);			\
									\
	/* Litlen symbol and extra length bits */			\
	STATIC_ASSERT(CAN_BUFFER(MAX_LITLEN_CODEWORD_LEN +		\
				 DEFLATE_MAX_EXTRA_LENGTH_BITS));	\
	ADD_BITS(c__->o.length.codewords[length__],			\
		 c__->o.length.lens[length__]);				\
									\
	if (!CAN_BUFFER(MAX_LITLEN_CODEWORD_LEN +			\
			DEFLATE_MAX_EXTRA_LENGTH_BITS +			\
			MAX_OFFSET_CODEWORD_LEN +			\
			DEFLATE_MAX_EXTRA_OFFSET_BITS))			\
		FLUSH_BITS();						\
									\
	/* Offset symbol */						\
	ADD_BITS(codes__->codewords.offset[offset_slot__],		\
		 codes__->lens.offset[offset_slot__]);			\
									\
	if (!CAN_BUFFER(MAX_OFFSET_CODEWORD_LEN +			\
			DEFLATE_MAX_EXTRA_OFFSET_BITS))			\
		FLUSH_BITS();						\
									\
	/* Extra offset bits */						\
	ADD_BITS(offset__ - deflate_offset_slot_base[offset_slot__],	\
		 deflate_extra_offset_bits[offset_slot__]);		\
									\
	FLUSH_BITS();							\
} while (0)

/*
 * Choose the best type of block to use (dynamic Huffman, static Huffman, or
 * uncompressed), then output it.
 *
 * The uncompressed data of the block is @block_begin[0..@block_length-1].  The
 * sequence of literals and matches that will be used to compress the block (if
 * a compressed block is chosen) is given by @sequences if it's non-NULL, or
 * else @c->p.n.optimum_nodes.  @c->freqs and @c->codes must be already set
 * according to the literals, matches, and end-of-block symbol.
 */
static void
deflate_flush_block(struct libdeflate_compressor *c,
		    struct deflate_output_bitstream *os,
		    const u8 *block_begin, u32 block_length,
		    const struct deflate_sequence *sequences,
		    bool is_final_block)
{
	/*
	 * It is hard to get compilers to understand that writes to 'os->next'
	 * don't alias 'os'.  That hurts performance significantly, as
	 * everything in 'os' would keep getting re-loaded.  ('restrict'
	 * *should* do the trick, but it's unreliable.)  Therefore, we keep all
	 * the output bitstream state in local variables, and output bits using
	 * macros.  This is similar to what the decompressor does.
	 */
	const u8 *in_next = block_begin;
	const u8 * const in_end = block_begin + block_length;
	bitbuf_t bitbuf = os->bitbuf;
	unsigned bitcount = os->bitcount;
	u8 *out_next = os->next;
	u8 * const out_fast_end =
		os->end - MIN(WORDBYTES - 1, os->end - out_next);
	/*
	 * The cost for each block type, in bits.  Start with the cost of the
	 * block header which is 3 bits.
	 */
	u32 dynamic_cost = 3;
	u32 static_cost = 3;
	u32 uncompressed_cost = 3;
	u32 best_cost;
	struct deflate_codes *codes;
	unsigned sym;

	ASSERT(block_length >= MIN_BLOCK_LENGTH ||
	       (is_final_block && block_length > 0));
	ASSERT(block_length <= MAX_BLOCK_LENGTH);
	ASSERT(bitcount <= 7);
	ASSERT((bitbuf & ~(((bitbuf_t)1 << bitcount) - 1)) == 0);
	ASSERT(out_next <= os->end);
	ASSERT(!os->overflow);

	/* Precompute the precode items and build the precode. */
	deflate_precompute_huffman_header(c);

	/* Account for the cost of encoding dynamic Huffman codes. */
	dynamic_cost += 5 + 5 + 4 + (3 * c->o.precode.num_explicit_lens);
	for (sym = 0; sym < DEFLATE_NUM_PRECODE_SYMS; sym++) {
		u32 extra = deflate_extra_precode_bits[sym];

		dynamic_cost += c->o.precode.freqs[sym] *
				(extra + c->o.precode.lens[sym]);
	}

	/* Account for the cost of encoding literals. */
	for (sym = 0; sym < 144; sym++) {
		dynamic_cost += c->freqs.litlen[sym] *
				c->codes.lens.litlen[sym];
		static_cost += c->freqs.litlen[sym] * 8;
	}
	for (; sym < 256; sym++) {
		dynamic_cost += c->freqs.litlen[sym] *
				c->codes.lens.litlen[sym];
		static_cost += c->freqs.litlen[sym] * 9;
	}

	/* Account for the cost of encoding the end-of-block symbol. */
	dynamic_cost += c->codes.lens.litlen[DEFLATE_END_OF_BLOCK];
	static_cost += 7;

	/* Account for the cost of encoding lengths. */
	for (sym = DEFLATE_FIRST_LEN_SYM;
	     sym < DEFLATE_FIRST_LEN_SYM + ARRAY_LEN(deflate_extra_length_bits);
	     sym++) {
		u32 extra = deflate_extra_length_bits[
					sym - DEFLATE_FIRST_LEN_SYM];

		dynamic_cost += c->freqs.litlen[sym] *
				(extra + c->codes.lens.litlen[sym]);
		static_cost += c->freqs.litlen[sym] *
				(extra + c->static_codes.lens.litlen[sym]);
	}

	/* Account for the cost of encoding offsets. */
	for (sym = 0; sym < ARRAY_LEN(deflate_extra_offset_bits); sym++) {
		u32 extra = deflate_extra_offset_bits[sym];

		dynamic_cost += c->freqs.offset[sym] *
				(extra + c->codes.lens.offset[sym]);
		static_cost += c->freqs.offset[sym] * (extra + 5);
	}

	/* Compute the cost of using uncompressed blocks. */
	uncompressed_cost += (-(bitcount + 3) & 7) + 32 +
			     (40 * (DIV_ROUND_UP(block_length,
						 UINT16_MAX) - 1)) +
			     (8 * block_length);

	/*
	 * Choose and output the cheapest type of block.  If there is a tie,
	 * prefer uncompressed, then static, then dynamic.
	 */

	best_cost = MIN(dynamic_cost, MIN(static_cost, uncompressed_cost));

	/* If the block isn't going to fit, then stop early. */
	if (DIV_ROUND_UP(bitcount + best_cost, 8) > os->end - out_next) {
		os->overflow = true;
		return;
	}
	/*
	 * Else, now we know that the block fits, so no further bounds checks on
	 * the output buffer are required until the next block.
	 */

	if (best_cost == uncompressed_cost) {
		/*
		 * Uncompressed block(s).  DEFLATE limits the length of
		 * uncompressed blocks to UINT16_MAX bytes, so if the length of
		 * the "block" we're flushing is over UINT16_MAX, we actually
		 * output multiple blocks.
		 */
		do {
			u8 bfinal = 0;
			size_t len = UINT16_MAX;

			if (in_end - in_next <= UINT16_MAX) {
				bfinal = is_final_block;
				len = in_end - in_next;
			}
			/* It was already checked that there is enough space. */
			ASSERT(os->end - out_next >=
			       DIV_ROUND_UP(bitcount + 3, 8) + 4 + len);
			/*
			 * Output BFINAL (1 bit) and BTYPE (2 bits), then align
			 * to a byte boundary.
			 */
			STATIC_ASSERT(DEFLATE_BLOCKTYPE_UNCOMPRESSED == 0);
			*out_next++ = (bfinal << bitcount) | bitbuf;
			if (bitcount > 5)
				*out_next++ = 0;
			bitbuf = 0;
			bitcount = 0;
			/* Output LEN and NLEN, then the data itself. */
			put_unaligned_le16(len, out_next);
			out_next += 2;
			put_unaligned_le16(~len, out_next);
			out_next += 2;
			memcpy(out_next, in_next, len);
			out_next += len;
			in_next += len;
		} while (in_next != in_end);
		/* Done outputting uncompressed block(s) */
		goto out;
	}

	if (best_cost == static_cost) {
		/* Static Huffman block */
		codes = &c->static_codes;
		ADD_BITS(is_final_block, 1);
		ADD_BITS(DEFLATE_BLOCKTYPE_STATIC_HUFFMAN, 2);
		FLUSH_BITS();
	} else {
		const unsigned num_explicit_lens = c->o.precode.num_explicit_lens;
		const unsigned num_precode_items = c->o.precode.num_items;
		unsigned precode_sym, precode_item;
		unsigned i;

		/* Dynamic Huffman block */

		codes = &c->codes;
		STATIC_ASSERT(CAN_BUFFER(1 + 2 + 5 + 5 + 4 + 3));
		ADD_BITS(is_final_block, 1);
		ADD_BITS(DEFLATE_BLOCKTYPE_DYNAMIC_HUFFMAN, 2);
		ADD_BITS(c->o.precode.num_litlen_syms - 257, 5);
		ADD_BITS(c->o.precode.num_offset_syms - 1, 5);
		ADD_BITS(num_explicit_lens - 4, 4);

		/* Output the lengths of the codewords in the precode. */
		if (CAN_BUFFER(3 * (DEFLATE_NUM_PRECODE_SYMS - 1))) {
			/*
			 * A 64-bit bitbuffer is just one bit too small to hold
			 * the maximum number of precode lens, so to minimize
			 * flushes we merge one len with the previous fields.
			 */
			precode_sym = deflate_precode_lens_permutation[0];
			ADD_BITS(c->o.precode.lens[precode_sym], 3);
			FLUSH_BITS();
			i = 1; /* num_explicit_lens >= 4 */
			do {
				precode_sym =
					deflate_precode_lens_permutation[i];
				ADD_BITS(c->o.precode.lens[precode_sym], 3);
			} while (++i < num_explicit_lens);
			FLUSH_BITS();
		} else {
			FLUSH_BITS();
			i = 0;
			do {
				precode_sym =
					deflate_precode_lens_permutation[i];
				ADD_BITS(c->o.precode.lens[precode_sym], 3);
				FLUSH_BITS();
			} while (++i < num_explicit_lens);
		}

		/*
		 * Output the lengths of the codewords in the litlen and offset
		 * codes, encoded by the precode.
		 */
		i = 0;
		do {
			precode_item = c->o.precode.items[i];
			precode_sym = precode_item & 0x1F;
			STATIC_ASSERT(CAN_BUFFER(MAX_PRE_CODEWORD_LEN + 7));
			ADD_BITS(c->o.precode.codewords[precode_sym],
				 c->o.precode.lens[precode_sym]);
			ADD_BITS(precode_item >> 5,
				 deflate_extra_precode_bits[precode_sym]);
			FLUSH_BITS();
		} while (++i < num_precode_items);
	}

	/* Output the literals and matches for a dynamic or static block. */
	ASSERT(bitcount <= 7);
	deflate_compute_full_len_codewords(c, codes);
#if SUPPORT_NEAR_OPTIMAL_PARSING
	if (sequences == NULL) {
		/* Output the literals and matches from the minimum-cost path */
		struct deflate_optimum_node *cur_node =
			&c->p.n.optimum_nodes[0];
		struct deflate_optimum_node * const end_node =
			&c->p.n.optimum_nodes[block_length];
		do {
			u32 length = cur_node->item & OPTIMUM_LEN_MASK;
			u32 offset = cur_node->item >> OPTIMUM_OFFSET_SHIFT;

			if (length == 1) {
				/* Literal */
				ADD_BITS(codes->codewords.litlen[offset],
					 codes->lens.litlen[offset]);
				FLUSH_BITS();
			} else {
				/* Match */
				WRITE_MATCH(c, codes, length, offset,
					    c->p.n.offset_slot_full[offset]);
			}
			cur_node += length;
		} while (cur_node != end_node);
	} else
#endif /* SUPPORT_NEAR_OPTIMAL_PARSING */
	{
		/* Output the literals and matches from the sequences list. */
		const struct deflate_sequence *seq;

		for (seq = sequences; ; seq++) {
			u32 litrunlen = seq->litrunlen_and_length &
					SEQ_LITRUNLEN_MASK;
			u32 length = seq->litrunlen_and_length >>
				     SEQ_LENGTH_SHIFT;
			unsigned lit;

			/* Output a run of literals. */
			if (CAN_BUFFER(4 * MAX_LITLEN_CODEWORD_LEN)) {
				for (; litrunlen >= 4; litrunlen -= 4) {
					lit = *in_next++;
					ADD_BITS(codes->codewords.litlen[lit],
						 codes->lens.litlen[lit]);
					lit = *in_next++;
					ADD_BITS(codes->codewords.litlen[lit],
						 codes->lens.litlen[lit]);
					lit = *in_next++;
					ADD_BITS(codes->codewords.litlen[lit],
						 codes->lens.litlen[lit]);
					lit = *in_next++;
					ADD_BITS(codes->codewords.litlen[lit],
						 codes->lens.litlen[lit]);
					FLUSH_BITS();
				}
				if (litrunlen-- != 0) {
					lit = *in_next++;
					ADD_BITS(codes->codewords.litlen[lit],
						 codes->lens.litlen[lit]);
					if (litrunlen-- != 0) {
						lit = *in_next++;
						ADD_BITS(codes->codewords.litlen[lit],
							 codes->lens.litlen[lit]);
						if (litrunlen-- != 0) {
							lit = *in_next++;
							ADD_BITS(codes->codewords.litlen[lit],
								 codes->lens.litlen[lit]);
						}
					}
					FLUSH_BITS();
				}
			} else {
				while (litrunlen--) {
					lit = *in_next++;
					ADD_BITS(codes->codewords.litlen[lit],
						 codes->lens.litlen[lit]);
					FLUSH_BITS();
				}
			}

			if (length == 0) { /* Last sequence? */
				ASSERT(in_next == in_end);
				break;
			}

			/* Output a match. */
			WRITE_MATCH(c, codes, length, seq->offset,
				    seq->offset_slot);
			in_next += length;
		}
	}

	/* Output the end-of-block symbol. */
	ASSERT(bitcount <= 7);
	ADD_BITS(codes->codewords.litlen[DEFLATE_END_OF_BLOCK],
		 codes->lens.litlen[DEFLATE_END_OF_BLOCK]);
	FLUSH_BITS();
out:
	ASSERT(bitcount <= 7);
	/*
	 * Assert that the block cost was computed correctly.  This is relied on
	 * above for the bounds check on the output buffer.  Also,
	 * libdeflate_deflate_compress_bound() relies on this via the assumption
	 * that uncompressed blocks will always be used when cheapest.
	 */
	ASSERT(8 * (out_next - os->next) + bitcount - os->bitcount == best_cost);
	os->bitbuf = bitbuf;
	os->bitcount = bitcount;
	os->next = out_next;
}

static void
deflate_finish_block(struct libdeflate_compressor *c,
		     struct deflate_output_bitstream *os,
		     const u8 *block_begin, u32 block_length,
		     const struct deflate_sequence *sequences,
		     bool is_final_block)
{
	c->freqs.litlen[DEFLATE_END_OF_BLOCK]++;
	deflate_make_huffman_codes(&c->freqs, &c->codes);
	deflate_flush_block(c, os, block_begin, block_length, sequences,
			    is_final_block);
}

/******************************************************************************/

/*
 * Block splitting algorithm.  The problem is to decide when it is worthwhile to
 * start a new block with new Huffman codes.  There is a theoretically optimal
 * solution: recursively consider every possible block split, considering the
 * exact cost of each block, and choose the minimum cost approach.  But this is
 * far too slow.  Instead, as an approximation, we can count symbols and after
 * every N symbols, compare the expected distribution of symbols based on the
 * previous data with the actual distribution.  If they differ "by enough", then
 * start a new block.
 *
 * As an optimization and heuristic, we don't distinguish between every symbol
 * but rather we combine many symbols into a single "observation type".  For
 * literals we only look at the high bits and low bits, and for matches we only
 * look at whether the match is long or not.  The assumption is that for typical
 * "real" data, places that are good block boundaries will tend to be noticeable
 * based only on changes in these aggregate probabilities, without looking for
 * subtle differences in individual symbols.  For example, a change from ASCII
 * bytes to non-ASCII bytes, or from few matches (generally less compressible)
 * to many matches (generally more compressible), would be easily noticed based
 * on the aggregates.
 *
 * For determining whether the probability distributions are "different enough"
 * to start a new block, the simple heuristic of splitting when the sum of
 * absolute differences exceeds a constant seems to be good enough.  We also add
 * a number proportional to the block length so that the algorithm is more
 * likely to end long blocks than short blocks.  This reflects the general
 * expectation that it will become increasingly beneficial to start a new block
 * as the current block grows longer.
 *
 * Finally, for an approximation, it is not strictly necessary that the exact
 * symbols being used are considered.  With "near-optimal parsing", for example,
 * the actual symbols that will be used are unknown until after the block
 * boundary is chosen and the block has been optimized.  Since the final choices
 * cannot be used, we can use preliminary "greedy" choices instead.
 */

/* Initialize the block split statistics when starting a new block. */
static void
init_block_split_stats(struct block_split_stats *stats)
{
	int i;

	for (i = 0; i < NUM_OBSERVATION_TYPES; i++) {
		stats->new_observations[i] = 0;
		stats->observations[i] = 0;
	}
	stats->num_new_observations = 0;
	stats->num_observations = 0;
}

/*
 * Literal observation.  Heuristic: use the top 2 bits and low 1 bits of the
 * literal, for 8 possible literal observation types.
 */
static forceinline void
observe_literal(struct block_split_stats *stats, u8 lit)
{
	stats->new_observations[((lit >> 5) & 0x6) | (lit & 1)]++;
	stats->num_new_observations++;
}

/*
 * Match observation.  Heuristic: use one observation type for "short match" and
 * one observation type for "long match".
 */
static forceinline void
observe_match(struct block_split_stats *stats, u32 length)
{
	stats->new_observations[NUM_LITERAL_OBSERVATION_TYPES +
				(length >= 9)]++;
	stats->num_new_observations++;
}

static void
merge_new_observations(struct block_split_stats *stats)
{
	int i;

	for (i = 0; i < NUM_OBSERVATION_TYPES; i++) {
		stats->observations[i] += stats->new_observations[i];
		stats->new_observations[i] = 0;
	}
	stats->num_observations += stats->num_new_observations;
	stats->num_new_observations = 0;
}

static bool
do_end_block_check(struct block_split_stats *stats, u32 block_length)
{
	if (stats->num_observations > 0) {
		/*
		 * Compute the sum of absolute differences of probabilities.  To
		 * avoid needing to use floating point arithmetic or do slow
		 * divisions, we do all arithmetic with the probabilities
		 * multiplied by num_observations * num_new_observations.  E.g.,
		 * for the "old" observations the probabilities would be
		 * (double)observations[i] / num_observations, but since we
		 * multiply by both num_observations and num_new_observations we
		 * really do observations[i] * num_new_observations.
		 */
		u32 total_delta = 0;
		u32 num_items;
		u32 cutoff;
		int i;

		for (i = 0; i < NUM_OBSERVATION_TYPES; i++) {
			u32 expected = stats->observations[i] *
				       stats->num_new_observations;
			u32 actual = stats->new_observations[i] *
				     stats->num_observations;
			u32 delta = (actual > expected) ? actual - expected :
							  expected - actual;

			total_delta += delta;
		}

		num_items = stats->num_observations +
			    stats->num_new_observations;
		/*
		 * Heuristic: the cutoff is when the sum of absolute differences
		 * of probabilities becomes at least 200/512.  As above, the
		 * probability is multiplied by both num_new_observations and
		 * num_observations.  Be careful to avoid integer overflow.
		 */
		cutoff = stats->num_new_observations * 200 / 512 *
			 stats->num_observations;
		/*
		 * Very short blocks have a lot of overhead for the Huffman
		 * codes, so only use them if it clearly seems worthwhile.
		 * (This is an additional penalty, which adds to the smaller
		 * penalty below which scales more slowly.)
		 */
		if (block_length < 10000 && num_items < 8192)
			cutoff += (u64)cutoff * (8192 - num_items) / 8192;

		/* Ready to end the block? */
		if (total_delta +
		    (block_length / 4096) * stats->num_observations >= cutoff)
			return true;
	}
	merge_new_observations(stats);
	return false;
}

static forceinline bool
ready_to_check_block(const struct block_split_stats *stats,
		     const u8 *in_block_begin, const u8 *in_next,
		     const u8 *in_end)
{
	return stats->num_new_observations >= NUM_OBSERVATIONS_PER_BLOCK_CHECK
		&& in_next - in_block_begin >= MIN_BLOCK_LENGTH
		&& in_end - in_next >= MIN_BLOCK_LENGTH;
}

static forceinline bool
should_end_block(struct block_split_stats *stats,
		 const u8 *in_block_begin, const u8 *in_next, const u8 *in_end)
{
	/* Ready to try to end the block (again)? */
	if (!ready_to_check_block(stats, in_block_begin, in_next, in_end))
		return false;

	return do_end_block_check(stats, in_next - in_block_begin);
}

/******************************************************************************/

static void
deflate_begin_sequences(struct libdeflate_compressor *c,
			struct deflate_sequence *first_seq)
{
	deflate_reset_symbol_frequencies(c);
	first_seq->litrunlen_and_length = 0;
}

static forceinline void
deflate_choose_literal(struct libdeflate_compressor *c, unsigned literal,
		       bool gather_split_stats, struct deflate_sequence *seq)
{
	c->freqs.litlen[literal]++;

	if (gather_split_stats)
		observe_literal(&c->split_stats, literal);

	STATIC_ASSERT(MAX_BLOCK_LENGTH <= SEQ_LITRUNLEN_MASK);
	seq->litrunlen_and_length++;
}

static forceinline void
deflate_choose_match(struct libdeflate_compressor *c,
		     u32 length, u32 offset, bool gather_split_stats,
		     struct deflate_sequence **seq_p)
{
	struct deflate_sequence *seq = *seq_p;
	unsigned length_slot = deflate_length_slot[length];
	unsigned offset_slot = deflate_get_offset_slot(offset);

	c->freqs.litlen[DEFLATE_FIRST_LEN_SYM + length_slot]++;
	c->freqs.offset[offset_slot]++;
	if (gather_split_stats)
		observe_match(&c->split_stats, length);

	seq->litrunlen_and_length |= length << SEQ_LENGTH_SHIFT;
	seq->offset = offset;
	seq->offset_slot = offset_slot;

	seq++;
	seq->litrunlen_and_length = 0;
	*seq_p = seq;
}

/*
 * Decrease the maximum and nice match lengths if we're approaching the end of
 * the input buffer.
 */
static forceinline void
adjust_max_and_nice_len(u32 *max_len, u32 *nice_len, size_t remaining)
{
	if (unlikely(remaining < DEFLATE_MAX_MATCH_LEN)) {
		*max_len = remaining;
		*nice_len = MIN(*nice_len, *max_len);
	}
}

/*
 * Choose the minimum match length for the greedy and lazy parsers.
 *
 * By default the minimum match length is 3, which is the smallest length the
 * DEFLATE format allows.  However, with greedy and lazy parsing, some data
 * (e.g. DNA sequencing data) benefits greatly from a longer minimum length.
 * Typically, this is because literals are very cheap.  In general, the
 * near-optimal parser handles this case naturally, but the greedy and lazy
 * parsers need a heuristic to decide when to use short matches.
 *
 * The heuristic we use is to make the minimum match length depend on the number
 * of different literals that exist in the data.  If there are many different
 * literals, then literals will probably be expensive, so short matches will
 * probably be worthwhile.  Conversely, if not many literals are used, then
 * probably literals will be cheap and short matches won't be worthwhile.
 */
static u32
choose_min_match_len(u32 num_used_literals, u32 max_search_depth)
{
	/* map from num_used_literals to min_len */
	static const u8 min_lens[] = {
		9, 9, 9, 9, 9, 9, 8, 8, 7, 7, 6, 6, 6, 6, 6, 6,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		/* The rest is implicitly 3. */
	};
	u32 min_len;

	STATIC_ASSERT(DEFLATE_MIN_MATCH_LEN <= 3);
	STATIC_ASSERT(ARRAY_LEN(min_lens) <= DEFLATE_NUM_LITERALS + 1);

	if (num_used_literals >= ARRAY_LEN(min_lens))
		return 3;
	min_len = min_lens[num_used_literals];
	/*
	 * With a low max_search_depth, it may be too hard to find long matches.
	 */
	if (max_search_depth < 16) {
		if (max_search_depth < 5)
			min_len = MIN(min_len, 4);
		else if (max_search_depth < 10)
			min_len = MIN(min_len, 5);
		else
			min_len = MIN(min_len, 7);
	}
	return min_len;
}

static u32
calculate_min_match_len(const u8 *data, size_t data_len, u32 max_search_depth)
{
	u8 used[256] = { 0 };
	u32 num_used_literals = 0;
	size_t i;

	/*
	 * For very short inputs, the static Huffman code has a good chance of
	 * being best, in which case there is no reason to avoid short matches.
	 */
	if (data_len < 512)
		return DEFLATE_MIN_MATCH_LEN;

	/*
	 * For an initial approximation, scan the first 4 KiB of data.  The
	 * caller may use recalculate_min_match_len() to update min_len later.
	 */
	data_len = MIN(data_len, 4096);
	for (i = 0; i < data_len; i++)
		used[data[i]] = 1;
	for (i = 0; i < 256; i++)
		num_used_literals += used[i];
	return choose_min_match_len(num_used_literals, max_search_depth);
}

/*
 * Recalculate the minimum match length for a block, now that we know the
 * distribution of literals that are actually being used (freqs->litlen).
 */
static u32
recalculate_min_match_len(const struct deflate_freqs *freqs,
			  u32 max_search_depth)
{
	u32 literal_freq = 0;
	u32 cutoff;
	u32 num_used_literals = 0;
	int i;

	for (i = 0; i < DEFLATE_NUM_LITERALS; i++)
		literal_freq += freqs->litlen[i];

	cutoff = literal_freq >> 10; /* Ignore literals used very rarely. */

	for (i = 0; i < DEFLATE_NUM_LITERALS; i++) {
		if (freqs->litlen[i] > cutoff)
			num_used_literals++;
	}
	return choose_min_match_len(num_used_literals, max_search_depth);
}

static forceinline const u8 *
choose_max_block_end(const u8 *in_block_begin, const u8 *in_end,
		     size_t soft_max_len)
{
	if (in_end - in_block_begin < soft_max_len + MIN_BLOCK_LENGTH)
		return in_end;
	return in_block_begin + soft_max_len;
}

/*
 * This is the level 0 "compressor".  It always outputs uncompressed blocks.
 */
static size_t
deflate_compress_none(const u8 *in, size_t in_nbytes,
		      u8 *out, size_t out_nbytes_avail)
{
	const u8 *in_next = in;
	const u8 * const in_end = in + in_nbytes;
	u8 *out_next = out;
	u8 * const out_end = out + out_nbytes_avail;

	/*
	 * If the input is zero-length, we still must output a block in order
	 * for the output to be a valid DEFLATE stream.  Handle this case
	 * specially to avoid potentially passing NULL to memcpy() below.
	 */
	if (unlikely(in_nbytes == 0)) {
		if (out_nbytes_avail < 5)
			return 0;
		/* BFINAL and BTYPE */
		*out_next++ = 1 | (DEFLATE_BLOCKTYPE_UNCOMPRESSED << 1);
		/* LEN and NLEN */
		put_unaligned_le32(0xFFFF0000, out_next);
		return 5;
	}

	do {
		u8 bfinal = 0;
		size_t len = UINT16_MAX;

		if (in_end - in_next <= UINT16_MAX) {
			bfinal = 1;
			len = in_end - in_next;
		}
		if (out_end - out_next < 5 + len)
			return 0;
		/*
		 * Output BFINAL and BTYPE.  The stream is already byte-aligned
		 * here, so this step always requires outputting exactly 1 byte.
		 */
		*out_next++ = bfinal | (DEFLATE_BLOCKTYPE_UNCOMPRESSED << 1);

		/* Output LEN and NLEN, then the data itself. */
		put_unaligned_le16(len, out_next);
		out_next += 2;
		put_unaligned_le16(~len, out_next);
		out_next += 2;
		memcpy(out_next, in_next, len);
		out_next += len;
		in_next += len;
	} while (in_next != in_end);

	return out_next - out;
}

/*
 * This is a faster variant of deflate_compress_greedy().  It uses the
 * ht_matchfinder rather than the hc_matchfinder.  It also skips the block
 * splitting algorithm and just uses fixed length blocks.  c->max_search_depth
 * has no effect with this algorithm, as it is hardcoded in ht_matchfinder.h.
 */
static void
deflate_compress_fastest(struct libdeflate_compressor * restrict c,
			 const u8 *in, size_t in_nbytes,
			 struct deflate_output_bitstream *os)
{
	const u8 *in_next = in;
	const u8 *in_end = in_next + in_nbytes;
	const u8 *in_cur_base = in_next;
	u32 max_len = DEFLATE_MAX_MATCH_LEN;
	u32 nice_len = MIN(c->nice_match_length, max_len);
	u32 next_hash = 0;

	ht_matchfinder_init(&c->p.f.ht_mf);

	do {
		/* Starting a new DEFLATE block */

		const u8 * const in_block_begin = in_next;
		const u8 * const in_max_block_end = choose_max_block_end(
				in_next, in_end, FAST_SOFT_MAX_BLOCK_LENGTH);
		struct deflate_sequence *seq = c->p.f.sequences;

		deflate_begin_sequences(c, seq);

		do {
			u32 length;
			u32 offset;
			size_t remaining = in_end - in_next;

			if (unlikely(remaining < DEFLATE_MAX_MATCH_LEN)) {
				max_len = remaining;
				if (max_len < HT_MATCHFINDER_REQUIRED_NBYTES) {
					do {
						deflate_choose_literal(c,
							*in_next++, false, seq);
					} while (--max_len);
					break;
				}
				nice_len = MIN(nice_len, max_len);
			}
			length = ht_matchfinder_longest_match(&c->p.f.ht_mf,
							      &in_cur_base,
							      in_next,
							      max_len,
							      nice_len,
							      &next_hash,
							      &offset);
			if (length) {
				/* Match found */
				deflate_choose_match(c, length, offset, false,
						     &seq);
				ht_matchfinder_skip_bytes(&c->p.f.ht_mf,
							  &in_cur_base,
							  in_next + 1,
							  in_end,
							  length - 1,
							  &next_hash);
				in_next += length;
			} else {
				/* No match found */
				deflate_choose_literal(c, *in_next++, false,
						       seq);
			}

			/* Check if it's time to output another block. */
		} while (in_next < in_max_block_end &&
			 seq < &c->p.f.sequences[FAST_SEQ_STORE_LENGTH]);

		deflate_finish_block(c, os, in_block_begin,
				     in_next - in_block_begin,
				     c->p.f.sequences, in_next == in_end);
	} while (in_next != in_end && !os->overflow);
}

/*
 * This is the "greedy" DEFLATE compressor. It always chooses the longest match.
 */
static void
deflate_compress_greedy(struct libdeflate_compressor * restrict c,
			const u8 *in, size_t in_nbytes,
			struct deflate_output_bitstream *os)
{
	const u8 *in_next = in;
	const u8 *in_end = in_next + in_nbytes;
	const u8 *in_cur_base = in_next;
	u32 max_len = DEFLATE_MAX_MATCH_LEN;
	u32 nice_len = MIN(c->nice_match_length, max_len);
	u32 next_hashes[2] = {0, 0};

	hc_matchfinder_init(&c->p.g.hc_mf);

	do {
		/* Starting a new DEFLATE block */

		const u8 * const in_block_begin = in_next;
		const u8 * const in_max_block_end = choose_max_block_end(
				in_next, in_end, SOFT_MAX_BLOCK_LENGTH);
		struct deflate_sequence *seq = c->p.g.sequences;
		u32 min_len;

		init_block_split_stats(&c->split_stats);
		deflate_begin_sequences(c, seq);
		min_len = calculate_min_match_len(in_next,
						  in_max_block_end - in_next,
						  c->max_search_depth);
		do {
			u32 length;
			u32 offset;

			adjust_max_and_nice_len(&max_len, &nice_len,
						in_end - in_next);
			length = hc_matchfinder_longest_match(
						&c->p.g.hc_mf,
						&in_cur_base,
						in_next,
						min_len - 1,
						max_len,
						nice_len,
						c->max_search_depth,
						next_hashes,
						&offset);

			if (length >= min_len &&
			    (length > DEFLATE_MIN_MATCH_LEN ||
			     offset <= 4096)) {
				/* Match found */
				deflate_choose_match(c, length, offset, true,
						     &seq);
				hc_matchfinder_skip_bytes(&c->p.g.hc_mf,
							  &in_cur_base,
							  in_next + 1,
							  in_end,
							  length - 1,
							  next_hashes);
				in_next += length;
			} else {
				/* No match found */
				deflate_choose_literal(c, *in_next++, true,
						       seq);
			}

			/* Check if it's time to output another block. */
		} while (in_next < in_max_block_end &&
			 seq < &c->p.g.sequences[SEQ_STORE_LENGTH] &&
			 !should_end_block(&c->split_stats,
					   in_block_begin, in_next, in_end));

		deflate_finish_block(c, os, in_block_begin,
				     in_next - in_block_begin,
				     c->p.g.sequences, in_next == in_end);
	} while (in_next != in_end && !os->overflow);
}

static forceinline void
deflate_compress_lazy_generic(struct libdeflate_compressor * restrict c,
			      const u8 *in, size_t in_nbytes,
			      struct deflate_output_bitstream *os, bool lazy2)
{
	const u8 *in_next = in;
	const u8 *in_end = in_next + in_nbytes;
	const u8 *in_cur_base = in_next;
	u32 max_len = DEFLATE_MAX_MATCH_LEN;
	u32 nice_len = MIN(c->nice_match_length, max_len);
	u32 next_hashes[2] = {0, 0};

	hc_matchfinder_init(&c->p.g.hc_mf);

	do {
		/* Starting a new DEFLATE block */

		const u8 * const in_block_begin = in_next;
		const u8 * const in_max_block_end = choose_max_block_end(
				in_next, in_end, SOFT_MAX_BLOCK_LENGTH);
		const u8 *next_recalc_min_len =
			in_next + MIN(in_end - in_next, 10000);
		struct deflate_sequence *seq = c->p.g.sequences;
		u32 min_len;

		init_block_split_stats(&c->split_stats);
		deflate_begin_sequences(c, seq);
		min_len = calculate_min_match_len(in_next,
						  in_max_block_end - in_next,
						  c->max_search_depth);
		do {
			u32 cur_len;
			u32 cur_offset;
			u32 next_len;
			u32 next_offset;

			/*
			 * Recalculate the minimum match length if it hasn't
			 * been done recently.
			 */
			if (in_next >= next_recalc_min_len) {
				min_len = recalculate_min_match_len(
						&c->freqs,
						c->max_search_depth);
				next_recalc_min_len +=
					MIN(in_end - next_recalc_min_len,
					    in_next - in_block_begin);
			}

			/* Find the longest match at the current position. */
			adjust_max_and_nice_len(&max_len, &nice_len,
						in_end - in_next);
			cur_len = hc_matchfinder_longest_match(
						&c->p.g.hc_mf,
						&in_cur_base,
						in_next,
						min_len - 1,
						max_len,
						nice_len,
						c->max_search_depth,
						next_hashes,
						&cur_offset);
			if (cur_len < min_len ||
			    (cur_len == DEFLATE_MIN_MATCH_LEN &&
			     cur_offset > 8192)) {
				/* No match found.  Choose a literal. */
				deflate_choose_literal(c, *in_next++, true,
						       seq);
				continue;
			}
			in_next++;

have_cur_match:
			/*
			 * We have a match at the current position.
			 * If it's very long, choose it immediately.
			 */
			if (cur_len >= nice_len) {
				deflate_choose_match(c, cur_len, cur_offset,
						     true, &seq);
				hc_matchfinder_skip_bytes(&c->p.g.hc_mf,
							  &in_cur_base,
							  in_next,
							  in_end,
							  cur_len - 1,
							  next_hashes);
				in_next += cur_len - 1;
				continue;
			}

			/*
			 * Try to find a better match at the next position.
			 *
			 * Note: since we already have a match at the *current*
			 * position, we use only half the 'max_search_depth'
			 * when checking the *next* position.  This is a useful
			 * trade-off because it's more worthwhile to use a
			 * greater search depth on the initial match.
			 *
			 * Note: it's possible to structure the code such that
			 * there's only one call to longest_match(), which
			 * handles both the "find the initial match" and "try to
			 * find a better match" cases.  However, it is faster to
			 * have two call sites, with longest_match() inlined at
			 * each.
			 */
			adjust_max_and_nice_len(&max_len, &nice_len,
						in_end - in_next);
			next_len = hc_matchfinder_longest_match(
						&c->p.g.hc_mf,
						&in_cur_base,
						in_next++,
						cur_len - 1,
						max_len,
						nice_len,
						c->max_search_depth >> 1,
						next_hashes,
						&next_offset);
			if (next_len >= cur_len &&
			    4 * (int)(next_len - cur_len) +
			    ((int)bsr32(cur_offset) -
			     (int)bsr32(next_offset)) > 2) {
				/*
				 * Found a better match at the next position.
				 * Output a literal.  Then the next match
				 * becomes the current match.
				 */
				deflate_choose_literal(c, *(in_next - 2), true,
						       seq);
				cur_len = next_len;
				cur_offset = next_offset;
				goto have_cur_match;
			}

			if (lazy2) {
				/* In lazy2 mode, look ahead another position */
				adjust_max_and_nice_len(&max_len, &nice_len,
							in_end - in_next);
				next_len = hc_matchfinder_longest_match(
						&c->p.g.hc_mf,
						&in_cur_base,
						in_next++,
						cur_len - 1,
						max_len,
						nice_len,
						c->max_search_depth >> 2,
						next_hashes,
						&next_offset);
				if (next_len >= cur_len &&
				    4 * (int)(next_len - cur_len) +
				    ((int)bsr32(cur_offset) -
				     (int)bsr32(next_offset)) > 6) {
					/*
					 * There's a much better match two
					 * positions ahead, so use two literals.
					 */
					deflate_choose_literal(
						c, *(in_next - 3), true, seq);
					deflate_choose_literal(
						c, *(in_next - 2), true, seq);
					cur_len = next_len;
					cur_offset = next_offset;
					goto have_cur_match;
				}
				/*
				 * No better match at either of the next 2
				 * positions.  Output the current match.
				 */
				deflate_choose_match(c, cur_len, cur_offset,
						     true, &seq);
				if (cur_len > 3) {
					hc_matchfinder_skip_bytes(&c->p.g.hc_mf,
								  &in_cur_base,
								  in_next,
								  in_end,
								  cur_len - 3,
								  next_hashes);
					in_next += cur_len - 3;
				}
			} else { /* !lazy2 */
				/*
				 * No better match at the next position.  Output
				 * the current match.
				 */
				deflate_choose_match(c, cur_len, cur_offset,
						     true, &seq);
				hc_matchfinder_skip_bytes(&c->p.g.hc_mf,
							  &in_cur_base,
							  in_next,
							  in_end,
							  cur_len - 2,
							  next_hashes);
				in_next += cur_len - 2;
			}
			/* Check if it's time to output another block. */
		} while (in_next < in_max_block_end &&
			 seq < &c->p.g.sequences[SEQ_STORE_LENGTH] &&
			 !should_end_block(&c->split_stats,
					   in_block_begin, in_next, in_end));

		deflate_finish_block(c, os, in_block_begin,
				     in_next - in_block_begin,
				     c->p.g.sequences, in_next == in_end);
	} while (in_next != in_end && !os->overflow);
}

/*
 * This is the "lazy" DEFLATE compressor.  Before choosing a match, it checks to
 * see if there's a better match at the next position.  If yes, it outputs a
 * literal and continues to the next position.  If no, it outputs the match.
 */
static void
deflate_compress_lazy(struct libdeflate_compressor * restrict c,
		      const u8 *in, size_t in_nbytes,
		      struct deflate_output_bitstream *os)
{
	deflate_compress_lazy_generic(c, in, in_nbytes, os, false);
}

/*
 * The lazy2 compressor.  This is similar to the regular lazy one, but it looks
 * for a better match at the next 2 positions rather than the next 1.  This
 * makes it take slightly more time, but compress some inputs slightly more.
 */
static void
deflate_compress_lazy2(struct libdeflate_compressor * restrict c,
		       const u8 *in, size_t in_nbytes,
		       struct deflate_output_bitstream *os)
{
	deflate_compress_lazy_generic(c, in, in_nbytes, os, true);
}

#if SUPPORT_NEAR_OPTIMAL_PARSING

/*
 * Follow the minimum-cost path in the graph of possible match/literal choices
 * for the current block and compute the frequencies of the Huffman symbols that
 * would be needed to output those matches and literals.
 */
static void
deflate_tally_item_list(struct libdeflate_compressor *c, u32 block_length)
{
	struct deflate_optimum_node *cur_node = &c->p.n.optimum_nodes[0];
	struct deflate_optimum_node *end_node =
		&c->p.n.optimum_nodes[block_length];

	do {
		u32 length = cur_node->item & OPTIMUM_LEN_MASK;
		u32 offset = cur_node->item >> OPTIMUM_OFFSET_SHIFT;

		if (length == 1) {
			/* Literal */
			c->freqs.litlen[offset]++;
		} else {
			/* Match */
			c->freqs.litlen[DEFLATE_FIRST_LEN_SYM +
					deflate_length_slot[length]]++;
			c->freqs.offset[c->p.n.offset_slot_full[offset]]++;
		}
		cur_node += length;
	} while (cur_node != end_node);

	/* Tally the end-of-block symbol. */
	c->freqs.litlen[DEFLATE_END_OF_BLOCK]++;
}

static void
deflate_choose_all_literals(struct libdeflate_compressor *c,
			    const u8 *block, u32 block_length)
{
	u32 i;

	deflate_reset_symbol_frequencies(c);
	for (i = 0; i < block_length; i++)
		c->freqs.litlen[block[i]]++;
	c->freqs.litlen[DEFLATE_END_OF_BLOCK]++;

	deflate_make_huffman_codes(&c->freqs, &c->codes);
}

/*
 * Compute the exact cost, in bits, that would be required to output the matches
 * and literals described by @c->freqs as a dynamic Huffman block.  The litlen
 * and offset codes are assumed to have already been built in @c->codes.
 */
static u32
deflate_compute_true_cost(struct libdeflate_compressor *c)
{
	u32 cost = 0;
	unsigned sym;

	deflate_precompute_huffman_header(c);

	memset(&c->codes.lens.litlen[c->o.precode.num_litlen_syms], 0,
	       DEFLATE_NUM_LITLEN_SYMS - c->o.precode.num_litlen_syms);

	cost += 5 + 5 + 4 + (3 * c->o.precode.num_explicit_lens);
	for (sym = 0; sym < DEFLATE_NUM_PRECODE_SYMS; sym++) {
		cost += c->o.precode.freqs[sym] *
			(c->o.precode.lens[sym] +
			 deflate_extra_precode_bits[sym]);
	}

	for (sym = 0; sym < DEFLATE_FIRST_LEN_SYM; sym++)
		cost += c->freqs.litlen[sym] * c->codes.lens.litlen[sym];

	for (; sym < DEFLATE_FIRST_LEN_SYM +
	       ARRAY_LEN(deflate_extra_length_bits); sym++)
		cost += c->freqs.litlen[sym] *
			(c->codes.lens.litlen[sym] +
			 deflate_extra_length_bits[sym - DEFLATE_FIRST_LEN_SYM]);

	for (sym = 0; sym < ARRAY_LEN(deflate_extra_offset_bits); sym++)
		cost += c->freqs.offset[sym] *
			(c->codes.lens.offset[sym] +
			 deflate_extra_offset_bits[sym]);
	return cost;
}

/* Set the current cost model from the codeword lengths specified in @lens. */
static void
deflate_set_costs_from_codes(struct libdeflate_compressor *c,
			     const struct deflate_lens *lens)
{
	unsigned i;

	/* Literals */
	for (i = 0; i < DEFLATE_NUM_LITERALS; i++) {
		u32 bits = (lens->litlen[i] ?
			    lens->litlen[i] : LITERAL_NOSTAT_BITS);

		c->p.n.costs.literal[i] = bits * BIT_COST;
	}

	/* Lengths */
	for (i = DEFLATE_MIN_MATCH_LEN; i <= DEFLATE_MAX_MATCH_LEN; i++) {
		unsigned length_slot = deflate_length_slot[i];
		unsigned litlen_sym = DEFLATE_FIRST_LEN_SYM + length_slot;
		u32 bits = (lens->litlen[litlen_sym] ?
			    lens->litlen[litlen_sym] : LENGTH_NOSTAT_BITS);

		bits += deflate_extra_length_bits[length_slot];
		c->p.n.costs.length[i] = bits * BIT_COST;
	}

	/* Offset slots */
	for (i = 0; i < ARRAY_LEN(deflate_offset_slot_base); i++) {
		u32 bits = (lens->offset[i] ?
			    lens->offset[i] : OFFSET_NOSTAT_BITS);

		bits += deflate_extra_offset_bits[i];
		c->p.n.costs.offset_slot[i] = bits * BIT_COST;
	}
}

/*
 * This lookup table gives the default cost of a literal symbol and of a length
 * symbol, depending on the characteristics of the input data.  It was generated
 * by scripts/gen_default_litlen_costs.py.
 *
 * This table is indexed first by the estimated match probability:
 *
 *	i=0: data doesn't contain many matches	[match_prob=0.25]
 *	i=1: neutral				[match_prob=0.50]
 *	i=2: data contains lots of matches	[match_prob=0.75]
 *
 * This lookup produces a subtable which maps the number of distinct used
 * literals to the default cost of a literal symbol, i.e.:
 *
 *	int(-log2((1 - match_prob) / num_used_literals) * BIT_COST)
 *
 * ... for num_used_literals in [1, 256] (and 0, which is copied from 1).  This
 * accounts for literals usually getting cheaper as the number of distinct
 * literals decreases, and as the proportion of literals to matches increases.
 *
 * The lookup also produces the cost of a length symbol, which is:
 *
 *	int(-log2(match_prob/NUM_LEN_SLOTS) * BIT_COST)
 *
 * Note: we don't currently assign different costs to different literal symbols,
 * or to different length symbols, as this is hard to do in a useful way.
 */
static const struct {
	u8 used_lits_to_lit_cost[257];
	u8 len_sym_cost;
} default_litlen_costs[] = {
	{ /* match_prob = 0.25 */
		.used_lits_to_lit_cost = {
			6, 6, 22, 32, 38, 43, 48, 51,
			54, 57, 59, 61, 64, 65, 67, 69,
			70, 72, 73, 74, 75, 76, 77, 79,
			80, 80, 81, 82, 83, 84, 85, 85,
			86, 87, 88, 88, 89, 89, 90, 91,
			91, 92, 92, 93, 93, 94, 95, 95,
			96, 96, 96, 97, 97, 98, 98, 99,
			99, 99, 100, 100, 101, 101, 101, 102,
			102, 102, 103, 103, 104, 104, 104, 105,
			105, 105, 105, 106, 106, 106, 107, 107,
			107, 108, 108, 108, 108, 109, 109, 109,
			109, 110, 110, 110, 111, 111, 111, 111,
			112, 112, 112, 112, 112, 113, 113, 113,
			113, 114, 114, 114, 114, 114, 115, 115,
			115, 115, 115, 116, 116, 116, 116, 116,
			117, 117, 117, 117, 117, 118, 118, 118,
			118, 118, 118, 119, 119, 119, 119, 119,
			120, 120, 120, 120, 120, 120, 121, 121,
			121, 121, 121, 121, 121, 122, 122, 122,
			122, 122, 122, 123, 123, 123, 123, 123,
			123, 123, 124, 124, 124, 124, 124, 124,
			124, 125, 125, 125, 125, 125, 125, 125,
			125, 126, 126, 126, 126, 126, 126, 126,
			127, 127, 127, 127, 127, 127, 127, 127,
			128, 128, 128, 128, 128, 128, 128, 128,
			128, 129, 129, 129, 129, 129, 129, 129,
			129, 129, 130, 130, 130, 130, 130, 130,
			130, 130, 130, 131, 131, 131, 131, 131,
			131, 131, 131, 131, 131, 132, 132, 132,
			132, 132, 132, 132, 132, 132, 132, 133,
			133, 133, 133, 133, 133, 133, 133, 133,
			133, 134, 134, 134, 134, 134, 134, 134,
			134,
		},
		.len_sym_cost = 109,
	}, { /* match_prob = 0.5 */
		.used_lits_to_lit_cost = {
			16, 16, 32, 41, 48, 53, 57, 60,
			64, 66, 69, 71, 73, 75, 76, 78,
			80, 81, 82, 83, 85, 86, 87, 88,
			89, 90, 91, 92, 92, 93, 94, 95,
			96, 96, 97, 98, 98, 99, 99, 100,
			101, 101, 102, 102, 103, 103, 104, 104,
			105, 105, 106, 106, 107, 107, 108, 108,
			108, 109, 109, 110, 110, 110, 111, 111,
			112, 112, 112, 113, 113, 113, 114, 114,
			114, 115, 115, 115, 115, 116, 116, 116,
			117, 117, 117, 118, 118, 118, 118, 119,
			119, 119, 119, 120, 120, 120, 120, 121,
			121, 121, 121, 122, 122, 122, 122, 122,
			123, 123, 123, 123, 124, 124, 124, 124,
			124, 125, 125, 125, 125, 125, 126, 126,
			126, 126, 126, 127, 127, 127, 127, 127,
			128, 128, 128, 128, 128, 128, 129, 129,
			129, 129, 129, 129, 130, 130, 130, 130,
			130, 130, 131, 131, 131, 131, 131, 131,
			131, 132, 132, 132, 132, 132, 132, 133,
			133, 133, 133, 133, 133, 133, 134, 134,
			134, 134, 134, 134, 134, 134, 135, 135,
			135, 135, 135, 135, 135, 135, 136, 136,
			136, 136, 136, 136, 136, 136, 137, 137,
			137, 137, 137, 137, 137, 137, 138, 138,
			138, 138, 138, 138, 138, 138, 138, 139,
			139, 139, 139, 139, 139, 139, 139, 139,
			140, 140, 140, 140, 140, 140, 140, 140,
			140, 141, 141, 141, 141, 141, 141, 141,
			141, 141, 141, 142, 142, 142, 142, 142,
			142, 142, 142, 142, 142, 142, 143, 143,
			143, 143, 143, 143, 143, 143, 143, 143,
			144,
		},
		.len_sym_cost = 93,
	}, { /* match_prob = 0.75 */
		.used_lits_to_lit_cost = {
			32, 32, 48, 57, 64, 69, 73, 76,
			80, 82, 85, 87, 89, 91, 92, 94,
			96, 97, 98, 99, 101, 102, 103, 104,
			105, 106, 107, 108, 108, 109, 110, 111,
			112, 112, 113, 114, 114, 115, 115, 116,
			117, 117, 118, 118, 119, 119, 120, 120,
			121, 121, 122, 122, 123, 123, 124, 124,
			124, 125, 125, 126, 126, 126, 127, 127,
			128, 128, 128, 129, 129, 129, 130, 130,
			130, 131, 131, 131, 131, 132, 132, 132,
			133, 133, 133, 134, 134, 134, 134, 135,
			135, 135, 135, 136, 136, 136, 136, 137,
			137, 137, 137, 138, 138, 138, 138, 138,
			139, 139, 139, 139, 140, 140, 140, 140,
			140, 141, 141, 141, 141, 141, 142, 142,
			142, 142, 142, 143, 143, 143, 143, 143,
			144, 144, 144, 144, 144, 144, 145, 145,
			145, 145, 145, 145, 146, 146, 146, 146,
			146, 146, 147, 147, 147, 147, 147, 147,
			147, 148, 148, 148, 148, 148, 148, 149,
			149, 149, 149, 149, 149, 149, 150, 150,
			150, 150, 150, 150, 150, 150, 151, 151,
			151, 151, 151, 151, 151, 151, 152, 152,
			152, 152, 152, 152, 152, 152, 153, 153,
			153, 153, 153, 153, 153, 153, 154, 154,
			154, 154, 154, 154, 154, 154, 154, 155,
			155, 155, 155, 155, 155, 155, 155, 155,
			156, 156, 156, 156, 156, 156, 156, 156,
			156, 157, 157, 157, 157, 157, 157, 157,
			157, 157, 157, 158, 158, 158, 158, 158,
			158, 158, 158, 158, 158, 158, 159, 159,
			159, 159, 159, 159, 159, 159, 159, 159,
			160,
		},
		.len_sym_cost = 84,
	},
};

/*
 * Choose the default costs for literal and length symbols.  These symbols are
 * both part of the litlen alphabet.
 */
static void
deflate_choose_default_litlen_costs(struct libdeflate_compressor *c,
				    const u8 *block_begin, u32 block_length,
				    u32 *lit_cost, u32 *len_sym_cost)
{
	u32 num_used_literals = 0;
	u32 literal_freq = block_length;
	u32 match_freq = 0;
	u32 cutoff;
	u32 i;

	/* Calculate the number of distinct literals that exist in the data. */
	memset(c->freqs.litlen, 0,
	       DEFLATE_NUM_LITERALS * sizeof(c->freqs.litlen[0]));
	cutoff = literal_freq >> 11; /* Ignore literals used very rarely. */
	for (i = 0; i < block_length; i++)
		c->freqs.litlen[block_begin[i]]++;
	for (i = 0; i < DEFLATE_NUM_LITERALS; i++) {
		if (c->freqs.litlen[i] > cutoff)
			num_used_literals++;
	}
	if (num_used_literals == 0)
		num_used_literals = 1;

	/*
	 * Estimate the relative frequency of literals and matches in the
	 * optimal parsing solution.  We don't know the optimal solution, so
	 * this can only be a very rough estimate.  Therefore, we basically use
	 * the match frequency from a greedy parse.  We also apply the min_len
	 * heuristic used by the greedy and lazy parsers, to avoid counting too
	 * many matches when literals are cheaper than short matches.
	 */
	match_freq = 0;
	i = choose_min_match_len(num_used_literals, c->max_search_depth);
	for (; i < ARRAY_LEN(c->p.n.match_len_freqs); i++) {
		match_freq += c->p.n.match_len_freqs[i];
		literal_freq -= i * c->p.n.match_len_freqs[i];
	}
	if ((s32)literal_freq < 0) /* shouldn't happen */
		literal_freq = 0;

	if (match_freq > literal_freq)
		i = 2; /* many matches */
	else if (match_freq * 4 > literal_freq)
		i = 1; /* neutral */
	else
		i = 0; /* few matches */

	STATIC_ASSERT(BIT_COST == 16);
	*lit_cost = default_litlen_costs[i].used_lits_to_lit_cost[
							num_used_literals];
	*len_sym_cost = default_litlen_costs[i].len_sym_cost;
}

static forceinline u32
deflate_default_length_cost(u32 len, u32 len_sym_cost)
{
	unsigned slot = deflate_length_slot[len];
	u32 num_extra_bits = deflate_extra_length_bits[slot];

	return len_sym_cost + (num_extra_bits * BIT_COST);
}

static forceinline u32
deflate_default_offset_slot_cost(unsigned slot)
{
	u32 num_extra_bits = deflate_extra_offset_bits[slot];
	/*
	 * Assume that all offset symbols are equally probable.
	 * The resulting cost is 'int(-log2(1/30) * BIT_COST)',
	 * where 30 is the number of potentially-used offset symbols.
	 */
	u32 offset_sym_cost = 4*BIT_COST + (907*BIT_COST)/1000;

	return offset_sym_cost + (num_extra_bits * BIT_COST);
}

/* Set default symbol costs for the first block's first optimization pass. */
static void
deflate_set_default_costs(struct libdeflate_compressor *c,
			  u32 lit_cost, u32 len_sym_cost)
{
	u32 i;

	/* Literals */
	for (i = 0; i < DEFLATE_NUM_LITERALS; i++)
		c->p.n.costs.literal[i] = lit_cost;

	/* Lengths */
	for (i = DEFLATE_MIN_MATCH_LEN; i <= DEFLATE_MAX_MATCH_LEN; i++)
		c->p.n.costs.length[i] =
			deflate_default_length_cost(i, len_sym_cost);

	/* Offset slots */
	for (i = 0; i < ARRAY_LEN(deflate_offset_slot_base); i++)
		c->p.n.costs.offset_slot[i] =
			deflate_default_offset_slot_cost(i);
}

static forceinline void
deflate_adjust_cost(u32 *cost_p, u32 default_cost, int change_amount)
{
	if (change_amount == 0)
		/* Block is very similar to previous; prefer previous costs. */
		*cost_p = (default_cost + 3 * *cost_p) / 4;
	else if (change_amount == 1)
		*cost_p = (default_cost + *cost_p) / 2;
	else if (change_amount == 2)
		*cost_p = (5 * default_cost + 3 * *cost_p) / 8;
	else
		/* Block differs greatly from previous; prefer default costs. */
		*cost_p = (3 * default_cost + *cost_p) / 4;
}

static forceinline void
deflate_adjust_costs_impl(struct libdeflate_compressor *c,
			  u32 lit_cost, u32 len_sym_cost, int change_amount)
{
	u32 i;

	/* Literals */
	for (i = 0; i < DEFLATE_NUM_LITERALS; i++)
		deflate_adjust_cost(&c->p.n.costs.literal[i], lit_cost,
				    change_amount);

	/* Lengths */
	for (i = DEFLATE_MIN_MATCH_LEN; i <= DEFLATE_MAX_MATCH_LEN; i++)
		deflate_adjust_cost(&c->p.n.costs.length[i],
				    deflate_default_length_cost(i,
								len_sym_cost),
				    change_amount);

	/* Offset slots */
	for (i = 0; i < ARRAY_LEN(deflate_offset_slot_base); i++)
		deflate_adjust_cost(&c->p.n.costs.offset_slot[i],
				    deflate_default_offset_slot_cost(i),
				    change_amount);
}

/*
 * Adjust the costs when beginning a new block.
 *
 * Since the current costs are optimized for the data already, it can be helpful
 * to reuse them instead of starting over with the default costs.  However, this
 * depends on how similar the new block is to the previous block.  Therefore,
 * use a heuristic to decide how similar the blocks are, and mix together the
 * current costs and the default costs accordingly.
 */
static void
deflate_adjust_costs(struct libdeflate_compressor *c,
		     u32 lit_cost, u32 len_sym_cost)
{
	u64 total_delta = 0;
	u64 cutoff;
	int i;

	/*
	 * Decide how different the current block is from the previous block,
	 * using the block splitting statistics from the current and previous
	 * blocks.  The more different the current block is, the more we prefer
	 * the default costs rather than the previous block's costs.
	 *
	 * The algorithm here is similar to the end-of-block check one, but here
	 * we compare two entire blocks rather than a partial block with a small
	 * extra part, and therefore we need 64-bit numbers in some places.
	 */
	for (i = 0; i < NUM_OBSERVATION_TYPES; i++) {
		u64 prev = (u64)c->p.n.prev_observations[i] *
			    c->split_stats.num_observations;
		u64 cur = (u64)c->split_stats.observations[i] *
			  c->p.n.prev_num_observations;

		total_delta += prev > cur ? prev - cur : cur - prev;
	}
	cutoff = ((u64)c->p.n.prev_num_observations *
		  c->split_stats.num_observations * 200) / 512;

	if (total_delta > 3 * cutoff)
		/* Big change in the data; just use the default costs. */
		deflate_set_default_costs(c, lit_cost, len_sym_cost);
	else if (4 * total_delta > 9 * cutoff)
		deflate_adjust_costs_impl(c, lit_cost, len_sym_cost, 3);
	else if (2 * total_delta > 3 * cutoff)
		deflate_adjust_costs_impl(c, lit_cost, len_sym_cost, 2);
	else if (2 * total_delta > cutoff)
		deflate_adjust_costs_impl(c, lit_cost, len_sym_cost, 1);
	else
		deflate_adjust_costs_impl(c, lit_cost, len_sym_cost, 0);
}

static void
deflate_set_initial_costs(struct libdeflate_compressor *c,
			  const u8 *block_begin, u32 block_length,
			  bool is_first_block)
{
	u32 lit_cost, len_sym_cost;

	deflate_choose_default_litlen_costs(c, block_begin, block_length,
					    &lit_cost, &len_sym_cost);
	if (is_first_block)
		deflate_set_default_costs(c, lit_cost, len_sym_cost);
	else
		deflate_adjust_costs(c, lit_cost, len_sym_cost);
}

/*
 * Find the minimum-cost path through the graph of possible match/literal
 * choices for this block.
 *
 * We find the minimum cost path from 'c->p.n.optimum_nodes[0]', which
 * represents the node at the beginning of the block, to
 * 'c->p.n.optimum_nodes[block_length]', which represents the node at the end of
 * the block.  Edge costs are evaluated using the cost model 'c->p.n.costs'.
 *
 * The algorithm works backwards, starting at the end node and proceeding
 * backwards one node at a time.  At each node, the minimum cost to reach the
 * end node is computed and the match/literal choice that begins that path is
 * saved.
 */
static void
deflate_find_min_cost_path(struct libdeflate_compressor *c,
			   const u32 block_length,
			   const struct lz_match *cache_ptr)
{
	struct deflate_optimum_node *end_node =
		&c->p.n.optimum_nodes[block_length];
	struct deflate_optimum_node *cur_node = end_node;

	cur_node->cost_to_end = 0;
	do {
		unsigned num_matches;
		u32 literal;
		u32 best_cost_to_end;

		cur_node--;
		cache_ptr--;

		num_matches = cache_ptr->length;
		literal = cache_ptr->offset;

		/* It's always possible to choose a literal. */
		best_cost_to_end = c->p.n.costs.literal[literal] +
				   (cur_node + 1)->cost_to_end;
		cur_node->item = (literal << OPTIMUM_OFFSET_SHIFT) | 1;

		/* Also consider matches if there are any. */
		if (num_matches) {
			const struct lz_match *match;
			u32 len;
			u32 offset;
			u32 offset_slot;
			u32 offset_cost;
			u32 cost_to_end;

			/*
			 * Consider each length from the minimum
			 * (DEFLATE_MIN_MATCH_LEN) to the length of the longest
			 * match found at this position.  For each length, we
			 * consider only the smallest offset for which that
			 * length is available.  Although this is not guaranteed
			 * to be optimal due to the possibility of a larger
			 * offset costing less than a smaller offset to code,
			 * this is a very useful heuristic.
			 */
			match = cache_ptr - num_matches;
			len = DEFLATE_MIN_MATCH_LEN;
			do {
				offset = match->offset;
				offset_slot = c->p.n.offset_slot_full[offset];
				offset_cost =
					c->p.n.costs.offset_slot[offset_slot];
				do {
					cost_to_end = offset_cost +
						c->p.n.costs.length[len] +
						(cur_node + len)->cost_to_end;
					if (cost_to_end < best_cost_to_end) {
						best_cost_to_end = cost_to_end;
						cur_node->item = len |
							(offset <<
							 OPTIMUM_OFFSET_SHIFT);
					}
				} while (++len <= match->length);
			} while (++match != cache_ptr);
			cache_ptr -= num_matches;
		}
		cur_node->cost_to_end = best_cost_to_end;
	} while (cur_node != &c->p.n.optimum_nodes[0]);

	deflate_reset_symbol_frequencies(c);
	deflate_tally_item_list(c, block_length);
	deflate_make_huffman_codes(&c->freqs, &c->codes);
}

/*
 * Choose the literals and matches for the current block, then output the block.
 *
 * To choose the literal/match sequence, we find the minimum-cost path through
 * the block's graph of literal/match choices, given a cost model.  However, the
 * true cost of each symbol is unknown until the Huffman codes have been built,
 * but at the same time the Huffman codes depend on the frequencies of chosen
 * symbols.  Consequently, multiple passes must be used to try to approximate an
 * optimal solution.  The first pass uses default costs, mixed with the costs
 * from the previous block when it seems appropriate.  Later passes use the
 * Huffman codeword lengths from the previous pass as the costs.
 *
 * As an alternate strategy, also consider using only literals.  The boolean
 * returned in *used_only_literals indicates whether that strategy was best.
 */
static void
deflate_optimize_and_flush_block(struct libdeflate_compressor *c,
				 struct deflate_output_bitstream *os,
				 const u8 *block_begin, u32 block_length,
				 const struct lz_match *cache_ptr,
				 bool is_first_block, bool is_final_block,
				 bool *used_only_literals)
{
	unsigned num_passes_remaining = c->p.n.max_optim_passes;
	u32 best_true_cost = UINT32_MAX;
	u32 true_cost;
	u32 only_lits_cost;
	u32 static_cost = UINT32_MAX;
	struct deflate_sequence seq_;
	struct deflate_sequence *seq = NULL;
	u32 i;

	/*
	 * On some data, using only literals (no matches) ends up being better
	 * than what the iterative optimization algorithm produces.  Therefore,
	 * consider using only literals.
	 */
	deflate_choose_all_literals(c, block_begin, block_length);
	only_lits_cost = deflate_compute_true_cost(c);

	/*
	 * Force the block to really end at the desired length, even if some
	 * matches extend beyond it.
	 */
	for (i = block_length;
	     i <= MIN(block_length - 1 + DEFLATE_MAX_MATCH_LEN,
		      ARRAY_LEN(c->p.n.optimum_nodes) - 1); i++)
		c->p.n.optimum_nodes[i].cost_to_end = 0x80000000;

	/*
	 * Sometimes a static Huffman block ends up being cheapest, particularly
	 * if the block is small.  So, if the block is sufficiently small, find
	 * the optimal static block solution and remember its cost.
	 */
	if (block_length <= c->p.n.max_len_to_optimize_static_block) {
		/* Save c->p.n.costs temporarily. */
		c->p.n.costs_saved = c->p.n.costs;

		deflate_set_costs_from_codes(c, &c->static_codes.lens);
		deflate_find_min_cost_path(c, block_length, cache_ptr);
		static_cost = c->p.n.optimum_nodes[0].cost_to_end / BIT_COST;
		static_cost += 7; /* for the end-of-block symbol */

		/* Restore c->p.n.costs. */
		c->p.n.costs = c->p.n.costs_saved;
	}

	/* Initialize c->p.n.costs with default costs. */
	deflate_set_initial_costs(c, block_begin, block_length, is_first_block);

	do {
		/*
		 * Find the minimum-cost path for this pass.
		 * Also set c->freqs and c->codes to match the path.
		 */
		deflate_find_min_cost_path(c, block_length, cache_ptr);

		/*
		 * Compute the exact cost of the block if the path were to be
		 * used.  Note that this differs from
		 * c->p.n.optimum_nodes[0].cost_to_end in that true_cost uses
		 * the actual Huffman codes instead of c->p.n.costs.
		 */
		true_cost = deflate_compute_true_cost(c);

		/*
		 * If the cost didn't improve much from the previous pass, then
		 * doing more passes probably won't be helpful, so stop early.
		 */
		if (true_cost + c->p.n.min_improvement_to_continue >
		    best_true_cost)
			break;

		best_true_cost = true_cost;

		/* Save the cost model that gave 'best_true_cost'. */
		c->p.n.costs_saved = c->p.n.costs;

		/* Update the cost model from the Huffman codes. */
		deflate_set_costs_from_codes(c, &c->codes.lens);

	} while (--num_passes_remaining);

	*used_only_literals = false;
	if (MIN(only_lits_cost, static_cost) < best_true_cost) {
		if (only_lits_cost < static_cost) {
			/* Using only literals ended up being best! */
			deflate_choose_all_literals(c, block_begin, block_length);
			deflate_set_costs_from_codes(c, &c->codes.lens);
			seq_.litrunlen_and_length = block_length;
			seq = &seq_;
			*used_only_literals = true;
		} else {
			/* Static block ended up being best! */
			deflate_set_costs_from_codes(c, &c->static_codes.lens);
			deflate_find_min_cost_path(c, block_length, cache_ptr);
		}
	} else if (true_cost >=
		   best_true_cost + c->p.n.min_bits_to_use_nonfinal_path) {
		/*
		 * The best solution was actually from a non-final optimization
		 * pass, so recover and use the min-cost path from that pass.
		 */
		c->p.n.costs = c->p.n.costs_saved;
		deflate_find_min_cost_path(c, block_length, cache_ptr);
		deflate_set_costs_from_codes(c, &c->codes.lens);
	}
	deflate_flush_block(c, os, block_begin, block_length, seq,
			    is_final_block);
}

static void
deflate_near_optimal_init_stats(struct libdeflate_compressor *c)
{
	init_block_split_stats(&c->split_stats);
	memset(c->p.n.new_match_len_freqs, 0,
	       sizeof(c->p.n.new_match_len_freqs));
	memset(c->p.n.match_len_freqs, 0, sizeof(c->p.n.match_len_freqs));
}

static void
deflate_near_optimal_merge_stats(struct libdeflate_compressor *c)
{
	unsigned i;

	merge_new_observations(&c->split_stats);
	for (i = 0; i < ARRAY_LEN(c->p.n.match_len_freqs); i++) {
		c->p.n.match_len_freqs[i] += c->p.n.new_match_len_freqs[i];
		c->p.n.new_match_len_freqs[i] = 0;
	}
}

/*
 * Save some literal/match statistics from the previous block so that
 * deflate_adjust_costs() will be able to decide how much the current block
 * differs from the previous one.
 */
static void
deflate_near_optimal_save_stats(struct libdeflate_compressor *c)
{
	int i;

	for (i = 0; i < NUM_OBSERVATION_TYPES; i++)
		c->p.n.prev_observations[i] = c->split_stats.observations[i];
	c->p.n.prev_num_observations = c->split_stats.num_observations;
}

static void
deflate_near_optimal_clear_old_stats(struct libdeflate_compressor *c)
{
	int i;

	for (i = 0; i < NUM_OBSERVATION_TYPES; i++)
		c->split_stats.observations[i] = 0;
	c->split_stats.num_observations = 0;
	memset(c->p.n.match_len_freqs, 0, sizeof(c->p.n.match_len_freqs));
}

/*
 * This is the "near-optimal" DEFLATE compressor.  It computes the optimal
 * representation of each DEFLATE block using a minimum-cost path search over
 * the graph of possible match/literal choices for that block, assuming a
 * certain cost for each Huffman symbol.
 *
 * For several reasons, the end result is not guaranteed to be optimal:
 *
 * - Nonoptimal choice of blocks
 * - Heuristic limitations on which matches are actually considered
 * - Symbol costs are unknown until the symbols have already been chosen
 *   (so iterative optimization must be used)
 */
static void
deflate_compress_near_optimal(struct libdeflate_compressor * restrict c,
			      const u8 *in, size_t in_nbytes,
			      struct deflate_output_bitstream *os)
{
	const u8 *in_next = in;
	const u8 *in_block_begin = in_next;
	const u8 *in_end = in_next + in_nbytes;
	const u8 *in_cur_base = in_next;
	const u8 *in_next_slide =
		in_next + MIN(in_end - in_next, MATCHFINDER_WINDOW_SIZE);
	u32 max_len = DEFLATE_MAX_MATCH_LEN;
	u32 nice_len = MIN(c->nice_match_length, max_len);
	struct lz_match *cache_ptr = c->p.n.match_cache;
	u32 next_hashes[2] = {0, 0};
	bool prev_block_used_only_literals = false;

	bt_matchfinder_init(&c->p.n.bt_mf);
	deflate_near_optimal_init_stats(c);

	do {
		/* Starting a new DEFLATE block */
		const u8 * const in_max_block_end = choose_max_block_end(
				in_block_begin, in_end, SOFT_MAX_BLOCK_LENGTH);
		const u8 *prev_end_block_check = NULL;
		bool change_detected = false;
		const u8 *next_observation = in_next;
		u32 min_len;

		/*
		 * Use the minimum match length heuristic to improve the
		 * literal/match statistics gathered during matchfinding.
		 * However, the actual near-optimal parse won't respect min_len,
		 * as it can accurately assess the costs of different matches.
		 *
		 * If the "use only literals" strategy happened to be the best
		 * strategy on the previous block, then probably the
		 * min_match_len heuristic is still not aggressive enough for
		 * the data, so force gathering literal stats only.
		 */
		if (prev_block_used_only_literals)
			min_len = DEFLATE_MAX_MATCH_LEN + 1;
		else
			min_len = calculate_min_match_len(
					in_block_begin,
					in_max_block_end - in_block_begin,
					c->max_search_depth);

		/*
		 * Find matches until we decide to end the block.  We end the
		 * block if any of the following is true:
		 *
		 * (1) Maximum block length has been reached
		 * (2) Match catch may overflow.
		 * (3) Block split heuristic says to split now.
		 */
		for (;;) {
			struct lz_match *matches;
			u32 best_len;
			size_t remaining = in_end - in_next;

			/* Slide the window forward if needed. */
			if (in_next == in_next_slide) {
				bt_matchfinder_slide_window(&c->p.n.bt_mf);
				in_cur_base = in_next;
				in_next_slide = in_next +
					MIN(remaining, MATCHFINDER_WINDOW_SIZE);
			}

			/*
			 * Find matches with the current position using the
			 * binary tree matchfinder and save them in match_cache.
			 *
			 * Note: the binary tree matchfinder is more suited for
			 * optimal parsing than the hash chain matchfinder.  The
			 * reasons for this include:
			 *
			 * - The binary tree matchfinder can find more matches
			 *   in the same number of steps.
			 * - One of the major advantages of hash chains is that
			 *   skipping positions (not searching for matches at
			 *   them) is faster; however, with optimal parsing we
			 *   search for matches at almost all positions, so this
			 *   advantage of hash chains is negated.
			 */
			matches = cache_ptr;
			best_len = 0;
			adjust_max_and_nice_len(&max_len, &nice_len, remaining);
			if (likely(max_len >= BT_MATCHFINDER_REQUIRED_NBYTES)) {
				cache_ptr = bt_matchfinder_get_matches(
						&c->p.n.bt_mf,
						in_cur_base,
						in_next - in_cur_base,
						max_len,
						nice_len,
						c->max_search_depth,
						next_hashes,
						matches);
				if (cache_ptr > matches)
					best_len = cache_ptr[-1].length;
			}
			if (in_next >= next_observation) {
				if (best_len >= min_len) {
					observe_match(&c->split_stats,
						      best_len);
					next_observation = in_next + best_len;
					c->p.n.new_match_len_freqs[best_len]++;
				} else {
					observe_literal(&c->split_stats,
							*in_next);
					next_observation = in_next + 1;
				}
			}

			cache_ptr->length = cache_ptr - matches;
			cache_ptr->offset = *in_next;
			in_next++;
			cache_ptr++;

			/*
			 * If there was a very long match found, don't cache any
			 * matches for the bytes covered by that match.  This
			 * avoids degenerate behavior when compressing highly
			 * redundant data, where the number of matches can be
			 * very large.
			 *
			 * This heuristic doesn't actually hurt the compression
			 * ratio very much.  If there's a long match, then the
			 * data must be highly compressible, so it doesn't
			 * matter much what we do.
			 */
			if (best_len >= DEFLATE_MIN_MATCH_LEN &&
			    best_len >= nice_len) {
				--best_len;
				do {
					remaining = in_end - in_next;
					if (in_next == in_next_slide) {
						bt_matchfinder_slide_window(
							&c->p.n.bt_mf);
						in_cur_base = in_next;
						in_next_slide = in_next +
							MIN(remaining,
							    MATCHFINDER_WINDOW_SIZE);
					}
					adjust_max_and_nice_len(&max_len,
								&nice_len,
								remaining);
					if (max_len >=
					    BT_MATCHFINDER_REQUIRED_NBYTES) {
						bt_matchfinder_skip_byte(
							&c->p.n.bt_mf,
							in_cur_base,
							in_next - in_cur_base,
							nice_len,
							c->max_search_depth,
							next_hashes);
					}
					cache_ptr->length = 0;
					cache_ptr->offset = *in_next;
					in_next++;
					cache_ptr++;
				} while (--best_len);
			}
			/* Maximum block length or end of input reached? */
			if (in_next >= in_max_block_end)
				break;
			/* Match cache overflowed? */
			if (cache_ptr >=
			    &c->p.n.match_cache[MATCH_CACHE_LENGTH])
				break;
			/* Not ready to try to end the block (again)? */
			if (!ready_to_check_block(&c->split_stats,
						  in_block_begin, in_next,
						  in_end))
				continue;
			/* Check if it would be worthwhile to end the block. */
			if (do_end_block_check(&c->split_stats,
					       in_next - in_block_begin)) {
				change_detected = true;
				break;
			}
			/* Ending the block doesn't seem worthwhile here. */
			deflate_near_optimal_merge_stats(c);
			prev_end_block_check = in_next;
		}
		/*
		 * All the matches for this block have been cached.  Now choose
		 * the precise end of the block and the sequence of items to
		 * output to represent it, then flush the block.
		 */
		if (change_detected && prev_end_block_check != NULL) {
			/*
			 * The block is being ended because a recent chunk of
			 * data differs from the rest of the block.  We could
			 * end the block at 'in_next' like the greedy and lazy
			 * compressors do, but that's not ideal since it would
			 * include the differing chunk in the block.  The
			 * near-optimal compressor has time to do a better job.
			 * Therefore, we rewind to just before the chunk, and
			 * output a block that only goes up to there.
			 *
			 * We then set things up to correctly start the next
			 * block, considering that some work has already been
			 * done on it (some matches found and stats gathered).
			 */
			struct lz_match *orig_cache_ptr = cache_ptr;
			const u8 *in_block_end = prev_end_block_check;
			u32 block_length = in_block_end - in_block_begin;
			bool is_first = (in_block_begin == in);
			bool is_final = false;
			u32 num_bytes_to_rewind = in_next - in_block_end;
			size_t cache_len_rewound;

			/* Rewind the match cache. */
			do {
				cache_ptr--;
				cache_ptr -= cache_ptr->length;
			} while (--num_bytes_to_rewind);
			cache_len_rewound = orig_cache_ptr - cache_ptr;

			deflate_optimize_and_flush_block(
						c, os, in_block_begin,
						block_length, cache_ptr,
						is_first, is_final,
						&prev_block_used_only_literals);
			memmove(c->p.n.match_cache, cache_ptr,
				cache_len_rewound * sizeof(*cache_ptr));
			cache_ptr = &c->p.n.match_cache[cache_len_rewound];
			deflate_near_optimal_save_stats(c);
			/*
			 * Clear the stats for the just-flushed block, leaving
			 * just the stats for the beginning of the next block.
			 */
			deflate_near_optimal_clear_old_stats(c);
			in_block_begin = in_block_end;
		} else {
			/*
			 * The block is being ended for a reason other than a
			 * differing data chunk being detected.  Don't rewind at
			 * all; just end the block at the current position.
			 */
			u32 block_length = in_next - in_block_begin;
			bool is_first = (in_block_begin == in);
			bool is_final = (in_next == in_end);

			deflate_near_optimal_merge_stats(c);
			deflate_optimize_and_flush_block(
						c, os, in_block_begin,
						block_length, cache_ptr,
						is_first, is_final,
						&prev_block_used_only_literals);
			cache_ptr = &c->p.n.match_cache[0];
			deflate_near_optimal_save_stats(c);
			deflate_near_optimal_init_stats(c);
			in_block_begin = in_next;
		}
	} while (in_next != in_end && !os->overflow);
}

/* Initialize c->p.n.offset_slot_full. */
static void
deflate_init_offset_slot_full(struct libdeflate_compressor *c)
{
	u32 offset_slot;
	u32 offset;
	u32 offset_end;

	for (offset_slot = 0; offset_slot < ARRAY_LEN(deflate_offset_slot_base);
	     offset_slot++) {
		offset = deflate_offset_slot_base[offset_slot];
		offset_end = offset +
			     (1 << deflate_extra_offset_bits[offset_slot]);
		do {
			c->p.n.offset_slot_full[offset] = offset_slot;
		} while (++offset != offset_end);
	}
}

#endif /* SUPPORT_NEAR_OPTIMAL_PARSING */

LIBDEFLATEAPI struct libdeflate_compressor *
libdeflate_alloc_compressor_ex(int compression_level,
			       const struct libdeflate_options *options)
{
	struct libdeflate_compressor *c;
	size_t size = offsetof(struct libdeflate_compressor, p);

	check_buildtime_parameters();

	/*
	 * Note: if more fields are added to libdeflate_options, this code will
	 * need to be updated to support both the old and new structs.
	 */
	if (options->sizeof_options != sizeof(*options))
		return NULL;

	if (compression_level < 0 || compression_level > 12)
		return NULL;

#if SUPPORT_NEAR_OPTIMAL_PARSING
	if (compression_level >= 10)
		size += sizeof(c->p.n);
	else
#endif
	{
		if (compression_level >= 2)
			size += sizeof(c->p.g);
		else if (compression_level == 1)
			size += sizeof(c->p.f);
	}

	c = libdeflate_aligned_malloc(options->malloc_func ?
				      options->malloc_func :
				      libdeflate_default_malloc_func,
				      MATCHFINDER_MEM_ALIGNMENT, size);
	if (!c)
		return NULL;
	c->free_func = options->free_func ?
		       options->free_func : libdeflate_default_free_func;

	c->compression_level = compression_level;

	/*
	 * The higher the compression level, the more we should bother trying to
	 * compress very small inputs.
	 */
	c->max_passthrough_size = 55 - (compression_level * 4);

	switch (compression_level) {
	case 0:
		c->max_passthrough_size = SIZE_MAX;
		c->impl = NULL; /* not used */
		break;
	case 1:
		c->impl = deflate_compress_fastest;
		/* max_search_depth is unused. */
		c->nice_match_length = 32;
		break;
	case 2:
		c->impl = deflate_compress_greedy;
		c->max_search_depth = 6;
		c->nice_match_length = 10;
		break;
	case 3:
		c->impl = deflate_compress_greedy;
		c->max_search_depth = 12;
		c->nice_match_length = 14;
		break;
	case 4:
		c->impl = deflate_compress_greedy;
		c->max_search_depth = 16;
		c->nice_match_length = 30;
		break;
	case 5:
		c->impl = deflate_compress_lazy;
		c->max_search_depth = 16;
		c->nice_match_length = 30;
		break;
	case 6:
		c->impl = deflate_compress_lazy;
		c->max_search_depth = 35;
		c->nice_match_length = 65;
		break;
	case 7:
		c->impl = deflate_compress_lazy;
		c->max_search_depth = 100;
		c->nice_match_length = 130;
		break;
	case 8:
		c->impl = deflate_compress_lazy2;
		c->max_search_depth = 300;
		c->nice_match_length = DEFLATE_MAX_MATCH_LEN;
		break;
	case 9:
#if !SUPPORT_NEAR_OPTIMAL_PARSING
	default:
#endif
		c->impl = deflate_compress_lazy2;
		c->max_search_depth = 600;
		c->nice_match_length = DEFLATE_MAX_MATCH_LEN;
		break;
#if SUPPORT_NEAR_OPTIMAL_PARSING
	case 10:
		c->impl = deflate_compress_near_optimal;
		c->max_search_depth = 35;
		c->nice_match_length = 75;
		c->p.n.max_optim_passes = 2;
		c->p.n.min_improvement_to_continue = 32;
		c->p.n.min_bits_to_use_nonfinal_path = 32;
		c->p.n.max_len_to_optimize_static_block = 0;
		deflate_init_offset_slot_full(c);
		break;
	case 11:
		c->impl = deflate_compress_near_optimal;
		c->max_search_depth = 100;
		c->nice_match_length = 150;
		c->p.n.max_optim_passes = 4;
		c->p.n.min_improvement_to_continue = 16;
		c->p.n.min_bits_to_use_nonfinal_path = 16;
		c->p.n.max_len_to_optimize_static_block = 1000;
		deflate_init_offset_slot_full(c);
		break;
	case 12:
	default:
		c->impl = deflate_compress_near_optimal;
		c->max_search_depth = 300;
		c->nice_match_length = DEFLATE_MAX_MATCH_LEN;
		c->p.n.max_optim_passes = 10;
		c->p.n.min_improvement_to_continue = 1;
		c->p.n.min_bits_to_use_nonfinal_path = 1;
		c->p.n.max_len_to_optimize_static_block = 10000;
		deflate_init_offset_slot_full(c);
		break;
#endif /* SUPPORT_NEAR_OPTIMAL_PARSING */
	}

	deflate_init_static_codes(c);

	return c;
}

LIBDEFLATEAPI struct libdeflate_compressor *
libdeflate_alloc_compressor(int compression_level)
{
	static const struct libdeflate_options defaults = {
		.sizeof_options = sizeof(defaults),
	};
	return libdeflate_alloc_compressor_ex(compression_level, &defaults);
}

LIBDEFLATEAPI size_t
libdeflate_deflate_compress(struct libdeflate_compressor *c,
			    const void *in, size_t in_nbytes,
			    void *out, size_t out_nbytes_avail)
{
	struct deflate_output_bitstream os;

	/*
	 * For extremely short inputs, or for compression level 0, just output
	 * uncompressed blocks.
	 */
	if (unlikely(in_nbytes <= c->max_passthrough_size))
		return deflate_compress_none(in, in_nbytes,
					     out, out_nbytes_avail);

	/* Initialize the output bitstream structure. */
	os.bitbuf = 0;
	os.bitcount = 0;
	os.next = out;
	os.end = os.next + out_nbytes_avail;
	os.overflow = false;

	/* Call the actual compression function. */
	(*c->impl)(c, in, in_nbytes, &os);

	/* Return 0 if the output buffer is too small. */
	if (os.overflow)
		return 0;

	/*
	 * Write the final byte if needed.  This can't overflow the output
	 * buffer because deflate_flush_block() would have set the overflow flag
	 * if there wasn't enough space remaining for the full final block.
	 */
	ASSERT(os.bitcount <= 7);
	if (os.bitcount) {
		ASSERT(os.next < os.end);
		*os.next++ = os.bitbuf;
	}

	/* Return the compressed size in bytes. */
	return os.next - (u8 *)out;
}

LIBDEFLATEAPI void
libdeflate_free_compressor(struct libdeflate_compressor *c)
{
	if (c)
		libdeflate_aligned_free(c->free_func, c);
}

unsigned int
libdeflate_get_compression_level(struct libdeflate_compressor *c)
{
	return c->compression_level;
}

LIBDEFLATEAPI size_t
libdeflate_deflate_compress_bound(struct libdeflate_compressor *c,
				  size_t in_nbytes)
{
	size_t max_blocks;

	/*
	 * Since the compressor never uses a compressed block when an
	 * uncompressed block is cheaper, the worst case can be no worse than
	 * the case where only uncompressed blocks are used.
	 *
	 * This is true even though up to 7 bits are "wasted" to byte-align the
	 * bitstream when a compressed block is followed by an uncompressed
	 * block.  This is because a compressed block wouldn't have been used if
	 * it wasn't cheaper than an uncompressed block, and uncompressed blocks
	 * always end on a byte boundary.  So the alignment bits will, at worst,
	 * go up to the place where the uncompressed block would have ended.
	 */

	/*
	 * Calculate the maximum number of uncompressed blocks that the
	 * compressor can use for 'in_nbytes' of data.
	 *
	 * The minimum length that is passed to deflate_flush_block() is
	 * MIN_BLOCK_LENGTH bytes, except for the final block if needed.  If
	 * deflate_flush_block() decides to use an uncompressed block, it
	 * actually will (in general) output a series of uncompressed blocks in
	 * order to stay within the UINT16_MAX limit of DEFLATE.  But this can
	 * be disregarded here as long as '2 * MIN_BLOCK_LENGTH <= UINT16_MAX',
	 * as in that case this behavior can't result in more blocks than the
	 * case where deflate_flush_block() is called with min-length inputs.
	 *
	 * So the number of uncompressed blocks needed would be bounded by
	 * DIV_ROUND_UP(in_nbytes, MIN_BLOCK_LENGTH).  However, empty inputs
	 * need 1 (empty) block, which gives the final expression below.
	 */
	STATIC_ASSERT(2 * MIN_BLOCK_LENGTH <= UINT16_MAX);
	max_blocks = MAX(DIV_ROUND_UP(in_nbytes, MIN_BLOCK_LENGTH), 1);

	/*
	 * Each uncompressed block has 5 bytes of overhead, for the BFINAL,
	 * BTYPE, LEN, and NLEN fields.  (For the reason explained earlier, the
	 * alignment bits at the very start of the block can be disregarded;
	 * they would otherwise increase the overhead to 6 bytes per block.)
	 * Therefore, the maximum number of overhead bytes is '5 * max_blocks'.
	 * To get the final bound, add the number of uncompressed bytes.
	 */
	return (5 * max_blocks) + in_nbytes;
}

/*** End of inlined file: deflate_compress.c ***/


/*** Start of inlined file: zlib_compress.c ***/

/*** Start of inlined file: zlib_constants.h ***/
#ifndef LIB_ZLIB_CONSTANTS_H
#define LIB_ZLIB_CONSTANTS_H

#define ZLIB_MIN_HEADER_SIZE	2
#define ZLIB_FOOTER_SIZE	4
#define ZLIB_MIN_OVERHEAD	(ZLIB_MIN_HEADER_SIZE + ZLIB_FOOTER_SIZE)

#define ZLIB_CM_DEFLATE		8

#define ZLIB_CINFO_32K_WINDOW	7

#define ZLIB_FASTEST_COMPRESSION	0
#define ZLIB_FAST_COMPRESSION		1
#define ZLIB_DEFAULT_COMPRESSION	2
#define ZLIB_SLOWEST_COMPRESSION	3

#endif /* LIB_ZLIB_CONSTANTS_H */

/*** End of inlined file: zlib_constants.h ***/

LIBDEFLATEAPI size_t
libdeflate_zlib_compress(struct libdeflate_compressor *c,
			 const void *in, size_t in_nbytes,
			 void *out, size_t out_nbytes_avail)
{
	u8 *out_next = out;
	u16 hdr;
	unsigned compression_level;
	unsigned level_hint;
	size_t deflate_size;

	if (out_nbytes_avail <= ZLIB_MIN_OVERHEAD)
		return 0;

	/* 2 byte header: CMF and FLG  */
	hdr = (ZLIB_CM_DEFLATE << 8) | (ZLIB_CINFO_32K_WINDOW << 12);
	compression_level = libdeflate_get_compression_level(c);
	if (compression_level < 2)
		level_hint = ZLIB_FASTEST_COMPRESSION;
	else if (compression_level < 6)
		level_hint = ZLIB_FAST_COMPRESSION;
	else if (compression_level < 8)
		level_hint = ZLIB_DEFAULT_COMPRESSION;
	else
		level_hint = ZLIB_SLOWEST_COMPRESSION;
	hdr |= level_hint << 6;
	hdr |= 31 - (hdr % 31);

	put_unaligned_be16(hdr, out_next);
	out_next += 2;

	/* Compressed data  */
	deflate_size = libdeflate_deflate_compress(c, in, in_nbytes, out_next,
					out_nbytes_avail - ZLIB_MIN_OVERHEAD);
	if (deflate_size == 0)
		return 0;
	out_next += deflate_size;

	/* ADLER32  */
	put_unaligned_be32(libdeflate_adler32(1, in, in_nbytes), out_next);
	out_next += 4;

	return out_next - (u8 *)out;
}

LIBDEFLATEAPI size_t
libdeflate_zlib_compress_bound(struct libdeflate_compressor *c,
			       size_t in_nbytes)
{
	return ZLIB_MIN_OVERHEAD +
	       libdeflate_deflate_compress_bound(c, in_nbytes);
}

/*** End of inlined file: zlib_compress.c ***/


/*** Start of inlined file: utils.c ***/
#ifdef FREESTANDING
#  define malloc NULL
#  define free NULL
#else
#  include <stdlib.h>
#endif

malloc_func_t libdeflate_default_malloc_func = malloc;
free_func_t libdeflate_default_free_func = free;

void *
libdeflate_aligned_malloc(malloc_func_t malloc_func,
			  size_t alignment, size_t size)
{
	void *ptr = (*malloc_func)(sizeof(void *) + alignment - 1 + size);

	if (ptr) {
		void *orig_ptr = ptr;

		ptr = (void *)ALIGN((uintptr_t)ptr + sizeof(void *), alignment);
		((void **)ptr)[-1] = orig_ptr;
	}
	return ptr;
}

void
libdeflate_aligned_free(free_func_t free_func, void *ptr)
{
	(*free_func)(((void **)ptr)[-1]);
}

LIBDEFLATEAPI void
libdeflate_set_memory_allocator(malloc_func_t malloc_func,
				free_func_t free_func)
{
	libdeflate_default_malloc_func = malloc_func;
	libdeflate_default_free_func = free_func;
}

/*
 * Implementations of libc functions for freestanding library builds.
 * Normal library builds don't use these.  Not optimized yet; usually the
 * compiler expands these functions and doesn't actually call them anyway.
 */
#ifdef FREESTANDING
#undef memset
void * __attribute__((weak))
memset(void *s, int c, size_t n)
{
	u8 *p = s;
	size_t i;

	for (i = 0; i < n; i++)
		p[i] = c;
	return s;
}

#undef memcpy
void * __attribute__((weak))
memcpy(void *dest, const void *src, size_t n)
{
	u8 *d = dest;
	const u8 *s = src;
	size_t i;

	for (i = 0; i < n; i++)
		d[i] = s[i];
	return dest;
}

#undef memmove
void * __attribute__((weak))
memmove(void *dest, const void *src, size_t n)
{
	u8 *d = dest;
	const u8 *s = src;
	size_t i;

	if (d <= s)
		return memcpy(d, s, n);

	for (i = n; i > 0; i--)
		d[i - 1] = s[i - 1];
	return dest;
}

#undef memcmp
int __attribute__((weak))
memcmp(const void *s1, const void *s2, size_t n)
{
	const u8 *p1 = s1;
	const u8 *p2 = s2;
	size_t i;

	for (i = 0; i < n; i++) {
		if (p1[i] != p2[i])
			return (int)p1[i] - (int)p2[i];
	}
	return 0;
}
#endif /* FREESTANDING */

#ifdef LIBDEFLATE_ENABLE_ASSERTIONS
#include <stdio.h>
#include <stdlib.h>
NORETURN void
libdeflate_assertion_failed(const char *expr, const char *file, int line)
{
	fprintf(stderr, "Assertion failed: %s at %s:%d\n", expr, file, line);
	abort();
}
#endif /* LIBDEFLATE_ENABLE_ASSERTIONS */

/*** End of inlined file: utils.c ***/


/*** Start of inlined file: cpu_features.c ***/

/*** Start of inlined file: cpu_features_common.h ***/
#ifndef LIB_CPU_FEATURES_COMMON_H
#define LIB_CPU_FEATURES_COMMON_H

#if defined(TEST_SUPPORT__DO_NOT_USE) && !defined(FREESTANDING)
   /* for strdup() and strtok_r() */
#  undef _ANSI_SOURCE
#  ifndef __APPLE__
#    undef _GNU_SOURCE
#    define _GNU_SOURCE
#  endif
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#endif

struct cpu_feature {
	u32 bit;
	const char *name;
};

#if defined(TEST_SUPPORT__DO_NOT_USE) && !defined(FREESTANDING)
/* Disable any features that are listed in $LIBDEFLATE_DISABLE_CPU_FEATURES. */
static inline void
disable_cpu_features_for_testing(u32 *features,
				 const struct cpu_feature *feature_table,
				 size_t feature_table_length)
{
	char *env_value, *strbuf, *p, *saveptr = NULL;
	size_t i;

	env_value = getenv("LIBDEFLATE_DISABLE_CPU_FEATURES");
	if (!env_value)
		return;
	strbuf = strdup(env_value);
	if (!strbuf)
		abort();
	p = strtok_r(strbuf, ",", &saveptr);
	while (p) {
		for (i = 0; i < feature_table_length; i++) {
			if (strcmp(p, feature_table[i].name) == 0) {
				*features &= ~feature_table[i].bit;
				break;
			}
		}
		if (i == feature_table_length) {
			fprintf(stderr,
				"unrecognized feature in LIBDEFLATE_DISABLE_CPU_FEATURES: \"%s\"\n",
				p);
			abort();
		}
		p = strtok_r(NULL, ",", &saveptr);
	}
	free(strbuf);
}
#else /* TEST_SUPPORT__DO_NOT_USE */
static inline void
disable_cpu_features_for_testing(u32 *features,
				 const struct cpu_feature *feature_table,
				 size_t feature_table_length)
{
}
#endif /* !TEST_SUPPORT__DO_NOT_USE */

#endif /* LIB_CPU_FEATURES_COMMON_H */

/*** End of inlined file: cpu_features_common.h ***/

#ifdef X86_CPU_FEATURES_KNOWN
/* Runtime x86 CPU feature detection is supported. */

/* Execute the CPUID instruction. */
static inline void
cpuid(u32 leaf, u32 subleaf, u32 *a, u32 *b, u32 *c, u32 *d)
{
#ifdef _MSC_VER
	int result[4];

	__cpuidex(result, leaf, subleaf);
	*a = result[0];
	*b = result[1];
	*c = result[2];
	*d = result[3];
#else
	__asm__ volatile("cpuid" : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)
			 : "a" (leaf), "c" (subleaf));
#endif
}

/* Read an extended control register. */
static inline u64
read_xcr(u32 index)
{
#ifdef _MSC_VER
	return _xgetbv(index);
#else
	u32 d, a;

	/*
	 * Execute the "xgetbv" instruction.  Old versions of binutils do not
	 * recognize this instruction, so list the raw bytes instead.
	 *
	 * This must be 'volatile' to prevent this code from being moved out
	 * from under the check for OSXSAVE.
	 */
	__asm__ volatile(".byte 0x0f, 0x01, 0xd0" :
			 "=d" (d), "=a" (a) : "c" (index));

	return ((u64)d << 32) | a;
#endif
}

static const struct cpu_feature x86_cpu_feature_table[] = {
	{X86_CPU_FEATURE_SSE2,		"sse2"},
	{X86_CPU_FEATURE_PCLMULQDQ,	"pclmulqdq"},
	{X86_CPU_FEATURE_AVX,		"avx"},
	{X86_CPU_FEATURE_AVX2,		"avx2"},
	{X86_CPU_FEATURE_BMI2,		"bmi2"},
	{X86_CPU_FEATURE_ZMM,		"zmm"},
	{X86_CPU_FEATURE_AVX512BW,	"avx512bw"},
	{X86_CPU_FEATURE_AVX512VL,	"avx512vl"},
	{X86_CPU_FEATURE_VPCLMULQDQ,	"vpclmulqdq"},
	{X86_CPU_FEATURE_AVX512VNNI,	"avx512_vnni"},
	{X86_CPU_FEATURE_AVXVNNI,	"avx_vnni"},
};

volatile u32 libdeflate_x86_cpu_features = 0;

static inline bool
os_supports_avx512(u64 xcr0)
{
#ifdef __APPLE__
	/*
	 * The Darwin kernel had a bug where it could corrupt the opmask
	 * registers.  See
	 * https://community.intel.com/t5/Software-Tuning-Performance/MacOS-Darwin-kernel-bug-clobbers-AVX-512-opmask-register-state/m-p/1327259
	 * Darwin also does not initially set the XCR0 bits for AVX512, but they
	 * are set if the thread tries to use AVX512 anyway.  Thus, to safely
	 * and consistently use AVX512 on macOS we'd need to check the kernel
	 * version as well as detect AVX512 support using a macOS-specific
	 * method.  We don't bother with this, especially given Apple's
	 * transition to arm64.
	 */
	return false;
#else
	return (xcr0 & 0xe6) == 0xe6;
#endif
}

/*
 * Don't use 512-bit vectors (ZMM registers) on Intel CPUs before Rocket Lake
 * and Sapphire Rapids, due to the overly-eager downclocking which can reduce
 * the performance of workloads that use ZMM registers only occasionally.
 */
static inline bool
allow_512bit_vectors(const u32 manufacturer[3], u32 family, u32 model)
{
#ifdef TEST_SUPPORT__DO_NOT_USE
	return true;
#endif
	if (memcmp(manufacturer, "GenuineIntel", 12) != 0)
		return true;
	if (family != 6)
		return true;
	switch (model) {
	case 85: /* Skylake (Server), Cascade Lake, Cooper Lake */
	case 106: /* Ice Lake (Server) */
	case 108: /* Ice Lake (Server) */
	case 126: /* Ice Lake (Client) */
	case 140: /* Tiger Lake */
	case 141: /* Tiger Lake */
		return false;
	}
	return true;
}

/* Initialize libdeflate_x86_cpu_features. */
void libdeflate_init_x86_cpu_features(void)
{
	u32 max_leaf;
	u32 manufacturer[3];
	u32 family, model;
	u32 a, b, c, d;
	u64 xcr0 = 0;
	u32 features = 0;

	/* EAX=0: Highest Function Parameter and Manufacturer ID */
	cpuid(0, 0, &max_leaf, &manufacturer[0], &manufacturer[2],
	      &manufacturer[1]);
	if (max_leaf < 1)
		goto out;

	/* EAX=1: Processor Info and Feature Bits */
	cpuid(1, 0, &a, &b, &c, &d);
	family = (a >> 8) & 0xf;
	model = (a >> 4) & 0xf;
	if (family == 6 || family == 0xf)
		model += (a >> 12) & 0xf0;
	if (family == 0xf)
		family += (a >> 20) & 0xff;
	if (d & (1 << 26))
		features |= X86_CPU_FEATURE_SSE2;
	/*
	 * No known CPUs have pclmulqdq without sse4.1, so in practice code
	 * targeting pclmulqdq can use sse4.1 instructions.  But to be safe,
	 * explicitly check for both the pclmulqdq and sse4.1 bits.
	 */
	if ((c & (1 << 1)) && (c & (1 << 19)))
		features |= X86_CPU_FEATURE_PCLMULQDQ;
	if (c & (1 << 27))
		xcr0 = read_xcr(0);
	if ((c & (1 << 28)) && ((xcr0 & 0x6) == 0x6))
		features |= X86_CPU_FEATURE_AVX;

	if (max_leaf < 7)
		goto out;

	/* EAX=7, ECX=0: Extended Features */
	cpuid(7, 0, &a, &b, &c, &d);
	if (b & (1 << 8))
		features |= X86_CPU_FEATURE_BMI2;
	if ((xcr0 & 0x6) == 0x6) {
		if (b & (1 << 5))
			features |= X86_CPU_FEATURE_AVX2;
		if (c & (1 << 10))
			features |= X86_CPU_FEATURE_VPCLMULQDQ;
	}
	if (os_supports_avx512(xcr0)) {
		if (allow_512bit_vectors(manufacturer, family, model))
			features |= X86_CPU_FEATURE_ZMM;
		if (b & (1 << 30))
			features |= X86_CPU_FEATURE_AVX512BW;
		if (b & (1U << 31))
			features |= X86_CPU_FEATURE_AVX512VL;
		if (c & (1 << 11))
			features |= X86_CPU_FEATURE_AVX512VNNI;
	}

	/* EAX=7, ECX=1: Extended Features */
	cpuid(7, 1, &a, &b, &c, &d);
	if ((a & (1 << 4)) && ((xcr0 & 0x6) == 0x6))
		features |= X86_CPU_FEATURE_AVXVNNI;

out:
	disable_cpu_features_for_testing(&features, x86_cpu_feature_table,
					 ARRAY_LEN(x86_cpu_feature_table));

	libdeflate_x86_cpu_features = features | X86_CPU_FEATURES_KNOWN;
}

#endif /* X86_CPU_FEATURES_KNOWN */

/*** End of inlined file: cpu_features.c ***/


/*** Start of inlined file: cpu_features.c ***/
/*
 * ARM CPUs don't have a standard way for unprivileged programs to detect CPU
 * features.  But an OS-specific way can be used when available.
 */

#ifdef __APPLE__
#  undef _ANSI_SOURCE
#  undef _DARWIN_C_SOURCE
#  define _DARWIN_C_SOURCE /* for sysctlbyname() */
#endif

#ifdef ARM_CPU_FEATURES_KNOWN
/* Runtime ARM CPU feature detection is supported. */

#ifdef __linux__
/*
 * On Linux, arm32 and arm64 CPU features can be detected by reading the
 * AT_HWCAP and AT_HWCAP2 values from /proc/self/auxv.
 *
 * Ideally we'd use the C library function getauxval(), but it's not guaranteed
 * to be available: it was only added to glibc in 2.16, and in Android it was
 * added to API level 18 for arm32 and level 21 for arm64.
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define AT_HWCAP	16
#define AT_HWCAP2	26

static void scan_auxv(unsigned long *hwcap, unsigned long *hwcap2)
{
	int fd;
	unsigned long auxbuf[32];
	int filled = 0;
	int i;

	fd = open("/proc/self/auxv", O_RDONLY);
	if (fd < 0)
		return;

	for (;;) {
		do {
			int ret = read(fd, &((char *)auxbuf)[filled],
				       sizeof(auxbuf) - filled);
			if (ret <= 0) {
				if (ret < 0 && errno == EINTR)
					continue;
				goto out;
			}
			filled += ret;
		} while (filled < 2 * sizeof(long));

		i = 0;
		do {
			unsigned long type = auxbuf[i];
			unsigned long value = auxbuf[i + 1];

			if (type == AT_HWCAP)
				*hwcap = value;
			else if (type == AT_HWCAP2)
				*hwcap2 = value;
			i += 2;
			filled -= 2 * sizeof(long);
		} while (filled >= 2 * sizeof(long));

		memmove(auxbuf, &auxbuf[i], filled);
	}
out:
	close(fd);
}

static u32 query_arm_cpu_features(void)
{
	u32 features = 0;
	unsigned long hwcap = 0;
	unsigned long hwcap2 = 0;

	scan_auxv(&hwcap, &hwcap2);

#ifdef ARCH_ARM32
	STATIC_ASSERT(sizeof(long) == 4);
	if (hwcap & (1 << 12))	/* HWCAP_NEON */
		features |= ARM_CPU_FEATURE_NEON;
#else
	STATIC_ASSERT(sizeof(long) == 8);
	if (hwcap & (1 << 1))	/* HWCAP_ASIMD */
		features |= ARM_CPU_FEATURE_NEON;
	if (hwcap & (1 << 4))	/* HWCAP_PMULL */
		features |= ARM_CPU_FEATURE_PMULL;
	if (hwcap & (1 << 7))	/* HWCAP_CRC32 */
		features |= ARM_CPU_FEATURE_CRC32;
	if (hwcap & (1 << 17))	/* HWCAP_SHA3 */
		features |= ARM_CPU_FEATURE_SHA3;
	if (hwcap & (1 << 20))	/* HWCAP_ASIMDDP */
		features |= ARM_CPU_FEATURE_DOTPROD;
#endif
	return features;
}

#elif defined(__APPLE__)
/* On Apple platforms, arm64 CPU features can be detected via sysctlbyname(). */

#include <sys/types.h>
#include <sys/sysctl.h>
#include <TargetConditionals.h>

static const struct {
	const char *name;
	u32 feature;
} feature_sysctls[] = {
	{ "hw.optional.neon",		  ARM_CPU_FEATURE_NEON },
	{ "hw.optional.AdvSIMD",	  ARM_CPU_FEATURE_NEON },
	{ "hw.optional.arm.FEAT_PMULL",	  ARM_CPU_FEATURE_PMULL },
	{ "hw.optional.armv8_crc32",	  ARM_CPU_FEATURE_CRC32 },
	{ "hw.optional.armv8_2_sha3",	  ARM_CPU_FEATURE_SHA3 },
	{ "hw.optional.arm.FEAT_SHA3",	  ARM_CPU_FEATURE_SHA3 },
	{ "hw.optional.arm.FEAT_DotProd", ARM_CPU_FEATURE_DOTPROD },
};

static u32 query_arm_cpu_features(void)
{
	u32 features = 0;
	size_t i;

	for (i = 0; i < ARRAY_LEN(feature_sysctls); i++) {
		const char *name = feature_sysctls[i].name;
		u32 val = 0;
		size_t valsize = sizeof(val);

		if (sysctlbyname(name, &val, &valsize, NULL, 0) == 0 &&
		    valsize == sizeof(val) && val == 1)
			features |= feature_sysctls[i].feature;
	}
	return features;
}
#elif defined(_WIN32)

#include <windows.h>

#ifndef PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE /* added in Windows SDK 20348 */
#  define PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE 43
#endif

static u32 query_arm_cpu_features(void)
{
	u32 features = ARM_CPU_FEATURE_NEON;

	if (IsProcessorFeaturePresent(PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE))
		features |= ARM_CPU_FEATURE_PMULL;
	if (IsProcessorFeaturePresent(PF_ARM_V8_CRC32_INSTRUCTIONS_AVAILABLE))
		features |= ARM_CPU_FEATURE_CRC32;
	if (IsProcessorFeaturePresent(PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE))
		features |= ARM_CPU_FEATURE_DOTPROD;

	/* FIXME: detect SHA3 support too. */

	return features;
}
#else
#error "unhandled case"
#endif

static const struct cpu_feature arm_cpu_feature_table[] = {
	{ARM_CPU_FEATURE_NEON,		"neon"},
	{ARM_CPU_FEATURE_PMULL,		"pmull"},
	{ARM_CPU_FEATURE_PREFER_PMULL,  "prefer_pmull"},
	{ARM_CPU_FEATURE_CRC32,		"crc32"},
	{ARM_CPU_FEATURE_SHA3,		"sha3"},
	{ARM_CPU_FEATURE_DOTPROD,	"dotprod"},
};

volatile u32 libdeflate_arm_cpu_features = 0;

void libdeflate_init_arm_cpu_features(void)
{
	u32 features = query_arm_cpu_features();

	/*
	 * On the Apple M1 processor, crc32 instructions max out at about 25.5
	 * GB/s in the best case of using a 3-way or greater interleaved chunked
	 * implementation, whereas a pmull-based implementation achieves 68 GB/s
	 * provided that the stride length is large enough (about 10+ vectors
	 * with eor3, or 12+ without).
	 *
	 * Assume that crc32 instructions are preferable in other cases.
	 */
#if (defined(__APPLE__) && TARGET_OS_OSX) || defined(TEST_SUPPORT__DO_NOT_USE)
	features |= ARM_CPU_FEATURE_PREFER_PMULL;
#endif

	disable_cpu_features_for_testing(&features, arm_cpu_feature_table,
					 ARRAY_LEN(arm_cpu_feature_table));

	libdeflate_arm_cpu_features = features | ARM_CPU_FEATURES_KNOWN;
}

#endif /* ARM_CPU_FEATURES_KNOWN */

/*** End of inlined file: cpu_features.c ***/

