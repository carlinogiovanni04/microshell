#ifndef BASE_H
#define BASE_H

#if defined(_MSC_VER)
#include <stddef.h>

typedef signed char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef unsigned long long usize;
typedef unsigned long long uptr;

typedef long long ssize_t;
#else
typedef __INT8_TYPE__ i8;
typedef __INT16_TYPE__ i16;
typedef __INT32_TYPE__ i32;
typedef __INT64_TYPE__ i64;

typedef __UINT8_TYPE__ u8;
typedef __UINT16_TYPE__ u16;
typedef __UINT32_TYPE__ u32;
typedef __UINT64_TYPE__ u64;

typedef __SIZE_TYPE__ usize;
typedef __SIZE_TYPE__ uptr;

typedef long long ssize_t;
typedef unsigned long long size_t;
#endif

typedef float f32;
typedef double f64;

typedef i32 b32;
#define true 1
#define false 0

#ifndef NULL
#define NULL ((void *)0)
#endif

#define header_implementation static inline

#if defined(__clang__) || defined(__GNUC__)
#define DEBUG_BREAK() __builtin_trap()
#elif defined(_MSC_VER)
#include <intrin.h>
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK()
#endif

#ifndef S_IFMT
#define S_IFMT 0170000
#define S_IFSOCK 0140000
#define S_IFLNK 0120000
#define S_IFREG 0100000
#define S_IFBLK 0060000
#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFIFO 0010000
#endif

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12

typedef struct {
  char name[256];
  u64 size;
  u32 mode;
  b32 is_dir;
} DirEntry;

typedef void (*DirEntryFn)(const DirEntry *entry, void *user);

header_implementation b32 sys_dir_foreach(const char *path, DirEntryFn fn,
                                          void *user);

/* Forward declarations of helpers defined further down the file. */
header_implementation u64 base_cstr_length(const char *cstr);

#if defined(_WIN32) || defined(_WIN64)

#define SHARED_STD_OUTPUT_HANDLE ((unsigned long)-11)
#define SHARED_STD_INPUT_HANDLE ((unsigned long)-10)
#define SHADER_STD_ERROR_HANDLE ((unsigned long)-12)

#if defined(_MSC_VER)
__declspec(dllimport) void *__stdcall GetStdHandle(unsigned long nStdHandle);
__declspec(dllimport) i32 __stdcall
WriteFile(void *hFile, const void *lpBuffer,
          unsigned long nNumberOfBytesToWrite,
          unsigned long *lpNumberOfBytesWritten, void *lpOverlapped);
__declspec(dllimport) i32 __stdcall ReadFile(void *hFile, void *lpBuffer,
                                             unsigned long nNumberOfBytesToRead,
                                             unsigned long *lpNumberOfBytesRead,
                                             void *lpOverlapped);

__declspec(dllimport) void *__stdcall FindFirstFileA(const char *lpFileName,
                                                     void *lpFindFileData);
__declspec(dllimport) i32 __stdcall FindNextFileA(void *hFindFile,
                                                  void *lpFindFileData);
__declspec(dllimport) i32 __stdcall FindClose(void *hFindFile);
__declspec(dllimport) i32 __stdcall CreateProcessA(const char *lpApplicationName,
                                      char *lpCommandLine, void *lpProcessAttributes,
                                      void *lpThreadAttributes, i32 bInheritHandles,
                                      u32 dwCreationFlags, void *lpEnvironment,
                                      const char *lpCurrentDirectory, void *lpStartupInfo,
                                      void *lpProcessInformation);
__declspec(dllimport) void *__stdcall malloc(size_t size);
__declspec(dllimport) void __stdcall free(void *memblock);
#else
__attribute__((dllimport)) void *__stdcall
GetStdHandle(unsigned long nStdHandle);
__attribute__((dllimport)) i32 __stdcall
WriteFile(void *hFile, const void *lpBuffer,
          unsigned long nNumberOfBytesToWrite,
          unsigned long *lpNumberOfBytesWritten, void *lpOverlapped);
__attribute__((dllimport)) i32 __stdcall
ReadFile(void *hFile, void *lpBuffer, unsigned long nNumberOfBytesToRead,
         unsigned long *lpNumberOfBytesRead, void *lpOverlapped);

