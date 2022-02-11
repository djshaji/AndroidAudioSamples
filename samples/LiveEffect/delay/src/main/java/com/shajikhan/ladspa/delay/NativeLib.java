package com.shajikhan.ladspa.delay;

public class NativeLib {

    // Used to load the 'delay' library on application startup.
    static {
        System.loadLibrary("delay");
    }

    /**
     * A native method that is implemented by the 'delay' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}