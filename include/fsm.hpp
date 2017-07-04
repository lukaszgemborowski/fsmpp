#include <tuple>
#include <meta/meta.hpp>

namespace fsm
{

namespace detail
{
struct null_context {};

} // namespace detail

// defines one transition between two states, this is type triple:
// StartState, Event trigger, StopState
template<typename S1, typename E, typename S2>
struct transition
{
	using start_t = S1;
	using stop_t = S2;
	using event_t = E;
};

template<typename... Ts>
struct transitions
{
	using list = meta::list<Ts...>;

	struct start_states_selector {
		template<typename T>
		using invoke = typename T::start_t;
	};

	struct stop_states_selector {
		template<typename T>
		using invoke = typename T::stop_t;
	};

	using start_states = meta::transform<list, start_states_selector>;
	using stop_states = meta::transform<list, stop_states_selector>;
	using unique_states = meta::unique<meta::concat<start_states, stop_states>>;

	using states_tuple_t = meta::apply<meta::quote<std::tuple>, unique_states>;
	using states_count = std::tuple_size<states_tuple_t>;

	struct state_events_func {
		template<typename T>
		using invoke = meta::list<typename T::start_t, typename T::event_t>;
	};

	// create list of lists. This is StartState, Event extract from transitions
	using state_events = meta::transform<list, state_events_func>;
};

template<typename, typename = detail::null_context> struct state_instances;

template<typename... States>
struct state_instances<std::tuple<States...>, detail::null_context>
{
	std::tuple<States...> states;
};

template<typename... States, typename Ctx>
struct state_instances<std::tuple<States...>, Ctx>
{
	std::tuple<States...> states;

	state_instances(Ctx &ctx) :
		states (std::make_tuple(States(ctx)...))
	{
	}
};

template<typename Trs, typename Context = detail::null_context>
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
	using index_to_state = typename std::tuple_element<I::value, typename Transitions::states_tuple_t>::type;

	template<typename I, typename E>
	using next_state = destination_state<index_to_state<I>, E>;

	template<typename T>
	using state_to_index = meta::find_index<meta::as_list<typename Transitions::states_tuple_t>, T>;

	template<typename I, typename E>
	using next_state_index = state_to_index<next_state<I, E>>;

public:
	fsm() :
		current (0)
	{
		std::get<0>(instances.states).enter();
	}

	fsm(Context &ctx) :
		instances (ctx),
		current (0)
	{
	}

	// handle event E
	template<typename E>
	bool on(const E &event)
	{
		return onForAllImpl(event, current, meta::make_index_sequence<Transitions::states_count::value>{});
	}

private:
	template<typename E, std::size_t... Is>
	bool onForAllImpl(const E &event, std::size_t atIdx, meta::index_sequence<Is...>)
	{
		bool handled = false;

		int dummy[Transitions::states_count::value] = {
			(
				onImpl<E, std::integral_constant<std::size_t, Is>>(event, atIdx, handled),
				0
			)...
		};

		return handled;
	}

	template<typename E, typename I>
	typename std::enable_if<handle_event<index_to_state<I>, E>::value == 1>::type
	onImpl(const E &event, std::size_t atIdx, bool &handled, typename std::tuple_element<I::value, typename Transitions::states_tuple_t>::type* = nullptr)
	{
		if (I::value == atIdx) {
			if (std::get<I::value>(instances.states).event(event)) {
				handled = true;
				std::get<I::value>(instances.states).exit();

				std::get<
					next_state_index<I, E>::value
				>(instances.states).enter();

				current = next_state_index<I, E>::value;
			}
		}
	}

	template<typename E, typename I>
	typename std::enable_if<handle_event<index_to_state<I>, E>::value == 0>::type
	onImpl(const E &event, std::size_t atIdx, bool &handled, typename std::tuple_element<I::value, typename Transitions::states_tuple_t>::type* = nullptr)
	{
	}

private:
	state_instances<typename Trs::states_tuple_t, Context> instances;
	int current;
};

} // namespace fsm
