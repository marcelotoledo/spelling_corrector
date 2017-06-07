/* 
 * spell.c --- spell corrector
 * 
 * Copyright  (C)  2007  Marcelo Toledo <marcelo@marcelotoledo.com>
 * 
 * Version: 1.0
 * Keywords: spell corrector
 * Author: Marcelo Toledo <marcelo@marcelotoledo.com>
 * Maintainer: Marcelo Toledo <marcelo@marcelotoledo.com>
 * URL: http://marcelotoledo.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Commentary: 
 * 
 * See http://www.marcelotoledo.com.
 * 
 * Code:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <search.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DICTIONARY "./big.txt"
#define DICT_SZ    3000000

const char delim[]    = ".,:;`/\"+-_(){}[]<>*&^%$#@!?~/|\\=1234567890 \t\n";
const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

static char *strtolower(char *word)
{
        char *s;
        
        for (s = word; *s; s++)
                *s = tolower(*s);
        
        return word;
}

static ENTRY *find(char *word)
{
        ENTRY e;

        e.key = word;
        return hsearch(e, FIND);
}

static int update(char *word)
{
        ENTRY *e = find(word);
        
        if (!e)
                return 0;
        
        e->data++;
        
        return 1;
}

static int read_file(ENTRY dict)
{
        char *file, *word, *w;
        FILE *fp = fopen(DICTIONARY, "r");
        struct stat sb;
        
        if (!fp)
                return 0;
        
        if (stat(DICTIONARY, &sb))
                return 0;
        
        file = malloc(sb.st_size);
        if (!file) {
                fclose(fp);
                return 0;
        }

        fread(file, sizeof(char), sb.st_size, fp);

        word = strtok(file, delim);
        while(word != NULL) {
                w = strtolower(strdup(word));
                
                if (!update(w)) {
                        dict.key  = w;
                        dict.data = 0;
                        hsearch(dict, ENTER);
                }
                
                word = strtok(NULL, delim);
        }
        
        free(file);
        fclose(fp);

        return 1;
}

static char *substr(char *str, int offset, int limit)
{
        char *new_str;
        int str_size = strlen(str);
        
        if ((limit > str_size) || ((offset + limit) > str_size) || 
            (str_size < 1) || (limit == 0))
                return NULL;
        
        new_str = malloc(limit+1 * sizeof(char));
        if (!new_str)
                return NULL;
        
        strncpy(new_str, str+offset, limit);
        *(new_str + limit) = '\0';
        
        return new_str;
}

static char *concat(char *str1, char *str2)
{
        if (!str1) { 
                str1 = malloc(sizeof(char));
                *str1 = '\0';
        }
        
        if (!str2) { 
                str2 = malloc(sizeof(char));
                *str2 = '\0';
        }
        
        str1 = realloc(str1, strlen(str1) + strlen(str2) + 1);
        return strcat(str1, str2);
}

static int deletion(char *word, char **array, int start_idx)
{
        int i, word_len = strlen(word);
        
        for (i = 0; i < word_len; i++)
                array[i + start_idx] = concat(substr(word, 0, i), substr(word, i+1, word_len-(i+1)));
        
        return i;
}

static int transposition(char *word, char **array, int start_idx)
{
        int i, word_len = strlen(word);
        
        for (i = 0; i < word_len-1; i++)
                array[i + start_idx] = concat(concat(substr(word, 0, i), 
                                                     substr(word, i+1, 1)), 
                                              concat(substr(word, i, 1), 
                                                     substr(word, i+2, word_len-(i+2))));
        
        return i;
}

static int alteration(char *word, char **array, int start_idx)
{
        int i, j, k, word_len = strlen(word);
        char c[2] = { 0, 0 };
        
        for (i = 0, k = 0; i < word_len; i++)
                for (j = 0; j < sizeof(alphabet); j++, k++) {
                        c[0] = alphabet[j];
                        array[start_idx + k] = concat(concat(substr(word, 0, i), (char *) &c), 
                                                      substr(word, i+1, word_len - (i+1)));
                }
        
        return k;
}

static int insertion(char *word, char **array, int start_idx)
{
        int i, j, k, word_len = strlen(word);
        char c[2] = { 0, 0 };

        for (i = 0, k = 0; i <= word_len; i++)
                for (j = 0; j < sizeof(alphabet); j++, k++) {
                        c[0] = alphabet[j];
                        array[start_idx + k] = concat(concat(substr(word, 0, i), (char *) &c), 
                                                      substr(word, i, word_len - i));
                }
        
        return k;
}

static int edits1_rows(char *word)
{
        register int size = strlen(word);
        
        return (size)                          + // deletion
               (size - 1)                      + // transposition
               (size * sizeof(alphabet))       + // alteration
               (size + 1) * sizeof(alphabet);    // insertion
}

static char **edits1(char *word)
{
        int next_idx;
        char **array = malloc(edits1_rows(word) * sizeof(char *));

        if (!array)
                return NULL;

        next_idx  = deletion(word, array, 0);
        next_idx += transposition(word, array, next_idx);
        next_idx += alteration(word, array, next_idx);
        insertion(word, array, next_idx);

        return array;
}

static int array_exist(char **array, int rows, char *word)
{
        int i;
        
        for (i = 0; i < rows; i++)
                if (!strcmp(array[i], word))
                        return 1;
        
        return 0;
}

static char **known_edits2(char **array, int rows, int *e2_rows)
{
        int i, j, res_size, e1_rows;
        char **res = NULL, **e1;
        
        for (i = 0, res_size = 0; i < rows; i++) {
                e1      = edits1(array[i]);
                e1_rows = edits1_rows(array[i]);
                
                for (j = 0; j < e1_rows; j++)
                        if (find(e1[j]) && !array_exist(res, res_size, e1[j])) {
                                res             = realloc(res, sizeof(char *) * (res_size + 1));
                                res[res_size++] = e1[j];
                        }
        }
        
        *e2_rows = res_size;
        
        return res;
}

static char *max(char **array, int rows)
{
        char *max_word = NULL;
        int i, max_size = 0;
        ENTRY *e;
        
        for (i = 0; i < rows; i++) {
                e = find(array[i]);
                if (e && ((int) e->data > max_size)) {
                        max_size = (int) e->data;
                        max_word = e->key;
                }
        }

        return max_word;
}

static void array_cleanup(char **array, int rows)
{
        int i;
        
        for (i = 0; i < rows; i++)
                free(array[i]);
}

static char *correct(char *word)
{
        char **e1, **e2, *e1_word, *e2_word, *res_word = word;
        int e1_rows, e2_rows;
        
        if (find(word))
                return word;
        
        e1_rows = edits1_rows(word);
        if (e1_rows) {
                e1      = edits1(word);
                e1_word = max(e1, e1_rows);

                if (e1_word) {
                        array_cleanup(e1, e1_rows);
                        free(e1);
                        return e1_word;
                }
        }

        e2 = known_edits2(e1, e1_rows, &e2_rows);
        if (e2_rows) {
                e2_word = max(e2, e2_rows);
                if (e2_word)
                        res_word = e2_word;
        }
        
        array_cleanup(e1, e1_rows);
        array_cleanup(e2, e2_rows);
        
        free(e1);
        free(e2);
        
        return res_word;
}

int main(int argc, char **argv)
{
        char *corrected_word;
        ENTRY dict;
        
        hcreate(DICT_SZ);
        
        if (!read_file(dict))
                return -1;
        
        corrected_word = correct(argv[1]);
        if (strcmp(corrected_word, argv[1])) {
                printf("Did you mean \"%s\"?\n", corrected_word);
        } else {
                printf("\"%s\" is correct!\n", argv[1]);
        }
        
        return 0;
}
