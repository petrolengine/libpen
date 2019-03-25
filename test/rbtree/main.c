#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>

#include "../../source/platform.h"

#include "../../3rd/rbtree.h"
#include "../../3rd/rbtree_augmented.h"

int nnodes = 5;
int perf_loops = 1;
int check_loops = 1;

struct test_node {
    uint32_t key;
    struct pen_rb_node rb;

    /* following fields used for testing augmented rbtree functionality */
    uint32_t val;
    uint32_t augmented;
};

static struct rb_root_cached root = RB_ROOT_CACHED;
static struct test_node *nodes = NULL;

static void insert(struct test_node *node, struct rb_root_cached *root)
{
    struct pen_rb_node **new = &root->rb_root.rb_node, *parent = NULL;
    uint32_t key = node->key;

    while (*new) {
        parent = *new;
        if (key < PEN_STRUCT_ENTRY(parent, struct test_node, rb)->key)
            new = &parent->rb_left;
        else
            new = &parent->rb_right;
    }

    rb_link_node(&node->rb, parent, new);
    rb_insert_color(&node->rb, &root->rb_root);
}

static void insert_cached(struct test_node *node, struct rb_root_cached *root)
{
    struct pen_rb_node **new = &root->rb_root.rb_node, *parent = NULL;
    uint32_t key = node->key;
    bool leftmost = true;

    while (*new) {
        parent = *new;
        if (key < PEN_STRUCT_ENTRY(parent, struct test_node, rb)->key)
            new = &parent->rb_left;
        else {
            new = &parent->rb_right;
            leftmost = false;
        }
    }

    rb_link_node(&node->rb, parent, new);
    rb_insert_color_cached(&node->rb, root, leftmost);
}

static inline void erase(struct test_node *node, struct rb_root_cached *root)
{
    rb_erase(&node->rb, &root->rb_root);
}

static inline void erase_cached(struct test_node *node, struct rb_root_cached *root)
{
    rb_erase_cached(&node->rb, root);
}


static inline uint32_t augment_recompute(struct test_node *node)
{
    uint32_t max = node->val, child_augmented;
    if (node->rb.rb_left) {
        child_augmented = PEN_STRUCT_ENTRY(node->rb.rb_left, struct test_node,
                       rb)->augmented;
        if (max < child_augmented)
            max = child_augmented;
    }
    if (node->rb.rb_right) {
        child_augmented = PEN_STRUCT_ENTRY(node->rb.rb_right, struct test_node,
                       rb)->augmented;
        if (max < child_augmented)
            max = child_augmented;
    }
    return max;
}

RB_DECLARE_CALLBACKS(static, augment_callbacks, struct test_node, rb,
             uint32_t, augmented, augment_recompute)

static void insert_augmented(struct test_node *node,
                 struct rb_root_cached *root)
{
    struct pen_rb_node **new = &root->rb_root.rb_node, *rb_parent = NULL;
    uint32_t key = node->key;
    uint32_t val = node->val;
    struct test_node *parent;

    while (*new) {
        rb_parent = *new;
        parent = PEN_STRUCT_ENTRY(rb_parent, struct test_node, rb);
        if (parent->augmented < val)
            parent->augmented = val;
        if (key < parent->key)
            new = &parent->rb.rb_left;
        else
            new = &parent->rb.rb_right;
    }

    node->augmented = val;
    rb_link_node(&node->rb, rb_parent, new);
    rb_insert_augmented(&node->rb, &root->rb_root, &augment_callbacks);
}

static void insert_augmented_cached(struct test_node *node,
                    struct rb_root_cached *root)
{
    struct pen_rb_node **new = &root->rb_root.rb_node, *rb_parent = NULL;
    uint32_t key = node->key;
    uint32_t val = node->val;
    struct test_node *parent;
    bool leftmost = true;

    while (*new) {
        rb_parent = *new;
        parent = PEN_STRUCT_ENTRY(rb_parent, struct test_node, rb);
        if (parent->augmented < val)
            parent->augmented = val;
        if (key < parent->key)
            new = &parent->rb.rb_left;
        else {
            new = &parent->rb.rb_right;
            leftmost = false;
        }
    }

    node->augmented = val;
    rb_link_node(&node->rb, rb_parent, new);
    rb_insert_augmented_cached(&node->rb, root,
                   leftmost, &augment_callbacks);
}


static void erase_augmented(struct test_node *node, struct rb_root_cached *root)
{
    rb_erase_augmented(&node->rb, &root->rb_root, &augment_callbacks);
}

static void erase_augmented_cached(struct test_node *node,
                   struct rb_root_cached *root)
{
    rb_erase_augmented_cached(&node->rb, root, &augment_callbacks);
}

static void init(void)
{
    int i;
    for (i = 0; i < nnodes; i++) {
        nodes[i].key = rand();
        nodes[i].val = rand();
    }
}

static bool is_red(struct pen_rb_node *rb)
{
    return !(rb->__rb_parent_color & 1);
}

static int black_path_count(struct pen_rb_node *rb)
{
    int count;
    for (count = 0; rb; rb = rb_parent(rb))
        count += !is_red(rb);
    return count;
}

static void check_postorder_foreach(int nr_nodes)
{
    struct test_node *cur, *n;
    int count = 0;
    rbtree_postorder_for_each_entry_safe(cur, n, &root.rb_root, rb)
        count++;

    assert(count == nr_nodes);
}

static void check_postorder(int nr_nodes)
{
    struct pen_rb_node *rb;
    int count = 0;
    for (rb = rb_first_postorder(&root.rb_root); rb; rb = rb_next_postorder(rb))
        count++;

    assert(count == nr_nodes);
}

