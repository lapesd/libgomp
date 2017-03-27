#ifndef ATOMIC_H
#define ATOMIC_H

#if defined(__i386__)||defined(__x86_64)
#  define kaapi_slowdown_cpu() \
  do { __asm__ __volatile__("pause\n\t"); } while (0)
#else
#  define kaapi_slowdown_cpu()
#endif


#  define kaapi_assert_debug(cond)
#  define kaapi_assert_debug_m(cond, msg)
#  define kaapi_assert_debug_fmt(cond, fmt, msg)
#  define KAAPI_LOG(l, fmt, ...)
#  define KAAPI_DEBUG_INST(inst)
#  define KAAPI_NODEBUG_INST(inst) inst

typedef struct kaapi_atomic8_t {
  volatile int8_t _counter;
} kaapi_atomic8_t;

typedef struct kaapi_atomic16_t {
  volatile int16_t _counter;
} kaapi_atomic16_t;

typedef struct kaapi_atomic32_t {
  volatile int32_t _counter;
} kaapi_atomic32_t;
typedef kaapi_atomic32_t kaapi_atomic_t;

typedef struct kaapi_atomic64_t {
  volatile int64_t _counter;
} kaapi_atomic64_t;

typedef struct kaapi_atomicptr_t {
  volatile intptr_t _counter;
} kaapi_atomicptr_t;

/* ========================= Low level memory barrier, inline for perf... so ============================= */
/** Implementation note
    - all functions or macros without _ORIG return the new value after apply the operation.
    - all functions or macros with ORIG return the old value before applying the operation.
    - this macros are based on GCC bultin functions.
*/
#if defined(KAAPI_DEBUG)
static inline int __kaapi_isaligned(const volatile void* a, size_t byte)
{
  kaapi_assert( (((uintptr_t)a) & ((unsigned long)byte - 1)) == 0 );
  return 1;
}
#  define __KAAPI_ISALIGNED_ATOMIC(a,instruction)\
  (__kaapi_isaligned( &(a)->_counter, sizeof((a)->_counter)) ? (instruction) : 0)
#else
static inline int __kaapi_isaligned(const volatile void* a, size_t byte)
{
  if ((((uintptr_t)a) & ((unsigned long)byte - 1)) == 0 ) return 1;
  return 0;
}
#  define __KAAPI_ISALIGNED_ATOMIC(a,instruction)\
  (instruction)
#endif

#define KAAPI_ATOMIC_READ(a) \
  __KAAPI_ISALIGNED_ATOMIC(a, (a)->_counter)

#define KAAPI_ATOMIC_WRITE(a, value) \
  __KAAPI_ISALIGNED_ATOMIC(a, (a)->_counter = value)

#define KAAPI_ATOMIC_WRITE_BARRIER(a, value) \
  __KAAPI_ISALIGNED_ATOMIC(a, (kaapi_writemem_barrier(), (a)->_counter = value))

//BEFORE:    __KAAPI_ISALIGNED_ATOMIC(a, (kaapi_writemem_barrier(), (a)->_counter = value))

#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)) || (__GNUC__ > 4) \
  || defined(__INTEL_COMPILER))
/* Note: ICC seems to also support these builtins functions */
#  if defined(__INTEL_COMPILER)
#    warning Using ICC. Please, check if icc really support atomic operations
/* ia64 impl using compare and exchange */
/*#    define KAAPI_CAS(_a, _o, _n) _InterlockedCompareExchange(_a, _n, _o ) */
#  endif

#  define KAAPI_ATOMIC_CAS(a, o, n) \
  __KAAPI_ISALIGNED_ATOMIC(a, __sync_bool_compare_and_swap( &((a)->_counter), o, n))

/* functions which return new value (NV) */
#  define KAAPI_ATOMIC_INCR(a) \
  __KAAPI_ISALIGNED_ATOMIC(a, __sync_add_and_fetch( &((a)->_counter), 1 ))

#  define KAAPI_ATOMIC_DECR(a) \
  __KAAPI_ISALIGNED_ATOMIC(a, __sync_sub_and_fetch( &((a)->_counter), 1 ))

#  define KAAPI_ATOMIC_ADD(a, value) \
  __KAAPI_ISALIGNED_ATOMIC(a, __sync_add_and_fetch( &((a)->_counter), value ))

#  define KAAPI_ATOMIC_SUB(a, value) \
  __KAAPI_ISALIGNED_ATOMIC(a, __sync_sub_and_fetch( &((a)->_counter), value ))

#  define KAAPI_ATOMIC_AND(a, o) \
  __KAAPI_ISALIGNED_ATOMIC(a, __sync_and_and_fetch( &((a)->_counter), o ))

#  define KAAPI_ATOMIC_OR(a, o) \
  __KAAPI_ISALIGNED_ATOMIC(a, __sync_or_and_fetch( &((a)->_counter), o ))

#  define KAAPI_ATOMIC_XOR(a, o) \
  __KAAPI_ISALIGNED_ATOMIC(a, __sync_xor_and_fetch( &((a)->_counter), o ))

