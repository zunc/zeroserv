/* 
 * File:   strutil.h
 * Author: khoai
 *
 * Created on May 20, 2015, 5:00 PM
 */

#ifndef STRUTIL_H
#define	STRUTIL_H

#define COMPARE(data, str) !strncmp(data, str, strlen(str))

static int ishex(int x) {
    return (x >= '0' && x <= '9') ||
        (x >= 'a' && x <= 'f') ||
        (x >= 'A' && x <= 'F');
}

static int uri_decode(const char *s, char *dec) {
    char *o;
    const char *end = s + strlen(s);
    int c;

    for (o = dec; s <= end; o++) {
        c = *s++;
        if (c == '+') c = ' ';
        else if (c == '%' && (!ishex(*s++) ||
            !ishex(*s++) ||
            !sscanf(s - 2, "%2x", &c)))
            return -1;

        if (dec) *o = c;
    }

    return o - dec;
}

#endif	/* STRUTIL_H */

