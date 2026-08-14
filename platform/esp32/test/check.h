/* The one assertion these tests need, and the tally that decides what main() returns.
   Stood in four copies before, one per test, all of them the same fifteen lines.

   Every check prints, passing or not, which is what an assertion helper would usually be
   too talkative for. Here it is the whole report: nothing wraps these binaries, so a run
   that printed only failures would leave "all checks passed" as the only evidence that
   anything ran at all. The simulator's tests do the opposite for the same reason - ctest
   wraps them and counts, so they only speak up when something is wrong.

   Kept out of stubs/, which stands in for the framework; this is the test's own
   vocabulary and not something the backend ever sees. */
#ifndef _CHECK_H_
#define _CHECK_H_

#include <cstdio>

/* Inline, so every test can include this and still link: one variable, one definition,
   however many translation units. */
inline int Failures = 0;

/* The description first and the verdict in a fixed column, so a run reads as a list of
   what was checked rather than as a list of results. */
inline void check(bool Ok, const char* What)
{
    printf("%-56s %s\n", What, Ok ? "ok" : "FAIL");
    if(!Ok) { Failures++; }
}

/* What main() ends with: the line a reader looks for, and the exit code run.sh reads. */
inline int report()
{
    printf("\n%s\n", (Failures == 0) ? "all checks passed" : "FAILURES");
    return (Failures == 0) ? 0 : 1;
}

#endif // _CHECK_H_
