#include <fsm.hpp>
#include "catch.hpp"

namespace
{

/* For details see simplest.cc */
struct Event {};

/* This is our Context shared between states */
struct Context {
	int state_b_count = 0;
};

struct StateA : public fsm::state<1>
{
	StateA(Context &) {}

	void enter() {}
	void exit() {}

	bool event(const Event &) { return true; }
};

struct StateB : public fsm::state<2>
{
	StateB(Context &ctx) : ctx_(ctx) {}
	void enter() 
	{
		ctx_.state_b_count ++;
	}

	void exit() {}

	Context &ctx_;
};
}

TEST_CASE("State machine with context", "[fsm]")
{
	using transition_table = fsm::transitions<
		fsm::transition<StateA, Event, StateB>
	>;

	Context ctx;
	fsm::fsm<transition_table, Context> sm(ctx);

	REQUIRE(sm.currentState() == 1);
	REQUIRE(ctx.state_b_count == 0);
	sm.on(Event{});

	REQUIRE(sm.currentState() == 2);
	REQUIRE(ctx.state_b_count == 1);

	sm.on(Event{});

	REQUIRE(sm.currentState() == 2);
	REQUIRE(ctx.state_b_count == 1);
}
