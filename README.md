# fsmpp

**still beta ;-)**

fsmpp is small header only library with simple state machine implementation. Goals:

* small, header only
* almost no dependencies (only [meta](https://github.com/ericniebler/meta) header library)
* no dynamic allocations (states are instantiated in std::tuple)

## Example
```c++
#include <fsm.hpp>

/* Define an Event to trigger state machine transition */
struct EventStep {};

/* Starting state, StateA with ID 1 */
struct StateA : public fsm::state<1>
{
	/* Each state has to have enter() and exit() methods
	 * called when entering or leaving the state */
	void enter() {}
	void exit() {}

	/* If there is transition from this state triggered by
	 * some particular event you need to create event() method
	 * with argument of Event type. Otherwise you will get
	 * compile time error. */
	bool event(const EventStep &)
	{
		/* return true to allow transition, false otherwise */
		return true;
	}
};

/* Last state, StateB with ID 2 */
struct StateB : public fsm::state<2>
{
	/* as before, enter() and exit() methods but no event()
	 * method because there won't be any outgoing transitions
	 * from this state. */
	void enter() {}
	void exit() {}
};

/* Simple state transition table, each element of
 * fsm::transitions describe one transition in form of:
 * transition<StartState, Trigger, EndState>. */ 
using transition_table = fsm::transitions<
	/* in this case there is only one transition, from
	 * StateA to StateB triggered by EventStep */
	fsm::transition<StateA, EventStep, StateB>
>;

void usage() {
	/* create instance of state machine with predefined
	 * transition table */
	fsm::fsm<transition_table> sm;

	assert(sm.currentState() == 1);
	sm.on(EventStep {});
	assert(sm.currentState() == 2);
	sm.on(EventStep {});
	assert(sm.currentState() == 2);
}

```
