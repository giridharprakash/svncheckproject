#ifndef DEBUGGERSTATE_H
#define DEBUGGERSTATE_H

#include "debugger_defs.h"

class DebuggerGDB;
class ProjectBuildTarget;

class DebuggerState
{
    public:
        DebuggerState(DebuggerGDB* plugin);
        ~DebuggerState();

        BreakpointsList& GetBreakpoints(){ return m_Breakpoints; }

        bool StartDriver(ProjectBuildTarget* target);
        void StopDriver();
        DebuggerDriver* GetDriver();

        void CleanUp();

        int AddBreakpoint(DebuggerBreakpoint* bp); // returns -1 if not found
        int AddBreakpoint(const wxString& file, int line, bool temp = false, const wxString& lineText = wxEmptyString); // returns -1 if not found
        DebuggerBreakpoint* RemoveBreakpoint(const wxString& file, int line, bool deleteit = true);
        DebuggerBreakpoint* RemoveBreakpoint(int idx, bool deleteit = true);
        void RemoveAllBreakpoints(const wxString& file, bool deleteit = true);
        void RemoveAllProjectBreakpoints(cbProject* prj);

        // helpers to keep in sync with the editors
        int RemoveBreakpointsRange(const wxString& file, int startline, int endline);
        void ShiftBreakpoints(const wxString& file, int startline, int nroflines);

        int HasBreakpoint(const wxString& file, int line); // returns -1 if not found
        DebuggerBreakpoint* GetBreakpoint(int idx);
        DebuggerBreakpoint* GetBreakpointByNumber(int num);
        void ResetBreakpoint(int idx);
        void ApplyBreakpoints();
    protected:
        void SetupBreakpointIndices();
        wxString ConvertToValidFilename(const wxString& filename);
        cbProject* FindProjectForFile(const wxString& file);

        DebuggerGDB* m_pPlugin;
        DebuggerDriver* m_pDriver;
        WatchesArray m_Watches;
        BreakpointsList m_Breakpoints;
        size_t m_BpAutoIndex;
};

#endif // DEBUGGERSTATE_H
