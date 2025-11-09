/* test_heap.c: Test program for the heap implementation with threads */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../heap.h"     // Changed: Add ../ to go up one directory
#include "../thread.h"   // Changed: Add ../
#include "../defs.h"     // Changed: Add ../
/* Test result tracking */
typedef struct {
    int total;
    int passed;
    int failed;
} test_results_t;

static test_results_t test_results = {0, 0, 0};

#ifdef KERNEL_MODE
#undef KERNEL_MODE
#endif

#define TEST_START(name) \
    do { \
        printf("\n=== TEST: %s ===\n", name); \
        test_results.total++; \
    } while(0)

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  ✓ PASS: %s\n", message); \
        } else { \
            printf("  ✗ FAIL: %s\n", message); \
            test_results.failed++; \
            return 0; \
        } \
    } while(0)

#define TEST_END() \
    do { \
        test_results.passed++; \
        printf("  TEST PASSED\n"); \
        return 1; \
    } while(0)

/* Helper function to create a dummy TCB */
tcb* create_dummy_thread(int tid, priority_t priority, uint32_t period, uint32_t exec_time) {
    tcb* thread = (tcb*)malloc(sizeof(tcb));
    if (!thread) return NULL;

    memset(thread, 0, sizeof(tcb));
    thread->tid = tid;
    thread->priority = priority;
    thread->period = period;
    thread->execution_time = exec_time;
    thread->remaining_time = exec_time;
    thread->state = THREAD_READY;
    thread->is_periodic = (period > 0);
    thread->max_jobs = -1; // infinite by default
    thread->jobs_done = 0;
    thread->next_arrival = 0;

    return thread;
}

/* Test 1: Basic heap creation and destruction */
int test_heap_create_destroy() {
    TEST_START("Heap Create/Destroy");

    thread_heap_t *h = heap_create(0);
    TEST_ASSERT(h != NULL, "Heap creation with default capacity");
    TEST_ASSERT(h->size == 0, "Initial size is 0");
    TEST_ASSERT(h->capacity >= 8, "Default capacity is at least 8");

    heap_destroy(h);
    printf("  ✓ Heap destroyed successfully\n");

    // Test with specific capacity
    h = heap_create(16);
    TEST_ASSERT(h != NULL, "Heap creation with capacity 16");
    TEST_ASSERT(h->capacity == 16, "Capacity set correctly");
    heap_destroy(h);

    TEST_END();
}

/* Test 2: Thread insertion and min-heap property */
int test_thread_heap_operations() {
    TEST_START("Thread Heap Operations (Min-Heap Property)");

    thread_heap_t *h = heap_create(0);
    TEST_ASSERT(h != NULL, "Heap created");

    // Test empty heap
    heap_node_t node;
    TEST_ASSERT(heap_peek(h, &node) == -1, "Peek on empty heap returns -1");
    TEST_ASSERT(heap_remove(h, &node) == -1, "Remove on empty heap returns -1");
    TEST_ASSERT(heap_size(h) == 0, "Empty heap size is 0");
    TEST_ASSERT(heap_is_empty(h) == 1, "heap_is_empty returns 1 for empty heap");

    // Create threads with different priorities
    tcb* t1 = create_dummy_thread(1, 50, 50, 10);
    tcb* t2 = create_dummy_thread(2, 30, 30, 5);
    tcb* t3 = create_dummy_thread(3, 80, 80, 15);
    tcb* t4 = create_dummy_thread(4, 10, 10, 2);  // Highest priority
    tcb* t5 = create_dummy_thread(5, 20, 20, 4);

    TEST_ASSERT(t1 && t2 && t3 && t4 && t5, "All threads created");

    // Insert threads
    printf("  Inserting threads with priorities: 50, 30, 80, 10, 20\n");
    TEST_ASSERT(heap_insert(h, t1) == 0, "Insert thread 1 (priority 50)");
    TEST_ASSERT(heap_insert(h, t2) == 0, "Insert thread 2 (priority 30)");
    TEST_ASSERT(heap_insert(h, t3) == 0, "Insert thread 3 (priority 80)");
    TEST_ASSERT(heap_insert(h, t4) == 0, "Insert thread 4 (priority 10)");
    TEST_ASSERT(heap_insert(h, t5) == 0, "Insert thread 5 (priority 20)");

    TEST_ASSERT(heap_size(h) == 5, "Heap size is 5");
    TEST_ASSERT(heap_is_empty(h) == 0, "heap_is_empty returns 0 for non-empty heap");

    // Peek at highest priority thread
    int peek_result = heap_peek(h, &node);
    TEST_ASSERT(peek_result == 0, "Peek returns 0 on success");
    TEST_ASSERT(node.tcb != NULL, "Peek returns a valid thread");
    TEST_ASSERT(node.priority == 10, "Highest priority thread (10) at root");
    TEST_ASSERT(heap_size(h) == 5, "Peek doesn't remove element");

    // Print heap state
    printf("  Current heap state:\n");
    heap_print(h);

    // Remove threads and verify they come out in priority order
    printf("\n  Removing threads in priority order:\n");
    priority_t expected_order[] = {10, 20, 30, 50, 80};
    for (int i = 0; i < 5; i++) {
        heap_node_t removed_node;
        int result = heap_remove(h, &removed_node);
        TEST_ASSERT(result == 0, "Remove returns 0 on success");
        TEST_ASSERT(removed_node.tcb != NULL, "Removed node contains valid thread");
        printf("    Removed: TID=%d, Priority=%d\n", removed_node.tcb->tid, removed_node.priority);
        TEST_ASSERT(removed_node.priority == expected_order[i], "Thread removed in correct priority order");
    }

    TEST_ASSERT(heap_size(h) == 0, "Heap empty after all removals");
    TEST_ASSERT(heap_is_empty(h) == 1, "heap_is_empty returns 1");

    // Cleanup
    free(t1); free(t2); free(t3); free(t4); free(t5);
    heap_destroy(h);

    TEST_END();
}

