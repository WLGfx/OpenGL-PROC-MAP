/////////////////////////////////////////////////////////////////////////////
Other notes:

using dbAtanFull(Xd,Yd)

O------1	90 degrees
|
|
|
2	0 degrees

using dbAtanFull(yd,Xd)

O------1	0 degrees	(0.0)	cos*x, sin*y
|
|
|
1	90 degrees	(pi/2)

/////////////////////////////////////////////////////////////////////////////


To add:

To make sure that each segment that is added I will have to re-code the add corridor and
add room functions to re-add the first edge from which it was originally spawned.

I'm wondering at this current moment in time whether I can get away with just adding that edge
after the segment has been added. If not, ah well...

When processing each seperate segment/limb the start vertex is simple enough as it will be the
first vertex from the original edge used to spawn it.

/////////////////////////////////////////////////////////////////////////////

LS_SEGMENTS
LS_RNDSEED
LS_LENMIN & LS_LENMAX
LS_ANGMIN & LS_ANGMAX
LS_SEGMIN & LS_SEGMAX
LS_CEDMIN & LS_CEDMAX
LS_CLNMIN & LS_CLNMAX
LS_NHGT
LS_NSEED
LS_NDIV

/////////////////////////////////////////////////////////////////////////////

Added segments to the _EDGES lists

/////////////////////////////////////////////////////////////////////////////

Updgraded the _VECT struct to hold the normals and texture coords.

/////////////////////////////////////////////////////////////////////////////
