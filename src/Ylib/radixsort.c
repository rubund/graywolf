/*- BSD copyright says:
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char SccsId[] = "@(#) radixsort.c (Yale) version 1.4 4/18/92" ;
#endif

#include <string.h>
#include <yalecad/base.h>
#define RADIX_PREFIX	4

/* radixsort.c, radixsort.h: linear-per-byte in-memory string sort
Daniel J. Bernstein, brnstnd@nyu.edu; Keith Bostic, bostic@ucbvax.berkeley.edu.
No dependencies.
Requires malloc, free, bcopy, bzero. Can use -DUCHAR_MAX.
10/5/91 DJB: Made c static, added ctr.
7/23/91 DJB: Baseline. radixsort/DJB 3.0. See top for BSD copyright.
No known patent problems.

Documentation in radixsort.3, portions based on BSD documentation.

History: I discovered adaptive distribution sort in 1987. (I haven't
published a paper on it---it's too trivial for that---though I did
mention it in a letter to Knuth.) It grew out of a suggestion quoted in
Knuth's section on quicksort et al., volume 3, page 128: ``John McCarthy
has suggested setting K \approx (u + v)/2, if all keys are known to lie
between u and v.'' What's the biggest problem with MSD radix sort? You
can waste lots of time considering ranges of keys that don't even exist!
In distribution sort, however, you usually start by finding the minimum
and maximum keys. Here McCarthy's suggestion pops in. To sort a pile of
floating-point numbers, find their minimum and maximum, divide the
interval evenly into n subintervals, and split the numbers into piles by
interval just as in MSD radix sort. Just as in quicksort, stop splitting
when a pile can be sorted efficiently by a small-scale method. That's
adaptive distribution sort, and it turns out to be hellishly fast for
sorting floating-point data. The credit really belongs with McCarthy---I
only generalized from 2 to n.

Adaptive distribution sort can be applied to strings, too: find the
first character where two strings in the pile differ, and distribute on
that character. There's no fine line between adaptive distribution sort
and MSD radix sort, but in any case you get a big speed boost from
sorting small piles by a small-scale method. See especially Knuth,
exercise 5.2.5-10.

Computer scientists should note that this method is linear in the number
of bytes being sorted. Sometime in 1989, as I recall, I saw a notice
that someone had discovered an o(n log n) method of sorting n integers.
The method depended on all sorts of weid bit manipulations and was
utterly impractical. As the integers had to be short anyway, MSD radix
sort worked in O(n) time. My guess is that most computer scientists
don't learn about MSD radix sort (and hence don't know that sorting is
linear-per-byte) because it's widely seen as having too big a constant
factor to be practical. This radixsort() is a constructive proof that
the opposite is true.

I ended up sending this code to Berkeley. Keith Bostic cleaned it up,
fixed a few bugs, added a shell sort for the small case, and did several
helpful optimizations. radixsort() will be part of BSD 4.4. I took back
his version and modified it to what you see below. Among other things, I
ported it from ANSI C back to the rest of the world, cleaned up some of
the comments, added a proof that part of the method actually works,
added the radixsort5(), radixsort7(), and radixsort3() variants,
restored the original indentation, fixed an overly conservative estimate
of the necessary stack size, and plugged a memory leak.
*/

#define blob unsigned char /* technically, shouldn't be typedefed */

/* KB says:
 * __rspartition is the cutoff point for a further partitioning instead
 * of a shellsort.  If it changes check __rsshell_increments.  Both of
 * these are exported, as the best values are data dependent.
 */
#ifndef NPARTITION
#define	NPARTITION 40
#endif
int __rspartition = NPARTITION;
int __rsshell_increments[] = { 4, 1, 0, 0, 0, 0, 0, 0 };

/* KB says:
 * Shellsort (diminishing increment sort) from Data Structures and
 * Algorithms, Aho, Hopcraft and Ullman, 1983 Edition, page 290;
 * see also Knuth Vol. 3, page 84. The increments are selected from
 * formula (8), page 95. Roughly O(N^3/2).
 */
