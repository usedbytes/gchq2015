/*
 * Solver for the GCHQ Director's 2015 Christmas puzzle (part 1)
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
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SIDE 25
#define MAX_NMARKS 10
#define INVALID_BIT (1 << 31)

char init_rows[SIDE][10] = {
	{ 7, 3, 1, 1, 7 },
	{ 1, 1, 2, 2, 1, 1 },
	{ 1, 3, 1, 3, 1, 1, 3, 1 },
	{ 1, 3, 1, 1, 6, 1, 3, 1 },
	{ 1, 3, 1, 5, 2, 1, 3, 1 },
	{ 1, 1, 2, 1, 1 },
	{ 7, 1, 1, 1, 1, 1, 7 },
	{ 3, 3 },
	{ 1, 2, 3, 1, 1, 3, 1, 1, 2 },
	{ 1, 1, 3, 2, 1, 1 },
	{ 4, 1, 4, 2, 1, 2 },
	{ 1, 1, 1, 1, 1, 4, 1, 3 },
	{ 2, 1, 1, 1, 2, 5 },
	{ 3, 2, 2, 6, 3, 1 },
	{ 1, 9, 1, 1, 2, 1 },
	{ 2, 1, 2, 2, 3, 1 },
	{ 3, 1, 1, 1, 1, 5, 1 },
	{ 1, 2, 2, 5 },
	{ 7, 1, 2, 1, 1, 1, 3 },
	{ 1, 1, 2, 1, 2, 2, 1 },
	{ 1, 3, 1, 4, 5, 1 },
	{ 1, 3, 1, 3, 10, 2 },
	{ 1, 3, 1, 1, 6, 6 },
	{ 1, 1, 2, 1, 1, 2 },
	{ 7, 2, 1, 2, 5 },
};

uint32_t init_row_masks[SIDE] = {
	[3]  = (3 << 3) | (3 << 12) | (1 << 21),
	[8]  = (3 << 6) | (1 << 10) | (3 << 14) | (1 << 18),
	[16] = (1 << 6) | (1 << 11) | (1 << 16) | (1 << 20),
	[21] = (3 << 3) | (3 << 9) | (1 << 15) | (3 << 20),
};

uint8_t init_cols[SIDE][10] = {
	{ 7, 2, 1, 1, 7 },
	{ 1, 1, 2, 2, 1, 1 },
	{ 1, 3, 1, 3, 1, 3, 1, 3, 1 },
	{ 1, 3, 1, 1, 5, 1, 3, 1 },
	{ 1, 3, 1, 1, 4, 1, 3, 1 },
	{ 1, 1, 1, 2, 1, 1 },
	{ 7, 1, 1, 1, 1, 1, 7 },
	{ 1, 1, 3 },
	{ 2, 1, 2, 1, 8, 2, 1 },
	{ 2, 2, 1, 2, 1, 1, 1, 2 },
	{ 1, 7, 3, 2, 1 },
	{ 1, 2, 3, 1, 1, 1, 1, 1 },
	{ 4, 1, 1, 2, 6 },
	{ 3, 3, 1, 1, 1, 3, 1 },
	{ 1, 2, 5, 2, 2 },
	{ 2, 2, 1, 1, 1, 1, 1, 2, 1 },
	{ 1, 3, 3, 2, 1, 8, 1 },
	{ 6, 2, 1 },
	{ 7, 1, 4, 1, 1, 3 },
	{ 1, 1, 1, 1, 4 },
	{ 1, 3, 1, 3, 7, 1 },
	{ 1, 3, 1, 1, 1, 2, 1, 1, 4 },
	{ 1, 3, 1, 4, 3, 3 },
	{ 1, 1, 2, 2, 2, 6, 1 },
	{ 7, 1, 3, 2, 1, 1 },
};

uint32_t init_col_masks[SIDE] = {
	[3]  = (1 << 3) | (1 << 21),
	[4]  = (1 << 3) | (1 << 21),
	[6]  = (1 << 8) | (1 << 16),
	[7]  = (1 << 8),
	[9]  = (1 << 21),
	[10] = (1 << 8) | (1 << 21),
	[11] = (1 << 16),
	[12] = (1 << 3),
	[13] = (1 << 3),
	[14] = (1 << 8),
	[15] = (1 << 8) | (1 << 21),
	[16] = (1 << 16),
	[18] = (1 << 8),
	[20] = (1 << 16) | (1 << 21),
	[21] = (1 << 3) | (1 << 21),
};

struct line {
	int nmarks;
	char marks[10];
	int spaces;
	bool has_mask;
	uint32_t mask;
	int n_cdd;
	uint32_t *cdd;
	bool fixed;
};

void dump_gridline(uint32_t cdd, bool fixed)
{
	int i;
	for (i = 0; i < SIDE; i++) {
		printf("|%s", cdd & 0x1 ? (fixed ? " @ " : " # ") : "   ");
		cdd >>= 1;
	}
	printf("|\n");
	for (i = 0; i < SIDE; i++)
		printf("+---");
	printf("+\n");
}

void dump_line(struct line *l, bool grid)
{
	int i;
	printf("nmarks: %d\n", l->nmarks);
	printf("marks: ");
	for (i = 0; i < l->nmarks; i++) {
		printf("%d, ", l->marks[i]);
	}
	printf("\n");
	printf("spaces: %d\n", l->spaces);
	if (l->has_mask) {
		printf("mask: 0x%08x\n", l->mask);
		dump_gridline(l->mask, false);
	}

	printf("n_cdd: %i\n", l->n_cdd);
	if (grid) {
		for (i = 0; i < l->n_cdd; i++)
			dump_gridline(l->cdd[i], l->fixed);
	}
	printf("fixed: %s\n", l->fixed ? "yes" : "no");
}

void init_line(struct line *l, char *marks, uint32_t mask)
{
	int i;
	memset(l, 0, sizeof(*l));
	memcpy(l->marks, marks, sizeof(*marks) * MAX_NMARKS);
	while (l->marks[l->nmarks++] && (l->nmarks <= 10));
	l->nmarks--;

	l->spaces = SIDE;
	for (i = 0; i < l->nmarks; i++) {
		l->spaces -= l->marks[i];
	}

	if (mask) {
		l->has_mask = true;
		l->mask = mask;
	}
}

uint32_t mark(char len) {
	return (1 << len) - 1;
};

void simple_candidate(struct line *l)
{
	int next_slot = 0;
	int i;
	if (l->n_cdd == 0)
		l->cdd = malloc(sizeof(l->cdd[0]));
	else
		l->n_cdd = 0;

	l->cdd[l->n_cdd] = 0;
	for (i = 0; i < l->nmarks; i++) {
		l->cdd[l->n_cdd] |= (mark(l->marks[i]) << next_slot);
		next_slot += l->marks[i] + 1;
	}
	if (l->has_mask)
		assert((l->mask & l->cdd[0]) == l->mask);
	l->n_cdd = 1;

	return;
}

int space_reqd(int nmarks, char *marks)
{
	int i;
	int reqd = 0;
	for (i = 0; i < nmarks; i++) {
		/* A space plus the mark */
		reqd += marks[i];
	}
	return reqd + (nmarks - 1);
}

