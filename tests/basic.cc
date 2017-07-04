#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <fsm.hpp>
#include <iostream>

struct PowerOff {};
struct PowerOn {};
struct Reset {};
struct StartOs {};
struct Fail {};

template<std::size_t ID>
struct StateBase : public fsm::state<ID>
{
public:
	StateBase(int &resetCounter) : resetCounter_ (resetCounter) {}

	void inc() { resetCounter_ ++; }
	int resetCounter() const { return resetCounter_; }

private:
	int &resetCounter_;
};

struct State_Off : public StateBase<1>
{
	using StateBase::StateBase;

	void enter() { std::cout << "State_Off enter\n"; }
	void exit() { std::cout << "State_Off exit\n"; }

	bool event(const PowerOn &) { std::cout << "PowerOn event\n"; return true; }
};

struct State_Booting : public StateBase<2>
{
	using StateBase::StateBase;

	void enter() { std::cout << "State_Booting enter\n"; }
	void exit() { std::cout << "State_Booting exit\n"; }

	bool event(const StartOs &) { std::cout << "StartOs event\n"; return true; }
	bool event(const PowerOff &) { std::cout << "PowerOff event\n"; return true; }
	bool event(const Fail &) { std::cout << "Can't fail in Booting state\n"; return false; }
};

struct State_RunningOs : public StateBase<3>
{
	using StateBase::StateBase;

	void enter() { std::cout << "State_RunningOs enter\n"; }
	void exit() { std::cout << "State_RunningOs exit\n"; }

	bool event(const PowerOff &) { std::cout << "PowerOff event\n"; return true; }
	bool event(const Reset &) { std::cout << "Reset event\n"; inc(); return true; }
};

using all = fsm::transitions<
	// normal startup off -> booting -> os
	fsm::transition<State_Off    , PowerOn, State_Booting>,
	fsm::transition<State_Booting, StartOs, State_RunningOs>,

	// switching off transitions
	fsm::transition<State_Booting, PowerOff, State_Off>,
	fsm::transition<State_RunningOs, PowerOff, State_Off>,

	// can be reset only in OS
	fsm::transition<State_RunningOs, Reset, State_Booting>,

	fsm::transition<State_Booting, Fail, State_Off>
>;

int resetCounter = 0;
fsm::fsm<all, int> computer(resetCounter);

TEST_CASE("basic transitions", "[fsm]")
{
	std::cout << "Current state: " << computer.currentState() << std::endl;
	computer.on(PowerOn{});
	std::cout << "Current state: " << computer.currentState() << std::endl;
	computer.on(StartOs{});
	std::cout << "Current state: " << computer.currentState() << std::endl;
	computer.on(Reset{});
	std::cout << "Current state: " << computer.currentState() << std::endl;
	computer.on(StartOs{});
	std::cout << "Current state: " << computer.currentState() << std::endl;
	computer.on(Reset{});
	std::cout << "Current state: " << computer.currentState() << std::endl;
	REQUIRE(computer.on(Fail{}) == false);
	std::cout << "Current state: " << computer.currentState() << std::endl;
	computer.on(PowerOff{});
	std::cout << "Current state: " << computer.currentState() << std::endl;

	std::cout << "PC reset counter: " << resetCounter << std::endl;
}
