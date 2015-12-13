/*
 * Brute forcer for GCHQ Director's 2015 Christmas puzzle (part 4)
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
#define ANSWER_A 1824745082
#define ANSWER_B 560037081

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

void destroy_dictionary(struct dict *d)
{
	while (d->nwords--) {
		free(d->words[d->nwords]);
	}
	free(d->words);
	free(d);
}

struct dict *build_dictionary()
{
	int i, nwords = 256, maxlen = 3;

	d = malloc(sizeof(*d));
	memset(d, 0, sizeof(*d));
	d->words = malloc(sizeof(*d->words) * nwords);
	d->wordlen = malloc(sizeof(*d->wordlen) * nwords);
	for (i = 0; i < nwords; i++) {
		d->words[i] = malloc(maxlen + 1);
		sprintf(d->words[i], "%d", i);
		d->wordlen[i] = strlen(d->words[i]);
	}
	d->nwords = nwords;

	return d;
}

/* Javascript hash function, identical to part3, but different answers
 *    function hsh(dat) {
 *        resultA = 3141592654;
 *        resultB = 1234567890;
 *        for (i=0; i<2; i++) {
 *            initA = resultA;
 *            initB = resultB;
 *            for (j=0; j<dat.length; j++) {
 *                resultA += dat.toLowerCase().charCodeAt(j);
 *                resultB = (resultA * 31) ^ resultB;
 *                tmp = resultA & resultA;
 *                resultA = resultB & resultB;
 *                resultB = tmp;
 *            }
 *            resultA = resultA ^ initA;
 *            resultB = resultB ^ initB;
 *        }
 *        return [resultA, resultB];
 *    }
 *    $("#answercheckform").submit(function(e) {
 *        answer = $("#answer_a").val() + '\0' + $("#answer_b").val() + '\0' + $("#answer_c").val();
 *        res = hsh(answer);
 *        if ((res[0] == 1824745082) && (res[1] == 560037081)) {
 *            $("#answercheckresult").html("All your answers are correct!<br/><br/>Please go to page <b>next.html</b> at IP address <b>"+$("#answer_a").val()+"."+$("#answer_b").val()+"."+$("#answer_c").val()+"</b> for Part 5.");
 *        } else {
 *            $("#answercheckresult").html("One or more of your answers is incorrect. Please try again.");
 *        }
 *        e.preventDefault();
 *    });
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
	char separators[4];
	int i;

	/* They've been a bit crafy here - there are only 3 answers
	 * but an IP address requires 4 numbers. I thought it was a
	 * typo at first but the Javascript confirms it - one of the
	 * numbers has a decimal point (making 2 parts of the IP).
	 * We can handle this by doing 4 levels of recursion and setting
	 * one of the separators to '.', moving the '.' after each full
	 * pass. This gives a complexity of:
	 *  (3 dot positions) * (pow(256, 4) combinations) =
	 *     ~13bn
	 * It's a lot, but it's feasible. This is also much faster than
	 * the part3 dictionary search because each hash is much shorter
	 */
	/* Our dictionary holds the strings "0" -> "255" */
	d = build_dictionary();
	for (i = 0; i < 3; i++) {
#ifdef DEBUG
		memset(separators, '-', sizeof(separators));
#else
		memset(separators, '\0', sizeof(separators));
#endif
		separators[i] = '.';
		if (recursive_solve(separators))
			break;
	}
	destroy_dictionary(d);
	return 0;
}
