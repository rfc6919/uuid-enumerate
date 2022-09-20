uuid-enumerate
==============

I recently remembered a StackOverflow question from years ago where
someone was asking where they could download a list of all the UUIDs.

This is clearly stupid, because the list will be really big. A much
better option is to just generate the list of all UUIDs locally. Much
faster than downloading it, that's for sure.

Since the list will be big, you'd want to generate it as efficiently as
possible. Standard UUID libraries will just be way too much overhead.
Even ``printf(3)`` will likely be too heavyweight. Hence
``uuid-enumerate``.

There may be some more performance to wring from this, but I'm pretty
happy with where I've gotten it to. Sharding is also supported, so you
can distribute the work across multiple machines for even more speedup.

Unfortunately I haven't been able to re-find the original SO question
that inspired this tool. :(

Usage
-----

``uuid-enumerate [shard_number/shard_count]``

Prints all the UUIDs to stdout. If sharding, the shard number is zero-
based and the shard count must be a power of two.

``./X-uuid-generate``

Invoke the cheat code. if ``(argv[0][2] == 'X')`` then run in a
restricted mode that only traverses 32 bits worth of UUID space, for
testing and benchmarking purposes.

Example
-------

``uuid-enumerate 1/8``

Enumerate UUIDs, as the second of 8 shards.

Building
--------

``clang -O3 -o uuid-enumerate uuid-enumerate.c`` (or gcc if you prefer)

There's a significant speedup from ``-O3`` - the loops get unrolled,
multiplication converted to serial addition, direct byte access from the
``uint64_t`` registers instead of shifts.

Performance
-----------

On a 2021 MBP (Apple M1 Pro)::

    rfc6919 % clang -O3 -o X-uuid-enumerate uuid-enumerate.c  
    rfc6919 % time ./X-uuid-enumerate > /dev/null 
    ./X-uuid-enumerate > /dev/null  24.77s user 0.10s system 98% cpu 25.235 total
