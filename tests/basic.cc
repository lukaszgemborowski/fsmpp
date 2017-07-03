#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <fsm.hpp>
#include <iostream>

struct PowerOff {};
struct PowerOn {};
struct Reset {};
struct StartOs {};

struct State_Off
{
	void enter() { std::cout << "State_Off enter\n"; }
	void exit() { std::cout << "State_Off exit\n"; }

	void event(const PowerOn &) { std::cout << "PowerOn event\n"; }
};

struct State_Booting
{
	void enter() { std::cout << "State_Booting enter\n"; }
	void exit() { std::cout << "State_Booting exit\n"; }

	void event(const StartOs &) { std::cout << "StartOs event\n"; }
	void event(const PowerOff &) { std::cout << "PowerOff event\n"; }
};

struct State_RunningOs
{
	void enter() { std::cout << "State_RunningOs enter\n"; }
	void exit() { std::cout << "State_RunningOs exit\n"; }

	void event(const PowerOff &) { std::cout << "PowerOff event\n"; }
	void event(const Reset &) { std::cout << "Reset event\n"; }
};

using all = fsm::transitions<
	// normal startup off -> booting -> os
	fsm::transition<State_Off    , PowerOn, State_Booting>,
	fsm::transition<State_Booting, StartOs, State_RunningOs>,

	// switching off transitions
	fsm::transition<State_Booting, PowerOff, State_Off>,
	fsm::transition<State_RunningOs, PowerOff, State_Off>,

	// can be reset only in OS
	fsm::transition<State_RunningOs, Reset, State_Booting>
>;

fsm::fsm<all> computer;

TEST_CASE("basic transitions", "[fsm]")
{
	computer.on(PowerOn{});
	computer.on(StartOs{});
	computer.on(Reset{});
	computer.on(StartOs{});
	computer.on(PowerOff{});
}
