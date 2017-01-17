#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define MAX_RETRY_COUNT 128
#define CHECKPOINT printf("CHECKPOINT\n"); fgets(buf, sizeof(buf), stdin);

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

int cmpI(const void *a, const void *b) {
    RECORD **c = (RECORD**)a;
    RECORD **d = (RECORD**)b;

    return strcmp((*c)->mac, (*d)->mac);
}

int main (int argc, char** argv) {
    char buf[32];
    RECORD *r, **idxMac, *key, **resB;
    char *mac, *rssi, *p;
    size_t macLen, rssiLen;
    FILE *f;
    unsigned int n = 0, i, nr, c;
    int timeout;

    if(argc != 6) return 1;
    if(sscanf(argv[4], "%u", &nr) != 1) return 1;
    if(sscanf(argv[5], "%d", &timeout) != 1) return 1;

    c = 0;
    r = (RECORD*)malloc(sizeof(*r) * nr);
    idxMac = (RECORD**)malloc(sizeof(*idxMac) * nr);
    while(fgets(buf, sizeof(buf), stdin)) {
        if((p = strchr(buf, '\n'))) *p = '\0';
        if((p = strchr(buf, '\r'))) *p = '\0';

        macLen = strcspn(buf, "\t");
        rssiLen = strlen(buf + macLen + 1);

        mac = (char*)malloc(macLen + 1);
        memcpy(mac, buf, macLen); mac[macLen] = '\0';
        rssi = (char*)malloc(rssiLen + 1);
        memcpy(rssi, buf + macLen + 1, rssiLen); rssi[rssiLen] = '\0';

        for(i = 0; i < n; i += 1) idxMac[i] = r + i;
        qsort(idxMac, n, sizeof(idxMac[0]), cmpI);
        key = malloc(sizeof(*key));
        key->mac = mac;
        
        resB = (RECORD**)bsearch(&key, idxMac, n, sizeof(idxMac[0]), cmpI);
        free(key);

        if(resB) {
            free((*resB)->rssi);
            (*resB)->rssi = rssi;
            free(mac);

            (*resB)->timestamp = time(0);
        }
        else {
            i = n;
            if(i >= nr) {
                free(r[nr - 1].mac);
                free(r[nr - 1].rssi);
                i = nr - 1;
            }
            r[i].mac = mac;
            r[i].rssi = rssi;
            r[i].timestamp = time(0);
            n = i + 1;
        }
        
        qsort(r, n, sizeof(r[0]), cmpR);
        for(i = n - 1; i < n; i -= 1) {
            if((time(0) - r[i].timestamp) > timeout) {
                free(r[i].mac);
                free(r[i].rssi);
                n -= 1;
            }
            else break;
        }
        
        f = fopen(argv[2], "w");
        if(!f) return -2;
        for(i = 0; i < n; i += 1)
            fprintf(f, "%s\t%s\t%u\n", r[i].mac, r[i].rssi, (unsigned int)(r[i].timestamp));
        fclose(f);

        remove(argv[3]);
        if(rename(argv[1], argv[3])) { c += 1; if(c >= MAX_RETRY_COUNT) return -2; }
        else if(c) c = 0;

        if(rename(argv[2], argv[1])) return -2;
        remove(argv[3]);
    }
    return 0;
}