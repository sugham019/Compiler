#pragma once

void sys_write(int fileDescriptor, char* bufferAddress, int size);
void sys_read(int fileDescriptor, char* bufferAddress, int size);