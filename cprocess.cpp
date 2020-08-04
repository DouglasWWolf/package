//=================================================================================================
// cprocess.cpp- Implements a process-management class
//=================================================================================================
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "cprocess.h"


//=================================================================================================
// Constructor() - Just sets the FILE* to NULL
//=================================================================================================
CProcess::CProcess() {m_file = NULL;}
//=================================================================================================


//=================================================================================================
// Destructor() - closes the FILE* if it's open
//=================================================================================================
CProcess::~CProcess() {close();}
//=================================================================================================

//=================================================================================================
// close() - closes the process if it's still running
//=================================================================================================
int CProcess::close()
{
    int exit_code = 0;
    if (m_file) exit_code = pclose(m_file);
    m_file = nullptr;
    return exit_code;
}
//=================================================================================================


//=================================================================================================
// open() - Runs an external process, but doesn't fetch the output
//=================================================================================================
bool CProcess::open(bool display, const char* fmt, ...)
{
    char buffer[1000];
    va_list args;

    // Create the command line as specified by the caller
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);

    // If the caller wishes us to display the commad line, do so
    if (display) printf("Running: %s\n", buffer);

    // Make sure there's not a currently open file
    close();

    // Start the process
    m_file = ::popen(buffer, "r");

    // And tell the caller if it worked
    return (m_file != nullptr);
}
//=================================================================================================



//=================================================================================================
// get_line() - Fetches a line of output from a currently open popen()
//=================================================================================================
bool CProcess::get_line(char* out, unsigned int buf_length)
{
    // If there's no process open, tell the caller
    if (m_file == nullptr) return false;

    // If we're able to fetch a line of output, tell the caller
    if (fgets(out, buf_length, m_file)) return true;

    // Tell the caller that there's no more output to be fetched
    return false;
}
//=================================================================================================


//=================================================================================================
// run() - Runs the process (via open()) and returns when process exits
//=================================================================================================
int CProcess::run(bool display, const char* fmt, ...)
{
    char    buffer[1000];
    va_list args;

    // Create the command line as specified by the caller
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    // If we fail to execute this process, tell the caller
    if (!open(display, "%s", buffer)) return 0;

    // Loop through each line and optionally display it
    while (get_line(buffer, sizeof buffer))
    {
        if (display) printf("%s", buffer);
    }

    // We're done with the process.
    return close();
}
//=================================================================================================