__attribute__((dllimport)) void *__stdcall
FindFirstFileA(const char *lpFileName, void *lpFindFileData);
__attribute__((dllimport)) i32 __stdcall FindNextFileA(void *hFindFile,
                                                       void *lpFindFileData);
__attribute__((dllimport)) i32 __stdcall FindClose(void *hFindFile);
__attribute__((dllimport)) i32 __stdcall CreateProcessA(
    const char *lpApplicationName, char *lpCommandLine, void *lpProcessAttributes,
    void *lpThreadAttributes, i32 bInheritHandles, u32 dwCreationFlags,
    void *lpEnvironment, const char *lpCurrentDirectory, void *lpStartupInfo,
    void *lpProcessInformation);
#endif

void *sys_alloc(usize size)
{
#ifdef _MSC_VER
  return malloc(size);
#else
  void *ptr = NULL;
  if (posix_memalign(&ptr, 16, size) != 0)

    return NULL;
  return ptr;
#endif
}
void sys_free(void *ptr){
#ifdef _MSC_VER
  free(ptr);
#else
  free(ptr);
#endif
}

header_implementation b32 sys_exec_command(const char *cmd) {
  char cmd_copy[512];
  usize i = 0;
  while (cmd[i] != '\0' && i < sizeof(cmd_copy) - 1) {
    cmd_copy[i] = cmd[i];
    i++;
  }
  cmd_copy[i] = '\0';

  void *startup_info = sys_alloc(104); // STARTUPINFOA size
  void *process_info = sys_alloc(24);   // PROCESS_INFORMATION size
  for (usize j = 0; j < 104; j++)
    ((u8 *)startup_info)[j] = 0;
  for (usize j = 0; j < 24; j++)
    ((u8 *)process_info)[j] = 0;

  ((u32 *)startup_info)[0] = 104; // cb
  if (!CreateProcessA(NULL, cmd_copy, NULL, NULL, false, 0, NULL, NULL,
                      startup_info, process_info)) {
    return false;
  }

  return true;
}

header_implementation void sys_print_char(char c) {
  void *stdout_handle = GetStdHandle(SHARED_STD_OUTPUT_HANDLE);
  unsigned long written;
  WriteFile(stdout_handle, &c, 1, &written, 0);
}

header_implementation void sys_print_prompt(char c) {
  void *stderr_handle = GetStdHandle(SHADER_STD_ERROR_HANDLE);
  unsigned long written;
  WriteFile(stderr_handle, &c, 1, &written, 0);
}

header_implementation ssize_t sys_read(i32 fd, void *buf, size_t count) {
  (void)fd;
  void *stdin_handle = GetStdHandle(SHARED_STD_INPUT_HANDLE);
  unsigned long bytes_read = 0;
  if (ReadFile(stdin_handle, buf, (unsigned long)count, &bytes_read, 0)) {
    return (ssize_t)bytes_read;
  }
  return -1;
}

typedef struct {
  u32 dwFileAttributes;
  u32 ftCreationTime[2];
  u32 ftLastAccessTime[2];
  u32 ftLastWriteTime[2];
  u32 nFileSizeHigh;
  u32 nFileSizeLow;
  u32 dwReserved0;
  u32 dwReserved1;
  char cFileName[260];
  char cAlternateFileName[14];
} Win32FindDataA;

#define FILE_ATTRIBUTE_READONLY 0x1
#define FILE_ATTRIBUTE_DIRECTORY 0x10

header_implementation b32 sys_dir_foreach(const char *path, DirEntryFn fn,
                                          void *user) {
  char pattern[512];
  u64 p = 0;
  while (path[p] != '\0' && p < sizeof(pattern) - 3) {
    pattern[p] = path[p];
    p++;
  }
  if (p > 0 && (pattern[p - 1] == '\\' || pattern[p - 1] == '/'))
    p--;
  pattern[p++] = '\\';
  pattern[p++] = '*';
  pattern[p] = '\0';

  Win32FindDataA data;
  void *handle = FindFirstFileA(pattern, &data);
  if (handle == (void *)(i64)-1)
    return false;

  do {
    const char *nm = data.cFileName;
    if (nm[0] == '.' && (nm[1] == '\0' || (nm[1] == '.' && nm[2] == '\0')))
      continue;

    DirEntry e = {0};
    u64 n = 0;
    while (nm[n] != '\0' && n < sizeof(e.name) - 1) {
      e.name[n] = nm[n];
      n++;
    }
    e.name[n] = '\0';

    e.is_dir = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    e.size = ((u64)data.nFileSizeHigh << 32) | (u64)data.nFileSizeLow;
    e.mode = e.is_dir ? (S_IFDIR | 0755u) : (S_IFREG | 0644u);
    if (data.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
      e.mode &= ~0222u;
    fn(&e, user);
  } while (FindNextFileA(handle, &data));

  FindClose(handle);
  return true;
}

#elif defined(__linux__) && (defined(__x86_64__) || defined(_M_X64))

header_implementation ssize_t sys_write(i32 fd, const void *buf, size_t count) {
  ssize_t ret;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"((i64)1), "D"((i64)fd), "S"(buf), "d"((i64)count)
                   : "rcx", "r11", "memory");
  return ret;
}

