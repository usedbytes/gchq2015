/*
 * Brute forcer for GCHQ Director's 2015 Christmas puzzle (part 3)
 * --------------------------------------------------------------
 * Copyright Brian Starkey 2015 <stark3y@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_DEPTH 4
#define ANSWER_A 608334672
#define ANSWER_B 46009587

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

struct dict {
	int nwords;
	int maxlen;
	char **words;
	int *wordlen;

	char *answers[MAX_DEPTH];
	long long int n_hashes;
};

struct dict *d;
char *fixed[MAX_DEPTH];

int max(int a, int b)
{
	return a > b ? a : b;
}

void destroy_dictionary(struct dict *d)
{
	while (d->nwords--) {
		free(d->words[d->nwords]);
	}
	free(d->words);
	free(d);
}

struct dict *load_dictionary(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	char linebuffer[100];
	int maxlen = 0;
	int nwords = 0, i = 0;

	d = malloc(sizeof(*d));
	memset(d, 0, sizeof(*d));
	while (!feof(fp)) {
		char *s = fgets(linebuffer, 100, fp);
		if (s) {
			maxlen = max(maxlen, strlen(s) - 1);
			nwords++;
		}
	}
	fseek(fp, 0, SEEK_SET);
	fprintf(stderr, "Loading %i entries. maxlen: %d\n", nwords, maxlen);

	d->words = malloc(sizeof(*d->words) * nwords);
	d->wordlen = malloc(sizeof(*d->wordlen) * nwords);
	while (!feof(fp)) {
		char *s = fgets(linebuffer, 100, fp);
		if (s) {
			int j;
			s[strlen(s) - 1] = '\0';
			for (j = 0; j < strlen(s); j++) {
				s[j] = tolower(s[j]);
			}
			d->wordlen[i] = strlen(s);
			d->words[i] = malloc(strlen(s) + 1);
			memcpy(d->words[i], s, strlen(s) + 1);
			i++;
		}
	}
	d->nwords = nwords;
	d->maxlen = maxlen;
	fprintf(stderr, "Done. Loaded %i of %i\n", i, nwords);

	return d;
}


/* Javascript checker:
 * function check(dat) {
 *     resultA = 3141592654;
 *     resultB = 1234567890;
 *     for (i=0; i<2; i++) {
 *         initA = resultA;
 *         initB = resultB;
 *         for (j=0; j<dat.length; j++) {
 *             resultA += dat.toLowerCase().charCodeAt(j);
 *             resultB = (resultA * 31) ^ resultB;
 *             tmp = resultA & resultA;
 *             resultA = resultB & resultB;
 *             resultB = tmp;
 *         }
 *         resultA = resultA ^ initA;
 *         resultB = resultB ^ initB;
 *     }
 *     return [resultA, resultB];
 * }
 * $("#answercheckform").submit(function(e) {
 *     answer = $("#word_a").val() + '\0' + $("#word_b").val() + '\0' + $("#word_c").val() + '\0' + $("#word_d").val();
 *     res = check(answer);
 *     if ((res[0] == 608334672) && (res[1] == 46009587)) {
 *         $("#answercheckresult").html("All your answers are correct!<br/><br/>Please now go to <b>http://www."+$("#word_a").val().toLowerCase()+"-"+$("#word_b").val().toLowerCase()+"-"+$("#word_c").val().toLowerCase()+".org.uk/"+$("#word_d").val().toLowerCase()+"</b> for Part 4.");
 *     } else {
 *         $("#answercheckresult").html("One or more of your answers is incorrect. Please try again.");
 *     }
 *     e.preventDefault();
 * });
 */

bool check(char *dat, int len)
{
	static const uint32_t answerA = ANSWER_A;
	static const uint32_t answerB = ANSWER_B;

	uint32_t resultA = 3141592654;
	uint32_t resultB = 1234567890;
	uint32_t tmp;
	int i, j;
	for (i = 0; i < 2; i++) {
		int initA = resultA;
		int initB = resultB;
		for (j = 0; j < len; j++) {
			resultA += dat[j];
			resultB = (resultA * 31) ^ resultB;
			tmp = resultA & resultA;
			resultA = resultB & resultB;
			resultB = tmp;
		}
		resultA = resultA ^ initA;
		resultB = resultB ^ initB;
	}

	return (resultA == answerA) && (resultB == answerB);
}

int add_element(int n, char *dat, int datlen, char *sep)
{
	int i;
	DBG("%*s> add_element n: %i, dat: %.*s (%i)\n", 4 * n, "", n, datlen, dat, datlen);
	if (n == MAX_DEPTH) {
		bool done = check(dat, datlen);
		DBG("%*s  hashed dat: '%.*s' (%i)\n", 4 * n, "",datlen, dat, datlen);
		d->n_hashes++;
		if (!(d->n_hashes & 0xFFFFF)) {
			fprintf(stderr, "\r%lli hashes computed", d->n_hashes);
		}
		if (done) {
			dat[datlen] = '\0';
			printf("\nFound match. Tried %lld combinations.\n", d->n_hashes);
			for (i = 0; i < MAX_DEPTH; i++)
				printf("%c: %s%c\n", 'A' + i, d->answers[i], sep[i]);
			return 1;
		}
		return 0;
	} else if (fixed[n]) {
		int wordlen = strlen(fixed[n]);
		memcpy(dat + datlen, fixed[n], wordlen);
		if (n < (MAX_DEPTH - 1)) {
			dat[datlen + wordlen] = sep[n];
			wordlen++;
		}
		d->answers[n] = fixed[n];
		DBG("%*s  adding '%s' (fixed[%d])\n", 4 * n, "", fixed[n], n);
		return add_element(n + 1, dat, datlen + wordlen, sep);
	} else {
		for (i = 0; i < d->nwords; i++) {
			int wordlen = d->wordlen[i];
			DBG("%*s  adding '%s' (%i)\n", 4 * n, "", d->words[i], i);
			memcpy(dat + datlen, d->words[i], wordlen);
			if (n < (MAX_DEPTH - 1)) {
				dat[datlen + wordlen] = sep[n];
				wordlen++;
			}
			d->answers[n] = d->words[i];
			if (add_element(n + 1, dat, datlen + wordlen, sep))
				return 1;
		}
	}

	return 0;
}

int recursive_solve(char *sep)
{
	char *dat;
	dat = malloc(sizeof(*dat) * (d->maxlen * 4) + 3);
	memset(dat, 0, (d->maxlen * 4) + 3);

	return add_element(0, dat, 0, sep);
}

int main(int argc, char *argv[])
{
#ifdef DEBUG
	char sep[MAX_DEPTH] = "---";
#else
	char sep[MAX_DEPTH] = "\0\0\0";
#endif
	fixed[0] = "cub";
	fixed[2] = "often";

  	d = load_dictionary("words.list");
	recursive_solve(sep);
	destroy_dictionary(d);
	return 0;
}
