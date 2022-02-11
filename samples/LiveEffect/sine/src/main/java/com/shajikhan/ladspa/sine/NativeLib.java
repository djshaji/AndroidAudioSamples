package com.shajikhan.ladspa.sine;

public class NativeLib {

    // Used to load the 'sine' library on application startup.
    static {
        System.loadLibrary("sine");
    }

    /**
     * A native method that is implemented by the 'sine' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}