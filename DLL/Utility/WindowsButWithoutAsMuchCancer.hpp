#pragma once

// This header file is dedicated to removing the awful pollution
// of the global namespace created by the Windows header, then including what's left.
// Thank you Microsoft for namespacing/prefixing absolutely nothing
// and using names like Rectangle, Ellipse, min, max, etc.
// that are so easy to create naming conflicts with!

#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI 
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

#include <windows.h>