void place_mark(struct line *l, uint32_t cdd, int start, int n, int *space)
{
	int reqd;
	if (n == l->nmarks) {
		/* Terminate */
		if (l->has_mask) {
			if ((l->mask & cdd) != l->mask)
				return;
		}

		if (*space == l->n_cdd) {
			l->cdd = realloc(l->cdd, sizeof(*l->cdd) * l->n_cdd * 2);
			*space = *space * 2;
		}

		l->cdd[l->n_cdd] = cdd;
		l->n_cdd++;

		return;
	}

	reqd = space_reqd(l->nmarks - n, &l->marks[n]);
	if (n == 0)
		reqd -= 1;
	for (start; start <= SIDE - reqd; start++)
		place_mark(l, cdd | mark(l->marks[n]) << start,
			   start + l->marks[n] + 1, n + 1, space);
}

void full_scan(struct line *l)
{
	int space = l->n_cdd;
	if (!space) {
		l->cdd = malloc(sizeof(*l->cdd) * 10);
		space = 10;
	}
	l->n_cdd = 0;
	place_mark(l, 0, 0, 0, &space);
}

void calculate_candidates(struct line *l)
{
	if (l->nmarks > l->spaces) {
		/* Simple case */
		simple_candidate(l);
	}

	full_scan(l);
}

void destroy_line(struct line *l)
{
	if (l->cdd)
		free(l->cdd);
	free(l);
}

void __prune(struct line *l)
{
	uint32_t *gap = NULL;
	int i, n = l->n_cdd;
	for (i = 0; i < n; i++) {
		if (l->cdd[i] & INVALID_BIT) {
			if (!gap)
				gap = &l->cdd[i];
			l->n_cdd--;
		} else if (gap) {
			*gap = l->cdd[i];
			gap++;
		}
	}
}

