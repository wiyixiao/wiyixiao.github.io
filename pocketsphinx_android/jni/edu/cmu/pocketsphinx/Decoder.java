/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package edu.cmu.pocketsphinx;

public class Decoder {
  private transient long swigCPtr;
  protected transient boolean swigCMemOwn;

  protected Decoder(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Decoder obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        pocketsphinxJNI.delete_Decoder(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public Decoder() {
    this(pocketsphinxJNI.new_Decoder__SWIG_0(), true);
  }

  public Decoder(Config c) {
    this(pocketsphinxJNI.new_Decoder__SWIG_1(Config.getCPtr(c), c), true);
  }

  public Config getConfig() {
    long cPtr = pocketsphinxJNI.Decoder_getConfig(swigCPtr, this);
    return (cPtr == 0) ? null : new Config(cPtr, false);
  }

  public int startUtt() {
    return pocketsphinxJNI.Decoder_startUtt__SWIG_0(swigCPtr, this);
  }

  public int startUtt(String uttid) {
    return pocketsphinxJNI.Decoder_startUtt__SWIG_1(swigCPtr, this, uttid);
  }

  public String getUttid() {
    return pocketsphinxJNI.Decoder_getUttid(swigCPtr, this);
  }

  public int endUtt() {
    return pocketsphinxJNI.Decoder_endUtt(swigCPtr, this);
  }

  public int processRaw(SWIGTYPE_p_short SDATA, long NSAMP, boolean no_search, boolean full_utt) {
    return pocketsphinxJNI.Decoder_processRaw__SWIG_0(swigCPtr, this, SWIGTYPE_p_short.getCPtr(SDATA), NSAMP, no_search, full_utt);
  }

  public int processRaw(short[] shorts, long nshorts, boolean no_search, boolean full_utt) {
    return pocketsphinxJNI.Decoder_processRaw__SWIG_1(swigCPtr, this, shorts, nshorts, no_search, full_utt);
  }

  public Hypothesis getHyp() {
    long cPtr = pocketsphinxJNI.Decoder_getHyp(swigCPtr, this);
    return (cPtr == 0) ? null : new Hypothesis(cPtr, false);
  }

}
