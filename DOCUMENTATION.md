# Librrb function API

The Librrb consists of three different function collections. The first two are
related to the RRB-tree and its transient version, whereas the last one is
debugging functions.

## RRB-tree Functions

All RRB-tree functions described here do not modify the RRB-tree in any way,
shape or form. Passing in any value will never modify the tree.

```c
const RRB* rrb_create(void)
```
Returns, in constant time, an immutable, empty RRB-Tree.

```c
uint32_t rrb_count(const RRB *rrb)
``` 
Returns, in constant time, the number of items in this RRB-Tree.

```c
void* rrb_nth(const RRB *rrb, uint32_t index)
```
Returns, in effectively constant time, the item at index `index`.

```c
const RRB* rrb_pop(const RRB *rrb)
```
Returns, in effectively constant time, a new RRB-Tree without the last item.

```c
void* rrb_peek(const RRB *rrb)
```
Returns, in constant time, the last item in this RRB-Tree.

```c
const RRB* rrb_push(const RRB *rrb, const void *elt)
```
Returns, in effectively constant time, a new RRB-Tree with `elt appended to the
end of the original RRB-Tree.

```c
const RRB* rrb_update(const RRB *rrb, uint32_t index, const void *elt)
```

Returns, in effectively constant time, a new RRB-Tree where the item at index
`index` is replaced by `elt`.

```c
const RRB* rrb_concat(const RRB *left, const RRB *right)
```
Returns, in O(log n) time, the concatenation of `left` `right` as a new
RRB-Tree.

```c
const RRB* rrb_slice(const RRB *rrb, uint32_t from, uint32_t to)
```
Returns, in effectively constant time, a new RRB-Tree which only contain the
items from index `from` to index `to` the original RRB-Tree.

## Transient Functions

Transient RRB-trees acts as defined in Chapter 3 in
[my master's thesis][thesis]. In the worst case, transient functions provide
same performance as their counterpart. Repeated use will in most cases increase
performance.

When a transient RRB-tree is *invalidated*, it cannot be used anymore (for
anything!). It is not legal to use a transient outside the thread it was created
in: The transients will check that the thread they are created in is where the
functions are called.

```c
TransientRRB* rrb_to_transient(const RRB *rrb)
```
Converts, in constant time, a persistent RRB-tree to its transient counterpart.
The persistent RRB-tree can still be used and will not be modified.

```c
const RRB* transient_to_rrb(TransientRRB *trrb)
```
Converts, in constant time, a transient RRB-tree to a persistent RRB-tree. The
transient RRB-tree  is *invalidated*.

```c
uint32_t transient_rrb_count(const TransientRRB *trrb)
```
Returns, in constant time, the number of elements in this transient RRB-tree.

```c
void* transient_rrb_nth(const TransientRRB *trrb, uint32_t index)
```
Returns, in effectively constant time, the item at index `index`.

```c
TransientRRB* transient_rrb_pop(TransientRRB *trrb)
```
Returns, in effectively constant time, a new transient RRB-tree without the last
item. The original transient RRB-tree is *invalidated*.

```c
transient_rrb_peek(const TransientRRB *trrb)
```
Returns, in constant time, the last element in this RRB-tree.

```c
TransientRRB* transient_rrb_push(TransientRRB *restrict trrb,
                                 const void *restrict elt)
```
Returns, in effectively constant time, a new transient RRB-Tree with `elt`
appended to the end of the original transient RRB-Tree. The original transient
RRB-tree is *invalidated*.

```c
TransientRRB* transient_rrb_update(TransientRRB *restrict trrb,
                                   uint32_t index, const void *restrict elt)
```
Returns, in effectively constant time, a new transient RRB-Tree where the item
at index `index` is replaced by `elt`. The original transient RRB-tree is
*invalidated*.

```c
TransientRRB* transient_rrb_slice(TransientRRB *trrb,
                                  uint32_t from, uint32_t to)
```

Returns, in effectively constant time, a new transient RRB-tree which only
contains the items from index `from` to index `to` in the original RRB-Tree. The
original transient RRB-tree is *invalidated*. It should be noted that this
function is not yet optimised, and is currently only a wrapper around
`rrb_slice` for completeness.


## Debugging Functions

Debugging functions have no performance guarantees, and may be slow. None of
these functions should be assumed to be thread safe. A dot file may be converted
to a graph through the Graphviz tool `dot`. For instance would `dot -Tpng -o
output.png input.do` convert the dot file `input.dot` to the png file
`output.png`.

Although the debugging functions are intented to be used for persistent
RRB-trees, they work completely fine on transient RRB-trees as well. Casting the
transient to a persistent RRB-tree by doing `(const RRB*)` should be sufficient
to avoid warnings. Note that unused slots in transient RRB-trees are not
visualised.


```c
int rrb_to_dot_file(const RRB *rrb, char *fname)
```

Writes a new file with the filename `fname` with a dot representation of the
RRB-Tree provided. If the file already exists, the original content is
overridden.

The total size of the file is returned in bytes, `-1` if `fopen` or `fclose`
returns an error, or `-2` if any `fprintf` call returns an error. `errno` is not
modified, so the error code will reside within this variable.

```c
DotFile dot_file_create(char *fname)
```
Creates and opens a DotFile. Note that a DotFile is passed by value, not by
pointer.

If the opening of the file, as done by `fopen`, errors, then `errno` is set as
by `fopen`.

```c
int dot_file_close(DotFile dot)
```
Writes and closes a DotFile to disk. Any call performed after the DotFile is
closed is considered an error.

The result of closing a DotFile is as `fclose`.

```c
int rrb_to_dot(DotFile dot, const RRB *rrb)
```
Writes the RRB-Tree to the dot file, if not already written. Shared structure
between RRB-Trees is visualised, with the exception of `NULL`/`nil` pointers.

Returns the amount of bytes written, or a negative number if any `fprintf` call
errors. The function will short-circuit if an error happens, and the `errno`
variable is not modified.

```c
int label_pointer(DotFile dot, const void *node, const char *name)
```
Labels a pointer (Usually an RRB-Tree pointer) in the dot file. It is legal with
multiple labels on a single pointer. If the pointer is `NULL`, the label will be
attached to the latest `NULL` pointer added to the dot file. Labelling pointers
not contained in the dot file is not an error, but will generate strange
visualisations.

Returns the amount of bytes written, or a negative number if any `fprintf` call
errors. The function will short-circuit if an error happens, and the `errno`
variable is not modified.

```c
uint32_t rrb_memory_usage(const RRB *const *rrbs, uint32_t rrb_count)
```
Calculates the expected memory used by `rrb_count` RRB-Trees, and takes into
account structural sharing between them.

```c
uint32_t validate_rrb(const RRB *rrb)
```
Validates the RRB-tree: Checks that it is consistent and satisfies properties an
RRB-tree should have. Prints out inconsistencies to stdout, and returns a
nonzero value if the RRB-tree is not on correct form. Theoretically only
needed by RRB-tree developers, but if you suspect there is a bug in the
RRB-tree, you can use this to verify this.

[thesis]: http://hypirion.com/thesis "Improving RRB-Tree Performance through Transience"
