package com.shajikhan.ladspa.filter;

public class NativeLib {

    // Used to load the 'filter' library on application startup.
    static {
        System.loadLibrary("filter");
    }

    /**
     * A native method that is implemented by the 'filter' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}