static void shellsort(p,index,n,tr)
register blob **p;
register blob *tr;
register int index;
register int n;
{
 register blob ch;
 register blob *s1;
 register blob *s2;
 register int incr;
 register int *incrp;
 register int t1;
 register int t2;

 incrp = __rsshell_increments;
 while (incr = *incrp++)
   for (t1 = incr;t1 < n;++t1)
     for (t2 = t1 - incr;t2 >= 0;)
      {
       s1 = p[t2] + index;
       s2 = p[t2 + incr] + index;
       while ((ch = tr[*s1++]) == tr[*s2] && ch)
         ++s2;
       if (ch > tr[*s2])
	{
         s1 = p[t2];
         p[t2] = p[t2 + incr];
         p[t2 + incr] = s1;
         t2 -= incr;
	}
       else
         break;
      }
}

/* KB says:
 * Stackp points to context structures, where each structure schedules a
 * partitioning.  Radixsort exits when the stack is empty.
 *
 * If the buckets are placed on the stack randomly, the worst case is when
 * all the buckets but one contain (npartitions + 1) elements and the bucket
 * pushed on the stack last contains the rest of the elements.  In this case,
 * stack growth is bounded by:
 *
 *	limit = (nelements / (npartitions + 1)) - 1;
 *
 * This is a very large number, 52,377,648 for the maximum 32-bit signed int.
 *
 * By forcing the largest bucket to be pushed on the stack first, the worst
 * case is when all but two buckets each contain (npartitions + 1) elements,
 * with the remaining elements split equally between the first and last
 * buckets pushed on the stack.  In this case, stack growth is bounded when:
 *
 *	for (partition_cnt = 0; nelements > npartitions; ++partition_cnt)
 *		nelements =
 *		    (nelements - (npartitions + 1) * (nbuckets - 2)) / 2;
 *
 * The bound is:
 *
 *	limit = partition_cnt * (nbuckets - 1);
 *
 * This is a much smaller number, 4590 for the maximum 32-bit signed int.

Note inserted by DJB: About time I proved this... Fix the number of
buckets, b. Any given pile of n elements is split into m stack piles and
b - m small-scale piles which immediately disappear. We ignore the case
where the pile is split into only one pile of its original size---any
pile will be split into smaller piles eventually. Say the stack is left
with piles of sizes p_1 ... p_m, each at least P + 1, none equal to n,
while x elements, for some x from 0 to m'P, are in small-scale piles.
(Here P is the cutoff.) We must have p_1 + ... + p_m + x = n. Depending
on the other (p,x) constraints chosen, we define s(n) as the maximum
stack size for n elements. As the subpile distributions are independent,
clearly

  s(n) = max_m max_(p,x) max {s(p_1),s(p_2) + 1,...,s(p_m) + m - 1}

over all valid m and (p,x). In particular, if m > 0 then we must have
n > p_1 >= P + 1, so if n <= P + 1 then the maximum is (vacuously) 0. So
s(n) is monotone increasing for n <= P + 1. An easy induction shows that
s(n) is in fact 0 for all n < 2P + 2.

Clearly s(n) is monotone: for n >= P + 2 we choose m = 1, p_1 = n - 1,
and x = 0, and we see s(n) >= s(p_1) = s(n - 1). For m = 0, the maximum
always equals 0, and for m = 1, the maximum always equals a previous
s(p_1), so we have

  s(n) = max { max_{k<n} s(k) , max_{m>=2} max_(p,x) max s(p_j) + j - 1 }.

For convenience we define t(n) = max_{m>=2} max_(p,x) max s(p_j) + j - 1.

Fix n. Fix m >= 2. Consider a (p,x) achieving the maximum value of
max s(p_j) + j - 1. Since p_1 >= P + 1, we have p_2 <= n - P - 1. If x
isn't 0, we can move an element from one of the small-scale piles to
stack pile p_2, under either of the constraints in question. This
increases p_2---hence does not decrease s(p_2)---without affecting the
other s(p_j). Hence there is a (p,x) with smaller x also achieving the
maximum.

So consider a (p,0) achieving the maximum, and say max s(p_j) + j - 1 is
achieved at j = i. If we exchange p_i and p_j while meeting the
constraints, we must not be at a higher maximum; in particular,
s(p_i) + j - 1 <= s(p_i) + i - 1, so j <= i.

Restrict attention the the case without constraints, NC. The choice of j
is unrestricted, so in particular m <= i and hence i = m. Thus t(n)
equals max_{m>=2} s(p_m) + m - 1. Since all p_j are at least P + 1, we
have

  p_m + (P + 1)(m - 1) <= p_m + ... + p_1 = n
  p_m <= n - (P + 1)(m - 1)
  s(p_m) <= s(n - (P + 1)(m - 1))
  t(n) <= max_{m>=2} s(n - (P + 1)(m - 1)) + m - 1    (NC),

and it is easy to see that for n >= (P + 1)m, we can choose p_m as
n - (P + 1)(m - 1) >= P + 1 and all other p_j = P + 1, so that the bound
is achieved:

  t(n) = max_{m>=2} s(n - (P + 1)(m - 1)) + m - 1  for  n/(P + 1) >= m.  (NC)

For 2 <= n/(P + 1) < b, we can choose m anywhere between 2 and
f = floor(n/(P + 1)) inclusive. Now

  s(n) = max { max s(k), max_{2<=m<f} s(n - (P + 1)(m - 1)) + m - 1 } (NC)

We claim inductively that s(n) = f - 1 for any n with floor(n/(P + 1)) =
f. This is true for f = 1. For larger f, s(n - (P + 1)(m - 1)) =
floor((n - (P + 1)(m - 1))/(P + 1)) - 1 = f - (m - 1) - 1 = f - m, so
that

  s(n) = max { max s(k), f - 1 }   (NC)

If n = (P + 1)f, then by inductive hypothesis s(k) <= f - 2, so that
s(n) = f - 1 as desired. Otherwise, by (sub)induction on n, s(k) is
still bounded by f - 1, so that s(n) = f - 1 always. Therefore:

  s(n) = floor(n/(P + 1)) - 1,  n >= P + 1.     (NC)

Now consider the push-largest-pile-first constraint, FC. This requires
that p_1 >= p_j for all j. Hence we cannot swap p_1 with p_i. However,
if i is not 1 then we can swap p_j with p_i for all j > 1, hence j <= i
for all j between 2 and m, hence i is m. Thus the maximum is achieved
either at i = 1 or at i = m, and we have

  t(n) = max_{m>=2} max_(p,0) max { s(p_m) + m - 1, s(p_1) }.   (FC)

Take a p vector achieving the maximum of max { s(p_m) + m - 1, s(p_1) },
with m fixed. Suppose some p_j for j between 2 and m - 1 inclusive is
larger than P + 1. (This is vacuous for m = 2.) Then we can subtract one
from p_j and add one to p_1, preserving the constraint and not
decreasing the maximum. Hence the maximum is achieved somewhere with all
p_j = P + 1 for 2 <= j < m, i.e.,

  p_1 + p_m + (P + 1)(m - 2) = n.   (FC)

Furthermore, p_1 >= p_m. Hence 2p_1 >= n - (P + 1)(m - 2), and p_1 can
range from ceiling((n - (P + 1)(m - 2))/2) up to n - (P + 1). Similarly,
2p_m <= n - (P + 1)(m - 2), so p_m can range from P + 1 up to
floor((n - (P + 1)(m - 2))/2). The global maximum of these quantities
simultaneously equals the global maximum of them individually, so if all
bounds can be achieved then

  t(n) = max_{m>=2} max { s(n - (P + 1)),
			  s(floor((n - (P + 1)(m - 2))/2)) + m - 1 }.  (FC)

This can be achieved if n - (P + 1) >= ceiling((n - (P + 1)(m - 2))/2)
and, equivalently, P + 1 <= floor((n - (P + 1)(m - 2))/2), since in that
case we can choose both p_1 and p_m as stated. These reduce after some
simple manipulation to n >= (P + 1)m, i.e., m <= floor(n/(P + 1)). For
other m it is not possible to choose any valid p_i (exercise).

We'd like to show inductively that for all n >= 2P + 2 we have

  s(n) = u(n) with
  u(n) = max_{m>=2} s(floor((n - (P + 1)(m - 2))/2)) + m - 1.   (FC,*)

To do this we need only show that the other terms of the maximum do not
``get in the way,'' i.e., that u(n) >= s(n - (P + 1)) and u(n) >= s(k)
for k < n. The second half is easy: s(k) = u(k), which is at most u(n)
by monotonicity of s. The first half also follows from the induction:

  s(n) = max_m s(floor((n - (P + 1)(m - 2))/2)) + m - 1
  s(n - (P + 1)) = u(n - (P + 1))
   = max_{m>=2} s(floor((n - (P + 1) - (P + 1)(m - 2))/2)) + m - 1
   <= max_{m>=2} s(floor((n - (P + 1)(m - 2))/2)) + m - 1
   = u(n)                    (FC)

again by the monotonicity of s. This proves (FC,*). For small n, i.e.,
floor(n/(P + 1)) = f with 2 <= f <= b, we can choose m = f, so s(n) is
at least s(floor((n - (P + 1)(f - 2))/2)) + f - 1. The floor term is at
most P + 1, so s(n) >= f - 1. Furthermore, s in the constrained case is
at most s in the unconstrained case, so s(n) = f - 1. For larger n, by
similar logic, the maximum is attained at m = b, so we finally have

  s(n) = floor(n/(P + 1)) - 1  for n < (b + 1)(P + 1)
  s(n) = s(floor((n - (P + 1)(b - 2))/2)) + b - 1  otherwise    (FC)

As in the first case s(n) is always bounded by b - 1, we can calculate
a bound on s(n) by repeatedly setting n to floor(n - (P + 1)(b - 2))/2)
until it is under 2P + 2 (so that s(n) = 0), counting the number of
iterations necessary, and multiplying by b - 1. And that's what we
wanted to prove.
*/

