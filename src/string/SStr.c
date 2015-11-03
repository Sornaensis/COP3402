#include <stdio.h>

#include <common/Common.h>
#include <string/SStr.h>

string new_sstr(char *str)
{
    size_t len = strlen(str);
    unsigned char* n_string = calloc(len+9, sizeof(char));
    unsigned char* offset = n_string + 8;

    ++len;
    n_string[0] = len >> 24;
    n_string[1] = (len << 8) >> 24;
    n_string[2] = (len << 16) >> 24;
    n_string[3] = (len << 24) >> 24;

    --len;
    n_string[4] = len >> 24;
    n_string[5] = (len << 8) >> 24;
    n_string[6] = (len << 16) >> 24;
    n_string[7] = (len << 24) >> 24;

    memcpy(offset, str, len);

    /* unsigned i; */
    /* for(i=0;i<len;++i) */
    /* { */
    /*     offset = append_sstr(offset, str[i]); */
    /* } */

    return (string)offset;
}

string cpy_sstr(string sstr)
{
    size_t len = length_sstr(sstr);
    string new = calloc(9+len, sizeof(char));

    memcpy(new, sstr-8, 8+len);

    unsigned char* s = (unsigned char*)&new[0];

    ++len;
    s[0] = len >> 24;
    s[1] = (len << 8) >> 24;
    s[2] = (len << 16) >> 24;
    s[3] = (len << 24) >> 24;

    return (new + 8);
}

string concat_c_str(string dest, char* src)
{
    if(src == NULL) return dest;
    string nsrc = new_sstr(src);
    if(dest == NULL) return nsrc;

    if(length_sstr(nsrc) == 0)
    {
        destroy_sstr(nsrc);
        return dest;
    }
    
    dest = concat_sstr(dest, nsrc);
    destroy_sstr(nsrc);

    return dest;
}

string concat_sstr(string dest, string src)
{
    if(src == NULL) return dest;
    if(dest == NULL) return src;

    if(length_sstr(src) == 0) return dest;

    unsigned i, srclen = length_sstr(src);
    for(i = 0;i<srclen;++i)
    {
        dest = append_sstr(dest, src[i]);
    }

    return dest;
}

int r_cmp_sstr(string c0, string c1, size_t i)
{
    if(i>=length_sstr(c0)+1)
        return -1;

    if(c0[i] != c1[i])
        return i;

    return r_cmp_sstr(c0, c1, i+1);
}

string append_sstr(string t, char c)
{
    if(t == NULL) return NULL;

    unsigned char* s = (unsigned char*)&t[-8];

    size_t alloc = 0,
           len = 0;

    alloc += s[0] << 24;
    alloc += s[1] << 16;
    alloc += s[2] << 8;
    alloc += s[3];

    len += s[4] << 24;
    len += s[5] << 16;
    len += s[6] << 8;
    len += s[7];

    ++len;

    if(len == alloc)
    {
        alloc = len + 1000;

        s = (unsigned char*) realloc(s, sizeof(unsigned char)*(alloc));

        s[0] = alloc >> 24;
        s[1] = (alloc << 8) >> 24;
        s[2] = (alloc << 16) >> 24;
        s[3] = (alloc << 24) >> 24;

    }


    if(s == NULL) return NULL;

    s[len+8] = '\0';
    s[len+7] = c;

    s[4] = len >> 24;
    s[5] = (len << 8) >> 24;
    s[6] = (len << 16) >> 24;
    s[7] = (len << 24) >> 24;

    return (string)(s + 8);
}

size_t length_sstr(string s)
{
    if(s == NULL) return 0;

    unsigned char* t = (unsigned char*)&s[-4];

    size_t len = 0;

    len += t[0] << 24;
    len += t[1] << 16;
    len += t[2] << 8;
    len += t[3];

    return len;
}

string destroy_sstr(string s)
{
    if(s == NULL)
        return NULL;

    free(s - 8);
    return NULL;
}

string reverse_sstr(string sstr)
{
    size_t i = 0;
    signed long long len = length_sstr(sstr)-1;

    if(len <= 0)
        return sstr;

    char buf[len+2];
    memset(buf, 0, len+2);

    for(;i<=len;++i)
    {
        buf[i] = sstr[(len-i)];
    }

    memcpy(sstr, buf, sizeof(char)*(len+1));

    return sstr;
}

string sub_sstr(string sstr, size_t start, size_t len)
{
    string new = new_sstr("");
    unsigned j = 0, i = start;
    for(; j<len; ++i, ++j)
    {
        new = append_sstr(new, sstr[i]);
    }

    return new;
}

bool find_in_sstr(string s, char t)
{
    size_t len = length_sstr(s);
    unsigned long i = 0;
    for(;i<len;++i)
    {
        if(t == s[i])
            return true;
    }

    return false;
}


void clip_end_sstr(string s, unsigned ct)
{
    size_t len = length_sstr(s);

    for(;len > 0 && ct > 0;--ct)
    {
        s[len-1] = '\0';

        --len;

        s[-4] = len >> 24;
        s[-3] = (len << 8) >> 24;
        s[-2] = (len << 16) >> 24;
        s[-1] = (len << 24) >> 24;
    }

}