/* Test 3: Rate-Monotonic Scheduling Simulation */
int test_rate_monotonic_scheduling() {
    TEST_START("Rate-Monotonic Scheduling Simulation");

    thread_heap_t *ready_queue = heap_create(0);
    TEST_ASSERT(ready_queue != NULL, "Ready queue created");

    printf("\n  Rate-Monotonic: Shorter period = Higher priority\n");
    printf("  Creating threads:\n");

    // Create threads with periods as priorities (RM: priority = period)
    tcb* threads[5];
    threads[0] = create_dummy_thread(100, 100, 100, 20);  // T=100ms, C=20ms
    threads[1] = create_dummy_thread(101, 50, 50, 10);    // T=50ms, C=10ms (higher priority)
    threads[2] = create_dummy_thread(102, 25, 25, 5);     // T=25ms, C=5ms (highest priority)
    threads[3] = create_dummy_thread(103, 75, 75, 15);    // T=75ms, C=15ms
    threads[4] = create_dummy_thread(104, 200, 200, 40);  // T=200ms, C=40ms (lowest priority)

    for (int i = 0; i < 5; i++) {
        TEST_ASSERT(threads[i] != NULL, "Thread created");
        printf("    Thread %d: TID=%d, Period=%u, Priority=%d\n",
               i, threads[i]->tid, threads[i]->period, threads[i]->priority);
    }

    // Insert all threads into ready queue
    printf("\n  Adding threads to ready queue...\n");
    for (int i = 0; i < 5; i++) {
        heap_insert(ready_queue, threads[i]);
    }

    printf("  Ready queue state:\n");
    heap_print(ready_queue);

    // Simulate scheduler: execute threads in priority order
    printf("\n  Scheduler execution order:\n");
    priority_t expected[] = {25, 50, 75, 100, 200};
    int idx = 0;
    while (!heap_is_empty(ready_queue)) {
        heap_node_t current_node;
        int result = heap_remove(ready_queue, &current_node);
        TEST_ASSERT(result == 0, "Dequeue successful");
        TEST_ASSERT(current_node.tcb != NULL, "Dequeued thread is not NULL");
        printf("    Execute: TID=%d, Priority=%d, Period=%u\n",
               current_node.tcb->tid, current_node.priority, current_node.tcb->period);
        TEST_ASSERT(current_node.priority == expected[idx], "Correct execution order");
        idx++;
    }

    // Cleanup
    for (int i = 0; i < 5; i++) {
        free(threads[i]);
    }
    heap_destroy(ready_queue);

    TEST_END();
}

/* Test 4: Heap resizing with threads */
int test_heap_resize_with_threads() {
    TEST_START("Heap Dynamic Resizing with Threads");

    thread_heap_t *h = heap_create(2);
    TEST_ASSERT(h != NULL, "Heap created with capacity 2");
    TEST_ASSERT(h->capacity == 2, "Initial capacity is 2");

    // Insert more than capacity to trigger resize
    printf("  Inserting 10 threads to trigger resize...\n");
    tcb* threads[10];
    for (int i = 0; i < 10; i++) {
        threads[i] = create_dummy_thread(i, (i + 1) * 10, (i + 1) * 10, i + 1);
        TEST_ASSERT(heap_insert(h, threads[i]) == 0, "Insert succeeded (with resize)");
    }

    TEST_ASSERT(h->capacity > 2, "Heap capacity increased");
    TEST_ASSERT(heap_size(h) == 10, "All 10 threads inserted");
    printf("  After insertions: size=%zu, capacity=%zu\n", heap_size(h), h->capacity);

    // Remove most elements to trigger shrink
    printf("  Removing 8 threads to trigger shrink...\n");
    heap_node_t removed_node;
    for (int i = 0; i < 8; i++) {
        int result = heap_remove(h, &removed_node);
        TEST_ASSERT(result == 0, "Remove succeeded");
    }

    printf("  After removals: size=%zu, capacity=%zu\n", heap_size(h), h->capacity);
    TEST_ASSERT(heap_size(h) == 2, "2 threads remaining");

    // Cleanup
    for (int i = 0; i < 10; i++) {
        free(threads[i]);
    }
    heap_destroy(h);

    TEST_END();
}

