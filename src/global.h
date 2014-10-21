/* 
 * File:   global.h
 * Author: khoai
 *
 * Created on August 15, 2014, 4:01 PM
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

struct global {
	int uid;
	int gid;
	int nbproc;
	int maxconn;
	int maxpipes;		/* max # of pipes */
	int maxsock;		/* max # of sockets */
	int rlimit_nofile;	/* default ulimit-n value : 0=unset */
	int rlimit_memmax;	/* default ulimit-d in megs value : 0=unset */
	int mode;
	int last_checks;
	int spread_checks;
	char *chroot;
	char *pidfile;
	char *node, *desc;		/* node name & description */
	char *log_tag;                  /* name for syslog */
	int logfac1, logfac2;
	int loglev1, loglev2;
	int minlvl1, minlvl2;
//	struct logsrv logsrv1, logsrv2;
	char *log_send_hostname;   /* set hostname in syslog header */
	struct {
		int maxpollevents; /* max number of poll events at once */
		int maxaccept;     /* max number of consecutive accept() */
		int options;       /* various tuning options */
		int recv_enough;   /* how many input bytes at once are "enough" */
		int bufsize;       /* buffer size in bytes, defaults to BUFSIZE */
		int maxrewrite;    /* buffer max rewrite size in bytes, defaults to MAXREWRITE */
		int client_sndbuf; /* set client sndbuf to this value if not null */
		int client_rcvbuf; /* set client rcvbuf to this value if not null */
		int server_sndbuf; /* set server sndbuf to this value if not null */
		int server_rcvbuf; /* set server rcvbuf to this value if not null */
		int chksize;       /* check buffer size in bytes, defaults to BUFSIZE */
	} tune;
//	struct listener stats_sock; /* unix socket listener for statistics */
//	struct proxy *stats_fe;     /* the frontend holding the stats settings */
};

extern struct global global;

#endif	/* GLOBAL_H */

