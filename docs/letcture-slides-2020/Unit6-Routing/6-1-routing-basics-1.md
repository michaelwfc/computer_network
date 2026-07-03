# 6-1-routing-basics

CS144	  
An	  Introduc/on	  to	  Computer	  Networks	  
Rou$ng	  
Basics	  

Nick	  McKeown	  
Professor	  of	  Electrical	  Engineering	  	  
and	  Computer	  Science,	  Stanford	  University	  

CS144,	  Stanford	  University	  

1	  

The	  Problem	  
A	  

B	  

How	  should	  packets	  from	  A	  reach	  B?	  
CS144,	  Stanford	  University	  

2	  

The	  Basics	  
Approaches	  
-

Flooding	  
Source	  rou/ng	  
Forwarding	  table	  
Spanning	  tree	  

Metrics	  
Shortest	  path	  spanning	  trees	  
Other	  types	  of	  rou/ng	  
-

Mul/path	  
Mul/cast	  

CS144,	  Stanford	  University	  

3	  

Flooding	  
A	  

B	  

•
•
•
CS144,	  Stanford	  University	  

Ineﬃcient	  link	  usage.	  
Packets	  can	  loop	  forever.	  
Used	  when	  we	  don’t	  know	  (or	  can’t	  trust)	  the	  topology.	  
4	  

Source	  Rou/ng	  
A	  

B	  

•
•
•
•

“End-­‐to-­‐end”	  solu/on	  –	  no	  support	  needed	  from	  network.	  
Packet	  carries	  a	  variable	  (and	  maybe	  long)	  list	  of	  addresses.	  
End	  host	  must	  know	  the	  topology	  and	  choose	  the	  route.	  
Used	  when	  end	  user	  wants	  to	  control	  the	  route.	  	  

CS144,	  Stanford	  University	  

5	  

Forwarding	  Table	  
Data	  

B

A	  

B	  

Address	  

Next-­‐hop	  

B	  

S2	  

C	  

S3	  

D	  

S3	  

S2	  
S1	  

S4	  
S3	  
C	  

•
•
•

D	  

An	  op/miza/on:	  Network	  handles	  hop-­‐by-­‐hop	  rou/ng.	  
Requires	  popula/on	  of	  forwarding	  tables.	  
Per-­‐des/na/on	  state,	  not	  (necessarily)	  per-­‐ﬂow	  state.	  

6	  

Spanning	  Tree	  
B	  

A	  
R1

C	  

R2

R3

Spanning	  tree	  

R4

R5

D	  
R6

R7
R8

X	  

- Spanning:	  It	  reaches	  all	  leaves	  
- Tree:	  It	  has	  no	  loops	  
7	  

What	  is	  our	  metric?	  
B	  

A	  

Choices	  

- Min	  distance	  
- Min	  hop-­‐count	  
- Min	  delay	  
- Max	  throughput	  
- Least-­‐loaded	  path	  
- Most	  reliable	  path	  
- Lowest	  cost	  path	  
- Most	  secure	  path	  
- …	  

R1

C	  

R2

R3

R4
R5

D	  
R6

R7
R8

X	  

8	  

Example	  Annotated	  Graph	  
B	  

A	  
1

R1
2

1

R2
2

5

R3

C	  

4

4

R4

R6

3

3

R5

D	  

2

R7

3

2

R8

X	  

9	  

Example	  Annotated	  Graph	  
B	  

A	  
1

R1
2

1

R2
2

5

R3

C	  

4

4

R4

R6

3

2

R5

D	  

2

R7

3

2

R8

X	  
Minimum	  cost	  spanning	  tree.	  	  
In	  this	  case,	  simple.	  	  
10	  

How	  about	  this	  network...!?	  

The	  Opte	  Project	  

11	  

The	  Basics	  
Approaches	  
-

Flooding	  
Source	  rou/ng	  
Forwarding	  table	  
Spanning	  tree	  

Metrics	  
Shortest	  path	  spanning	  trees	  
Other	  types	  of	  rou/ng	  
-

Mul/path	  
Mul/cast	  

CS144,	  Stanford	  University	  

12	  

Mul/path	  
B	  

A	  
R1

C	  

R2

R3

R4

R5

D	  
R6

R7
R8

X	  

13	  

Mul/cast	  
B	  

A	  
R1

C	  

R2

R3

R4

R5

D	  
R6

R7
R8

X	  

14	  

Mul/cast	  
B	  

A	  
R1

C	  

R2

R3

R4

R5

D	  
R6

R7
R8

X	  

15	  

Summary	  
There	  are	  several	  ways	  to	  route	  packets	  
across	  a	  network,	  star/ng	  with	  the	  simplest	  
method,	  ﬂooding.	  	  
In	  prac/ce,	  we	  use	  rou/ng	  algorithms	  (aka	  
rou/ng	  protocols)	  to	  calculate	  routes.	  
Ojen,	  the	  algorithms	  calculate	  the	  minimum	  
cost	  spanning	  tree	  to	  the	  des/na/on.	  
Other	  types	  of	  rou/ng	  include:	  mul/path	  to	  
spread	  load	  over	  links,	  and	  mul/cast	  to	  
deliver	  to	  mul/ple	  end	  hosts.	  
CS144,	  Stanford	  University	  

16	  

CS144,	  Stanford	  University	  

17	  
