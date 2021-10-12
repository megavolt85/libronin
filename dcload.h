#ifndef DCLOAD_H
#define DCLOAD_H

#define DCLOAD_MAGIC_ADDRESS		(0x8c004004)
#define DCLOAD_SYSCALL_ADDRESS		(0x8c004008)
#define DCLOAD_MAGIC_SYSCALL_OFS	(DCLOAD_SYSCALL_ADDRESS - DCLOAD_MAGIC_ADDRESS)

#define DCLOAD_MAGIC_VALUE		(0xdeadbeef)

#define	DCLOAD_READ		(0)
#define	DCLOAD_WRITE		(1)
#define	DCLOAD_OPEN		(2)
#define	DCLOAD_CLOSE		(3)
#define	DCLOAD_CREAT		(4)
#define	DCLOAD_LINK		(5)
#define	DCLOAD_UNLINK		(6)
#define	DCLOAD_CHDIR		(7)
#define	DCLOAD_CHMOD		(8)
#define	DCLOAD_LSEEK		(9)
#define	DCLOAD_FSTAT		(10)
#define	DCLOAD_TIME		(11)
#define	DCLOAD_STAT		(12)
#define	DCLOAD_UTIME		(13)
//#define	DCLOAD_14	(14) /* Invalid syscall */
#define	DCLOAD_EXIT		(15)
#define	DCLOAD_OPENDIR		(16)
#define	DCLOAD_CLOSEDIR		(17)
#define	DCLOAD_READDIR		(18)
#define	DCLOAD_GETHOSTINFO	(19)
#define	DCLOAD_GDBPACKET	(20)

#ifndef ASSEMBLER
extern void dclWriteStdout(const char *str, int len);
#define dclPutStrConst(a)	dclWriteStdout(a, sizeof(a)-1)
#define dclPutStr(a)		dclWriteStdout(a, strlen(a))
#endif

#endif
