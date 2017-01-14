#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define MAX_RETRY_COUNT 128

typedef struct {
    char *mac;
    char *rssi;
    time_t timestamp;
} RECORD;

int cmpR(const void *a, const void *b) {
    RECORD *c = (RECORD*)a;
    RECORD *d = (RECORD*)b;

    if(c->timestamp < d->timestamp) return 1;
    else if(c->timestamp > d->timestamp) return -1;
    else return 0;
}

int main (int argc, char** argv) {
    RECORD r[32];
    char buf[32];
    char *mac, *rssi, *p;
    size_t macLen, rssiLen;
    FILE *f;
    unsigned int n = 0, i, c;
    int res;

    if(argc != 4) return 1;
    while(fgets(buf, sizeof(buf), stdin)) {
        if((p = strchr(buf, '\n'))) *p = '\0';
        if((p = strchr(buf, '\r'))) *p = '\0';

        macLen = strcspn(buf, "\t");
        rssiLen = strlen(buf + macLen + 1);
        i = n;
        if(i >= 32) {
            free(r[31].mac);
            free(r[31].rssi);
            i = 31;
        }
        mac = (char*)malloc(macLen + 1);
        memcpy(mac, buf, macLen); mac[macLen] = '\0';
        rssi = (char*)malloc(rssiLen + 1);
        memcpy(rssi, buf + macLen + 1, rssiLen); rssi[rssiLen] = '\0';
        r[i].mac = mac;
        r[i].rssi = rssi;
        r[i].timestamp = time(0);
        n = i + 1;
        qsort(r, n, sizeof(r[0]), cmpR);

        f = fopen(argv[2], "w");
        if(!f) return -2;
        for(i = 0; i < n; i += 1)
            fprintf(f, "%s\t%s\t%u\n", r[i].mac, r[i].rssi, (unsigned int)(r[i].timestamp));
        fclose(f);

        c = 0; res = ~0;
        while(res) { if(c >= MAX_RETRY_COUNT) return -2; res = rename(argv[1], argv[3]); c += 1; if(errno == ENOENT && res) break; }

        if(rename(argv[2], argv[1])) return -2;
        remove(argv[3]);
    }
    return 0;
}