#ifndef UCHAR_MAX /* XXX: we aren't even giving a chance for a definition! */
#define UCHAR_MAX 256
#endif
#define	NBUCKETS (UCHAR_MAX + 1)

typedef struct
 {
  blob **bot;
  int index;
  int n;
 }
context;

#define	STACKPUSH { \
  stackp->bot = p; \
  stackp->n = n; \
  stackp->index = index; \
  ++stackp; \
}

#define	STACKPOP { \
  if (stackp == stack) \
    break; \
  --stackp; \
  bot = stackp->bot; \
  n = stackp->n; \
  index = stackp->index; \
}

/* KB says:
 * This uses a simple sort as soon as a bucket crosses a cutoff point,
 * rather than sorting the entire list after partitioning is finished.
 * This should be an advantage.

Note from DJB: The original comment read ``There is no strong evidence
that this is an advantage.'' Depressing. Here's what I wrote in response:

   Of course it's an advantage: it has to be, I coded it that way. :-)
   Seriously, I just coded the sort that way since I was following Knuth's
   description of MSD to the hilt. As you can imagine, though, doing the
   sort this way saves just a bit of paging of the index array. It also
   means that the simple sort doesn't have to worry about crossing past
   already-determined boundaries---for an average 2x gain. Trust me, it's
   an advantage.
*/

