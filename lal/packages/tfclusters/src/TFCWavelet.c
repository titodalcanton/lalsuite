/******** <lalVerbatim file="TFCWaveletCV"> ********
Author: Sylvestre, J
$Id$
********* </lalVerbatim> ********/


#include "lal/LALRCSID.h"

NRCSID (TFCWAVELETC, "$Id$");

#include <lal/TFCWavelet.h>
#include <lal/TFClusters.h>
#include <lal/SeqFactories.h>
#include <math.h>
#include <strings.h>

/* apply one level of wavelet transform */
     /* output vectors must be pre-allocated, and be larger and equal to input length / 2 */
     /* input must be a power of 2 */
void 
LALWaveletFilter (
		  LALStatus *status,
		  TFCWavelet *wave
		  )
{

  REAL4Vector *smooth,
              *detail,
              *input,
              *wavelet;

  INT4 sig;
  UINT4 i,j,jtmp;

  INITSTATUS (status, "LALWaveletFilter",TFCWAVELETC);
  ATTATCHSTATUSPTR (status);

  ASSERT ( wave, status, TFCWAVELETH_ENULLP, TFCWAVELETH_MSGENULLP );

  smooth = wave->smooth;
  detail = wave->detail;
  input = wave->input;
  wavelet = wave->wavelet;

  ASSERT ( smooth, status, TFCWAVELETH_ENULLP, TFCWAVELETH_MSGENULLP );
  ASSERT ( detail, status, TFCWAVELETH_ENULLP, TFCWAVELETH_MSGENULLP );
  ASSERT ( input, status, TFCWAVELETH_ENULLP, TFCWAVELETH_MSGENULLP );
  ASSERT ( wavelet, status, TFCWAVELETH_ENULLP, TFCWAVELETH_MSGENULLP );

  ASSERT ( 2 * smooth->length == input->length, status, TFCWAVELETH_ELEN, TFCWAVELETH_MSGELEN );
  ASSERT ( 2 * detail->length == input->length, status, TFCWAVELETH_ELEN, TFCWAVELETH_MSGELEN );

  /* NOTE: should check power of 2 */

  /* main loop */
  for(i=0; i<input->length - wavelet->length + 1; i+=2) {
    smooth->data[i>>1] = detail->data[i>>1] = 0.0;
    for(j=0, sig = 1; j<wavelet->length; j++, sig*=-1) {
      smooth->data[i>>1] += wavelet->data[j] * input->data[i+j];
      detail->data[i>>1] += sig * wavelet->data[wavelet->length-j-1] * input->data[i+j];
    }
  }

  /* wrapped around */
  for(;i<input->length;i+=2) { 
    smooth->data[i>>1] = detail->data[i>>1] = 0.0;
    for(j=0, sig = 1; j<input->length - i; j++, sig *=-1) {
      smooth->data[i>>1] += wavelet->data[j] * input->data[i+j];
      detail->data[i>>1] += sig * wavelet->data[wavelet->length-j-1] * input->data[i+j];
    }

    jtmp=j;
    for(;j<wavelet->length; j++, sig *=-1) {
      smooth->data[i>>1] += wavelet->data[j] * input->data[j-jtmp];
      detail->data[i>>1] += sig * wavelet->data[wavelet->length-j-1] * input->data[j-jtmp];
    }
  }

  /* Normal exit */
  DETATCHSTATUSPTR (status);
  RETURN (status);
  
}



