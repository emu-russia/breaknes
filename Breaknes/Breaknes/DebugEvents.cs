// Used to notify the debugger of emulation lifecycle events

using System.Text;
using System.Globalization;
using System.ComponentModel;
using System.Collections.Generic;
using System.Collections;

namespace Breaknes
{
    public enum DebugEvent
    {
        BoardChanged = 0,
        LoadRom,
        Step,
    }

    public delegate void DebugEventHandler(DebugEvent e);

    public class DebugEventsHub
    {
        List<DebugEventHandler> listeners = new List<DebugEventHandler>();

        public void AddListener(DebugEventHandler inst)
        {
            listeners.Add(inst);
        }

        public void RemoveListener(DebugEventHandler inst)
        {
            listeners.Remove(inst);
        }

        public void SignalDebugEvent(DebugEvent e)
        {
            foreach (var inst in listeners)
            {
                inst.Invoke(e);
            }
        }
    }
}