header_implementation void sys_print_char(char c) {
  sys_write(1, &c, 1);
}

header_implementation void sys_print_prompt(char c) {
  sys_write(2, &c, 1);
}

header_implementation ssize_t sys_read(i32 fd, void *buf, size_t count) {
  ssize_t ret;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"(0), "D"(fd), "S"(buf), "d"(count)
                   : "rcx", "r11", "memory");
  return ret;
}

#define O_RDONLY 0
#define O_DIRECTORY 0x10000
#define AT_SYMLINK_NOFOLLOW 0x100

typedef struct {
  u64 st_dev;
  u64 st_ino;
  u64 st_nlink;
  u32 st_mode;
  u32 st_uid;
  u32 st_gid;
  u32 __pad0;
  u64 st_rdev;
  i64 st_size;
  i64 st_blksize;
  i64 st_blocks;
  i64 st_atime;
  u64 st_atime_nsec;
  i64 st_mtime;
  u64 st_mtime_nsec;
  i64 st_ctime;
  u64 st_ctime_nsec;
  i64 __unused[3];
} SysStat;

typedef struct {
  u64 d_ino;
  i64 d_off;
  u16 d_reclen;
  u8 d_type;
  char d_name[];
} LinuxDirent;

header_implementation i32 sys_open(const char *path, i32 flags, i32 mode) {
  i64 ret;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"((i64)2), "D"(path), "S"((i64)flags), "d"((i64)mode)
                   : "rcx", "r11", "memory");
  return (i32)ret;
}

header_implementation i32 sys_close(i32 fd) {
  i64 ret;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"((i64)3), "D"((i64)fd)
                   : "rcx", "r11", "memory");
  return (i32)ret;
}

header_implementation ssize_t sys_getdents64(i32 fd, void *buf, size_t count) {
  ssize_t ret;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"((i64)217), "D"((i64)fd), "S"(buf), "d"(count)
                   : "rcx", "r11", "memory");
  return ret;
}

header_implementation i32 sys_fstatat(i32 dirfd, const char *path, SysStat *st,
                                      i32 flags) {
  i64 ret;
  register i64 r10 __asm__("r10") = (i64)flags;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"((i64)262), "D"((i64)dirfd), "S"(path), "d"(st),
                     "r"(r10)
                   : "rcx", "r11", "memory");
  return (i32)ret;
}

header_implementation b32 sys_dir_foreach(const char *path, DirEntryFn fn,
                                          void *user) {
  i32 fd = sys_open(path, O_RDONLY | O_DIRECTORY, 0);
  if (fd < 0)
    return false;

  char buf[4096];
  ssize_t nread;
  while ((nread = sys_getdents64(fd, buf, sizeof buf)) > 0) {
    i32 pos = 0;
    while (pos < nread) {
      LinuxDirent *d = (LinuxDirent *)(buf + pos);
      pos += (i32)d->d_reclen;

      const char *nm = d->d_name;
      if (nm[0] == '.' && (nm[1] == '\0' || (nm[1] == '.' && nm[2] == '\0')))
        continue;

      DirEntry e = {0};
      u64 n = 0;
      while (nm[n] != '\0' && n < sizeof(e.name) - 1) {
        e.name[n] = nm[n];
        n++;
      }
      e.name[n] = '\0';

      SysStat st = {0};
      if (sys_fstatat(fd, nm, &st, AT_SYMLINK_NOFOLLOW) == 0) {
        e.mode = st.st_mode;
        e.size = (u64)st.st_size;
        e.is_dir = ((st.st_mode & S_IFMT) == S_IFDIR);
      } else {
        e.mode = (d->d_type == DT_DIR) ? S_IFDIR : S_IFREG;
        e.is_dir = (d->d_type == DT_DIR);
      }
      fn(&e, user);
    }
  }
  sys_close(fd);
  return true;
}