void 
LALComputeWaveletSpectrogram (
			      LALStatus *status, 
			      Spectrogram *out, 
			      TFCWParams *tspec, 
			      REAL4TimeSeries *tseries
			      )
{
  REAL8 *spower;
  REAL4Vector **previousSmooth, *tmpVect, *smoothI, *detailI;
  REAL4Vector first;
  TFPlaneParams *params;
  UINT4 smin, smax, is, N, i, NPrev, ip, imax;
  REAL4 max;
  TFCWavelet twav;


  INITSTATUS (status, "LALWaveletFilter",TFCWAVELETC);
  ATTATCHSTATUSPTR (status);

  ASSERT ( out, status, TFCWAVELETH_ENULLP, TFCWAVELETH_MSGENULLP );
  ASSERT ( tspec, status, TFCWAVELETH_ENULLP, TFCWAVELETH_MSGENULLP );
  ASSERT ( tseries, status, TFCWAVELETH_ENULLP, TFCWAVELETH_MSGENULLP );

  ASSERT ( (REAL8)(tseries->data->length) * tseries->deltaT ==
           (REAL8)(tspec->timeBins) * tspec->deltaT,
	   status, TFCWAVELETH_EINCOMP, TFCWAVELETH_MSGEINCOMP );

  ASSERT( !(out->power), status, TFCWAVELETH_ENNULLP, TFCWAVELETH_MSGENNULLP );
  ASSERT( !(out->params), status, TFCWAVELETH_ENNULLP, TFCWAVELETH_MSGENNULLP );

  /* set tf parameters */
  params = (TFPlaneParams *)LALMalloc(sizeof(TFPlaneParams));
  if(!params)
    {ABORT(status, TFCWAVELETH_EMALLOC, TFCWAVELETH_MSGEMALLOC );}
  out->params = params;

  params->timeBins = tspec->timeBins;
  params->freqBins = tspec->freqBins;
  params->deltaT = tspec->deltaT;
  params->flow = 1.0 / tspec->maxScale;
  
  /* allocate memory for simple spectrogram */
  spower = (REAL8 *)LALMalloc(tspec->timeBins * tspec->freqBins * sizeof(REAL8));
  if(!spower)
    {ABORT(status, TFCWAVELETH_EMALLOC, TFCWAVELETH_MSGEMALLOC );}
  bzero(spower, tspec->timeBins * tspec->freqBins * sizeof(REAL8));

  for(i=imax=0, max = tspec->wavelet->data[0]; 
      i<tspec->wavelet->length; i++) {
    if(tspec->wavelet->data[i] > max) {
      imax = i;
      max = tspec->wavelet->data[i];
    }
  }
    

  /* main construction */

  /* determine scales of interest */
  smin = (UINT4)floor(log(tspec->minScale) / log(2.0));
  smax = (UINT4)ceil(log(tspec->maxScale) / log(2.0));

  ASSERT (smax - smin + 1 == tspec->freqBins, status, TFCWAVELETH_EINCOMP, TFCWAVELETH_MSGEINCOMP );

  ASSERT (tspec->deltaT == pow(2.0,(double)smin) * tseries->deltaT, status, TFCWAVELETH_EINCOMP, TFCWAVELETH_MSGEINCOMP );


  /* get to finest scale */
  N = tseries->data->length;
  first.length = N;
  first.data = tseries->data->data;
  twav.input = &first;

  twav.wavelet = tspec->wavelet;

  for(is = 0; is<smin; is++) {

    N>>=1;

    smoothI = detailI = NULL;

    LALSCreateVector(status->statusPtr, &smoothI, N);
    CHECKSTATUSPTR (status);

    LALSCreateVector(status->statusPtr, &detailI, N);
    CHECKSTATUSPTR (status);

    twav.smooth = smoothI;
    twav.detail = detailI;

    LALWaveletFilter(status->statusPtr, &twav);
    CHECKSTATUSPTR (status);

    if(is) {
      LALSDestroyVector(status->statusPtr, &(twav.input));
      CHECKSTATUSPTR (status);  
    }

    twav.input = twav.smooth;

    LALSDestroyVector(status->statusPtr, &detailI);
    CHECKSTATUSPTR (status);    
  }


  /* loop over scales */
  NPrev = 1;
  previousSmooth = (REAL4Vector **)LALMalloc(sizeof(REAL4Vector *));
  if(!previousSmooth)
    {ABORT(status, TFCWAVELETH_EMALLOC, TFCWAVELETH_MSGEMALLOC );}
 
  tmpVect = NULL;
  LALSCreateVector(status->statusPtr, &tmpVect, N);
  CHECKSTATUSPTR (status);

  memcpy(tmpVect->data, twav.smooth->data, N * sizeof(REAL4));

  *previousSmooth = tmpVect;

  for(; is<=smax; is++) {

    REAL4Vector *detail = NULL, *input = NULL;
    REAL4Vector **newPrev;
    UINT4 oldNPrev = NPrev, ssiz, dsiz, algn;

    N >>= 1; /* previous power of 2 */
    NPrev = 1<<(1+is-smin);


    if(is == smin) {
      algn = 0;
    }
    else {
      algn = imax * (1<<(1+is-smin)) - 2;
    }


    newPrev = (REAL4Vector **)LALMalloc(NPrev*sizeof(REAL4Vector *));
    if(!newPrev)
      {ABORT(status, TFCWAVELETH_EMALLOC, TFCWAVELETH_MSGEMALLOC );}

    LALSCreateVector(status->statusPtr, &detail, N);
    CHECKSTATUSPTR (status);

    LALSCreateVector(status->statusPtr, &input, N<<1);
    CHECKSTATUSPTR (status);

    for(ip = 0; ip < oldNPrev; ip++) {
      
      REAL4Vector *smooth = NULL;

      /* allocate appropriate memory */    
      LALSCreateVector(status->statusPtr, &smooth, N);
      CHECKSTATUSPTR (status);

      newPrev[ip] = smooth;

      twav.input = previousSmooth[ip];
      twav.wavelet = tspec->wavelet;

      twav.smooth = smooth;
      twav.detail = detail;

      LALWaveletFilter(status->statusPtr, &twav);
      CHECKSTATUSPTR (status);

      /* plug into output */
      if(smin <= is && is <= smax) {
	ssiz = 1<<(1 + is - smin);
	for(i=0;ssiz*i + algn + ip < params->timeBins; i++) {
	  spower[(is-smin) + params->freqBins * (ssiz*i + algn + ip)] = smooth->data[i] * smooth->data[i];
	}
      }

      /* do shifted one */
      smooth = NULL;
      LALSCreateVector(status->statusPtr, &smooth, N);
      CHECKSTATUSPTR (status);

      newPrev[ip+oldNPrev] = smooth;
      twav.smooth = smooth;

      /* buffer needed */
      memcpy(input->data, previousSmooth[ip]->data+1, ((N<<1) - 1)*sizeof(REAL4));
      input->data[(N<<1) - 1] = 0;

      twav.input = input;

      LALWaveletFilter(status->statusPtr, &twav);
      CHECKSTATUSPTR (status);

      /* plug into output */
      if(smin <= is && is <= smax) {
	ssiz = 1<<(1 + is - smin);
	dsiz = 1<<(is-smin);
	for(i=0;ssiz*i + algn + dsiz + ip < params->timeBins;i++) {
	  spower[(is-smin) + params->freqBins * (ssiz*i + algn + dsiz + ip)] = smooth->data[i] * smooth->data[i];
	}
      }
    }

    /* clean up */
    for(ip = 0; ip < oldNPrev; ip++) {
      tmpVect = previousSmooth[ip];
      LALSDestroyVector(status->statusPtr, &tmpVect);
      CHECKSTATUSPTR (status);
    }
    LALFree(previousSmooth);

    previousSmooth = newPrev;

    LALSDestroyVector(status->statusPtr, &input);
    CHECKSTATUSPTR (status);

    LALSDestroyVector(status->statusPtr, &detail);
    CHECKSTATUSPTR (status);
  }

  for(ip = 0; ip < NPrev; ip++) {
    tmpVect = previousSmooth[ip];
    LALSDestroyVector(status->statusPtr, &tmpVect);
    CHECKSTATUSPTR (status);
  }
  LALFree(previousSmooth);


  out->power = spower;

  /* Debug crap 
     {
     FILE *outfile;
  
     outfile = fopen("test.dat","w");
     fwrite(spower,sizeof(REAL8),out->params->timeBins * out->params->freqBins, outfile);
     fclose(outfile);
     }
  */

  /* Normal exit */
  DETATCHSTATUSPTR (status);
  RETURN (status);
}
