#ifndef CAPTURE_H
#define CAPTURE_H


int capture_open_device(const char *device);
int capture_close();
void* capture(void *arg);

#endif // CAPTURE_H
