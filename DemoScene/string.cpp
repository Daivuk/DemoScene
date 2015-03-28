int string_len(const char* str)
{
    int ret = 0;
    while (*str)
    {
        str++;
        ++ret;
    }
    return ret;
}