/* Test 5: Edge cases */
int test_edge_cases() {
    TEST_START("Edge Cases");

    // Test NULL heap operations
    TEST_ASSERT(heap_insert(NULL, NULL) == -1, "Insert to NULL heap returns -1");

    heap_node_t node;
    TEST_ASSERT(heap_peek(NULL, &node) == -1, "Peek NULL heap returns -1");
    TEST_ASSERT(heap_remove(NULL, &node) == -1, "Remove from NULL heap returns -1");
    TEST_ASSERT(heap_size(NULL) == 0, "Size of NULL heap is 0");
    TEST_ASSERT(heap_is_empty(NULL) == 1, "NULL heap is empty");

    // Test inserting NULL thread
    thread_heap_t *h = heap_create(0);
    TEST_ASSERT(heap_insert(h, NULL) == -1, "Insert NULL thread returns -1");

    // Test same priority threads
    printf("  Testing threads with same priority...\n");
    tcb* t1 = create_dummy_thread(1, 50, 50, 10);
    tcb* t2 = create_dummy_thread(2, 50, 50, 15);
    tcb* t3 = create_dummy_thread(3, 50, 50, 20);

    heap_insert(h, t1);
    heap_insert(h, t2);
    heap_insert(h, t3);

    TEST_ASSERT(heap_size(h) == 3, "All same-priority threads inserted");

    heap_node_t r1, r2, r3;
    heap_remove(h, &r1);
    heap_remove(h, &r2);
    heap_remove(h, &r3);

    TEST_ASSERT(r1.tcb && r2.tcb && r3.tcb, "All threads removed successfully");
    TEST_ASSERT(r1.priority == 50 && r2.priority == 50 && r3.priority == 50,
                "All removed threads have same priority");

    free(t1); free(t2); free(t3);
    heap_destroy(h);

    TEST_END();
}

/* Test 6: Peek without removing */
int test_peek_behavior() {
    TEST_START("Peek Behavior (Non-Destructive)");

    thread_heap_t *h = heap_create(0);

    tcb* t1 = create_dummy_thread(1, 30, 30, 5);
    tcb* t2 = create_dummy_thread(2, 10, 10, 2);
    tcb* t3 = create_dummy_thread(3, 20, 20, 3);

    heap_insert(h, t1);
    heap_insert(h, t2);
    heap_insert(h, t3);

    // Peek multiple times
    heap_node_t peeked;
    for (int i = 0; i < 3; i++) {
        int result = heap_peek(h, &peeked);
        TEST_ASSERT(result == 0, "Peek succeeded");
        TEST_ASSERT(peeked.priority == 10, "Always returns highest priority (10)");
        TEST_ASSERT(heap_size(h) == 3, "Size unchanged after peek");
    }

    // Now remove and verify it's the same thread
    heap_node_t removed;
    heap_remove(h, &removed);
    TEST_ASSERT(removed.priority == 10, "Removed thread matches peeked thread");
    TEST_ASSERT(removed.tcb->tid == peeked.tcb->tid, "Same thread TID");

    free(t1); free(t2); free(t3);
    heap_destroy(h);

    TEST_END();
}

/* Print test summary */
void print_test_summary() {
    printf("\n");
    printf("=====================================\n");
    printf("       TEST SUMMARY\n");
    printf("=====================================\n");
    printf("Total Tests:  %d\n", test_results.total);
    printf("Passed:       %d\n", test_results.passed);
    printf("Failed:       %d\n", test_results.failed);
    printf("Success Rate: %.1f%%\n",
           test_results.total > 0 ? (100.0 * test_results.passed / test_results.total) : 0.0);
    printf("=====================================\n");

    if (test_results.failed == 0) {
        printf("🎉 ALL TESTS PASSED! 🎉\n");
    } else {
        printf("⚠️  SOME TESTS FAILED ⚠️\n");
    }
}

int main(void) {
    printf("=====================================\n");
    printf("  THREAD HEAP IMPLEMENTATION TEST\n");
    printf("  (Min-Heap Priority Queue)\n");
    printf("=====================================\n");

    // Run all tests
    test_heap_create_destroy();
    test_thread_heap_operations();
    test_rate_monotonic_scheduling();
    test_heap_resize_with_threads();
    test_edge_cases();
    test_peek_behavior();

    // Print summary
    print_test_summary();

    return (test_results.failed == 0) ? 0 : 1;
}