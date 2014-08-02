/*
 * Copyright (c) 2013-2014 Jean Niklas L'orange. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

// Embedded into rrb.c, so functions and structs used here may have been taken
// from there.

#include <stdio.h>
#include <stdarg.h>

typedef struct {
  uint32_t len;
  uint32_t cap;
  const void **elems;
} DotArray;

struct DotFile_ {
  FILE *file;
  DotArray *array;
};

static DotArray* dot_array_create(void);
static char dot_array_contains(const DotArray *arr, const void *elem);
static void dot_array_add(DotArray *arr, const void *elem);

static int tree_node_to_dot(DotFile dot, const TreeNode *node, char print_table);
static int leaf_node_to_dot(DotFile dot, const LeafNode *root);
static int internal_node_to_dot(DotFile dot, const InternalNode *root, char print_table);
static int size_table_to_dot(DotFile dot, const InternalNode *node);

static uint32_t node_size(DotArray *arr, const TreeNode *node);

// Dot Array impl

static DotArray* dot_array_create() {
  DotArray *arr = RRB_MALLOC(sizeof(DotArray));
  arr->len = 0;
  arr->cap = 32;
  arr->elems = RRB_MALLOC(arr->cap * sizeof(const void *));
  return arr;
}

static char dot_array_contains(const DotArray *arr, const void *elem) {
  for (uint32_t i = 0; i < arr->len; i++) {
    if (arr->elems[i] == elem) {
      return true;
    }
  }
  return false;
}

static void dot_array_add(DotArray *arr, const void *elem) {
  if (!dot_array_contains(arr, elem)) {
    // Grow array if needed
    if (arr->len == arr->cap) {
      arr->cap *= 2;
      arr->elems = RRB_REALLOC(arr->elems, arr->cap * sizeof(const void *));
    }
    arr->elems[arr->len] = elem;
    arr->len++;
  }
}

static int null_counter = 0;

// This is a hack to ensure sizes are of correct size: Assumes the variable t
// and sum is defined, and that both are set to 0.
#define SHORT_CIRCUIT(expr) do {                \
    t = expr;                                   \
    if (t < 0) {                                \
      return t;                                 \
    }                                           \
    sum += t; t = 0;                            \
  } while (0)                                   \


int label_pointer(DotFile dot, const void *node, const char *name) {
  int t, sum = 0;
  SHORT_CIRCUIT(fprintf(dot.file, "  \"%s\";\n", name));
  if (node == NULL) { // latest NIL node will be referred to.
    SHORT_CIRCUIT(fprintf(dot.file, "  \"%s\" -> s%d;\n", name, null_counter - 1));
  }
  else {
    SHORT_CIRCUIT(fprintf(dot.file, "  \"%s\" -> s%p;\n", name, node));
  }
  return sum;
}

static char dot_file_contains(const DotFile dot, const void *elem) {
  return dot_array_contains(dot.array, elem);
}

static void dot_file_add(DotFile dot, const void *elem) {
  dot_array_add(dot.array, elem);
}

DotFile dot_file_create_safely(char *loch, int* fprint_err) {
  FILE *file = fopen(loch, "w");
  if (file == NULL) {
    DotFile err = {.file = NULL, .array = NULL};
    return err;
  }
  DotArray *arr = dot_array_create();
  DotFile dot_file = {.file = file, .array = arr};
  *fprint_err = fprintf(file, "digraph g {\n  bgcolor=transparent;\n"
                        "  node [shape=none];\n");
  return dot_file;
}

DotFile dot_file_create(char *loch) {
  int ignore;
  return dot_file_create_safely(loch, &ignore);
}

int dot_file_close(DotFile dot) {
  int fail = 0;
  if (fprintf(dot.file, "}\n") < 0) {
    fail |= 1;
  }
  if (fclose(dot.file) != 0) {
    fail |= 2;
  }
  return -fail;
}

int rrb_to_dot_file(const RRB *rrb, char *loch) {
  DotFile dot = dot_file_create(loch);
  if (dot.file == NULL) {
    return -1;
  }
  int written = rrb_to_dot(dot, rrb);
  if (written < 0) {
    return -2;
  }
  if (dot_file_close(dot) != 0) {
    return -1;
  }
  return written;
}

int rrb_to_dot(DotFile dot, const RRB *rrb) {
  int t, sum = 0;
  if (!dot_file_contains(dot, rrb)) {
    dot_file_add(dot, rrb);
    SHORT_CIRCUIT(fprintf(dot.file,
            "  s%p [label=<\n<table border=\"0\" cellborder=\"1\" "
            "cellspacing=\"0\" cellpadding=\"6\" align=\"center\">\n"
            "  <tr>\n"
            "    <td height=\"36\" width=\"25\">%d</td>\n"
            "    <td height=\"36\" width=\"25\">%d</td>\n"
            "    <td height=\"36\" width=\"25\" port=\"root\"></td>\n"
            "    <td height=\"36\" width=\"25\">%d</td>\n"
            "    <td height=\"36\" width=\"25\" port=\"tail\"></td>\n"
            "  </tr>\n"
            "</table>>];\n",
                          rrb, rrb->cnt, rrb->shift, rrb->tail_len));
    if (rrb->tail == NULL) {
      SHORT_CIRCUIT(fprintf(dot.file, "  s%d [label=\"NIL\"];\n", null_counter));
      SHORT_CIRCUIT(fprintf(dot.file, "  s%p:tail -> s%d;\n", rrb, null_counter++));
    }
    else {
      SHORT_CIRCUIT(fprintf(dot.file, "  s%p:tail -> s%p:body;\n", rrb, rrb->tail));
      // Consider doing the rank=same thing.
      SHORT_CIRCUIT(leaf_node_to_dot(dot, rrb->tail));
    }
    if (rrb->root == NULL) {
      SHORT_CIRCUIT(fprintf(dot.file, "  s%p:root -> s%d;\n", rrb, null_counter));
    }
    else {
      SHORT_CIRCUIT(fprintf(dot.file, "  s%p:root -> s%p:body;\n", rrb, rrb->root));
    }
    SHORT_CIRCUIT(tree_node_to_dot(dot, rrb->root, true));
  }
  return sum;
}

static int tree_node_to_dot(DotFile dot, const TreeNode *root, char print_table) {
  if (root == NULL) {
    return fprintf(dot.file, "  s%d [label=\"NIL\"];\n",
                   null_counter++);
  }
  switch (root->type) {
  case LEAF_NODE:
    return leaf_node_to_dot(dot, (const LeafNode *) root);
  case INTERNAL_NODE:
    return internal_node_to_dot(dot, (const InternalNode *) root, print_table);
  }
}

static int internal_node_to_dot(DotFile dot, const InternalNode *root,
                                char print_table) {
  int t, sum = 0;
  if (!dot_file_contains(dot, root)) {
    dot_file_add(dot, root);
    SHORT_CIRCUIT(fprintf(dot.file,
            "  s%p [label=<\n<table border=\"0\" cellborder=\"1\" "
            "cellspacing=\"0\" cellpadding=\"6\" align=\"center\" port=\"body\">\n"
            "  <tr>\n"
            "    <td height=\"36\" width=\"25\" port=\"table\"></td>\n",
                          root));
    for (uint32_t i = 0; i < root->len; i++) {
      SHORT_CIRCUIT(fprintf(dot.file,
                           "    <td height=\"36\" width=\"25\" port=\"%d\">%d</td>\n",
                            i, i));
    }
    SHORT_CIRCUIT(fprintf(dot.file, "  </tr>\n</table>>];\n"));

    if (print_table) {
      if (root->size_table == NULL) {
        SHORT_CIRCUIT(size_table_to_dot(dot, root));
        SHORT_CIRCUIT(fprintf(dot.file, "  {rank=same; s%p; s%d;}\n",
                              root, null_counter - 1));
        SHORT_CIRCUIT(fprintf(dot.file, "  s%d -> s%p:table [dir=back];\n",
                              null_counter - 1, root));
      }
      else if (!dot_file_contains(dot, root->size_table)) {
        // Only do if size table isn't already placed
        // set rrb node and size table at same rank

        SHORT_CIRCUIT(fprintf(dot.file, "  {rank=same; s%p; s%p;}\n",
                              root, root->size_table));
        SHORT_CIRCUIT(size_table_to_dot(dot, root));
      }
      if (root->size_table != NULL) {
        // "Hack" to get nodes at correct position
        SHORT_CIRCUIT(fprintf(dot.file, "  s%p:last -> s%p:table [dir=back];\n",
                              root->size_table, root));
      }

    }

    for (uint32_t i = 0; i < root->len; i++) {
      if (root->child[i] == NULL) {
        SHORT_CIRCUIT(fprintf(dot.file, "  s%p:%d -> s%d;\n", root, i,
                              null_counter));
      }
      else {
        SHORT_CIRCUIT(fprintf(dot.file, "  s%p:%d -> s%p:body;\n", root, i,
                              root->child[i]));
      }
      SHORT_CIRCUIT(tree_node_to_dot(dot, (TreeNode *) root->child[i],
                                     print_table));
    }
  }
  return sum;
}


static int size_table_to_dot(DotFile dot, const InternalNode *node) {
  int t, sum = 0;
  if (node->size_table == NULL) {
    return fprintf(dot.file, "  s%d [color=indianred, label=\"NIL\"];\n",
                   null_counter++);
  }
  if (!dot_file_contains(dot, node->size_table)) {
    dot_file_add(dot, node->size_table);
    RRBSizeTable *table = node->size_table;
    SHORT_CIRCUIT(fprintf(dot.file,
            "  s%p [color=indianred, label=<\n"
            "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\" "
            "cellpadding=\"6\" align=\"center\" port=\"body\">\n"
            "  <tr>\n",
            table));
    for (uint32_t i = 0; i < node->len; i++) {
      int remaining_nodes = (i+1) < node->len;
      SHORT_CIRCUIT(
        fprintf(dot.file, "    <td height=\"36\" width=\"25\" %s>%d</td>\n",
                !remaining_nodes ? "port=\"last\"" : "",
                table->size[i]));
    }
    SHORT_CIRCUIT(fprintf(dot.file, "  </tr>\n</table>>];\n"));
  }
  return sum;
}

static int leaf_node_to_dot(DotFile dot, const LeafNode *root) {
  int t, sum = 0;
  if (!dot_file_contains(dot, root)) {
    dot_file_add(dot, root);
    SHORT_CIRCUIT(fprintf(dot.file,
            "  s%p [color=darkolivegreen3, label=<\n"
            "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\" "
            "cellpadding=\"6\" align=\"center\" port=\"body\">\n"
            "  <tr>\n",
            root));
    for (uint32_t i = 0; i < root->len; i++) {
      uintptr_t leaf = (uintptr_t) root->child[i];
      SHORT_CIRCUIT(
        fprintf(dot.file, "    <td height=\"36\" width=\"25\">%lx</td>\n",
                leaf));
    }
    SHORT_CIRCUIT(
      fprintf(dot.file, "  </tr>\n</table>>];\n"));
  }
  return sum;
}

static uint32_t node_size(DotArray *set, const TreeNode *root) {
  if (root == NULL || dot_array_contains(set, (const void *) root)) {
    return 0;
  }
  dot_array_add(set, (const void *) root);
  switch (root->type) {
  case LEAF_NODE: {
    const LeafNode *leaf = (const LeafNode *) root;
    return sizeof(LeafNode) + sizeof(void *) * leaf->len;
  }
  case INTERNAL_NODE: {
    const InternalNode *internal = (const InternalNode *) root;
    uint32_t size_table_bytes = 0;
    if (internal->size_table != NULL) {
      size_table_bytes = sizeof(uint32_t) * internal->len;
    }
    uint32_t node_bytes = sizeof(InternalNode) + size_table_bytes
                        + sizeof(struct InternalNode *) * internal->len;
    for (uint32_t i = 0; i < internal->len; i++) {
      node_bytes += node_size(set, (const TreeNode *) internal->child[i]);
    }
    return node_bytes;
  }
  }
}

// Need to use GCC for using this. Clang cannot handle vararg -g-symbols
// correctly (yet)
void nodes_to_dot_file(char *loch, int ncount, ...) {
  va_list nodes;
  DotFile dot = dot_file_create(loch);

  va_start(nodes, ncount);
  for (int i = 0; i < ncount; i++) {
    tree_node_to_dot(dot, va_arg(nodes, TreeNode*), true);
  }
  va_end(nodes);
  dot_file_close(dot);
}

static void validate_subtree(const TreeNode *root, uint32_t expected_size,
                             uint32_t root_shift, uint32_t *fail) {
  if (root_shift == LEAF_NODE_SHIFT) { // leaf node
    if (root->type != LEAF_NODE) {
      puts("Expected this node to be a leaf node, but it claims to be "
           "something else.\n Will treat it like a leaf node, "
           "so may segfault.");
      *fail = 1;
    }
    const LeafNode *leaf = (const LeafNode *) root;
    if (leaf->len != expected_size) {
      printf("Leaf node claims to be %u elements long, but was expected to be "
             "%u\n elements long. Will attempt to read %u elements.\n",
             leaf->len, expected_size, MAX(leaf->len, expected_size));
      *fail = 1;
    }
    uintptr_t c = 0;
    // dummy counter to avoid optimization at lower levels (although probably
    // unneccesary). Note that this will probably be filtered out with -O2 and
    // higher, so run with '-O0 -g'.
    for (uint32_t i = 0; i < MAX(leaf->len, expected_size); i++) {
      c += (uintptr_t) leaf->child[i];
    }
  }
  else {
    if (root->type != INTERNAL_NODE) {
      puts("Expected this node to be an internal node, but it claims to be "
           "something else.\n Will treat it like an internal node, "
           "so may segfault.");
      *fail = 1;
    }
    const InternalNode *internal = (const InternalNode *) root;
    if (internal->size_table != NULL) {
      // expected size should be consistent with what's in the last size table
      // slot
      if (internal->size_table->size[internal->len-1] != expected_size) {
        printf("Expected subtree to be of size %u, but its size table says it "
               "is %u.\n", expected_size,
               internal->size_table->size[internal->len-1]);
        *fail = 1;
      }
      for (uint32_t i = 0; i < internal->len; i++) {
        uint32_t size_sub_trie = internal->size_table->size[i]
                               - (i == 0 ? 0 : internal->size_table->size[i-1]);
        validate_subtree((const TreeNode *) internal->child[i], size_sub_trie,
                         DEC_SHIFT(root_shift), fail);
      }
    }
    else { // internal->size_table == NULL
      // this tree may contain at most (internal->len << shift) elements, not
      // more. Effectively, the tree contains (len - 1) << shift + last_tree_len
      // (1 << shift) >= last_tree_len > 0
      const uint32_t child_shift = DEC_SHIFT(root_shift);
      const uint32_t child_max_size = 1 << root_shift;

      if (expected_size > internal->len * child_max_size) {
        printf("Expected size (%u) is larger than what can possibly be inside "
               "this subtree: %u.\n", expected_size,
               internal->len * child_max_size);
        *fail = 1;
      }
      else if (expected_size < ((internal->len - 1) * child_max_size)) {
        printf("Expected size (%u) is smaller than %u, implying that some "
               "non-rightmost node\n is not completely populated.\n",
               expected_size, ((internal->len - 1) << root_shift));
        *fail = 1;
      }
      for (uint32_t i = 0; i < internal->len - 1; i++) {
        validate_subtree((const TreeNode *) internal->child[i], child_max_size,
                         child_shift, fail);
      }
      validate_subtree((const TreeNode *) internal->child[internal->len - 1],
                       expected_size - ((internal->len - 1) * child_max_size),
                       child_shift, fail);
    }
  }
}

uint32_t validate_rrb(const RRB *rrb) {
  // ensure the rrb tree is consistent
  uint32_t fail = 0;
  // the rrb tree should always have a tail
  if (rrb->tail->len != rrb->tail_len) {
    fail = 1;
    printf("The tail of this rrb-tree says it is of length %u, but the rrb head"
           "claims it\nis %u elements long.", rrb->tail->len, rrb->tail_len);
  }
  else {
    validate_subtree((TreeNode *) rrb->tail, rrb->tail_len, LEAF_NODE_SHIFT,
                     &fail);
  }
  if (rrb->root == NULL) {
    if (rrb->cnt - rrb->tail_len != 0) {
      printf("Root is null, but the size of the vector "
             "(excluding its tail) is %u.\n",
             rrb->cnt - rrb->tail_len);
      fail = 1;
    }
  }
  else {
    validate_subtree(rrb->root, rrb->cnt - rrb->tail_len,
                      rrb->shift, &fail);
  }
  return fail;
}

uint32_t rrb_memory_usage(const RRB *const *rrbs, uint32_t rrb_count) {
  DotArray *set = dot_array_create();
  uint32_t sum = 0;
  for (uint32_t i = 0; i < rrb_count; i++) {
    if (!dot_array_contains(set, (const void *) rrbs[i])) {
      dot_array_add(set, (const void *) rrbs[i]);
      sum += sizeof(RRB) + node_size(set, rrbs[i]->root);
      sum += node_size(set, rrbs[i]->tail);
    }
  }
  return sum;
}
