/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 */

#pragma once

#define Logger Logger_C

void LoggerInit();
extern "C" void Logger(const char *fmt...);
extern "C" void Logger_C(const char *fmt, ...);
extern "C" void Logger_chars(int n, const char *data);

// EOF