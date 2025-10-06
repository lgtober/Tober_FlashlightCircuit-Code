using System;
using System.Collections.Concurrent;
using System.IO.Ports;
using System.Threading;
using UnityEngine;

public class DataInOut01 : MonoBehaviour
{
    [Header("Serial Settings")]
    [Tooltip("Leave blank to auto-detect on macOS (usbmodem/usbserial).")]
    public string portName = "/dev/cu.usbmodem101";
    public int baudRate = 115200;
    [Tooltip("Try to auto-pick first matching macOS serial device if portName is blank or invalid.")]
    public bool autoDetectPort = true;
    [Tooltip("Serial read timeout in ms (used on the background reader thread).")]
    public int readTimeoutMs = 100;

    [Header("Live Data (from Arduino)")]
    public int flashlightOn = 0;   // 0 = off, 1 = on
    public int brightness = 0;     // 0–255
    public int colorIndex = 0;     // 0=White, 1=Red, 2=Green, 3=Blue

    [Header("Unity Light to Control")]
    public Light targetLight;

        void LateUpdate()
        {
            if (targetLight == null) return;

            targetLight.enabled = (flashlightOn == 1);

            // brightness as intensity (scale 0–255 → 0–5 for example)
            targetLight.intensity = brightness / 51f; // maps 0–255 to ~0–5

            // color
            switch (colorIndex)
            {
                case 0: targetLight.color = Color.white; break;
                case 1: targetLight.color = Color.red; break;
                case 2: targetLight.color = Color.green; break;
                case 3: targetLight.color = Color.blue; break;
            }
        }


    SerialPort _port;
    Thread _readerThread;
    volatile bool _runReader;
    readonly ConcurrentQueue<string> _lines = new ConcurrentQueue<string>();
    string _lastError = null;

    void OnEnable()
    {
        TryOpenPort();
    }

    void OnDisable()
    {
        StopReaderAndClose();
    }

    void Update()
    {
        // Surface any async errors to the Console
        if (!string.IsNullOrEmpty(_lastError))
        {
            Debug.LogError(_lastError);
            _lastError = null;
        }

        // Drain queued lines; parse the most recent valid one
        while (_lines.TryDequeue(out var line))
        {
            var parts = line.Trim().Split(new[] { ' ', '\t' }, StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length < 3) continue;

            if (int.TryParse(parts[0], out var onOff) &&
                int.TryParse(parts[1], out var bright) &&
                int.TryParse(parts[2], out var color))
            {
                flashlightOn = onOff;
                brightness = bright;
                colorIndex = color;
            }
        }

    }

    void TryOpenPort()
    {
        // Validate/auto-detect port
        if (autoDetectPort)
        {
            var chosen = ChoosePort(portName);
            if (!string.IsNullOrEmpty(chosen)) portName = chosen;
        }

        if (string.IsNullOrEmpty(portName))
        {
            Debug.LogError("No serial port specified/found.");
            return;
        }

        try
        {
            _port = new SerialPort(portName, baudRate);
            _port.NewLine = "\n";           // Arduino println() ends with '\n'
            _port.ReadTimeout = readTimeoutMs;
            _port.DtrEnable = true;         // Often needed for native USB boards
            _port.RtsEnable = true;

            _port.Open();
            _port.DiscardInBuffer();
            Debug.Log($"Opened serial port: {portName} @ {baudRate}");

            // Start background reader
            _runReader = true;
            _readerThread = new Thread(ReaderLoop) { IsBackground = true, Name = "SerialReader" };
            _readerThread.Start();
        }
        catch (Exception ex)
        {
            Debug.LogError($"Error opening serial port '{portName}': {ex.Message}");
            SafeClose();
        }
    }

    void ReaderLoop()
    {
        try
        {
            while (_runReader && _port != null && _port.IsOpen)
            {
                try
                {
                    // ReadLine blocks up to ReadTimeout
                    string line = _port.ReadLine(); // expects newline-terminated lines
                    if (!string.IsNullOrWhiteSpace(line))
                        _lines.Enqueue(line);
                }
                catch (TimeoutException)
                {
                    // Normal: just try again
                }
                catch (Exception ex)
                {
                    _lastError = $"Serial read error: {ex.Message}";
                    break;
                }
            }
        }
        finally
        {
            // Reader is exiting; main thread will close port
        }
    }

    void StopReaderAndClose()
    {
        _runReader = false;

        if (_readerThread != null)
        {
            try { _readerThread.Join(500); } catch { /* ignore */ }
            _readerThread = null;
        }

        SafeClose();
    }

    void SafeClose()
    {
        if (_port != null)
        {
            try
            {
                if (_port.IsOpen) _port.Close();
            }
            catch { /* ignore */ }
            finally
            {
                _port.Dispose();
                _port = null;
            }
        }
    }

    static string ChoosePort(string preferred)
    {
#if UNITY_EDITOR_OSX || UNITY_STANDALONE_OSX
        // On macOS, look for a plausible device if preferred is empty or does not exist
        try
        {
            var ports = SerialPort.GetPortNames();
            // If user gave a port and it exists, keep it
            if (!string.IsNullOrEmpty(preferred))
            {
                foreach (var p in ports) if (p == preferred) return preferred;
            }

            // Otherwise pick first usbmodem or usbserial
            foreach (var p in ports)
                if (p.Contains("usbmodem") || p.Contains("usbserial"))
                    return p;

            // Fallback: if any ports exist, return the first
            if (ports.Length > 0) return ports[0];
        }
        catch { /* ignore */ }
#endif
        return preferred; // return what we were given (may be empty)
    }
}

