#include <string.h> //FIXME: External dependecy.
#include "soundcommon.h"
#include "sound.h"
#include "report.h"

struct buffer buff;
#include "arm_sound_code.h"

#define SNDREGADDR(x) (0xa0700000 + (x))

/* These two macros are based on NetBSD's DC port */

/*
   G2 bus cycles must not be interrupted by IRQs or G2 DMA.
   The following paired macros will take the necessary precautions.
 */

#define DMAC_CHCR3 *((volatile unsigned int *)0xffa0003c)

static __inline unsigned int irq_disable(void)
{
  register unsigned int sr, tmp;
  __asm__("stc sr,%0" : "=r" (sr));
  tmp = (sr & 0xefffff0f) | 0xf0;
  __asm__("ldc %0,sr" : : "r" (tmp));
  return sr;
}

static __inline void irq_restore(unsigned int sr)
{
  __asm__("ldc %0,sr" : : "r" (sr));
}

#define G2_LOCK(OLD1, OLD2) \
    do { \
        OLD1 = irq_disable(); \
        /* suspend any G2 DMA here... */ \
        OLD2 = DMAC_CHCR3; \
        DMAC_CHCR3 = OLD2 & ~1; \
        while((*(volatile unsigned int *)0xa05f688c) & 0x20) \
            ; \
    } while(0)

#define G2_UNLOCK(OLD1, OLD2) \
    do { \
        /* resume any G2 DMA here... */ \
        DMAC_CHCR3 = OLD2; \
        irq_restore(OLD1); \
    } while(0)

/* Read one dword from G2 */
unsigned int g2_read_32(unsigned int address) {
    int old1, old2;
    unsigned int out;

    G2_LOCK(old1, old2);
    out = *((volatile unsigned int*)address);
    G2_UNLOCK(old1, old2);

    return out;
}

/* Write one dword to G2 */
void g2_write_32(unsigned int address, unsigned int value) {
    int old1, old2;

    G2_LOCK(old1, old2);
    *((volatile unsigned int*)address) = value;
    G2_UNLOCK(old1, old2);
}

/* When writing to the SPU RAM, this is required at least every 8 32-bit
   writes that you execute */
void g2_fifo_wait() {
    volatile unsigned int const *g2_fifo = (volatile unsigned int*)0xa05f688c;
    int i;

    for(i = 0; i < 0x1800; i++) {
        if(!(*g2_fifo & 0x11)) break;
    }
}

/* Set CDDA volume: values are 0-15 */
void spu_cdda_volume(int left_volume, int right_volume) {
    if(left_volume > 15)
        left_volume = 15;

    if(right_volume > 15)
        right_volume = 15;

    g2_fifo_wait();
    g2_write_32(SNDREGADDR(0x2040),
                (g2_read_32(SNDREGADDR(0x2040)) & ~0xff00) | (left_volume << 8));
    g2_write_32(SNDREGADDR(0x2044),
                (g2_read_32(SNDREGADDR(0x2044)) & ~0xff00) | (right_volume << 8));
}

void spu_cdda_pan(int left_pan, int right_pan) {
    if(left_pan < 16)
        left_pan = ~(left_pan - 16);

    left_pan &= 0x1f;

    if(right_pan < 16)
        right_pan = ~(right_pan - 16);

    right_pan &= 0x1f;

    g2_fifo_wait();
    g2_write_32(SNDREGADDR(0x2040),
                (g2_read_32(SNDREGADDR(0x2040)) & ~0xff) | (left_pan << 0));
    g2_write_32(SNDREGADDR(0x2044),
                (g2_read_32(SNDREGADDR(0x2044)) & ~0xff) | (right_pan << 0));
}

/* Initialize CDDA stuff */
static void spu_cdda_init() {
    spu_cdda_volume(15, 15);
    spu_cdda_pan(0, 31);
}

void *memcpy4(void *s1, const void *s2, unsigned int n)
{
  unsigned int *p1 = s1;
  const unsigned int *p2 = s2;
  n+=3;
  n>>=2;
  while(n--)
    *p1++ = *p2++;
  return s1;
}

#ifdef STEREO
void *memcpy4s(void *s1, const void *s2, unsigned int n)
{
  unsigned int *p1a = s1;
  unsigned int *p1b = (void*)(((char *)s1)+SAMPLES_TO_BYTES(STEREO_OFFSET));
  const unsigned int *p2 = s2;
  n+=3;
  n>>=2;
  while(n--) {
#if SAMPLE_MODE == 0
    unsigned int a = *p2++;
    unsigned int b = *p2++;
    *p1a++ = (a & 0xffff) | ((b & 0xffff)<<16);
    *p1b++ = ((a & 0xffff0000)>>16) | (b & 0xffff0000);
#else
#error 8 bit stereo not implemented...
#endif
  }
  return s1;
}
#endif

void *memset4(void *s, int c, unsigned int n)
{
  unsigned int *p = s;
  n+=3;
  n>>=2;
  while(n--)
    *p++ = c;
  return s;  
}

void init_arm()
{
  int i;
  //TODO: Providing your own ARM code as an argument might be nice.
  *((volatile unsigned long *)(void *)0xa0702c00) |= 1;
  memset4((void*)0xa0800000, 0, 2*1024*1024);
  memcpy4((void*)0xa0800000, arm_sound_code, sizeof(arm_sound_code));
  *((volatile unsigned long *)(void *)0xa0702c00) &= ~1;
  //FIXME: Some sort of sleep here would be cleaner...
  for(i=0; i<0x200000; i++);
  spu_cdda_init();
}

int fillpos;

int read_sound_int(volatile int *p)
{
  while((*((volatile int *)(void *)0xa05f688c))&32);
  return *p;
}

static void write_sound_int(volatile int *p, int v)
{
  while((*((volatile int *)(void *)0xa05f688c))&32);
  *p = v;
}

static void wait_sound_command(int n)
{
  while(read_sound_int(&SOUNDSTATUS->cmdstatus)!=n);
}

void do_sound_command(int cmd)
{
  wait_sound_command(0);
  write_sound_int(&SOUNDSTATUS->cmd, cmd);
  write_sound_int(&SOUNDSTATUS->cmdstatus, 1);
  wait_sound_command(2);
  write_sound_int(&SOUNDSTATUS->cmdstatus, 0);
  wait_sound_command(0);
}

void start_sound()
{
  while(read_sound_int(&SOUNDSTATUS->mode) != MODE_PLAY) {
    memset4((void*)RING_BUF, 0, SAMPLES_TO_BYTES(RING_BUFFER_SAMPLES+1));
#ifdef STEREO
    memset4((void*)(RING_BUF+STEREO_OFFSET), 0,
	    SAMPLES_TO_BYTES(RING_BUFFER_SAMPLES+1));
#endif
    fillpos = 0;
    do_sound_command(CMD_SET_MODE(MODE_PLAY));
  }
}

void stop_sound()
{
  while(read_sound_int(&SOUNDSTATUS->mode) != MODE_PAUSE)
    do_sound_command(CMD_SET_MODE(MODE_PAUSE));
}

int sound_device_open = 0;

