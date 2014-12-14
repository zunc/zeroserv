/* 
 * File:   util.h
 * Author: khoai
 *
 * Created on April 3, 2014, 2:41 PM
 */

#ifndef UTIL_H
#define	UTIL_H

uint32_t jenkins_hash(uint8_t *key, size_t len);
int make_socket_non_blocking(int fd);
unsigned int crc32(unsigned int crc, const char *buf, size_t len);

static unsigned short tcp_checksum(const char *buf, unsigned size) {
    // ref: http://locklessinc.com/articles/tcp_checksum/
    if (!buf)
        return 0;

    unsigned long long sum = 0;
    const unsigned long long *b = (unsigned long long *) buf;

    unsigned t1, t2;
    unsigned short t3, t4;

    /* Main loop - 8 bytes at a time */
    while (size >= sizeof (unsigned long long)) {
        unsigned long long s = *b++;
        sum += s;
        if (sum < s) sum++;
        size -= 8;
    }

    /* Handle tail less than 8-bytes long */
    buf = (const char *) b;
    if (size & 4) {
        unsigned s = *(unsigned *) buf;
        sum += s;
        if (sum < s) sum++;
        buf += 4;
    }

    if (size & 2) {
        unsigned short s = *(unsigned short *) buf;
        sum += s;
        if (sum < s) sum++;
        buf += 2;
    }

    if (size) {
        unsigned char s = *(unsigned char *) buf;
        sum += s;
        if (sum < s) sum++;
    }

    /* Fold down to 16 bits */
    t1 = sum;
    t2 = sum >> 32;
    t1 += t2;
    if (t1 < t2) t1++;
    t3 = t1;
    t4 = t1 >> 16;
    t3 += t4;
    if (t3 < t4) t3++;

    return ~t3;
}

#endif	/* UTIL_H */