int Yradixsort5(l1,n,endchar,tab,indexstart)
blob **l1;
register int n;
unsigned int endchar; /* could use blob, but chars are unsafe with prototypes */
blob *tab;
int indexstart;
{
 register int i;
 register int index;
 register int t1;
 register int t2;
 register blob **l2;
 register blob **p;
 register blob **bot;
 register blob *tr;
 context *stack;
 context *stackp;
 static int c[NBUCKETS + 1];
 static int *ctr[NBUCKETS + 1];
 int max;
 blob ltab[NBUCKETS]; /* local (default) table */

 if (n <= 1)
   return 0;

 /* Allocate the stack. */
 t1 = (__rspartition + 1) * (NBUCKETS - 2);
 for (i = 0,t2 = n;t2 > __rspartition;i += NBUCKETS - 1)
   t2 = (t2 - t1)/2; /* could go negative! but that's okay */
 if (!i)
   stack = stackp = 0;
 else
   if (!(stack = stackp = YMALLOC(i, context) ))
     return -1;

 /* KB says:
  * There are two arrays, one provided by the user (l1), and the
  * temporary one (l2). The data is sorted to the temporary stack,
  * and then copied back. The speedup of using index to determine
  * which stack the data is on and simply swapping stacks back and
  * forth, thus avoiding the copy every iteration, turns out to not
  * be any faster than the current implementation.
  */
 if (!(l2 = YMALLOC( n, blob *)))
  {
   YFREE(stackp);
   return -1;
  }

 /* KB says:
  * tr references a table of sort weights; multiple entries may
  * map to the same weight; EOS char must have the lowest weight.
  */
 if (tab)
   tr = tab;
 else
  {
   t2 = endchar;
   for (t1 = 0;t1 < t2;++t1)
     ltab[t1] = t1 + 1;
   ltab[t2] = 0;
   for (t1 = t2 + 1;t1 < NBUCKETS;++t1)
     ltab[t1] = t1;
   tr = ltab;
  }

 for (t1 = 0;t1 < NBUCKETS;++t1)
   ctr[t1] = c + tr[t1];

 /* First sort is entire pile. */
 bot = l1;
 index = indexstart;

 for (;;)
  {
   /* Clear the bucket count array. XXX: This isn't portable to */
   /* machines where the byte representation of int 0 isn't all */
   /* zeros. :-) */
   bzero((char *)c,sizeof(c));

   /* Compute number of items that sort to the same bucket for this index. */
   p = bot;
   i = n;
   while (--i >= 0)
     ++*ctr[(*p++)[index]];

   /* KB says:
    * Sum the number of characters into c, dividing the temp stack
    * into the right number of buckets for this bucket, this index.
    * c contains the cumulative total of keys before and included in
    * this bucket, and will later be used as an index to the bucket. 
    * c[NBUCKETS] contains the total number of elements, for determining
    * how many elements the last bucket contains. At the same time, we
    * find the largest bucket so it gets pushed first.
    */
   t2 = __rspartition;
   max = t1 = 0;
   for (i = 0;i <= NBUCKETS;++i)
    {
     if (c[i] > t2)
      {
       t2 = c[i];
       max = i;
      }
     t1 = c[i] += t1;
    }

   /* Partition the elements into buckets; c decrements through the */
   /* bucket, and ends up pointing to the first element of the bucket. */
   i = n;
   while (--i >= 0)
    {
     --p;
     l2[--*ctr[(*p)[index]]] = *p;
    }

   /* Copy the partitioned elements back to the user stack. */
   bcopy(l2,bot,n * sizeof(blob *));

   ++index;
   /* KB says:
    * Sort buckets as necessary; don't sort c[0], it's the
    * EOS character bucket, and nothing can follow EOS.
    */
   for (i = max;i;--i)
    {
     if ((n = c[i + 1] - (t1 = c[i])) < 2)
       continue;
     p = bot + t1;
     if (n > __rspartition)
       STACKPUSH
     else
       shellsort(p,index,n,tr);
    }
   for (i = max + 1;i < NBUCKETS;++i)
    {
     if ((n = c[i + 1] - (t1 = c[i])) < 2)
       continue;
     p = bot + t1;
     if (n > __rspartition)
       STACKPUSH
     else
       shellsort(p,index,n,tr);
    }
   /* Break out when stack is empty */
   STACKPOP
  }

 YFREE(l2);
 if( stack ) YFREE(stack);
 return 0;
}