/* --- Process launching ---------------------------------------------------- */

#define O_CLOEXEC 02000000
#define X_OK 1
#define AT_FDCWD (-100)

#define EXEC_CMD_MAX 1024
#define EXEC_PATH_MAX 1024
#define EXEC_MAX_ARGS 64

/* The environment of the current process, provided by the C runtime. */
extern char **environ;

header_implementation i64 sys_fork(void) {
  i64 ret;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"((i64)57)
                   : "rcx", "r11", "memory");
  return ret;
}

header_implementation i64 sys_execve(const char *path, char *const argv[],
                                     char *const envp[]) {
  i64 ret;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"((i64)59), "D"(path), "S"(argv), "d"(envp)
                   : "rcx", "r11", "memory");
  return ret;
}

header_implementation i64 sys_wait4(i32 pid, i32 *status, i32 options,
                                    void *rusage) {
  i64 ret;
  register i64 r10 __asm__("r10") = (i64)rusage;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"((i64)61), "D"((i64)pid), "S"(status), "d"((i64)options),
                     "r"(r10)
                   : "rcx", "r11", "memory");
  return ret;
}

header_implementation i32 sys_pipe2(i32 *fds, i32 flags) {
  i64 ret;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"((i64)293), "D"(fds), "S"((i64)flags)
                   : "rcx", "r11", "memory");
  return (i32)ret;
}

header_implementation i32 sys_access(const char *path, i32 mode) {
  i64 ret;
  __asm__ volatile("syscall"
                   : "=a"(ret)
                   : "a"((i64)21), "D"(path), "S"((i64)mode)
                   : "rcx", "r11", "memory");
  return (i32)ret;
}

header_implementation void sys_exit_group(i32 code) {
  __asm__ volatile("syscall"
                   :
                   : "a"((i64)231), "D"((i64)code)
                   : "rcx", "r11", "memory");
  __builtin_unreachable();
}

header_implementation void sys_write_str(i32 fd, const char *s) {
  sys_write(fd, s, base_cstr_length(s));
}

header_implementation const char *exec_errno_message(i32 err) {
  switch (err) {
  case 1:
    return "Operation not permitted";
  case 2:
    return "No such file or directory";
  case 8:
    return "Exec format error";
  case 12:
    return "Cannot allocate memory";
  case 13:
    return "Permission denied";
  case 14:
    return "Bad address";
  case 20:
    return "Not a directory";
  case 21:
    return "Is a directory";
  case 23:
    return "Too many open files in system";
  case 24:
    return "Too many open files";
  case 26:
    return "Text file busy";
  case 36:
    return "File name too long";
  case 40:
    return "Too many levels of symbolic links";
  default:
    return "Unknown error";
  }
}

header_implementation b32 exec_is_space(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' ||
         c == '\f';
}

/*
 * Splits a command line into an argv array. Single quotes, double quotes and
 * backslash escapes are honored. The tokens are written back into `scratch`
 * (compacted in place), so `argv` points into that buffer.
 */
