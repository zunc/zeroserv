/* 
 * File:   signal.h
 * Author: khoai
 *
 * Created on August 19, 2014, 5:31 PM
 */

#ifndef TYPE_SIGNAL_H
#define	TYPE_SIGNAL_H

struct signal_descriptor {
	int count; /* number of times raised */
	void (*handler)(int sig);
};

#endif	/* TYPE_SIGNAL_H */

