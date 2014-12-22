/* vi:set ts=8 sts=4 sw=4:
 *
 * The bulk of this file is lifted from Vim's own vimrun.exe, but has been
 * modified to not allocate a console if the -s flag is specified, otherwise it
 * attempts to reproduce the behavior of the original executable.
 *
 * Original copyright notice:
 * VIM - Vi IMproved	by Bram Moolenaar
 *			this file by Vince Negri
 */

/*
 * vimrun.c - Tiny Win32 program to safely run an external command in a
 *	      DOS console.
 *	      This program is required to avoid that typing CTRL-C in the DOS
 *	      console kills Vim.  Now it only kills vimrun.
 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#ifdef _WIN32_WINNT
# undef _WIN32_WINNT
# define _WIN32_WINNT 0x0501
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    /* Au revoir, hourglass. */
    MSG msg;
    PostMessage(NULL, WM_NULL, 0, 0);
    GetMessage(&msg, NULL, 0, 0);

    const char *p = GetCommandLine();
    DWORD retval;
    int silent = 0;
    int inquote = 0;

    /* Skip the executable name, which might be in "". */
    while (*p)
    {
        if (*p == '"')
            inquote = !inquote;
        else if (!inquote && *p == ' ')
        {
            ++p;
            /* Eat all whitespace before continuing. */
            while (*p == ' ') p++;
            break;
        }
        ++p;
    }

    /* "-s" argument: don't wait for a key hit. */
    if (p[0] == '-' && p[1] == 's' && p[2] == ' ')
    {
        silent = 1;
        p += 3;
        while (*p == ' ')
            ++p;
    }

    char args[8192];
    args[0] = 0;

    strcat(args, "cmd.exe /c \"");
    strcat(args, p);
    strcat(args, "\"");

    if (!silent) {
        if (AllocConsole()) {
            ShowWindow(GetConsoleWindow(), SW_SHOW);
            AttachConsole(GetCurrentProcessId());
            freopen("CON", "w", stdout);
        }

        /* Print the command, including quotes and redirection. */
        puts(p);
    }

    /* Do it! */
    PROCESS_INFORMATION procInfo;
    memset(&procInfo, 0, sizeof(PROCESS_INFORMATION));

    STARTUPINFO startInfo;
    memset(&startInfo, 0, sizeof(STARTUPINFO));
    startInfo.cb = sizeof(STARTUPINFO);
    startInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_FORCEOFFFEEDBACK;
    startInfo.wShowWindow = SW_HIDE;

    if (!silent) {
        startInfo.dwFlags |= STARTF_USESTDHANDLES;
        startInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        startInfo.hStdOutput =  GetStdHandle(STD_OUTPUT_HANDLE);
        startInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    }

    if (!CreateProcess(NULL, args, NULL, NULL, !silent, silent ? CREATE_NEW_CONSOLE : 0, NULL, NULL, &startInfo, &procInfo)) {
        return GetLastError();
    }

    WaitForSingleObject(procInfo.hProcess, INFINITE);
    if (!GetExitCodeProcess(procInfo.hProcess, &retval))
        retval = 0;

    if (!silent) {
        if (retval != 0)
            printf("shell returned %ld\n", retval);

        puts("Hit any key to close this window...");
        getch();
    }


    CloseHandle(procInfo.hThread);
    CloseHandle(procInfo.hProcess);

    return retval;
}
