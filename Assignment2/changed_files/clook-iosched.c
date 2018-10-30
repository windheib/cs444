/*
 * elevator clook
 * based on noop_iosched.c
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

// Create struct with struct list_head in it 
struct clook_data {
	struct list_head queue;
};



// takes: request_queue pointer, request pointer and pointer to next request

static void clook_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}



// EDIT THIS FUNCTION
static int clook_dispatch(struct request_queue *q, int force)
{
	struct clook_data *nd = q->elevator->elevator_data; // nd points to request_queue's elevator data 

	// if the queue is not empty then 
	if (!list_empty(&nd->queue)) { 
		struct request *rq; // Create request pointer
		rq = list_entry(nd->queue.next, struct request, queuelist); // rq points to the request that is in the queue  
		list_del_init(&rq->queuelist); 
		
		/*
		* Insert rq into dispatch queue of q.  Queue lock must be held on
		* entry.  rq is sort instead into the dispatch queue. To be used by
		* specific elevators.
		*/
		printk(KERN_DEBUG "CLOOK: Dispatching in sector %llu \n", blk_rq_pos(rq));
		elv_dispatch_sort(q, rq);

		return 1;
	}
	printk(KERN_DEBUG "CLOOK: Queue is empty! \n");
	return 0;
}



// EDIT THIS FUNCTION
// Adds the request to the queue in the correct location
// TAKES: pointer to the request queue and a request pointer

static void clook_add_request(struct request_queue *q, struct request *rq)
{
	// nd is a clook_data pointer to the elevator data
	// Gets the data structure for the elevator 
	struct clook_data *nd = q->elevator->elevator_data;

	struct list_head *c_location = NULL;

	list_for_each(c_location, &nd->queue)
	{
		/* Iterate through requests and breaks when the current request has a higher
		* sector number than the request to be added or when reaching the end of the queue 
		*/
		if(rq_end_sector(list_entry(c_location, struct request, queuelist)) > rq_end_sector(rq))
		{
			break;
		}
	}
	// Adds the new request right before c_location in the queue
	list_add_tail(&rq->queuelist, &nd->queue); 

}



static struct request *
clook_former_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}


// Returns a request pointer 
static struct request *
clook_latter_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}


// Create elevator queue
static int clook_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct clook_data *nd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;

	INIT_LIST_HEAD(&nd->queue);

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	return 0;
}


// 
static void clook_exit_queue(struct elevator_queue *e)
{
	struct clook_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_clook = {
	.ops = {
		.elevator_merge_req_fn		= clook_merged_requests,
		.elevator_dispatch_fn		= clook_dispatch,
		.elevator_add_req_fn		= clook_add_request,
		.elevator_former_req_fn		= clook_former_request,
		.elevator_latter_req_fn		= clook_latter_request,
		.elevator_init_fn		= clook_init_queue,
		.elevator_exit_fn		= clook_exit_queue,
	},
	.elevator_name = "clook",
	.elevator_owner = THIS_MODULE,
};

static int __init clook_init(void)
{
	return elv_register(&elevator_clook);
}

static void __exit clook_exit(void)
{
	elv_unregister(&elevator_clook);
}

module_init(clook_init);
module_exit(clook_exit);


MODULE_AUTHOR("Jens Axboe");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("clook IO scheduler");
