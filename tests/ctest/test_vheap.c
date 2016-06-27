/**
 * @file
 *
 * @brief
 *
 * @copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 */

#include <tests_internal.h>

int maxcomp (void * a, void * b)
{
	if (!a || !b) return 1;
	if (*((int *)a) > *((int *)b))
		return 1;
	else
		return 0;
}

int mincomp (void * a, void * b)
{
	if (!a || !b) return 1;
	if (*((int *)a) < *((int *)b))
		return 1;
	else
		return 0;
}

static void test_errors ()
{
	succeed_if (!elektraVheapInit (mincomp, 0), "init 0 working");
	succeed_if (!elektraVheapInit (mincomp, -1), "init -1 working");
	succeed_if (!elektraVheapInit (NULL, 1), "init NULL cmp working");

	succeed_if (!elektraVheapIsEmpty (NULL), "isEmpty NULL working");

	succeed_if (!elektraVheapRemove (NULL), "remove NULL working");
	Vheap * h = elektraVheapInit (mincomp, 4);
	exit_if_fail (h, "vheap init error");
	succeed_if (!elektraVheapRemove (NULL), "remove empty working");
	elektraVheapDestroy (h);

	succeed_if (!elektraVheapInsert (NULL, NULL), "insert NULL working");
}


static void test_empty ()
{
	Vheap * h = elektraVheapInit (mincomp, 4);
	exit_if_fail (h, "vheap init error");
	succeed_if (elektraVheapIsEmpty (h), "should be empty");
	elektraVheapInsert (h, NULL);
	succeed_if (!elektraVheapIsEmpty (h), "should not be empty");
	elektraVheapRemove (h);
	succeed_if (elektraVheapIsEmpty (h), "should be empty");
	elektraVheapDestroy (h);
}

static void test_data_max_ordered ()
{
	Vheap * h = elektraVheapInit (maxcomp, 100);
	exit_if_fail (h, "vheap init error");
	int data[100];
	for (int i = 0; i < 99; ++i)
	{
		data[i] = i;
		succeed_if (elektraVheapInsert (h, &data[i]), "insert error");
	}
	int max = 100;
	for (int i = 0; i < 99; ++i)
	{

		void * dv = elektraVheapRemove (h);
		succeed_if (dv, "remove error");
		int d = *(int *)dv;
		succeed_if (d < max, "ascending error");
		max = d;
	}
	elektraVheapDestroy (h);

	h = elektraVheapInit (maxcomp, 100);
	exit_if_fail (h, "vheap init error");
	for (int i = 99; i >= 0; --i)
	{
		data[i] = i;
		succeed_if (elektraVheapInsert (h, &data[i]), "insert error");
	}
	max = 100;
	for (int i = 0; i < 99; ++i)
	{
		void * dv = elektraVheapRemove (h);
		succeed_if (dv, "remove error");
		int d = *(int *)dv;
		succeed_if (d < max, "descending error");
		max = d;
	}
	elektraVheapDestroy (h);
}

static void test_data_min_ordered ()
{
	Vheap * h = elektraVheapInit (mincomp, 100);
	exit_if_fail (h, "vheap init error");
	int data[100];
	for (int i = 0; i < 99; ++i)
	{
		data[i] = i;
		succeed_if (elektraVheapInsert (h, &data[i]), "insert error");
	}
	int min = -1;
	for (int i = 0; i < 99; ++i)
	{
		void * dv = elektraVheapRemove (h);
		succeed_if (dv, "remove error");
		int d = *(int *)dv;
		succeed_if (d > min, "ascending error");
		min = d;
	}
	elektraVheapDestroy (h);

	h = elektraVheapInit (mincomp, 100);
	exit_if_fail (h, "vheap init error");
	for (int i = 99; i >= 0; --i)
	{
		data[i] = i;
		succeed_if (elektraVheapInsert (h, &data[i]), "insert error");
	}
	min = -1;
	for (int i = 0; i < 99; ++i)
	{
		void * dv = elektraVheapRemove (h);
		succeed_if (dv, "remove error");
		int d = *(int *)dv;
		succeed_if (d > min, "descending error");
		min = d;
	}
	elektraVheapDestroy (h);
}