int Yradixsort_pref(l1,n)
blob **l1; register int n;
{
 return Yradixsort5(l1,n,EOS,NULL,RADIX_PREFIX);
}

int Yradixsort4(l1,n,endchar,tab)
blob **l1; register int n; unsigned int endchar; blob *tab;
{
 return Yradixsort5(l1,n,endchar,tab,0);
}


int Yradixsort(l1,n,tab,endchar)
blob **l1; register int n; blob *tab; unsigned int endchar;
{
 return Yradixsort5(l1,n,endchar,tab,0);
}

int Yradixsort3(l1,n,endchar)
blob **l1; register int n; unsigned int endchar; 
{
 return Yradixsort5(l1,n,endchar,(blob *) 0,0);
}


/* BSD sccs says:
static char sccsid[] = "@(#)radixsort.c  5.7 (Berkeley) 2/23/91";

but this is (heavily) modified.
*/

/* build a prefix for the string.  returns where to add to the string */
char *Yradix_prefix( buffer, num )
char *buffer ;
INT num ;
{
    buffer[0] = (char) ((num >> 24) & 0x000000FF) ;
    buffer[1] = (char) ((num >> 16) & 0x000000FF) ;
    buffer[2] = (char) ((num >> 8)  & 0x000000FF) ;
    buffer[3] = (char) num & 0x000000FF ;
    buffer[4] = EOS ;
    return( buffer+4 ) ;
} /* end Yradix_prefix() */

