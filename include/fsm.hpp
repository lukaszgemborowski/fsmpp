#include <tuple>
#include <meta/meta.hpp>

namespace fsm
{

namespace detail
{
struct EvEnterState {};
struct EvExitState {};

template<typename T>
struct state {
	T actual;

	template<typename E>
	void event(const E &event)
	{
		actual.event(event);
	}

	void event(const EvEnterState &)
	{
		actual.enter();
	}

	void event(const EvExitState &)
	{
		actual.exit();
	}
};


} // namespace detail

// defines one transition between two states, this is type triple:
// StartState, Event trigger, StopState
template<typename S1, typename E, typename S2>
struct transition
{
	using start_t = detail::state<S1>;
	using stop_t = detail::state<S2>;
	using event_t = E;
};

template<typename... Ts>
struct transitions
{
	using list = meta::list<Ts...>;

	struct transition_to_state {
		template<typename T>
		using invoke = typename T::start_t;
	};

	using states = meta::transform<list, transition_to_state>;
	using unique_states = meta::unique<states>;

	using states_tuple_t = meta::apply<meta::quote<std::tuple>, unique_states>;
	using states_count = std::tuple_size<states_tuple_t>;

	struct state_events_func {
		template<typename T>
		using invoke = meta::list<typename T::start_t, typename T::event_t>;
	};

	// create list of lists. This is StartState, Event extract from transitions
	using state_events = meta::transform<list, state_events_func>;
};

template<typename Trs>
struct state_instances
{
	typename Trs::states_tuple_t states;
};

template<typename Trs>
struct fsm
{
private:
	// helper templates all down to the public section
	using Transitions = Trs;

	// count all (S[tartState], E[vent]) pairs in provided list
	// this one is needed to determine if specified state S is handling event E
	template<typename S, typename E>
	using handle_event = meta::count<
		typename Transitions::state_events,
		meta::list<S, E>>;

	// meta-function for checking if provided T (struct transition) has the same S[tate] and E[vent]
	// as provided in template arguments
	template<typename S, typename E>
	struct check_dest {
		template<typename T>
		using invoke = meta::and_<std::is_same<S, typename T::start_t>, std::is_same<E, typename T::event_t>>;
	};

	// selecting transition matching StartState-Event criteria
	template<typename S, typename E>
	using destination = meta::front<meta::find_if<typename Transitions::list, check_dest<S, E>>>;

	// determining stop state from StartState, Event pair
	template<typename S, typename E>
	using destination_state = typename destination<S, E>::stop_t;

	template<typename I>
	using index_to_state = std::tuple_element_t<I::value, typename Transitions::states_tuple_t>;

	template<typename I, typename E>
	using next_state = destination_state<index_to_state<I>, E>;

	template<typename T>
	using state_to_index = meta::find_index<meta::as_list<typename Transitions::states_tuple_t>, T>;

	template<typename I, typename E>
	using next_state_index = state_to_index<next_state<I, E>>;

	template<typename I>
	using state_t_by_index = std::tuple_element_t<I::value, typename Transitions::states_tuple_t>;

public:
	fsm() :
		current (0)
	{
		std::get<0>(instances.states).event(detail::EvEnterState{});
	}

	// handle event E
	template<typename E>
	void on(const E &event)
	{
		onForAllImpl(event, current, meta::make_index_sequence<Transitions::states_count::value>{});
	}

private:
	template<typename E, std::size_t... Is>
	void onForAllImpl(const E &event, std::size_t atIdx, meta::index_sequence<Is...>)
	{
		int dummy[Transitions::states_count::value] = {
			(
				onImpl<E, std::integral_constant<std::size_t, Is>>(event, atIdx),
				0
			)...
		};
	}

	template<typename E, typename I>
	std::enable_if_t<handle_event<state_t_by_index<I>, E>::value == 1>
	onImpl(const E &event, std::size_t atIdx, std::tuple_element_t<I::value, typename Transitions::states_tuple_t>* = nullptr)
	{
		if (I::value == atIdx) {
			std::get<I::value>(instances.states).event(event);
			std::get<I::value>(instances.states).event(detail::EvExitState{});

			std::get<
				next_state_index<I, E>::value
			>(instances.states).event(detail::EvEnterState{});

			current = next_state_index<I, E>::value;
		}
	}

	template<typename E, typename I>
	std::enable_if_t<handle_event<state_t_by_index<I>, E>::value == 0>
	onImpl(const E &event, std::size_t atIdx, std::tuple_element_t<I::value, typename Transitions::states_tuple_t>* = nullptr)
	{
	}

private:
	state_instances<Trs> instances;
	int current;
};

} // namespace fsm
