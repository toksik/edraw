#include <string.h>

/***
 * Rules for debugging messages
 *
 * 1. All messages start with a capital letter
 * 2. Every function begins with a call of "edraw_called"
 * 3. Message classification:
 *     fatal -> may lead to operations on invalid memory, file descriptors,
 *              etc. or to dramatically wrong results (eg. no result)
 *     warning -> indicates something very likely unintended happend or
 *                non-critical errors which may not happend while debugging
 *     debug -> helps developers to understand what the program is doing and
 *              what they may have done wrong
 * 4. "Fatal" and "warning" messages explain the problem for advanced users,
 *    instead "debug" messages are only intended to help developers
 * Examples
 *  fatal: "Out of memory", "Mapping framebuffer into memory failed"
 *  warning: "Cannot close the framebuffer's file descriptor",
 *           "Update array is invalid, doing full update instead"
 *  debug: "Destroying uninitialised surface"
 */
#ifdef EDRAW_DEBUG
# define EDRAW_WARN_ALL
# define edraw_debug(function, msg)                         \
	(write(2, "D [" function "] " msg "\n", strlen(msg) \
	       +strlen(function)+6))
#else
# define edraw_debug(function, msg) (0)
#endif

#ifdef EDRAW_EXIT_FATAL
# define EDRAW_WARN_FATAL
# define edraw_exit_fatal()                  \
{                                            \
	write(2, "fatal error: exit\n", 19); \
	exit(1);                             \
}
#else
# define edraw_exit_fatal() (0)
#endif

#ifdef EDRAW_WARN_ALL
# define EDRAW_WARN_FATAL
# define edraw_warn(function, msg)                          \
	(write(2, "W [" function "] " msg "\n", strlen(msg) \
	       +strlen(function)+6))
#else
# define edraw_warn(function, msg) (0)
#endif

#ifdef EDRAW_WARN_FATAL
# define edraw_fatal(function, msg)                        \
{                                                          \
	write(2, "F [" function "] " msg "\n", strlen(msg) \
			+strlen(function)+6);              \
	edraw_exit_fatal();                                \
}
#else
# define edraw_fatal(function, msg) (0)
#endif

#ifdef EDRAW_DEBUG_CALLS
# define edraw_called(name) (write(2, "C [" name "]" "\n", strlen(name)+5))
#else
# define edraw_called(name) (0)
#endif
