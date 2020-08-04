//=================================================================================================
// cprocess.h - Defines a process-management class
//=================================================================================================
#pragma once
#include <stdio.h>
#include <stdlib.h>

//=================================================================================================
// class CProcess - Launches processes
//=================================================================================================
class CProcess
{
public:

    // Constructor and destructor
    CProcess();
    ~CProcess();

    // Calls ::popen() with printf-style parameters
    bool open(bool display, const char* fmt, ...);

    // After an open, this fetches the next line of output
    bool get_line(char* buffer, unsigned int buf_length);

    // Automatically closed by Run() or when GetLine returns false
    // Returns the process exit code
    int  close();

    // Calls open, and fetches all of the output, optionally printing it
    // Returns the process exit code
    int  run(bool display, const char* fmt, ...);

protected:

    // This is the FILE* managed by open and get_line
    FILE*   m_file;
};
//=================================================================================================
