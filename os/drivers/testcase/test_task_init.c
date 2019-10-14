/****************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <debug.h>
#include <sched.h>

#include <tinyara/sched.h>
#include <tinyara/kmalloc.h>

#include "sched/sched.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define TEST_STACK_SIZE (2048)
#define TEST_PRIORITY   (100)
#define TEST_TASK_NAME  ("test_taskinit")

/****************************************************************************
 * Global Variables
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int test_task_init(main_t entry)
{
	struct task_tcb_s *tcb;
	uint32_t *stack;
	int ret;

	tcb = (struct task_tcb_s *)kmm_zalloc(sizeof(struct task_tcb_s));
	if (!tcb) {
		berr("Failed: no memory for tcb\n");
		return -ENOMEM;
	}

	stack = (uint32_t *)kumm_malloc(TEST_STACK_SIZE);
	if (!stack) {
		berr("Failed: no memory for stack\n");
		ret = -ENOMEM;
		goto errout_with_tcb;
	}

	/* positive test */

	ret = task_init((struct tcb_s *)tcb, (const char *)TEST_TASK_NAME, (int)TEST_PRIORITY, stack, TEST_STACK_SIZE, entry, NULL);
	if (ret < 0) {
		berr("Failed: task_init %d\n", ret);
		ret = -get_errno();
		goto errout_with_stack;
	}

	/* Check the TCB values */

	if ((tcb->cmn.sched_priority != TEST_PRIORITY) || (tcb->cmn.stack_alloc_ptr != stack) || (tcb->cmn.entry.main != entry)) {
		berr("Failed: set values, %d, %x, %x %d\n", tcb->cmn.pid, tcb->cmn.stack_alloc_ptr, tcb->cmn.entry.main);
		ret = -ENXIO;
		goto errout_with_task;
	}

	ret = task_activate((FAR struct tcb_s *)tcb);
	if (ret < 0) {
		berr("Failed : task_activate() %d\n", ret);
		ret = -get_errno();
		goto errout_with_task;
	}

	return ret;

errout_with_task:
	sched_removeblocked((struct tcb_s *)tcb);
	sched_releasetcb(&tcb->cmn, TCB_FLAG_TTYPE_TASK);
	return ret;

errout_with_stack:
	kumm_free(stack);

errout_with_tcb:
	kmm_free(tcb);
	return ret;
}
