Description of distributed REM algorithm
========================================

Edge request
------------
This is a request sent to a processor, asking it to run on its local tree, try to conclude, are send a new request to another processor.

### Context
 - **process**: current process id
 - **p(vertex)**: points to the parent of *vertex* in the disjoint-set structure
 - **owner(vertex)**: id of the process that was assigned this vertex
 - **local_root(vertex)**: classical find operation, with classical compression

### Inputs: 'task (r_x, r_y)'
 - **r_x**, **r_y**: current edges we compute on, we want to check if they are already in the same component
 - **p(r_y)**: if **r_y** is not owned by this process, this is the only way to get its parent
 - **(x, y)**: the edges that initialy started this request

### Assumptions
 - (a1) *owner(r_x) = process*
 - (a2) *owner(r_y) != process ==> owner(p(r_y)) != process*
   We can easily check this before sending a request and replace r_y with p(r_y) if necessary.
   Thus, we won't need to send p(r_y), but owner(r_y) will have to set it at retreival.

### Algorithm
```python
# We can localy move up as much as we want because:
r_x <- local_root(r_x)

# Here, we can localy work on this process as much as possible
if owner(r_x) == owner(r_y):
    r_y <- local_root(r_y)

# Basic cases, as we did the classical find operation, they could be unchecked and avoid a communication
if p(r_x) == p(r_y):
    return
if p(r_x) == r_y:
    p(r_x) <- p(r_y)
    return
if p(r_y) == r_x: # Can be removed if (a2) is implemented
    p(r_y) <- p(r_x)
    return

# Trying to move on the tree
if p(r_x) < p(r_y):
    if r_x == p(r_x):
        p(r_x) <- p(r_y)
        mark (x, y)
    else:
        z <- p[r_x]
        p(r_x) <- p(r_y)
        send the task (z, r_y) to owner(z)
else:
    if r_y == p(r_y):
        # ask owner(r_y) to set p(r_y) <- p(r_x)
        # mark (x, y)
        #  ... wouldn't be enough: p(r_y) could have been updated
        send the task (r_y, r_x) to owner(r_y)
    else:
        z <- p(r_y) # necessary if owner[r_y] == process
        ask owner(r_y) to set p(r_y) <- p(r_x)
        send the task (z, r_x) to owner(z)
```

### Messages
During the distributed phase, process can exchange two kind of messages.

#### Update
Ask to change a parent, if it offers an enhancement.
 - **node**: the node that needs a parental update
 - **val**: the id of the node's new parent
Thus *8 bytes* of data to send if 32 bits integers are used.

#### Task
Ask to take controll of a find-union operation.
 - **(x, y)**: the original edge we want to insert
 - **r_x**: a local vertex
 - **r_y**: an arbitrary vertex
 - **p(r_y)**: if r_y is not owned by current process, its parent
Thus *20 bytes* of data to send if 32 bits integers are used.