header_implementation b32 exec_split_args(const char *cmd, char *scratch,
                                          usize scratch_size, char **argv,
                                          usize max_args, usize *out_count) {
  usize len = base_cstr_length(cmd);
  if (len + 1 > scratch_size)
    return false;

  for (usize i = 0; i <= len; i++)
    scratch[i] = cmd[i];

  usize read = 0;
  usize write = 0;
  usize count = 0;

  while (read < len) {
    while (read < len && exec_is_space(scratch[read]))
      read++;
    if (read >= len)
      break;
    if (count >= max_args)
      return false;

    char *token = scratch + write;
    usize token_len = 0;

    while (read < len && !exec_is_space(scratch[read])) {
      char c = scratch[read];

      if (c == '\'') {
        read++;
        while (read < len && scratch[read] != '\'')
          token[token_len++] = scratch[read++];
        if (read >= len)
          return false; // unterminated quote
        read++;
        continue;
      }

      if (c == '"') {
        read++;
        while (read < len && scratch[read] != '"') {
          if (scratch[read] == '\\' && read + 1 < len) {
            char escaped = scratch[read + 1];
            if (escaped == '"' || escaped == '\\' || escaped == '$' ||
                escaped == '`') {
              token[token_len++] = escaped;
            } else {
              token[token_len++] = '\\';
              token[token_len++] = escaped;
            }
            read += 2;
            continue;
          }
          token[token_len++] = scratch[read++];
        }
        if (read >= len)
          return false; // unterminated quote
        read++;
        continue;
      }

      if (c == '\\' && read + 1 < len) {
        read++;
        token[token_len++] = scratch[read++];
        continue;
      }

      token[token_len++] = c;
      read++;
    }

    /* Consume the separator before writing the terminator so that the
     * compacted output never overwrites input that is still unread. */
    if (read < len)
      read++;

    token[token_len] = '\0';
    write += token_len + 1;
    argv[count++] = token;
  }

  argv[count] = NULL;
  *out_count = count;
  return true;
}

header_implementation const char *exec_env_get(const char *name) {
  usize name_len = base_cstr_length(name);
  for (char **entry = environ; entry && *entry; entry++) {
    const char *e = *entry;
    usize i = 0;
    while (i < name_len && e[i] == name[i])
      i++;
    if (i == name_len && e[i] == '=')
      return e + name_len + 1;
  }
  return NULL;
}

header_implementation b32 exec_is_executable(const char *path) {
  SysStat st = {0};
  if (sys_fstatat(AT_FDCWD, path, &st, 0) != 0)
    return false;
  if ((st.st_mode & S_IFMT) != S_IFREG)
    return false;
  return sys_access(path, X_OK) == 0;
}

/*
 * Resolves a program name to a full path. Names containing a '/' are used
 * as-is, everything else is looked up in PATH.
 */
header_implementation b32 exec_resolve_program(const char *program, char *out,
                                               usize out_size) {
  usize prog_len = base_cstr_length(program);
  if (prog_len == 0)
    return false;

  for (usize i = 0; i < prog_len; i++) {
    if (program[i] == '/') {
      if (prog_len + 1 > out_size)
        return false;
      for (usize j = 0; j <= prog_len; j++)
        out[j] = program[j];
      return true;
    }
  }

  const char *path = exec_env_get("PATH");
  if (!path)
    path = "/usr/local/bin:/usr/bin:/bin";

  const char *cursor = path;
  while (*cursor != '\0') {
    const char *sep = cursor;
    while (*sep != '\0' && *sep != ':')
      sep++;
    usize dir_len = (usize)(sep - cursor);

    if (dir_len + 1 + prog_len + 1 > out_size)
      return false;

    /* An empty path component means the current directory. */
    usize w = 0;
    if (dir_len == 0) {
      out[w++] = '.';
    } else {
      for (usize i = 0; i < dir_len; i++)
        out[w++] = cursor[i];
    }
    out[w++] = '/';
    for (usize i = 0; i < prog_len; i++)
      out[w++] = program[i];
    out[w] = '\0';

    if (exec_is_executable(out))
      return true;

    if (*sep == '\0')
      break;
    cursor = sep + 1;
  }

  return false;
}

/*
 * Runs a command line in a child process and waits for it to finish.
 * Returns false if the program could not be launched.
 */