/* given a prefix string returns the number (prefix) at the start */
INT Yradix_number( buffer )
char *buffer ;
{
    unsigned int num ;
    unsigned int temp ;

    temp = ( ((unsigned int) buffer[0]) << 24) & 0xFF000000 ;
    num  = temp ;
    temp = ( ((unsigned int) buffer[1]) << 16) & 0x00FF0000 ;
    num  |= temp ;
    temp = ( ((unsigned int) buffer[2]) <<  8) & 0x0000FF00 ;
    num  |= temp ;
    temp = ( ((unsigned int) buffer[3]) ) & 0x000000FF ;
    num  |= temp ;
    return( num ) ;
} /* end INT Yradix_number() */

char *Yradix_suffix( buffer )
char *buffer ;
{
    return( buffer+4 ) ;
} /* end Yradix_suffix() */

char *Yradix_pref_clone( buffer )
char *buffer ;
{
    INT i, len ;
    char *new_string ;

    len = strlen( buffer + 4 ) + 4 + 1 ;
    new_string = YMALLOC( len, char ) ;
    for( i = 0 ; i < len ; i++ ){
	new_string[i] = buffer[i] ;
    }
    return( new_string ) ;

} /* end Yradix_pref_clone() */


#ifdef TEST


#define NUM_ALLOC 10

typedef struct {
    char sort_weight[15] ;
    int weight ;
} INFO, *INFOPTR ;


main()
{
    INFOPTR *array ;
    INFOPTR aptr ;
    INT i, num ;
    char buffer[20] ;
    char *bufferptr ;
    char *sort_field ;

    
    Yset_random_seed( Yrandom_seed() ) ;
    array = YMALLOC( NUM_ALLOC, INFOPTR ) ;
    for( i = 0 ; i < NUM_ALLOC ; i++ ){
	aptr = array[i] = YMALLOC( 1, INFO ) ;
	aptr->weight = Yacm_random() ;
	/* store the original place in the prefix of the sort field */
	/* Yradix_prefix returns back the new start of the string */
	sort_field = Yradix_prefix( aptr->sort_weight, i ) ;
	sprintf( sort_field, "%10d", aptr->weight ) ;
	fprintf( stderr, "array[%d] = %d\n", i, aptr->weight ) ;
    }

    Yradixsort_pref( array, NUM_ALLOC ) ;

    fprintf( stderr, "\nAfter the sort:\n" ) ;
    for( i = 0 ; i < NUM_ALLOC ; i++ ){
	aptr = array[i] ;
	num = Yradix_number( aptr->sort_weight ) ;
	fprintf( stderr, "array[%d] = %10d (originally array[%d])\n", 
	    i, aptr->weight, num ) ;
    }

    num = array[0]->weight ;

    fprintf( stderr, "number = %d\n", num ) ;
    bufferptr = Yradix_prefix( buffer, num ) ;
    fprintf( stderr, "prefix = %s\n", bufferptr ) ;
    num = Yradix_number( buffer ) ;
    fprintf( stderr, "number = %d\n", num ) ;


} /* end main() */


#endif /* TEST */