/* linux functions which return old value */
#  define KAAPI_ATOMIC_AND_ORIG(a, o) \
  __KAAPI_ISALIGNED_ATOMIC(a, __sync_fetch_and_and( &((a)->_counter), o ))

#  define KAAPI_ATOMIC_OR_ORIG(a, o) \
  __KAAPI_ISALIGNED_ATOMIC(a, __sync_fetch_and_or( &((a)->_counter), o ))

#  define KAAPI_ATOMIC_XOR_ORIG(a, o) \
  __KAAPI_ISALIGNED_ATOMIC(a, __sync_fetch_and_xor( &((a)->_counter), o ))

/* linux 8 bit versions
 */
#  define KAAPI_ATOMIC_CAS8(a, o, n)   KAAPI_ATOMIC_CAS(a, o, n)
#  define KAAPI_ATOMIC_INCR8(a)        KAAPI_ATOMIC_INCR(a)
#  define KAAPI_ATOMIC_DECR8(a)        KAAPI_ATOMIC_DECR(a)
#  define KAAPI_ATOMIC_ADD8(a, value)  KAAPI_ATOMIC_ADD(a, value)
#  define KAAPI_ATOMIC_SUB8(a, value)  KAAPI_ATOMIC_SUB(a, value)
#  define KAAPI_ATOMIC_AND8(a, o)      KAAPI_ATOMIC_AND(a, o)
#  define KAAPI_ATOMIC_OR8(a, o)       KAAPI_ATOMIC_OR(a,o)
#  define KAAPI_ATOMIC_XOR8(a, o)      KAAPI_ATOMIC_XOR(a,o)
#  define KAAPI_ATOMIC_AND8_ORIG(a, o) KAAPI_ATOMIC_AND_ORIG(a,o)
#  define KAAPI_ATOMIC_OR8_ORIG(a, o)  KAAPI_ATOMIC_OR_ORIG(a, o)
#  define KAAPI_ATOMIC_XOR8_ORIG(a, o) KAAPI_ATOMIC_XOR_ORIG(a, o)

/* linux 64 bit versions
 */
#  define KAAPI_ATOMIC_CAS64(a, o, n)   KAAPI_ATOMIC_CAS(a, o, n)
#  define KAAPI_ATOMIC_INCR64(a)        KAAPI_ATOMIC_INCR(a)
#  define KAAPI_ATOMIC_DECR64(a)        KAAPI_ATOMIC_DECR(a)
#  define KAAPI_ATOMIC_ADD64(a, value)  KAAPI_ATOMIC_ADD(a, value)
#  define KAAPI_ATOMIC_SUB64(a, value)  KAAPI_ATOMIC_SUB(a, value)
#  define KAAPI_ATOMIC_AND64(a, o)      KAAPI_ATOMIC_AND(a, o)
#  define KAAPI_ATOMIC_OR64(a, o)       KAAPI_ATOMIC_OR(a,o)
#  define KAAPI_ATOMIC_XOR64(a, o)      KAAPI_ATOMIC_XOR(a,o)
#  define KAAPI_ATOMIC_AND64_ORIG(a, o) KAAPI_ATOMIC_AND_ORIG(a,o)
#  define KAAPI_ATOMIC_OR64_ORIG(a, o)  KAAPI_ATOMIC_OR_ORIG(a, o)
#  define KAAPI_ATOMIC_XOR64_ORIG(a, o) KAAPI_ATOMIC_XOR_ORIG(a, o)


#elif defined(__APPLE__) /* if gcc version on Apple is less than 4.1 */
#  warning "ON THIS ARCHITECTURE, PLEASE USE MORE RECENT GCC COMPILER (>=4.1)"
#else
#  error "Please add support for atomic operations on this system/architecture"
#endif /* GCC > 4.1 */


#if defined(__i386__)||defined(__x86_64)
#  define kaapi_slowdown_cpu() \
  do { __asm__ __volatile__("pause\n\t"); } while (0)
#else
#  define kaapi_slowdown_cpu()
#endif

static inline void kaapi_writemem_barrier()
{
#  if defined(__x86_64) || defined(__i386__)
  /* not need lfence on X86 archi: read are ordered */
  __asm__ __volatile__ ("mfence":::"memory");
#  elif defined(__GNUC__)
  __sync_synchronize();
#  else
#  error "Compiler not supported"
  /* bultin ?? */
#  endif
}

static inline void kaapi_readmem_barrier()
{
#  if defined(__x86_64) || defined(__i386__)
  /* not need lfence on X86 archi: read are ordered */
  __asm__ __volatile__ ("":::"memory");
#  elif defined(__GNUC__)
  __sync_synchronize();
#  else
#  error "Compiler not supported"
  /* xlC ->__lwsync() / bultin */
#  endif
}

