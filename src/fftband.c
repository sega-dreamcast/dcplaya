#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "fftband.h"

#include "sysdebug.h"

#define FFT_BAND_FIX 12

float R(const float radix, int band)
{
  float r, p;
  int i;

  for (i=0,p=0,r=1; i<band; ++i) {
    r *= radix;
    p += r;
  }
  return p;
}

static float r_radix(float v, float a, float b, int band, int level)
{
  float r, mid;

  if (level >= 27) {
    return a;
  }
  mid = (a+b) / 2;
  r = R(mid, band);
  if (v < r) {
    return r_radix(v, a, mid, band, level+1);
  } else {
    return r_radix(v, mid, b, band, level+1);
  }
}

static unsigned int higher_radix(float v, int band)
{
  int i;

  for (i=1; ; ++i) {
    float r = R(i, band);
    if (r >= v) {
      break;
    }
  }
  return i;
}

static float radix_calc(int size, int band)
{
  unsigned int radix = higher_radix(size, band);
  return r_radix(size, 0, radix, band, 0);
}

int fftband_reset(fftband_t * band)
{
  memset(band, 0, sizeof(*band));
  return 0;
}

void fftband_limit(fftband_t * band, const float fmin, const float fmax)
{
  unsigned int fm, fM;
  fm = (fmin < 0) ? 0 : fmin;
  fM = (fmax < 0) ? 0 : fmax;

  if (fM < fm) {
    fM ^= fm; 
    fm ^= fM; 
    fM ^= fm; 
  }
  band->fmin = fm;
  band->fmax = fM;
}

static void fftband_setup(fftband_t * band,
			  unsigned int oof0, unsigned int nbSamples)
{
  //  nbSamples <<= 8;
/*   const unsigned int oof0 = (nbSamples << FFT_BAND_FIX) / samplingFrq; */
  unsigned int bMin, bMax;

  nbSamples <<= FFT_BAND_FIX;

  bMin = band->fmin * oof0;
  bMax = band->fmax * oof0;

  if (bMin >= nbSamples/2) {
    bMin = (nbSamples/2)-1;
  }
  if (bMax >= nbSamples/2) {
    bMax = nbSamples/2-1;
  }

  band->bmin = bMin;
  band->bmax = bMax;

  if (bMin == bMax) {
    band->bminscale = 1 << FFT_BAND_FIX;
    band->bmaxscale = 0;
    band->bdivisor  = 1 << FFT_BAND_FIX;
  } else {
    int dMin = bMin & ((1 << FFT_BAND_FIX)-1);
    int dMax = bMax & ((1 << FFT_BAND_FIX)-1);
    band->bdivisor = bMax - bMin;
    band->bminscale = (1 << FFT_BAND_FIX) - dMin;
    band->bmaxscale = dMax;
  }
}


fftbands_t * fftband_create(int n, int fft_size, int sampling,
			    const fftband_limit_t * limits)
{
  int i;
  fftbands_t * bands;
  int size = sizeof(fftbands_t) + n * sizeof(fftband_t) - sizeof(bands->band);

  SDDEBUG("[%s] n:%d fft:%d sampling:%d f0:%.2f\n",
	  __FUNCTION__, n, fft_size, sampling, (float)sampling/fft_size);
  
  if (bands = malloc(size), !bands) {
    return 0;
  }
  memset(bands,0,size);
  bands->n = n;
  bands->sampling = sampling;
  bands->fftsize = fft_size;
  bands->oof0 = (fft_size << FFT_BAND_FIX) / sampling;

  if (limits) {
    for (i=0; i<n; ++i) {
      fftband_limit(bands->band+i, limits[i].fmin, limits[i].fmax);
    }
  } else {
    float radix = radix_calc(fft_size/2,n);
    float fmin, fmax, j, r;
    float band_width = (float)sampling / (float)fft_size;

    for (i=0, j=0, r=radix, fmin=0; i<n; ++i) {
      j += r;
      r *= radix;
      fmax = band_width * j;
      fftband_limit(bands->band+i, fmin, fmax);
      fmin = fmax;
    }
  }

  for (i=0; i<n; ++i) {
    fftband_t * b = bands->band+i;
    fftband_setup(b, bands->oof0, fft_size);
/*     SDDEBUG("[%s] f[%d-%d] b[%x-%x] s[%x-%x/%x]\n", __FUNCTION__, */
/* 	    b->fmin, b->fmax, b->bmin, b->bmax, */
/* 	    b->bminscale, b->bmaxscale, b->bdivisor); */
  }

  return bands;
}

static unsigned int fftband_calc_value(fftband_t * band, const uint16 * fft)
{
  unsigned int w;
  int i = band->bmin >> 12, j = band->bmax >> 12;

  // Calculates power of this band from FFT
  w = fft[i] * band->bminscale;
  for (++i; i<j; ++i) {
    w += fft[i] << FFT_BAND_FIX;
  }
  w += fft[i] * band->bmaxscale;
  w /= band->bdivisor;

  return w;
}

void fftband_update(fftbands_t * bands, const uint16 * fft)
{
  unsigned int w;
  int i;
/*   int previdx = bands->tapidx; */
/*   int idx = (previdx+1) & 3; */
/*   bands->tapidx = idx; */

  for (i=0; i<bands->n; ++i) {
    fftband_t * b = bands->band + i;
    w = fftband_calc_value(b,fft);
/*     b->tapacu -= b->tap[idx]; */
/*     w = (w + b->tap[previdx]) >> 1; */
/*     b->tap[idx] = w; */
/*     b->tapacu += w; */

    b->v = (w + b->v) >> 1;
  }
}