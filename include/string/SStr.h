#ifndef SSTR_H_INCLUDED
#define SSTR_H_INCLUDED
#include <stdlib.h>
#include <string.h>

#include <common/Common.h>

/*

      _        _             
     | |      (_)            
  ___| |_ _ __ _ _ __   __ _ 
 / __| __| '__| | '_ \ / _` |
 \__ \ |_| |  | | | | | (_| |
 |___/\__|_|  |_|_| |_|\__, |
                        __/ |
                       |___/ 
 
 * String functions to make manipulating strings 
 * just, like, super duper easy.
 * ☺
*/

typedef char* string;
typedef const char * const cstring;

// *** Simple string structure functions
string new_sstr(char *str);
string cpy_sstr(string sstr);
string concat_sstr(string dest, string src);
string concat_c_str(string dest, char* src);
int r_cmp_sstr(string c0, string c1, size_t i);
bool find_in_sstr(string s, char t);
size_t length_sstr(string s);
string append_sstr(string s, char c);
string reverse_sstr(string sstr);
string sub_sstr(string sstr, size_t start, size_t len);
string destroy_sstr(string s);
void clip_end_sstr(string s, unsigned ct);

#define cmp_sstr(c0, c1) r_cmp_sstr((c0), (c1), 0)
#endif