static void check(int nr_nodes)
{
    struct pen_rb_node *rb;
    int count = 0, blacks = 0;
    uint32_t prev_key = 0;

    for (rb = rb_first(&root.rb_root); rb; rb = rb_next(rb)) {
        struct test_node *node = PEN_STRUCT_ENTRY(rb, struct test_node, rb);
        printf("%u %u\n", node->key, prev_key);
        assert(node->key >= prev_key);
        assert(!(is_red(rb) &&
                 (!rb_parent(rb) || is_red(rb_parent(rb)))));
        if (!count)
            blacks = black_path_count(rb);
        else
            assert(!((!rb->rb_left || !rb->rb_right) &&
                     blacks != black_path_count(rb)));
        prev_key = node->key;
        count++;
    }

    assert(count == nr_nodes);
    assert(count >= (1 << black_path_count(rb_last(&root.rb_root))) - 1);

    check_postorder(nr_nodes);
    check_postorder_foreach(nr_nodes);
}

static void check_augmented(int nr_nodes)
{
    struct pen_rb_node *rb;

    check(nr_nodes);
    for (rb = rb_first(&root.rb_root); rb; rb = rb_next(rb)) {
        struct test_node *node = PEN_STRUCT_ENTRY(rb, struct test_node, rb);
        assert(node->augmented == augment_recompute(node));
    }
}

static int rbtree_test_init(void)
{
    int i, j;
    time_t t1, t2, t;
    struct pen_rb_node *node;

    nodes = calloc(nnodes, sizeof(*nodes));
    if (!nodes)
        return -ENOMEM;

    printf("rbtree testing\n");

    init();

    t1 = time(NULL);

    for (i = 0; i < perf_loops; i++) {
        for (j = 0; j < nnodes; j++)
            insert(nodes + j, &root);
        for (j = 0; j < nnodes; j++)
            erase(nodes + j, &root);
    }

    t2 = time(NULL);
    t= t2 - t1;

    t= t / perf_loops;
    printf(" -> test 1 (latency of nnodes insert+delete): %llu s\n",
           (unsigned long long)t);

    t1 = time(NULL);

    for (i = 0; i < perf_loops; i++) {
        for (j = 0; j < nnodes; j++)
            insert_cached(nodes + j, &root);
        for (j = 0; j < nnodes; j++)
            erase_cached(nodes + j, &root);
    }

    t2 = time(NULL);
    t= t2 - t1;

    t= t / perf_loops;
    printf(" -> test 2 (latency of nnodes cached insert+delete): %llu cycles\n",
           (unsigned long long)t);

    for (i = 0; i < nnodes; i++)
        insert(nodes + i, &root);

    t1 = time(NULL);

    for (i = 0; i < perf_loops; i++) {
        for (node = rb_first(&root.rb_root); node; node = rb_next(node))
            ;
    }

    t2 = time(NULL);
    t= t2 - t1;

    t= t / perf_loops;
    printf(" -> test 3 (latency of inorder traversal): %llu cycles\n",
           (unsigned long long)t);

    t1 = time(NULL);

    for (i = 0; i < perf_loops; i++)
        node = rb_first(&root.rb_root);

    t2 = time(NULL);
    t= t2 - t1;

    t= t / perf_loops;
    printf(" -> test 4 (latency to fetch first node)\n");
    printf("        non-cached: %llu cycles\n", (unsigned long long)t);

    t1 = time(NULL);

    for (i = 0; i < perf_loops; i++)
        node = rb_first_cached(&root);

    t2 = time(NULL);
    t= t2 - t1;

    t= t / perf_loops;
    printf("        cached: %llu cycles\n", (unsigned long long)t);

    for (i = 0; i < nnodes; i++)
        erase(nodes + i, &root);

    /* run checks */
    for (i = 0; i < check_loops; i++) {
        init();
        for (j = 0; j < nnodes; j++) {
            check(j);
            insert(nodes + j, &root);
        }
        for (j = 0; j < nnodes; j++) {
            check(nnodes - j);
            erase(nodes + j, &root);
        }
        check(0);
    }

    printf("augmented rbtree testing\n");

    init();

    t1 = time(NULL);

    for (i = 0; i < perf_loops; i++) {
        for (j = 0; j < nnodes; j++)
            insert_augmented(nodes + j, &root);
        for (j = 0; j < nnodes; j++)
            erase_augmented(nodes + j, &root);
    }

    t2 = time(NULL);
    t= t2 - t1;

    t= t / perf_loops;
    printf(" -> test 1 (latency of nnodes insert+delete): %llu cycles\n", (unsigned long long)t);

    t1 = time(NULL);

    for (i = 0; i < perf_loops; i++) {
        for (j = 0; j < nnodes; j++)
            insert_augmented_cached(nodes + j, &root);
        for (j = 0; j < nnodes; j++)
            erase_augmented_cached(nodes + j, &root);
    }

    t2 = time(NULL);
    t= t2 - t1;

    t= t / perf_loops;
    printf(" -> test 2 (latency of nnodes cached insert+delete): %llu cycles\n", (unsigned long long)t);

    for (i = 0; i < check_loops; i++) {
        init();
        for (j = 0; j < nnodes; j++) {
            check_augmented(j);
            insert_augmented(nodes + j, &root);
        }
        for (j = 0; j < nnodes; j++) {
            check_augmented(nnodes - j);
            erase_augmented(nodes + j, &root);
        }
        check_augmented(0);
    }

    free(nodes);

    return -EAGAIN; /* Fail will directly unload the module */
}

static void rbtree_test_exit(void)
{
    printf("test exit\n");
}

int
main()
{
    rbtree_test_init();
    rbtree_test_exit();
    return 0;
}