header_implementation b32 sys_exec_command(const char *cmd) {
  if (cmd == NULL || cmd[0] == '\0')
    return false;

  char scratch[EXEC_CMD_MAX];
  char *argv[EXEC_MAX_ARGS + 1];
  usize argc = 0;

  if (!exec_split_args(cmd, scratch, sizeof scratch, argv, EXEC_MAX_ARGS,
                        &argc)) {
    sys_write_str(2, "exec: invalid command line\n");
    return false;
  }
  if (argc == 0)
    return false;

  char program[EXEC_PATH_MAX];
  if (!exec_resolve_program(argv[0], program, sizeof program)) {
    sys_write_str(2, "exec: '");
    sys_write_str(2, cmd);
    sys_write_str(2, "': command not found\n");
    return false;
  }

  /*
   * A close-on-exec pipe reports a failed exec back to the parent: the pipe
   * is closed automatically when the program starts, so anything readable
   * afterwards is the errno of a failed exec.
   */
  i32 pipe_fds[2];
  if (sys_pipe2(pipe_fds, O_CLOEXEC) != 0) {
    sys_write_str(2, "exec: could not create pipe\n");
    return false;
  }

  i64 pid = sys_fork();
  if (pid < 0) {
    sys_close(pipe_fds[0]);
    sys_close(pipe_fds[1]);
    sys_write_str(2, "exec: fork failed\n");
    return false;
  }

  if (pid == 0) {
    /* Child: replace this process image with the requested program. */
    sys_close(pipe_fds[0]);
    i64 result = sys_execve(program, argv, (char *const *)environ);
    i32 err = (i32)(-result);
    sys_write(pipe_fds[1], &err, sizeof err);
    sys_exit_group(127);
  }

  /* Parent: collect the exec result, then wait for the child to finish. */
  sys_close(pipe_fds[1]);
  i32 err = 0;
  ssize_t n = sys_read(pipe_fds[0], &err, sizeof err);
  sys_close(pipe_fds[0]);

  b32 success = true;
  if (n == (ssize_t)sizeof err) {
    success = false;
    sys_write_str(2, "exec: '");
    sys_write_str(2, cmd);
    sys_write_str(2, "': ");
    sys_write_str(2, exec_errno_message(err));
    sys_write_str(2, "\n");
  }

  i32 status = 0;
  sys_wait4((i32)pid, &status, 0, NULL);
  return success;
}

#else
#error "UNSUPPORTED ARCH"
#endif

#if defined(_MSC_VER)
#include <stdarg.h>
#else
typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)
#endif

header_implementation void print_int(i64 value, i32 base) {
  if (value == 0) {
    sys_print_char('0');
    return;
  }

  char buffer[32];
  i32 i = 0;
  u64 u_value = (u64)value;

  if (base == 10 && value < 0) {
    sys_print_char('-');
    u_value = (u64)(-value);
  }

  while (u_value > 0) {
    char digit = (char)(u_value % (u64)base);
    buffer[i++] = (digit < 10) ? ('0' + digit) : ('a' + (digit - 10));
    u_value /= (u64)base;
  }

  while (i > 0) {
    sys_print_char(buffer[--i]);
  }
}

header_implementation void base_cstr_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  for (const char *p = format; *p != '\0'; p++) {
    if (*p != '%') {
      sys_print_char(*p);
      continue;
    }

    p++;
    switch (*p) {
    case 'c': {
      char c = (char)va_arg(args, i32);
      sys_print_char(c);
      break;
    }
    case 's': {
      char *s = va_arg(args, char *);
      while (*s)
        sys_print_char(*s++);
      break;
    }
    case 'd': {
      i32 d = va_arg(args, i32);
      print_int(d, 10);
      break;
    }
    case 'x': {
      u32 x = va_arg(args, u32);
      print_int(x, 16);
      break;
    }
    case '%': {
      sys_print_char('%');
      break;
    }
    default:
      sys_print_char('%');
      sys_print_char(*p);
      break;
    }
  }
  va_end(args);
}

header_implementation u64 base_cstr_length(const char *cstr) {
  u64 i = 0;
  while (cstr[i] != '\0')
    ++i;
  return i;
}

static char s_pushback_char = 0;
static i32 s_has_pushback = 0;

header_implementation i32 base_get_char(void) {
  if (s_has_pushback) {
    s_has_pushback = 0;
    return (u8)s_pushback_char;
  }
  char c;
  ssize_t n = sys_read(0, &c, 1);
  if (n <= 0)
    return -1;
  return (u8)c;
}

header_implementation void base_unget_char(char c) {
  s_pushback_char = c;
  s_has_pushback = 1;
}

header_implementation void base_skip_whitespace(void) {
  i32 c;
  while ((c = base_get_char()) != -1) {
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '\v' &&
        c != '\f') {
      base_unget_char((char)c);
      break;
    }
  }
}

