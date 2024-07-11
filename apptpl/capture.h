#ifndef CAPTURE_H
#define CAPTURE_H


extern int capture_shutdown;

int capture_open_device(const char *device);
int capture_close();
void* capture(void *arg);
int list_devices();

#endif // CAPTURE_H
