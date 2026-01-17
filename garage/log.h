
#ifdef TRACE
void log_printf(const char *fmt, ...);
#define LOG(...) log_printf(__VA_ARGS__)
#else
#define LOG(...) do {} while (0)
#endif