int prune_line(struct line *l, int n, struct line **against)
{
	int i, x, j;
	int pruned = 0;
	uint32_t test_mask = (1 << n);

	for (i = 0; i < l->n_cdd; i++) {
		/* Two ways to prune:
		 *  - Either doesn't match the mask anymore
		 *  - Or doesn't match any of its counterparts
		 */
		if (l->has_mask && ((l->cdd[i] & l->mask) != l->mask)) {
			l->cdd[i] |= INVALID_BIT;
			continue;
		}

		for (x = 0; x < SIDE; x++) {
			struct line *test = against[x];
			uint32_t source_mask = (1 << x);

			for (j = 0; j < test->n_cdd; j++) {
				/* These would be a match. Stop looking */
				if (((test->cdd[j] & test_mask) && (l->cdd[i] & source_mask)) ||
				    (!(test->cdd[j] & test_mask) && !(l->cdd[i] & source_mask)))
					break;
			}
			if (j == test->n_cdd) {
				/* There was no match. Can't be a candidate */
				pruned++;
				l->cdd[i] |= INVALID_BIT;
				break;
			}
		}
	}

	__prune(l);

	return pruned;
}


int prune_and_fix(struct line **dim, struct line **against)
{
	int fixed = 0;
	int i;
	for (i = 0; i < SIDE; i++) {
		struct line *l = dim[i];
		prune_line(l, i, against);

		/* Fix any lines with only one cdd (set mask) */
		if (l->n_cdd == 1 && !l->fixed) {
			if (l->has_mask)
				assert((l->cdd[0] & l->mask) == l->mask);
			l->mask = l->cdd[0];
			l->fixed = 1;
			fixed++;
		}
	}
	return fixed;
}

void update_masks(struct line **given, struct line **set)
{
	int shift = 0;
	for (shift = 0; shift < SIDE; shift++) {
		struct line *target = set[shift];
		uint32_t newmask = 0;
		int i;
		for (i = 0; i < SIDE; i++) {
			struct line *source = given[i];
			newmask |= ((source->mask >> shift) & 1) << i;
		}
		if (target->has_mask)
			assert((target->mask & newmask) == target->mask);
		target->mask = newmask | target->mask;
		target->has_mask = true;
	}
}

void output_ascii(struct line **rows)
{
	int i;
	for (i = 0; i < SIDE; i++) {
		printf("+---");
	}
	printf("+\n");
	for (i = 0; i < SIDE; i++) {
		struct line *l = rows[i];
		assert(l->fixed);
		dump_gridline(l->mask, l->fixed);
	}


}

void output_pbm(struct line **rows)
{
	int i;
	printf("P1\n");
	printf("%d %d\n", SIDE, SIDE);
	for (i = 0; i < SIDE; i++) {
		struct line *l = rows[i];
		uint32_t cdd = l->cdd[0];
		int j;
		for (j = 0; j < SIDE; j++) {
			printf("%d ", cdd & 0x1);
			cdd >>= 1;
		}
		putchar('\n');
	}
}

int initialise(struct line *array[2][SIDE])
{
	int n_cdd = 0, i;
	struct line **rows = array[0];
	struct line **cols = array[1];

	for (i = 0; i < SIDE; i++) {
		struct line *l = malloc(sizeof(*l));
		init_line(l, init_rows[i], init_row_masks[i]);
		calculate_candidates(l);
		rows[i] = l;
		n_cdd += l->n_cdd;
	}
	for (i = 0; i < SIDE; i++) {
		struct line *l = malloc(sizeof(*l));
		init_line(l, init_cols[i], init_col_masks[i]);
		calculate_candidates(l);
		cols[i] = l;
		n_cdd += l->n_cdd;
	}

	return n_cdd;
}

void teardown(struct line *array[2][SIDE])
{
	int i;
	for (i = 0; i < 2; i++) {
		int j;
		for (j = 0; j < SIDE; j++) {
			destroy_line(array[i][j]);
			array[i][j] = NULL;
		}
	}
}

int main(int argc, char *argv[])
{
	struct line *array[2][SIDE];
	struct line **rows = array[0];
	struct line **cols = array[1];
	int i, fixed;

	/*
	 * Initialise. Calculate all possible rows/cols based on the
	 * starting conditions
	 */
	i = initialise(array);
	fprintf(stderr, "Starting with %d possible candidate lines\n", i);

	while (1) {
		fprintf(stderr, ".");

		/* Calculate rows */
		fixed = prune_and_fix(rows, cols);

		/* Update column masks */
		update_masks(rows, cols);

		/* Calculate columns */
		fixed += prune_and_fix(cols, rows);

		/* Update row masks */
		update_masks(cols, rows);

		/* Hopefully we've converged... */
		if (!fixed)
			break;
	}
	fprintf(stderr, "\n");

	for (i = 0; i < SIDE; i++) {
		struct line *l = rows[i];
		assert(l->fixed);
	}

	if ((argc == 2) && !strcmp(argv[1], "pbm")) {
		output_pbm(rows);
	} else {
		fprintf(stderr, "Using ASCII output. "
			"Try '%s pbm' to generate a PBM image\n",
		       argv[0]);
		output_ascii(rows);
	}

	teardown(array);
	return 0;
}