header_implementation i32 base_cstr_scanf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  i32 assigned_count = 0;

  for (const char *p = format; *p != '\0'; p++) {
    if (*p != '%') {
      if (*p == ' ' || *p == '\t' || *p == '\n') {
        base_skip_whitespace();
      } else {
        i32 c = base_get_char();
        if (c == -1 || c != *p) {
          if (c != -1)
            base_unget_char((char)c);
          break;
        }
      }
      continue;
    }

    p++;
    if (*p == '\0')
      break;

    switch (*p) {
    case 'c': {
      char *dest = va_arg(args, char *);
      i32 c = base_get_char();
      if (c == -1) {
        va_end(args);
        return assigned_count;
      }
      *dest = (char)c;
      assigned_count++;
      break;
    }
    case 's': {
      char *dest = va_arg(args, char *);
      base_skip_whitespace();
      i32 c;
      i32 written = 0;
      while ((c = base_get_char()) != -1) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
          base_unget_char((char)c);
          break;
        }
        dest[written++] = (char)c;
      }
      dest[written] = '\0';
      if (written > 0) {
        assigned_count++;
      } else {
        va_end(args);
        return assigned_count;
      }
      break;
    }
    case 'd': {
      i32 *dest = va_arg(args, i32 *);
      base_skip_whitespace();
      i32 c = base_get_char();
      if (c == -1) {
        va_end(args);
        return assigned_count;
      }

      i32 sign = 1;
      i64 val = 0;
      b32 has_digits = false;

      if (c == '-') {
        sign = -1;
        c = base_get_char();
      } else if (c == '+') {
        c = base_get_char();
      }

      while (c != -1 && c >= '0' && c <= '9') {
        val = val * 10 + (c - '0');
        has_digits = true;
        c = base_get_char();
      }

      if (c != -1) {
        base_unget_char((char)c);
      }

      if (has_digits) {
        *dest = (i32)(sign * val);
        assigned_count++;
      } else {
        va_end(args);
        return assigned_count;
      }
      break;
    }
    case 'x': {
      u32 *dest = va_arg(args, u32 *);
      base_skip_whitespace();
      i32 c = base_get_char();
      if (c == -1) {
        va_end(args);
        return assigned_count;
      }

      if (c == '0') {
        i32 next_c = base_get_char();
        if (next_c == 'x' || next_c == 'X') {
        } else {
          if (next_c != -1)
            base_unget_char((char)next_c);
          base_unget_char('0');
        }
      } else {
        base_unget_char((char)c);
      }

      u64 val = 0;
      b32 has_digits = false;

      while ((c = base_get_char()) != -1) {
        i32 digit = -1;
        if (c >= '0' && c <= '9')
          digit = c - '0';
        else if (c >= 'a' && c <= 'f')
          digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
          digit = c - 'A' + 10;

        if (digit == -1) {
          base_unget_char((char)c);
          break;
        }
        val = val * 16 + digit;
        has_digits = true;
      }

      if (has_digits) {
        *dest = (u32)val;
        assigned_count++;
      } else {
        va_end(args);
        return assigned_count;
      }
      break;
    }
    case '%': {
      i32 c = base_get_char();
      if (c != '%') {
        if (c != -1)
          base_unget_char((char)c);
      }
      break;
    }
    default: {
      i32 c = base_get_char();
      if (c != -1)
        base_unget_char((char)c);
      break;
    }
    }
  }

  va_end(args);
  return assigned_count;
}

header_implementation i32 base_cstr_compare(const char *a, const char *b) {
  while (*a && (*a == *b)) {
    a++;
    b++;
  }
  return *(const u8 *)a - *(const u8 *)b;
}

header_implementation void base_cstr_prompt(const char *s) {
  while (*s) {
    sys_print_prompt(*s++);
  }
}

header_implementation u64 base_cstr_readline(char *buffer, u64 max_size) {
  u64 i = 0;
  while (i < max_size - 1) {
    i32 c = base_get_char();
    if (c == -1) {
      buffer[i] = '\0';
      /* End of input: report it when nothing was read at all. */
      return (i == 0) ? (u64)-1 : i;
    }
    if (c == '\n') {
      break;
    }
    if (c == '\r') {
      continue;
    }
    buffer[i++] = (char)c;
  }
  buffer[i] = '\0';
  return i;
}

typedef struct {
  u8 *data;
  u64 size;
} String8;

#define STRING8(s) ((s), (base_cstr_length(s)))

#define ASSERT(c)                                                              \
  if ((c) == false) {                                                          \
    base_cstr_printf("Assertion Failed: %s : line %d\n", __FILE__, __LINE__);  \
    DEBUG_BREAK();                                                             \
  }

#endif
