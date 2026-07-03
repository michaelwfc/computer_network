

---
<!-- page 1 -->


1

CS144
An	
  Introduc/on	
  to	
  Computer	
  Networks

**Rou$ng**

Basics

**Nick	
  McKeown**
Professor	
  of	
  Electrical	
  Engineering
and	
  Computer	
  Science,	
  Stanford	
  University

CS144,	
  Stanford	
  University



---
<!-- page 2 -->


2

## The	
  Problem

CS144,	
  Stanford	
  University

A

B

How	
  should	
  packets	
  from	
  A	
  reach	
  B?



---
<!-- page 3 -->


3

## The	
  Basics

Approaches

-
Flooding
-
Source	
  rou/ng
-
Forwarding	
  table
-
Spanning	
  tree
Metrics
Shortest	
  path	
  spanning	
  trees
Other	
  types	
  of	
  rou/ng

-
Mul/path
-
Mul/cast

CS144,	
  Stanford	
  University



---
<!-- page 4 -->


4

## Flooding

CS144,	
  Stanford	
  University

A

B

•
Ineﬃcient	
  link	
  usage.
•
Packets	
  can	
  loop	
  forever.
•
Used	
  when	
  we	
  don’t	
  know	
  (or	
  can’t	
  trust)	
  the	
  topology.



---
<!-- page 5 -->


5

## Source	
  Rou/ng

CS144,	
  Stanford	
  University

A

B

•
“End-­‐to-­‐end”	
  solu/on	
  –	
  no	
  support	
  needed	
  from	
  network.
•
Packet	
  carries	
  a	
  variable	
  (and	
  maybe	
  long)	
  list	
  of	
  addresses.
•
End	
  host	
  must	
  know	
  the	
  topology	
  and	
  choose	
  the	
  route.
•
Used	
  when	
  end	
  user	
  wants	
  to	
  control	
  the	
  route.



---
<!-- page 6 -->


6

## Forwarding	
  Table

**Address**
**Next-­‐hop**

B
S 2
C
S 3
D
S 3

A

B

S 1

S 2

S 4

S 3

C

D

Data
B

•
An	
  op/miza/on:	
  Network	
  handles	
  hop-­‐by-­‐hop	
  rou/ng.
•
Requires	
  popula/on	
  of	
  forwarding	
  tables.
•
Per-­‐des/na/on	
  state,	
  not	
  (necessarily)	
  per-­‐ﬂow	
  state.



---
<!-- page 7 -->


7

## Spanning	
  Tree

R 7

R 6
R 4
R 2
R 1

R 8

R 5
R 3

A

X

B

C

D

Spanning	
  tree

- Spanning:	
  It	
  reaches	
  all	
  leaves
- Tree:	
  It	
  has	
  no	
  loops



---
<!-- page 8 -->


8

## What	
  is	
  our	
  metric?

R 7

R 6
R 4
R 2
R 1

R 8
R 5
R 3

A

X

B

C

D

Choices

- Min	
  distance
- Min	
  hop-­‐count
- Min	
  delay
- Max	
  throughput
- Least-­‐loaded	
  path
- Most	
  reliable	
  path
- Lowest	
  cost	
  path
- Most	
  secure	
  path
- …



---
<!-- page 9 -->


9

## Example	
  Annotated	
  Graph

R 7

R 6
R 4
R 2
R 1

R 8

R 5
R 3

A

X

B

C

D

1
1
4

2

4

2
2
3

2
3
5
3



---
<!-- page 10 -->


10

## Example	
  Annotated	
  Graph

R 7

R 6
R 4
R 2
R 1

R 8

R 5
R 3

A

X

B

C

D

1
1
4

2

4

2
2
3

2
3
5
2

Minimum	
  cost	
  spanning	
  tree.
In	
  this	
  case,	
  simple.



---
<!-- page 11 -->


11

## How	
  about	
  this	
  network...!?

The	
  Opte	
  Project



---
<!-- page 12 -->


12

## The	
  Basics

Approaches

-
Flooding
-
Source	
  rou/ng
-
Forwarding	
  table
-
Spanning	
  tree
Metrics
Shortest	
  path	
  spanning	
  trees
Other	
  types	
  of	
  rou/ng

-
Mul/path
-
Mul/cast

CS144,	
  Stanford	
  University



---
<!-- page 13 -->


13

## Mul/path

R 7

R 6
R 4
R 2
R 1

R 8

R 5
R 3

A

X

B

C

D



---
<!-- page 14 -->


14

## Mul/cast

R 7

R 6
R 4
R 2
R 1

R 8

R 5
R 3

A

X

B

C

D



---
<!-- page 15 -->


15

## Mul/cast

R 7

R 6
R 4
R 2
R 1

R 8

R 5
R 3

A

X

B

C

D



---
<!-- page 16 -->


16

## Summary

There	
  are	
  several	
  ways	
  to	
  route	
  packets

across	
  a	
  network,	
  star/ng	
  with	
  the	
  simplest
method,	
  ﬂooding.
In	
  prac/ce,	
  we	
  use	
  rou/ng	
  algorithms	
  (aka

rou/ng	
  protocols)	
  to	
  calculate	
  routes.
Ojen,	
  the	
  algorithms	
  calculate	
  the	
  minimum

cost	
  spanning	
  tree	
  to	
  the	
  des/na/on.
Other	
  types	
  of	
  rou/ng	
  include:	
  mul/path	
  to

spread	
  load	
  over	
  links,	
  and	
  mul/cast	
  to
deliver	
  to	
  mul/ple	
  end	
  hosts.

CS144,	
  Stanford	
  University



---
<!-- page 17 -->


17
CS144,	
  Stanford	
  University