/* should be both read & write barrier */
static inline void kaapi_mem_barrier()
{
#  if defined(__x86_64) || defined(__i386__)
  /* not need lfence on X86 archi: read are ordered */
  __asm__ __volatile__ ("mfence":::"memory");
#  elif defined(__GNUC__)
  __sync_synchronize();
#  else
#  error "Compiler not supported"
  /* bultin ?? */
#  endif
}

/** Note on scheduler lock:
  KAAPI_SCHED_LOCK_CAS -> lock state == 1 iff lock is taken, else 0
  KAAPI_SCHED_LOCK_CAS not defined: see
    Sewell, P., Sarkar, S., Owens, S., Nardelli, F. Z., and Myreen, M. O. 2010.
    x86-TSO: a rigorous and usable programmer's model for x86 multiprocessors.
    Commun. ACM 53, 7 (Jul. 2010), 89-97.
    DOI= http://doi.acm.org/10.1145/1785414.1785443
*/
typedef struct kaapi_lock_t {
  volatile int32_t  _counter;
  volatile int32_t  _sync;     /* used for fastes waitlock synchronization */
#if defined(KAAPI_DEBUG)
  volatile uint32_t _owner;
  volatile uint32_t _unlocker;
  volatile uint32_t _magic;
#endif
} kaapi_lock_t;

#if defined(KAAPI_DEBUG)
#  define KAAPI_LOCK_INITIALIZER { 1, 0, -1U, -1U, 123123123U }
#else
#  define KAAPI_LOCK_INITIALIZER { 1, 0 }
#endif

static inline int kaapi_atomic_initlock( kaapi_lock_t* lock )
{
  kaapi_assert_debug( lock->_magic != 123123123U);
  KAAPI_DEBUG_INST(lock->_magic = 123123123U;)
    KAAPI_DEBUG_INST(lock->_owner = -1U;)
    KAAPI_DEBUG_INST(lock->_unlocker = -1U;)
    lock->_sync = 0;
  KAAPI_ATOMIC_WRITE_BARRIER(lock,1);
  return 0;
}

static inline int kaapi_atomic_destroylock( kaapi_lock_t* lock KAAPI_NODEBUG_INST(__attribute__((unused))) )
{
  kaapi_assert_debug( KAAPI_ATOMIC_READ(lock) == 1 );
  kaapi_assert_debug( lock->_magic == 123123123U);
  kaapi_assert_debug( lock->_owner == -1U);
  KAAPI_DEBUG_INST(lock->_magic = 0101010101U;)
    return 0;
}

static inline int kaapi_atomic_trylock( kaapi_lock_t* lock )
{
  kaapi_assert_debug( lock->_magic == 123123123U);
  if ((KAAPI_ATOMIC_READ(lock) ==1) && (KAAPI_ATOMIC_DECR(lock) ==0))
    {
      KAAPI_DEBUG_INST(lock->_owner = kaapi_get_self_kid();)
	KAAPI_DEBUG_INST(lock->_unlocker = -1U;)
	return 1;
    }
  return 0;
}

static inline int kaapi_atomic_lock( kaapi_lock_t* lock )
{
  kaapi_assert_debug( lock->_magic == 123123123U);
 acquire:
  if (KAAPI_ATOMIC_DECR(lock) ==0)
    {
      KAAPI_DEBUG_INST(lock->_owner = kaapi_get_self_kid();)
	KAAPI_DEBUG_INST(lock->_unlocker = -1U;)
	return 0;
    }
  while (KAAPI_ATOMIC_READ(lock) <=0)
    {
      kaapi_assert_debug( lock->_magic == 123123123U);
      kaapi_slowdown_cpu();
    }
  goto acquire;
}


static inline int kaapi_atomic_unlock( kaapi_lock_t* lock )
{
  kaapi_assert_debug( lock->_magic == 123123123U);
  kaapi_assert_debug( lock->_unlocker == -1U);
  kaapi_assert_debug( lock->_owner == kaapi_get_self_kid() );
  kaapi_assert_debug( KAAPI_ATOMIC_READ(lock) <= 0);
  KAAPI_DEBUG_INST(lock->_unlocker = lock->_owner;)
    KAAPI_DEBUG_INST(lock->_owner = -1U;)

    if (lock->_sync !=0)
      lock->_sync = 0;
  kaapi_writemem_barrier();
  KAAPI_ATOMIC_WRITE(lock, 1);
  return 0;
}

static inline void kaapi_atomic_waitlock( kaapi_lock_t* lock)
{
  kaapi_assert_debug( lock->_magic == 123123123U);

  lock->_sync = 1;

  /* barrier to made visible local modification before reading lock->_sync */
  kaapi_mem_barrier();

  /* wait until reaches the unlocked state */
  while ((lock->_sync !=0) && (KAAPI_ATOMIC_READ(lock) <=0))
    kaapi_slowdown_cpu();
}

static inline int kaapi_atomic_assertlocked( kaapi_lock_t* lock)
{
  kaapi_assert_debug( lock->_magic == 123123123U);
  return KAAPI_ATOMIC_READ(lock) <=0;
}

#endif /* ATOMIC_H */
