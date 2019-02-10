

package org.webrtc;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.EnumSet;
import java.util.logging.Logger;
import java.util.logging.Level;


public class Logging
{
    private static final Logger fallbackLogger = Logger.getLogger("org.webrtc.Logging");
    private static volatile boolean tracingEnabled;
    private static volatile boolean nativeLibLoaded;

    static {
        try
        {
            System.loadLibrary("jingle_peerconnection_so");
            nativeLibLoaded = true;
        } catch (UnsatisfiedLinkError t)
        {
            
            fallbackLogger.setLevel(Level.ALL);

            fallbackLogger.log(Level.WARNING, "Failed to load jingle_peerconnection_so: ", t);
        }
    }

    
    public enum TraceLevel {
        TRACE_NONE(0x0000),
        TRACE_STATEINFO(0x0001),
        TRACE_WARNING(0x0002),
        TRACE_ERROR(0x0004),
        TRACE_CRITICAL(0x0008),
        TRACE_APICALL(0x0010),
        TRACE_DEFAULT(0x00ff),
        TRACE_MODULECALL(0x0020),
        TRACE_MEMORY(0x0100),
        TRACE_TIMER(0x0200),
        TRACE_STREAM(0x0400),
        TRACE_DEBUG(0x0800),
        TRACE_INFO(0x1000),
        TRACE_TERSEINFO(0x2000),
        TRACE_ALL(0xffff);

        public final int level;
        TraceLevel(int level)
        {
            this.level = level;
        }
    };

    
    public enum Severity {
        LS_SENSITIVE, LS_VERBOSE, LS_INFO, LS_WARNING, LS_ERROR,
    };

    public static void enableLogThreads()
    {
        if (!nativeLibLoaded) {
            fallbackLogger.log(Level.WARNING, "Cannot enable log thread because native lib not loaded.");
            return;
        }

        nativeEnableLogThreads();
    }

    public static void enableLogTimeStamps()
    {
        if (!nativeLibLoaded) {
            fallbackLogger.log(Level.WARNING,
                               "Cannot enable log timestamps because native lib not loaded.");
            return;
        }

        nativeEnableLogTimeStamps();
    }

    
    
    public static synchronized void enableTracing(
        String path, EnumSet<TraceLevel> levels, Severity severity)
    {
        if (!nativeLibLoaded) {
            fallbackLogger.log(Level.WARNING, "Cannot enable tracing because native lib not loaded.");
            return;
        }

        if (tracingEnabled) {
            return;
        }

        int nativeLevel = 0;

        for (TraceLevel level : levels) {
            nativeLevel |= level.level;
        }

        nativeEnableTracing(path, nativeLevel, severity.ordinal());
        tracingEnabled = true;
    }

    public static void log(Severity severity, String tag, String message)
    {
        if (tracingEnabled) {
            nativeLog(severity.ordinal(), tag, message);
            return;
        }

        
        Level level;

        switch (severity) {
        case LS_ERROR:
            level = Level.SEVERE;
            break;

        case LS_WARNING:
            level = Level.WARNING;
            break;

        case LS_INFO:
            level = Level.INFO;
            break;

        default:
            level = Level.FINE;
            break;
        }

        fallbackLogger.log(level, tag + ": " + message);
    }

    public static void d(String tag, String message)
    {
        log(Severity.LS_INFO, tag, message);
    }

    public static void e(String tag, String message)
    {
        log(Severity.LS_ERROR, tag, message);
    }

    public static void w(String tag, String message)
    {
        log(Severity.LS_WARNING, tag, message);
    }

    public static void e(String tag, String message, Throwable e)
    {
        log(Severity.LS_ERROR, tag, message);
        log(Severity.LS_ERROR, tag, e.toString());
        log(Severity.LS_ERROR, tag, getStackTraceString(e));
    }

    public static void w(String tag, String message, Throwable e)
    {
        log(Severity.LS_WARNING, tag, message);
        log(Severity.LS_WARNING, tag, e.toString());
        log(Severity.LS_WARNING, tag, getStackTraceString(e));
    }

    public static void v(String tag, String message)
    {
        log(Severity.LS_VERBOSE, tag, message);
    }

    private static String getStackTraceString(Throwable e)
    {
        if (e == null) {
            return "";
        }

        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        e.printStackTrace(pw);
        return sw.toString();
    }

    private static native void nativeEnableTracing(
        String path, int nativeLevels, int nativeSeverity);
    private static native void nativeEnableLogThreads();
    private static native void nativeEnableLogTimeStamps();
    private static native void nativeLog(int severity, String tag, String message);
}
