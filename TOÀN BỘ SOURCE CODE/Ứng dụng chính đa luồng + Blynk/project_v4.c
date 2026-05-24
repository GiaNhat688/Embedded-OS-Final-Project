#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define BLYNK_TOKEN "2tNHijXeZqOhXD7LAiUM2DXVJPNUG18Y"

int temp = 0, humi = 0, gas = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_dht11(void *arg) {
    while(1) {
        int fd = open("/dev/dht11", O_RDONLY);
        if (fd >= 0) {
            unsigned char data[5] = {0};
            if (read(fd, data, 5) == 5) {
                if (data[0] > 0 && data[2] > 0 && data[0] <= 100 && data[2] <= 50) {
                    pthread_mutex_lock(&mutex);
                    humi = data[0]; temp = data[2];
                    pthread_mutex_unlock(&mutex);
                }
            }
            close(fd);
        }
        sleep(2);
    }
    return NULL;
}

void *thread_mq2(void *arg) {
    char buf[16];
    while(1) {
        int fd = open("/dev/mq2", O_RDONLY);
        if (fd >= 0) {
            memset(buf, 0, 16);
            if (read(fd, buf, 15) > 0) {
                pthread_mutex_lock(&mutex);
                gas = atoi(buf);
                pthread_mutex_unlock(&mutex);
            }
            close(fd);
        }
        sleep(1);
    }
    return NULL;
}

void *thread_lcd(void *arg) {
    while(1) {
        int fd = open("/dev/lcd", O_WRONLY);
        if (fd >= 0) {
            char msg[32];
            pthread_mutex_lock(&mutex);
            snprintf(msg, sizeof(msg), "T:%dC H:%d%%\nGas:%d", temp, humi, gas);
            pthread_mutex_unlock(&mutex);
            write(fd, msg, strlen(msg));
            close(fd);
        }
        sleep(1);
    }
    return NULL;
}

void *thread_buzzer(void *arg) {
    while(1) {
        int fd = open("/dev/buzzer", O_WRONLY);
        if (fd >= 0) {
            pthread_mutex_lock(&mutex);
            int g = gas;
            pthread_mutex_unlock(&mutex);
            if (g > 4000) write(fd, "0", 1);
            else write(fd, "1", 1);
            close(fd);
        }
        sleep(1);
    }
    return NULL;
}
void *thread_print(void *arg) {
    while(1) {
        pthread_mutex_lock(&mutex);
        printf("Running: T:%d H:%d Gas:%d\n", temp, humi, gas);
        pthread_mutex_unlock(&mutex);
        sleep(2);
    }
    return NULL;
}

void *thread_blynk(void *arg) {
    char cmd[512];
    while(1) {
        pthread_mutex_lock(&mutex);
        int t = temp, h = humi, g = gas;
        pthread_mutex_unlock(&mutex);

        snprintf(cmd, sizeof(cmd),
            "curl -s \"https://blynk.cloud/external/api/batch/update"
            "?token=" BLYNK_TOKEN
            "&V0=%d&V1=%d&V2=%d\"", t, h, g);
        system(cmd);
        printf("Sent to Blynk: T:%d H:%d Gas:%d\n", t, h, g);

        if (g > 4000) {
            snprintf(cmd, sizeof(cmd),
                "curl -s \"https://blynk.cloud/external/api/logEvent"
                "?token=" BLYNK_TOKEN "&code=fire_alert\"");
            system(cmd);
            printf("FIRE ALERT sent to Blynk!\n");
        }
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2, t3, t4, t5, t6;
    pthread_create(&t1, NULL, thread_dht11, NULL);
    pthread_create(&t2, NULL, thread_mq2, NULL);
    pthread_create(&t3, NULL, thread_lcd, NULL);
    pthread_create(&t4, NULL, thread_buzzer, NULL);
    pthread_create(&t5, NULL, thread_print, NULL);
    pthread_create(&t6, NULL, thread_blynk, NULL);
    pthread_join(t1, NULL); pthread_join(t2, NULL);
    pthread_join(t3, NULL); pthread_join(t4, NULL);
    pthread_join(t5, NULL); pthread_join(t6, NULL);
    return 0;
}
