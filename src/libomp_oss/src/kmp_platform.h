/*
 * kmp_platform.h -- header for determining operating system and architecture
 */

/* <copyright>
    Copyright (c) 1997-2016 Intel Corporation.  All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of Intel Corporation nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

</copyright> */

#ifndef KMP_PLATFORM_H
#define KMP_PLATFORM_H

/* ---------------------- Operating system recognition ------------------- */

#define KMP_OS_LINUX    0
#define KMP_OS_FREEBSD  0
#define KMP_OS_NETBSD   0
#define KMP_OS_DARWIN   0
#define KMP_OS_WINDOWS  0
#define KMP_OS_CNK      0
#define KMP_OS_UNIX     0  /* disjunction of KMP_OS_LINUX, KMP_OS_DARWIN etc. */


#ifdef _WIN32
# undef KMP_OS_WINDOWS
# define KMP_OS_WINDOWS 1
#endif

#if ( defined __APPLE__ && defined __MACH__ )
# undef KMP_OS_DARWIN
# define KMP_OS_DARWIN 1
#endif

// in some ppc64 linux installations, only the second condition is met
#if ( defined __linux )
# undef KMP_OS_LINUX
# define KMP_OS_LINUX 1
#elif ( defined __linux__)
# undef KMP_OS_LINUX
# define KMP_OS_LINUX 1
#else
#endif

#if ( defined __FreeBSD__ )
# undef KMP_OS_FREEBSD
# define KMP_OS_FREEBSD 1
#endif

#if ( defined __NetBSD__ )
# undef KMP_OS_NETBSD
# define KMP_OS_NETBSD 1
#endif

#if ( defined __bgq__ )
# undef KMP_OS_CNK
# define KMP_OS_CNK 1
#endif

#if (1 != KMP_OS_LINUX + KMP_OS_FREEBSD + KMP_OS_NETBSD + KMP_OS_DARWIN + KMP_OS_WINDOWS)
# error Unknown OS
#endif

#if KMP_OS_LINUX || KMP_OS_FREEBSD || KMP_OS_NETBSD || KMP_OS_DARWIN
# undef KMP_OS_UNIX
# define KMP_OS_UNIX 1
#endif

/* ---------------------- Architecture recognition ------------------- */

#define KMP_ARCH_X86        0
#define KMP_ARCH_X86_64     0
#define KMP_ARCH_AARCH64    0
#define KMP_ARCH_PPC64_BE   0
#define KMP_ARCH_PPC64_LE   0
#define KMP_ARCH_PPC64 (KMP_ARCH_PPC64_LE || KMP_ARCH_PPC64_BE)

#if KMP_OS_WINDOWS
# if defined _M_AMD64
#  undef KMP_ARCH_X86_64
#  define KMP_ARCH_X86_64 1
# else
#  undef KMP_ARCH_X86
#  define KMP_ARCH_X86 1
# endif
#endif

#if KMP_OS_UNIX
# if defined __x86_64
#  undef KMP_ARCH_X86_64
#  define KMP_ARCH_X86_64 1
# elif defined __i386
#  undef KMP_ARCH_X86
#  define KMP_ARCH_X86 1
# elif defined __powerpc64__
#  if defined __LITTLE_ENDIAN__
#   undef KMP_ARCH_PPC64_LE
#   define KMP_ARCH_PPC64_LE 1
#  else
#   undef KMP_ARCH_PPC64_BE
#   define KMP_ARCH_PPC64_BE 1
#  endif
# elif defined __aarch64__
#  undef KMP_ARCH_AARCH64
#  define KMP_ARCH_AARCH64 1
# endif
#endif

#if defined(__ARM_ARCH_7__)   || defined(__ARM_ARCH_7R__)  || \
    defined(__ARM_ARCH_7A__)
# define KMP_ARCH_ARMV7 1
#endif

#if defined(KMP_ARCH_ARMV7)   || defined(__ARM_ARCH_6__)   || \
    defined(__ARM_ARCH_6J__)  || defined(__ARM_ARCH_6K__)  || \
    defined(__ARM_ARCH_6Z__)  || defined(__ARM_ARCH_6T2__) || \
    defined(__ARM_ARCH_6ZK__)
# define KMP_ARCH_ARMV6 1
#endif

#if defined(KMP_ARCH_ARMV6)   || defined(__ARM_ARCH_5T__)  || \
    defined(__ARM_ARCH_5E__)  || defined(__ARM_ARCH_5TE__) || \
    defined(__ARM_ARCH_5TEJ__)
# define KMP_ARCH_ARMV5 1
#endif

#if defined(KMP_ARCH_ARMV5)   || defined(__ARM_ARCH_4__)   || \
    defined(__ARM_ARCH_4T__)
# define KMP_ARCH_ARMV4 1
#endif

#if defined(KMP_ARCH_ARMV4)   || defined(__ARM_ARCH_3__)   || \
    defined(__ARM_ARCH_3M__)
# define KMP_ARCH_ARMV3 1
#endif

#if defined(KMP_ARCH_ARMV3)   || defined(__ARM_ARCH_2__)
# define KMP_ARCH_ARMV2 1
#endif

#if defined(KMP_ARCH_ARMV2)
# define KMP_ARCH_ARM 1
#endif

// Intel(R) Many Integrated Core Architecture detection
#if defined(__MIC__) || defined(__MIC2__)
# define KMP_MIC  1
// Intel(R) Composer XE (13.0) defines both __MIC__ and __MIC2__ !
# if __MIC2__ || __KNC__
#  define KMP_MIC1 0
#  define KMP_MIC2 1
# else
#  define KMP_MIC1 1
#  define KMP_MIC2 0
# endif
#else
# define KMP_MIC  0
# define KMP_MIC1 0
# define KMP_MIC2 0
#endif

/* Specify 32 bit architectures here */
#define KMP_32_BIT_ARCH (KMP_ARCH_X86 || KMP_ARCH_ARM)

// TODO: Fixme - This is clever, but really fugly
#if (1 != KMP_ARCH_X86 + KMP_ARCH_X86_64 + KMP_ARCH_ARM + KMP_ARCH_PPC64 + KMP_ARCH_AARCH64)
# error Unknown or unsupported architecture
#endif

#endif // KMP_PLATFORM_H