static void test_grow_shrink ()
{
	int maxElem = 101;
	int data = 42;
	for (int minSize = 1; minSize <= 100; ++minSize)
	{
		int actualSize = minSize;
		Vheap * h = elektraVheapInit (mincomp, minSize);
		exit_if_fail (h, "vheap init error");
		for (int i = 1; i <= maxElem; ++i)
		{
			succeed_if (elektraVheapInsert (h, &data), " insert error");
			if (i > actualSize)
			{
				// grow
				actualSize <<= 1;
			}
			succeed_if (actualSize == h->size, "grow error");
		}
		for (int i = maxElem - 1; i >= 0; --i)
		{
			succeed_if (elektraVheapRemove (h), "remove error");
			if (actualSize > minSize && i <= actualSize >> 2)
			{
				// shrink
				actualSize >>= 1;
			}
			succeed_if (actualSize == h->size, "shrink error");
		}
		elektraVheapDestroy (h);
	}
}

static void test_data_max_mixed ()
{
	Vheap * h = elektraVheapInit (maxcomp, 100);
	exit_if_fail (h, "vheap init error");
	int data[100];
	for (int i = 0; i < 99; ++i)
	{
		data[i] = i % 10;
		succeed_if (elektraVheapInsert (h, &data[i]), "insert error");
	}
	int max = 100;
	for (int i = 0; i < 99; ++i)
	{

		void * dv = elektraVheapRemove (h);
		succeed_if (dv, "remove error");
		int d = *(int *)dv;
		succeed_if (d < max || d == max, "ascending error");
		max = d;
	}
	elektraVheapDestroy (h);

	h = elektraVheapInit (maxcomp, 100);
	exit_if_fail (h, "vheap init error");
	for (int i = 99; i >= 0; --i)
	{
		data[i] = i % 10;
		succeed_if (elektraVheapInsert (h, &data[i]), "insert error");
	}
	max = 100;
	for (int i = 0; i < 99; ++i)
	{
		void * dv = elektraVheapRemove (h);
		succeed_if (dv, "remove error");
		int d = *(int *)dv;
		succeed_if (d < max || d == max, "descending error");
		max = d;
	}
	elektraVheapDestroy (h);
}

static void test_data_min_mixed ()
{
	Vheap * h = elektraVheapInit (mincomp, 100);
	exit_if_fail (h, "vheap init error");
	int data[100];
	for (int i = 0; i < 99; ++i)
	{
		data[i] = i % 10;
		succeed_if (elektraVheapInsert (h, &data[i]), "insert error");
	}
	int min = -1;
	for (int i = 0; i < 99; ++i)
	{
		void * dv = elektraVheapRemove (h);
		succeed_if (dv, "remove error");
		int d = *(int *)dv;
		succeed_if (d > min || d == min, "ascending error");
		min = d;
	}
	elektraVheapDestroy (h);

	h = elektraVheapInit (mincomp, 100);
	exit_if_fail (h, "vheap init error");
	for (int i = 99; i >= 0; --i)
	{
		data[i] = i % 10;
		succeed_if (elektraVheapInsert (h, &data[i]), "insert error");
	}
	min = -1;
	for (int i = 0; i < 99; ++i)
	{
		void * dv = elektraVheapRemove (h);
		succeed_if (dv, "remove error");
		int d = *(int *)dv;
		succeed_if (d > min || d == min, "descending error");
		min = d;
	}
	elektraVheapDestroy (h);
}

int main (int argc, char ** argv)
{
	printf ("VHEAP       TESTS\n");
	printf ("==================\n\n");

	init (argc, argv);

	test_errors ();
	test_grow_shrink ();
	test_data_max_ordered ();
	test_data_min_ordered ();
	test_data_max_mixed ();
	test_data_min_mixed ();
	test_empty ();

	printf ("\ntest_vheap RESULTS: %d test(s) done. %d error(s).\n", nbTest, nbError);

	return nbError;
}
